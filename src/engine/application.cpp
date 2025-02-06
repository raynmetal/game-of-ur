#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>

#include "ecs_world.hpp"
#include "resource_database.hpp"
#include "window_context_manager.hpp"
#include "input_system/input_system.hpp"
#include "scene_system.hpp"
#include "sim_system.hpp"
#include "scene_loading.hpp"
#include "material.hpp"
#include "render_system.hpp"

#include "../app/fly_camera.hpp"

#include "application.hpp"

constexpr uint32_t kMaxSimStep { 5000 };
constexpr uint32_t kMinSimStep { 1000/120 };

std::weak_ptr<Application> Application::s_pInstance {};
bool Application::s_instantiated { false };

Application::Application(const std::string& projectPath) {
    s_instantiated = true;

    std::filesystem::path currentResourcePath { projectPath };
    const std::filesystem::path projectRootDirectory { currentResourcePath.parent_path() };

    std::ifstream jsonFileStream;

    jsonFileStream.open(currentResourcePath.string());
    nlohmann::json projectJSON { nlohmann::json::parse(jsonFileStream) };
    jsonFileStream.close();

    mSimulationStep = projectJSON[0].at("simulation_step").get<uint32_t>();
    {
        char assertionMessage[100];
        sprintf(assertionMessage, "Simulation step must be between %dms and %dms", kMinSimStep, kMaxSimStep);
        assert(mSimulationStep >= kMinSimStep && mSimulationStep <= kMaxSimStep && assertionMessage);
    }

    mSceneSystem = ECSWorld::getSingletonSystem<SceneSystem>();

    initialize(projectJSON[0].at("window_configuration"));

    const std::string& inputFile{ projectJSON[0].at("input_map_path").get<std::string>() };
    currentResourcePath = projectRootDirectory / inputFile;
    jsonFileStream.open(currentResourcePath.string());
    const nlohmann::json inputJSON { nlohmann::json::parse(jsonFileStream) };
    jsonFileStream.close();

    const std::string& rootSceneFile { projectJSON[0].at("root_scene_path").get<std::string>() };
    currentResourcePath = projectRootDirectory / rootSceneFile;

    mInputManager.loadInputConfiguration(inputJSON[0]);
    ResourceDatabase::addResourceDescription(
        nlohmann::json {
            {"name", "root_scene"},
            {"type", "SimObject"},
            {"method", "fromSceneFile"},
            {"parameters", {
                {"path", currentResourcePath.string()},
            }},
        }
    );
}

void Application::execute() {
    mSceneSystem.lock()->addNode(
        ResourceDatabase::getRegisteredResource<SimObject>("root_scene"),
        "/"
    );
    WindowContext& windowContext { WindowContext::getInstance() };

    // Timing related variables
    uint32_t previousTicks { SDL_GetTicks() };
    uint32_t simulationTicks { previousTicks };

    // Application loop begins
    SDL_Event event;
    bool quit {false};
    ECSWorld& rootWorld { mSceneSystem.lock()->getRootWorld() };
    ViewportNode& rootViewport { mSceneSystem.lock()->getRootViewport() };
    ApploopEventDispatcher::applicationStart();
    rootWorld.beginFrame();
    while(true) {
        //Handle events before anything else
        while(SDL_PollEvent(&event)) {
            // TODO: We probably want to locate this elsewhere? I'm not sure. Let's see.
            switch(event.type) {
                case SDL_QUIT:
                    quit=true;
                break;
                case SDL_WINDOWEVENT:
                    windowContext.handleWindowEvent(event.window);
                break;
                default:
                    mInputManager.queueInput(event);
                break;
            }
            if(quit) break;
        }
        if(quit) break;
        // update time related variables
        GLuint currentTicks { SDL_GetTicks() };

        // Apply simulation updates, if possible
        while(currentTicks - simulationTicks >= mSimulationStep) {
            rootWorld.beginFrame();
            simulationTicks += mSimulationStep;

            // Send inputs, if any, to their various listeners
            std::vector<std::pair<ActionDefinition, ActionData>> triggeredActions { mInputManager.getTriggeredActions(simulationTicks) };
            rootViewport.getActionDispatch().dispatchActions(triggeredActions);

            // update objects according to calculated delta
            ApploopEventDispatcher::simulationStep(mSimulationStep);
        }
        // Calculate progress towards the next simulation step
        const float simulationProgress { static_cast<float>(currentTicks - simulationTicks) / mSimulationStep};
        ApploopEventDispatcher::postSimulationStep(simulationProgress);

        // Render a frame
        ApploopEventDispatcher::preRenderStep(simulationProgress);
        rootWorld.getSystem<RenderSystem>()->execute(simulationProgress);
        ApploopEventDispatcher::postRenderStep(simulationProgress);
    }
    ApploopEventDispatcher::applicationEnd();
}

Application::~Application() {
    cleanup();
}

std::shared_ptr<Application> Application::instantiate(const std::string& projectPath) {
    assert(!s_instantiated && s_pInstance.expired() && "Application may only be initialized once");
    std::shared_ptr<Application> instance { new Application{projectPath} };
    s_pInstance = instance;
    s_instantiated = true;
    return instance;
}

Application& Application::getInstance() {
    assert(!s_pInstance.expired() && "Application has not been instantiated");
    return *s_pInstance.lock();
}

void Application::initialize(const nlohmann::json& windowProperties) {
    // IMPORTANT: call get instance, just to initialize the window
    // TODO: stupid name bound to trip me up sooner or later. Replace it.
    WindowContext::initialize(windowProperties);

    // The framebuffer resource manager depends on the texture resource manager existing (especially 
    // during destruction at end of program.) Instantiate these managers in reverse order of their
    // dependence
    ResourceDatabase::getInstance();
    Material::Init();
    ApploopEventDispatcher::applicationInitialize();
}

void Application::cleanup() {
    mSceneSystem.lock()->getRootWorld().cleanup();
    Material::Clear();
}
