#include <filesystem>
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
#include "engine/input_system/input_system.hpp"

#include "app/fly_camera.hpp"
#include "app/back_and_forth.hpp"
#include "app/revolve.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

const GLuint kSimulationStep{ 1000/30 };

void init();
void cleanup();

int main(int argc, char* argv[]) {
    init();

    InputManager inputManager {};

    InputSourceDescription mouseMotionControl {
        .mAttributes {
            (2&InputAttributes::N_AXES)
            | InputAttributes::HAS_STATE_VALUE
            | InputAttributes::HAS_CHANGE_VALUE
            | InputAttributes::STATE_IS_LOCATION
        },
        .mDeviceType { DeviceType::MOUSE },
        .mControlType { ControlType::POINT }
    };

    std::shared_ptr<SimObject> camera { 
        SimpleECS::getSystem<SimSystem>()->createSimObject<Placement, Transform, CameraProperties, SceneNode>({}, {}, {}, {}) 
    };
    camera->addComponent<FlyCamera>();

    // TODO: Move all input registration and scene setup code elsewhere and have them read, hopefully, from
    // some kind of plaintext config file

    // Create the camera action context, and define what actions are available to it
    inputManager.registerActionContext("Camera");
    inputManager["Camera"].registerAction(
        nlohmann::json {
            {"name", "Rotate"},
            {"nAxes", 2},
            {"hasChangeValue", true},
            {"hasStateValue", false},
            {"hasButtonValue", false},
            {"hasNegative", false},
            {"stateIsLocation", false},
        }
    );
    inputManager["Camera"].registerAction(
        nlohmann::json {
            {"name", "Move"},
            {"nAxes", 2},
            {"hasChangeValue", false},
            {"hasStateValue", true},
            {"hasButtonValue", false},
            {"hasNegative", true},
            {"stateIsLocation", false},
        }
    );
    inputManager["Camera"].registerAction(
        nlohmann::json {
            {"name", "ToggleControl"},
            {"nAxes", 0},
            {"hasChangeValue", false},
            {"hasStateValue", false},
            {"hasButtonValue", true},
            {"hasNegative", false},
            {"stateIsLocation", false},
        }
    );
    inputManager["Camera"].registerAction(
        nlohmann::json {
            {"name", "UpdateFOV"},
            {"nAxes", 1},
            {"hasChangeValue", true},
            {"hasStateValue", false},
            {"hasButtonValue", false},
            {"hasNegative", false},
            {"stateIsLocation", false},
        }
    );

    // Create an action context for render controls
    inputManager.registerActionContext("Graphics");
    inputManager["Graphics"].registerAction(
        nlohmann::json {
            {"name", "UpdateGamma"},
            {"nAxes", 1},
            {"hasChangeValue", true},
            {"hasStateValue", false},
            {"hasButtonValue", false},
            {"hasNegative", false},
            {"stateIsLocation", false},
        }
    );
    inputManager["Graphics"].registerAction(
        nlohmann::json{
            {"name", "UpdateExposure"},
            {"nAxes", 1},
            {"hasChangeValue", true},
            {"hasStateValue", false},
            {"hasButtonValue", false},
            {"hasNegative", false},
            {"stateIsLocation", false},
        }
    );
    inputManager["Graphics"].registerAction(
        nlohmann::json{
            {"name", "RenderNextTexture"},
            {"nAxes", 0},
            {"hasChangeValue", false},
            {"hasStateValue", false},
            {"hasButtonValue", true},
            {"hasNegative", false},
            {"stateIsLocation", false},
        }
    );


    // Map input events to the camera actions just defined
    inputManager["Camera"].registerInputBind(
        "Rotate", AxisFilter::X_POS, // Action and target axis
        InputCombo{ // Input combination and filter
            .mMainControl {
                .mControl { mouseMotionControl },
                .mAxisFilter{ AxisFilter::X_CHANGE_POS },
            }, 
            .mTrigger { InputCombo::Trigger::ON_CHANGE }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Rotate", AxisFilter::X_NEG, // Action and target axis
        InputCombo{ // Input combination and filter
            .mMainControl {
                .mControl { mouseMotionControl },
                .mAxisFilter{ AxisFilter::X_CHANGE_NEG },
            }, 
            .mTrigger { InputCombo::Trigger::ON_CHANGE }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Rotate", AxisFilter::Y_POS,
        InputCombo{ // Input combination and filter
            .mMainControl {
                .mControl { mouseMotionControl },
                .mAxisFilter{ AxisFilter::Y_CHANGE_POS },
            },
            .mTrigger { InputCombo::Trigger::ON_CHANGE }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Rotate", AxisFilter::Y_NEG,
        InputCombo{ // Input combination and filter
            .mMainControl {
                .mControl { mouseMotionControl },
                .mAxisFilter{ AxisFilter::Y_CHANGE_NEG },
            },
            .mTrigger { InputCombo::Trigger::ON_CHANGE }
        }
    );
    inputManager["Camera"].registerInputBind(
        "ToggleControl", AxisFilter::SIMPLE,
        InputCombo{
            .mMainControl{
                .mControl {
                    .mAttributes { HAS_BUTTON_VALUE },
                    .mControl { SDLK_1 },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{AxisFilter::SIMPLE},
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::X_POS,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_d },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::X_POS,
        InputCombo {
            .mMainControl {
                .mControl{
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_d },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_RELEASE }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::X_NEG,
        InputCombo {
            .mMainControl {
                .mControl{
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_a },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::X_NEG,
        InputCombo {
            .mMainControl {
                .mControl{
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_a },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_RELEASE }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::Y_POS,
        InputCombo {
            .mMainControl {
                .mControl{
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_w },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::Y_POS,
        InputCombo {
            .mMainControl {
                .mControl{
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_w },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_RELEASE}
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::Y_NEG,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_s },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );
    inputManager["Camera"].registerInputBind(
        "Move", AxisFilter::Y_NEG,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_s },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter{ AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_RELEASE }
        }
    );
    inputManager["Camera"].registerInputBind(
        "UpdateFOV", AxisFilter::X_POS,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_COMMA },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter { AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );
    inputManager["Camera"].registerInputBind(
        "UpdateFOV", AxisFilter::X_NEG,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_PERIOD },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter { AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );

    // Map input events to render system controls
    inputManager["Graphics"].registerInputBind(
        "UpdateGamma", AxisFilter::X_NEG,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_j },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter { AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS}
        }
    );
    inputManager["Graphics"].registerInputBind(
        "UpdateGamma", AxisFilter::X_POS,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_k },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter { AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS}
        }
    );
    inputManager["Graphics"].registerInputBind(
        "UpdateExposure", AxisFilter::X_NEG,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_u },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter { AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS}
        }
    );
    inputManager["Graphics"].registerInputBind(
        "UpdateExposure", AxisFilter::X_POS,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_i },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter { AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS}
        }
    );
    inputManager["Graphics"].registerInputBind(
        "RenderNextTexture", AxisFilter::SIMPLE,
        InputCombo {
            .mMainControl {
                .mControl {
                    .mAttributes { InputAttributes::HAS_BUTTON_VALUE },
                    .mControl { SDLK_TAB },
                    .mDeviceType { DeviceType::KEYBOARD },
                    .mControlType { ControlType::BUTTON },
                },
                .mAxisFilter { AxisFilter::SIMPLE },
            },
            .mTrigger { InputCombo::Trigger::ON_PRESS }
        }
    );

    const float sqrt2 { sqrt(2.f) };
    std::vector<std::shared_ptr<SimObject>> lightEntities {
        { // Flashlight
            SimpleECS::getSystem<SimSystem>()->createSimObject<
                LightEmissionData, Placement, Transform, SceneNode
            >(
                LightEmissionData::MakeSpotLight(
                    4.f,
                    13.f,
                    glm::vec3(2.f),
                    glm::vec3(4.f),
                    glm::vec3(.3f),
                    .07f,
                    .03f
                ),
                {
                    glm::vec4(glm::vec3(0.f), 1.f),
                    glm::quat(),
                    glm::vec3{1.f}
                },
                {},
                // Make the flashlight a child of the camera's scene node so that 
                // it moves along with the camera
                { .mParent{ camera->getEntityID() } } 
            )
        },
        { // Weak point light
            SimpleECS::getSystem<SimSystem>()->createSimObject<LightEmissionData, Placement, Transform, SceneNode>(
                {
                    LightEmissionData::MakePointLight(
                        glm::vec3(2.f, 0.6f, 1.2f),
                        glm::vec3(3.1f, 1.04f, .32f),
                        glm::vec3(0.1f, 0.01f, 0.03f),
                        .07f,
                        .02f
                    )
                },
                {
                    glm::vec4{0.f, 1.5f, -1.f, 1.f},
                    glm::quat{},
                    glm::vec3{1.f}
                },
                {},
                {}
            )
        },
        { // Sunlight
            SimpleECS::getSystem<SimSystem>()->createSimObject<LightEmissionData, Placement, Transform, SceneNode> (
                {
                    LightEmissionData::MakeDirectionalLight(
                        glm::vec3{20.f},
                        glm::vec3{20.f},
                        glm::vec3{.4f}
                    )
                },
                {
                    glm::vec4{glm::vec3(0.f), 1.f},
                    glm::quat { glm::vec3 {
                        glm::radians(-20.f), // pitch
                        glm::radians(180.f), // yaw
                        glm::radians(0.f) // roll
                    }},
                    glm::vec3{sqrt2, sqrt2, 1.f}
                },
                {},
                {}
            )
        }
    };
    for(int i{0}; i < 2; ++i) {
        Placement placement { lightEntities[i]->getCoreComponent<Placement>() };
        placement.mScale = glm::vec3 { lightEntities[i]->getCoreComponent<LightEmissionData>().mRadius };
        lightEntities[i]->updateCoreComponent<Placement>(placement);
    }
    lightEntities[2]->addComponent<Revolve>(); // make sunlight revolve
    ResourceDatabase::addResourceDescription({
        {"name", "boardPieceModel"},
        {"type", StaticModel::getName()},
        {"method", StaticModelFromFile::getName()},
        {"parameters", {
            {"path", "data/models/Generic Board Piece.obj"},
        }}
    });

    std::shared_ptr<SimObject> boardPiece  {
        SimpleECS::getSystem<SimSystem>()->createSimObject<std::shared_ptr<StaticModel>, Placement, Transform, SceneNode> (
            ResourceDatabase::getResource<StaticModel>("boardPieceModel"),
            {{0.f, -1.f, -1.f, 1.f}},
            {},
            {}
        )
    };
    boardPiece->addComponent<BackAndForth>();

    std::vector<std::shared_ptr<SimObject>> boardPieces(20, nullptr);
    EntityID parentEntity { boardPiece->getEntityID() };

    for(std::size_t i{0}; i < 20; ++i) {
        boardPieces[i] = SimpleECS::getSystem<SimSystem>()->createSimObject<std::shared_ptr<StaticModel>, Placement, Transform, SceneNode>(
            boardPiece->getCoreComponent<std::shared_ptr<StaticModel>>(),
            boardPiece->getCoreComponent<Placement>(),
            boardPiece->getCoreComponent<Transform>(),
            SceneNode { .mParent { parentEntity } }
        );
        boardPieces[i]->addComponent<BackAndForth>();

        Placement boardPiecePlacement { boardPieces[i]->getCoreComponent<Placement>() };

        boardPiecePlacement.mPosition.x = 0.f;
        boardPiecePlacement.mPosition.y = 2.f;
        boardPiecePlacement.mPosition.z = 0.f;
        boardPiecePlacement.mOrientation = glm::rotate(boardPiecePlacement.mOrientation, glm::radians(360.f/(1+20)), {0.f, 0.f, -1.f});

        boardPieces[i]->updateCoreComponent<Placement>(boardPiecePlacement);

        // each board piece is the parent of the board piece made in
        // the next iteration
        parentEntity = boardPieces[i]->getEntityID();
    }

    inputManager["Camera"].registerActionHandler("Rotate", std::shared_ptr<FlyCamera>(camera, &(camera->getComponent<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("ToggleControl", std::shared_ptr<FlyCamera>(camera, &(camera->getComponent<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("Move", std::shared_ptr<FlyCamera>(camera, &(camera->getComponent<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("UpdateFOV", std::shared_ptr<FlyCamera>(camera, &(camera->getComponent<FlyCamera>())));
    inputManager["Graphics"].registerActionHandler("UpdateGamma", SimpleECS::getSystem<RenderSystem>());
    inputManager["Graphics"].registerActionHandler("UpdateExposure", SimpleECS::getSystem<RenderSystem>());
    inputManager["Graphics"].registerActionHandler("RenderNextTexture", SimpleECS::getSystem<RenderSystem>());

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

void init() {
    std::cout << "This program is running" << std::endl;

    // IMPORTANT: call get instance, just to initialize the window
    // TODO: stupid name bound to trip me up sooner or later. Replace it.
    WindowContextManager::getInstance(gWindowWidth, gWindowHeight);

    // The framebuffer resource manager depends on the texture resource manager existing (especially 
    // during destruction at end of program.) Instantiate these managers in reverse order of their
    // dependence
    ResourceDatabase::getInstance();
    Material::Init();

    SimpleECS::registerComponentTypes<Placement, LightEmissionData, std::shared_ptr<StaticModel>, Transform, SceneNode, SimSystem::SimCore, CameraProperties>();
    SimpleECS::registerSystem<SceneSystem, Transform, SceneNode, Placement>();
    SimpleECS::registerSystem<SimSystem, SimSystem::SimCore>();
    SimpleECS::registerSystem<CameraSystem, Transform, CameraProperties>();
    SimpleECS::registerSystem<RenderSystem, CameraProperties>();
}

void cleanup() {
    SimpleECS::cleanup();
    Material::Clear();
}
