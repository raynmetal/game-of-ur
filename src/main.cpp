#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/sim_system.hpp"
#include "engine/simple_ecs.hpp"
#include "engine/window_context_manager.hpp"
#include "engine/resource_database.hpp"
#include "engine/light.hpp"
#include "engine/model.hpp"
#include "engine/render_system.hpp"
#include "engine/scene_system.hpp"
#include "engine/scene_loading.hpp"
#include "engine/input_system/input_system.hpp"

#include "app/fly_camera.hpp"
#include "app/back_and_forth.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

const GLuint kSimulationStep{ 1000/30 };

void initialize();
void cleanup();
void observeDirectionChange(float direction) {
    std::cout << "direction changed to: " << direction << "\n";
}

int main(int argc, char* argv[]) {
    initialize();

    InputManager inputManager {};
    std::ifstream jsonFileStream;
    std::string inputPath { "data/input_bindings.json" };
    jsonFileStream.open(inputPath);
    nlohmann::json inputBindingJSON { nlohmann::json::parse(jsonFileStream) };
    jsonFileStream.close();
    inputManager.loadInputConfiguration(inputBindingJSON.at(0));

    nlohmann::json sceneDescription {
        {"name", "test_scene_1"},
        {"type", "SimObject"},
        {"method", "fromSceneFile"},
        {"parameters", {
            {"path", "data/test_scene_1.json"},
        }},
    };

    ResourceDatabase::addResourceDescription(sceneDescription);
    std::shared_ptr<SimObject> partialScene { ResourceDatabase::getRegisteredResource<SimObject>(sceneDescription.at("name").get<std::string>()) };
    {
        std::shared_ptr<SceneNode> flashlight { partialScene->getByPath("/camera/flashlight/") };
        // TODO: set scale automatically according to  the flashlight's light
        // emission radius. How?
        Placement placement { flashlight->getComponent<Placement>() };
        placement.mScale = { glm::vec3(flashlight->getComponent<LightEmissionData>().mRadius)};
        flashlight->updateComponent<Placement>(placement);
    }

    SimpleECS::getSystem<SceneSystem>()->addNode(partialScene, "/");

    std::shared_ptr<SimObject> camera { partialScene->getByPath<std::shared_ptr<SimObject>>("/camera/") };
    inputManager["Camera"].registerActionHandler("Rotate", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("ToggleControl", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("Move", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("UpdateFOV", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
    inputManager["Graphics"].registerActionHandler("UpdateGamma", SimpleECS::getSystem<RenderSystem>());
    inputManager["Graphics"].registerActionHandler("UpdateExposure", SimpleECS::getSystem<RenderSystem>());
    inputManager["Graphics"].registerActionHandler("RenderNextTexture", SimpleECS::getSystem<RenderSystem>());

    // Scene should be remembered by the scene system 
    // now; no need to store a reference to it ourselves
    partialScene = nullptr;

    // testing the signals system with a signal emitted whenever the
    // first board piece changes directions
    SignalTracker mainsTracker {};
    SignalObserver<float> observerDirectionChanged1 { mainsTracker, "directionChangeObserved1", observeDirectionChange };
    SignalObserver<float> observerDirectionChanged2 { mainsTracker, "directionChangeObserved2", observeDirectionChange };

    BackAndForth& firstBackAndForth { SimpleECS::getSystem<SceneSystem>()->getByPath<BackAndForth&>("/partial_scene_root/board_piece/@BackAndForth") };
    BackAndForth& thirdBackAndForth { SimpleECS::getSystem<SceneSystem>()->getByPath<BackAndForth&>("/partial_scene_root/board_piece/board_piece/board_piece/@BackAndForth") };

    // connection using observer and signal directly
    observerDirectionChanged1.connect(firstBackAndForth.mSigDirectionChanged);
    // connection using observer's tracker, by observer's and signal's names
    mainsTracker.connect("directionChanged", "directionChangeObserved2", thirdBackAndForth);

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

    //Main event loop
    glClearColor(0.f, 0.f, 0.f, 1.f);
    SDL_Event event;
    bool quit {false};
    glEnable(GL_FRAMEBUFFER_SRGB);

    ApploopEventDispatcher::applicationStart();
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
                inputManager.queueInput(event);
            }
        }
        if(quit) break;
        ++renderFrame;

        // update time related variables
        GLuint currentTicks { SDL_GetTicks() };

        // Apply simulation updates, if possible
        while(currentTicks - simulationTicks >= kSimulationStep) {
            ++simFrame;
            simulationTicks += kSimulationStep;

            // Send inputs, if any, to their various listeners
            inputManager.dispatch(simulationTicks);

            // update objects according to calculated delta
            ApploopEventDispatcher::simulationStep(kSimulationStep);
        }
        // Calculate progress towards the next simulation step
        const float simulationProgress { static_cast<float>(currentTicks - simulationTicks) / kSimulationStep };
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

        SimpleECS::endFrame();
    }
    ApploopEventDispatcher::applicationEnd();

    // ... and then die
    cleanup();
    return 0;
}

void initialize() {
    std::cout << "This program is running" << std::endl;

    // IMPORTANT: call get instance, just to initialize the window
    // TODO: stupid name bound to trip me up sooner or later. Replace it.
    WindowContextManager::getInstance(gWindowWidth, gWindowHeight);

    // The framebuffer resource manager depends on the texture resource manager existing (especially 
    // during destruction at end of program.) Instantiate these managers in reverse order of their
    // dependence
    ResourceDatabase::getInstance();
    Material::Init();

    SimpleECS::initialize();
}

void cleanup() {
    SimpleECS::cleanup();
    Material::Clear();
}
