#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>

#include "simple_ecs.hpp"
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

constexpr uint16_t kMaxWidth { 4096 };
constexpr uint16_t kMinWidth { 800 };
constexpr uint16_t kMaxHeight { 2304 };
constexpr uint16_t kMinHeight { 600 };
constexpr uint32_t kMaxSimStep { 2000 };
constexpr uint32_t kMinSimStep { 1000/90 };

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

    mTitle = projectJSON[0].at("title").get<std::string>();
    mWindowWidth = projectJSON[0].at("window_width").get<uint16_t>();
    mWindowHeight = projectJSON[0].at("window_height").get<uint16_t>();
    mSimulationStep = projectJSON[0].at("simulation_step").get<uint32_t>();
    {
        char assertionMessage[100];
        sprintf(assertionMessage, "Window width must be between %dpx and %dpx", kMinWidth, kMaxWidth);
        assert(mWindowWidth >= kMinWidth && mWindowWidth <= kMaxWidth && assertionMessage);
        sprintf(assertionMessage, "Window height must be between %dpx and %dpx", kMinHeight, kMaxHeight);
        assert(mWindowHeight >= kMinHeight && mWindowHeight <= kMaxHeight && assertionMessage);
        sprintf(assertionMessage, "Simulation step must be between %dms and %dms", kMinSimStep, kMaxSimStep);
        assert(mSimulationStep >= kMinSimStep && mSimulationStep <= kMaxSimStep && assertionMessage);
    }

    initialize();

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
    std::cout << "Application executing" << std::endl;
    SimpleECS::getSystem<SceneSystem>()->addNode(
        ResourceDatabase::getRegisteredResource<SimObject>("root_scene"),
        "/"
    );

    // Timing related variables
    uint32_t previousTicks { SDL_GetTicks() };
    uint32_t simulationTicks { previousTicks };
    // Current frame
    GLuint simFrame { 0 };
    GLuint renderFrame { 0 };
    // Framerate measuring variables
    float framerate {0.f};
    const float frameratePoll {1.f};
    float framerateCounter {0.f};

    // Application loop begins
    SDL_Event event;
    bool quit {false};
    ApploopEventDispatcher::applicationStart();
    SimpleECS::beginFrame();
    while(true) {
        //Handle events before anything else
        while(SDL_PollEvent(&event)) {
            // TODO: We probably want to locate this elsewhere? I'm not sure. Let's see.
            if(
                event.type == SDL_QUIT
            ) {
                quit = true;
                break;
            }
            else {
                mInputManager.queueInput(event);
            }
        }
        if(quit) break;
        ++renderFrame;

        // update time related variables
        GLuint currentTicks { SDL_GetTicks() };

        // Apply simulation updates, if possible
        while(currentTicks - simulationTicks >= mSimulationStep) {
            SimpleECS::beginFrame();
            ++simFrame;
            simulationTicks += mSimulationStep;

            // Send inputs, if any, to their various listeners
            mInputManager.dispatch(simulationTicks);

            // update objects according to calculated delta
            ApploopEventDispatcher::simulationStep(mSimulationStep);
        }
        // Calculate progress towards the next simulation step
        const float simulationProgress { static_cast<float>(currentTicks - simulationTicks) / mSimulationStep};
        ApploopEventDispatcher::postSimulationStep(simulationProgress);

        // Measure average framerate, biased towards previously measured framerate
        float deltaTime {
            (currentTicks - previousTicks)/1000.f
        };
        previousTicks = currentTicks;
        const float framerateBias { .8f };
        framerate = framerate * (framerateBias) + (1.f - framerateBias)/(deltaTime > .0001f? deltaTime: .0001f);
        framerateCounter += deltaTime;
        while(framerateCounter > frameratePoll) {
            std::cout << "Frame " << renderFrame << ", Simulation Frame " << simFrame << " -- Progress to next sim frame: " << simulationProgress*100.f << "%\n";
            std::cout << "Framerate: " << framerate << " fps\n";

            framerateCounter -= frameratePoll;
        }

        // Render a frame
        ApploopEventDispatcher::preRenderStep(simulationProgress);
        SimpleECS::getSystem<RenderSystem>()->execute(simulationProgress);
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

void Application::initialize() {
    std::cout << "Application initialized" << std::endl;

    // IMPORTANT: call get instance, just to initialize the window
    // TODO: stupid name bound to trip me up sooner or later. Replace it.
    WindowContextManager::getInstance(mTitle, mWindowWidth, mWindowHeight);

    // The framebuffer resource manager depends on the texture resource manager existing (especially 
    // during destruction at end of program.) Instantiate these managers in reverse order of their
    // dependence
    ResourceDatabase::getInstance();
    Material::Init();

    SimpleECS::initialize();
}

void Application::cleanup() {
    SimpleECS::cleanup();
    Material::Clear();
}
