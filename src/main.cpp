#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/sim_system.hpp"
#include "engine/simple_ecs.hpp"
#include "app/fly_camera.hpp"
#include "engine/window_context_manager.hpp"
#include "engine/texture_manager.hpp"
#include "engine/shader_program_manager.hpp"
#include "engine/framebuffer_manager.hpp"
#include "engine/mesh_manager.hpp"
#include "engine/material_manager.hpp"
#include "engine/model_manager.hpp"
#include "engine/light.hpp"
#include "engine/render_stage.hpp"
#include "engine/shapegen.hpp"
#include "engine/render_system.hpp"
#include "engine/scene_system.hpp"
#include "engine/input_system/input_system.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

const GLuint kSimulationStep{ 1000/30 };

void init();
void cleanup();

int main(int argc, char* argv[]) {
    init();

    InputManager inputManager {};

    InputIdentity mouseMotionControl {
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
        "Rotate",
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_CHANGE_VALUE
    );
    inputManager["Camera"].registerAction(
        "Move",
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_STATE_VALUE
        | InputAttributes::HAS_NEGATIVE
    );
    inputManager["Camera"].registerAction(
        "ToggleControl",
        InputAttributes::HAS_BUTTON_VALUE
    );
    inputManager["Camera"].registerAction(
        "UpdateFOV",
        (1&InputAttributes::N_AXES)
        | InputAttributes::HAS_CHANGE_VALUE
    );

    // Create an action context for render controls
    inputManager.registerActionContext("Graphics");
    inputManager["Graphics"].registerAction(
        "UpdateGamma",
        (1&InputAttributes::N_AXES)
        | InputAttributes::HAS_CHANGE_VALUE
    );
    inputManager["Graphics"].registerAction(
        "UpdateExposure",
        (1&InputAttributes::N_AXES)
        | InputAttributes::HAS_CHANGE_VALUE
    );
    inputManager["Graphics"].registerAction(
        "RenderNextTexture",
        InputAttributes::HAS_BUTTON_VALUE
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
    std::vector<Entity> lightEntities {
        { // Flashlight
            SimpleECS::createEntity<LightEmissionData, Placement, Transform, SceneNode>(
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
                { .mParent{ camera->getEntityID() } }
            )
        },
        { // Weak point light
            SimpleECS::createEntity<LightEmissionData, Placement, Transform, SceneNode>(
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
            SimpleECS::createEntity<LightEmissionData, Placement, Transform, SceneNode> (
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
        Placement placement { lightEntities[i].getComponent<Placement>() };
        placement.mScale = glm::vec3 { lightEntities[i].getComponent<LightEmissionData>().mRadius };
        lightEntities[i].updateComponent<Placement>(placement);
    }

    Entity boardPiece { 
        SimpleECS::createEntity<ModelHandle, Placement, Transform, SceneNode>(
            ModelManager::getInstance().registerResource(
                "data/models/Generic Board Piece.obj", 
                {"data/models/Generic Board Piece.obj"}
            ),
            {{0.f, -1.f, -1.f, 1.f}},
            {},
            {}
        )
    };

    std::vector<Entity> boardPieces{};
    EntityID parentEntity { boardPiece.getID() };
    for(std::size_t i{0}; i < 20; ++i) {
        boardPieces.push_back(boardPiece);

        Placement boardPiecePlacement { boardPieces[i].getComponent<Placement>() };
        SceneNode boardPieceSceneNode { boardPieces[i].getComponent<SceneNode>() };

        boardPiecePlacement.mPosition.x = 0.f;
        boardPiecePlacement.mPosition.y = 2.f;
        boardPiecePlacement.mPosition.z = 0.f;
        boardPiecePlacement.mOrientation = glm::rotate(boardPiecePlacement.mOrientation, glm::radians(360.f/(1+20)), {0.f, 0.f, -1.f});
        boardPieceSceneNode.mParent = parentEntity;

        boardPieces[i].updateComponent<Placement>(boardPiecePlacement);
        boardPieces[i].updateComponent<SceneNode>(boardPieceSceneNode);

        // each board piece is the parent of the board piece made in
        // the next iteration
        parentEntity = boardPieces[i].getID();
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

        // Get references to the entities I'm going to frequently update
        Entity& sunlight { lightEntities[2] };
        Entity& flashlight { lightEntities[0] };

        // update time related variables
        GLuint currentTicks { SDL_GetTicks() };

        // Apply simulation updates, if possible
        while(currentTicks - simulationTicks >= kSimulationStep) {
            ++simFrame;
            simulationTicks += kSimulationStep;

            // Send inputs, if any, to their various listeners
            inputManager.dispatch(simulationTicks);

            // update objects according to calculated delta
            Placement sunlightPlacement { sunlight.getComponent<Placement>() };
            sunlightPlacement.mOrientation = glm::rotate(sunlight.getComponent<Placement>().mOrientation, kSimulationStep/(1000.f*10.f), {0.f, 1.f, 0.f});
            sunlight.updateComponent<Placement>(sunlightPlacement);
            for(Entity& piece: boardPieces) {
                Placement piecePlacement { piece.getComponent<Placement>() };
                piecePlacement.mPosition.z = glm::sin(glm::radians(simulationTicks/10.f + piece.getID()*45.f));
                piece.updateComponent<Placement>(piecePlacement);
            }
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

            std::cout << "flashlight parent entity: " << flashlight.getComponent<SceneNode>().mParent << "\n";
            std::cout << "flashlight model matrix: \n" << glm::to_string(flashlight.getComponent<Transform>().mModelMatrix) << "\n";

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
    TextureManager::getInstance();
    ShaderProgramManager::getInstance();
    MaterialManager::getInstance();
    Material::Init();
    MeshManager::getInstance();
    FramebufferManager::getInstance();
    ModelManager::getInstance();

    SimpleECS::registerComponentTypes<Placement, LightEmissionData, ModelHandle, Transform, SceneNode, SimSystem::SimCore, CameraProperties>();
    SimpleECS::registerSystem<SceneSystem, Transform, SceneNode, Placement>();
    SimpleECS::registerSystem<SimSystem, SimSystem::SimCore>();
    SimpleECS::registerSystem<CameraSystem, Transform, CameraProperties>();
    SimpleECS::registerSystem<RenderSystem, CameraProperties>();
}

void cleanup() {
    SimpleECS::cleanup();
    Material::Clear();
}
