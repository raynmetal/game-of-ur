#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/simple_ecs.hpp"
#include "engine/fly_camera.hpp"
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

    gComponentManager.registerComponentArray<Placement>();
    gComponentManager.registerComponentArray<LightEmissionData>();
    gComponentManager.registerComponentArray<ModelHandle>();
    gComponentManager.registerComponentArray<Transform>();
    gComponentManager.registerComponentArray<SceneNode>();

    Signature sceneSystemSignature {};

    sceneSystemSignature.set(gComponentManager.getComponentType<Transform>(), true);
    sceneSystemSignature.set(gComponentManager.getComponentType<SceneNode>(), true);
    sceneSystemSignature.set(gComponentManager.getComponentType<Placement>(), true);

    gSystemManager.registerSystem<RenderSystem>(Signature{});
    gSystemManager.registerSystem<SceneSystem>(sceneSystemSignature);

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
                .mControl{
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

    const float sqrt2 { sqrt(2.f) };
    std::vector<Entity> lightEntities(3);

    //build spotlight
    lightEntities[0].addComponent<LightEmissionData>(
        LightEmissionData::MakeSpotLight(
            4.f,
            13.f,
            glm::vec3(2.f),
            glm::vec3(4.f),
            glm::vec3(.3f),
            .07f,
            .03f           
        )
    );
    lightEntities[0].addComponent<Placement>({
        glm::vec4(glm::vec3(0.f), 1.f),
        glm::quat(),
        glm::vec3(lightEntities[0].getComponent<LightEmissionData>().mRadius)
    });
    lightEntities[0].addComponent<Transform>({});
    lightEntities[0].addComponent<SceneNode>({});

    // build point light
    lightEntities[1].addComponent<LightEmissionData>(
        LightEmissionData::MakePointLight(
            glm::vec3(2.f, 0.6f, 1.2f),
            glm::vec3(3.1f, 1.04f, .32f),
            glm::vec3(0.1f, 0.01f, 0.03f),
            .07f,
            .02f
        )
    );
    lightEntities[1].addComponent<Placement>({
        glm::vec4{0.f, 1.5f, -1.f, 1.f},
        glm::quat{},
        glm::vec3{lightEntities[1].getComponent<LightEmissionData>().mRadius}
    });
    lightEntities[1].addComponent<Transform>({});
    lightEntities[1].addComponent<SceneNode>({});

    // build directional (sun) light
    lightEntities[2].addComponent<LightEmissionData>(
        LightEmissionData::MakeDirectionalLight(
            glm::vec3{20.f},
            glm::vec3{20.f},
            glm::vec3{.4f}
        )
    );
    lightEntities[2].addComponent<Placement>({
        glm::vec4{glm::vec3(0.f), 1.f},
        glm::quat { glm::vec3 {
            glm::radians(-20.f), // pitch
            glm::radians(180.f), // yaw
            glm::radians(0.f) // roll
        }},
        glm::vec3{sqrt2, sqrt2, 1.f}
    });
    lightEntities[2].addComponent<Transform>({});
    lightEntities[2].addComponent<SceneNode>({});

    Entity boardPiece {};
    boardPiece.addComponent<ModelHandle>(
        ModelManager::getInstance().registerResource("data/models/Generic Board Piece.obj", {"data/models/Generic Board Piece.obj"})
    );
    boardPiece.addComponent<Placement>({
        {0.f, -1.f, -1.f, 1.f}
    });
    boardPiece.addComponent<Transform>({});
    boardPiece.addComponent<SceneNode>({});
    std::vector<Entity> boardPieces(20);
    EntityID parentEntity { boardPiece.getID() };
    for(std::size_t i{0}; i < boardPieces.size(); ++i) {
        boardPieces[i].copy(boardPiece);

        Placement& boardPiecePlacement { boardPieces[i].getComponent<Placement>() };
        SceneNode& boardPieceSceneNode { boardPieces[i].getComponent<SceneNode>() };

        boardPiecePlacement.mPosition.x = 0.f;
        boardPiecePlacement.mPosition.y = 2.f;
        boardPiecePlacement.mPosition.z = 0.f;
        boardPiecePlacement.mOrientation = glm::rotate(boardPiecePlacement.mOrientation, glm::radians(360.f/(1+boardPieces.size())), {0.f, 0.f, -1.f});
        boardPieceSceneNode.mParent = parentEntity;

        // each board piece is the parent of the board piece made in
        // the next iteration
        parentEntity = boardPieces[i].getID();
    }

    std::shared_ptr<FlyCamera> camera { std::make_shared<FlyCamera>(FlyCamera {glm::vec3(0.f), 0.f, 0.f, 0.f}) };
    camera->setLookSensitivity(20.f);
    inputManager["Camera"].registerActionHandler("Rotate", camera);
    inputManager["Camera"].registerActionHandler("ToggleControl", camera);
    inputManager["Camera"].registerActionHandler("Move", camera);
    inputManager["Camera"].registerActionHandler("UpdateFOV", camera);

    float exposure { 1.f };
    float gamma { 2.2f };

    gSystemManager.getSystem<SceneSystem>()->rebuildGraph();
    gSystemManager.getSystem<SceneSystem>()->updateTransforms();

    //Timing related variables
    GLuint previousTicks { SDL_GetTicks() };
    float framerate {0.f};
    const float frameratePoll {1.f};
    float framerateCounter {0.f};

    //Main event loop
    glClearColor(0.f, 0.f, 0.f, 1.f);
    SDL_Event event;
    bool quit {false};
    glEnable(GL_FRAMEBUFFER_SRGB);

    while(true) {
        //Handle events before anything else
        while(SDL_PollEvent(&event)) {
            if(
                event.type == SDL_QUIT || (
                    event.type == SDL_KEYUP
                    && event.key.keysym.sym == SDLK_ESCAPE
                )
            ) {
                quit = true;
                break;
            }
            if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_TAB) {
                const std::size_t currScreenTexture { gSystemManager.getSystem<RenderSystem>()->getCurrentScreenTexture() };
                gSystemManager.getSystem<RenderSystem>()->setScreenTexture(1 + currScreenTexture);
            }
            inputManager.queueInput(event);
            // if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_PERIOD) {
            //     exposure += .1f;
            //     tonemappingRenderStage.getMaterial("screenMaterial").getResource().updateFloatProperty(
            //         "exposure", exposure
            //     );
            // }
            // if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_COMMA) {
            //     if (exposure < .1f) exposure = 0.f;
            //     else exposure -= .1f; 
            //     tonemappingRenderStage.getMaterial("screenMaterial").getResource().updateFloatProperty(
            //         "exposure", exposure
            //     );
            // }
            // if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_j) {
            //     gamma -= .1f;
            //     if(gamma < 1.6f) gamma = 1.6f;
            //     tonemappingRenderStage.getMaterial("screenMaterial").getResource().updateFloatProperty(
            //         "gamma", gamma 
            //     );
            // }
            // else if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_k) {
            //     gamma += .1f;
            //     if(gamma > 3.f) gamma = 3.f;
            //     tonemappingRenderStage.getMaterial("screenMaterial").getResource().updateFloatProperty(
            //         "gamma", gamma 
            //     );
            // }
            // camera.processInput(event);
        }
        if(quit) break;

        Entity& flashlight { lightEntities[0] };
        Entity& sunlight {lightEntities[2]};

        // update time related variables
        GLuint currentTicks { SDL_GetTicks() };
        float deltaTime {
            (currentTicks - previousTicks)/1000.f
        };
        previousTicks = currentTicks;
        framerate = framerate * .8f + .2f/(deltaTime > .0001f? deltaTime: .0001f);
        framerateCounter += deltaTime;
        if(framerateCounter > frameratePoll) {
            std::cout << "Framerate: " << framerate << " fps\n";
            framerateCounter -= frameratePoll;
        }

        // Send inputs, if any, to their various listeners
        inputManager.dispatch(currentTicks);

        // update objects according to calculated delta
        camera->update(deltaTime);
        flashlight.getComponent<Placement>().mPosition = glm::vec4(camera->getPosition(), 1.f);
        flashlight.getComponent<Placement>().mOrientation = glm::quat_cast(camera->getRotationMatrix());
        gSystemManager.getSystem<SceneSystem>()->markDirty(flashlight.getID());
        sunlight.getComponent<Placement>().mOrientation = glm::rotate(sunlight.getComponent<Placement>().mOrientation, deltaTime/10.f, {0.f, 1.f, 0.f});
        gSystemManager.getSystem<SceneSystem>()->markDirty(sunlight.getID());
        for(Entity& piece: boardPieces) {
            auto& piecePlacement { piece.getComponent<Placement>() };
            piecePlacement.mPosition.z = glm::sin(glm::radians(currentTicks/10.f + piece.getID()*45.f));
            gSystemManager.getSystem<SceneSystem>()->markDirty(piece.getID());
        }
        gSystemManager.getSystem<SceneSystem>()->updateTransforms();
        gSystemManager.getSystem<RenderSystem>()->updateCameraMatrices(*camera);

        GLenum error = glGetError();
        if(error!= GL_FALSE) {
            glewGetErrorString(error);
            std::cout << "Error occurred during mesh attribute setting: "  << error
                << ":" << glewGetErrorString(error) << std::endl;
            throw error;
        }

        gSystemManager.getSystem<RenderSystem>()->execute();
    }

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
}

void cleanup() {
    gSystemManager.unregisterAll();
    gComponentManager.unregisterAll();
    Material::Clear();
}
