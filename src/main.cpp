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
#include "engine/input_system/input_system.hpp"

#include "app/fly_camera.hpp"
#include "app/back_and_forth.hpp"
#include "app/revolve.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

const GLuint kSimulationStep{ 1000/30 };

void initialize();
void cleanup();

int main(int argc, char* argv[]) {
    initialize();

    InputManager inputManager {};

    std::ifstream jsonFileStream;
    std::string inputPath { "data/input_bindings.json" };
    jsonFileStream.open(inputPath);
    nlohmann::json inputBindingJSON { nlohmann::json::parse(jsonFileStream) };
    jsonFileStream.close();
    inputManager.loadInputConfiguration(inputBindingJSON.at(0));

    std::shared_ptr<SimObject> camera { SimObject::create<CameraProperties>({}, "camera", {}) };
    camera->addAspect<FlyCamera>();

    const float sqrt2 { sqrt(2.f) };
    std::shared_ptr<SceneNode> flashlight { SceneNode::create<LightEmissionData>(
        {
            glm::vec4(glm::vec3(0.f), 1.f),
            glm::quat(),
            glm::vec3{1.f}
        },
        "flashlight",
        LightEmissionData::MakeSpotLight(
            4.f,
            13.f,
            glm::vec3(2.f),
            glm::vec3(4.f),
            glm::vec3(.3f),
            .07f,
            .03f
        )
    )};
    {
        Placement placement { flashlight->getComponent<Placement>() };
        placement.mScale = { glm::vec3(flashlight->getComponent<LightEmissionData>().mRadius)};
        flashlight->updateComponent<Placement>(placement);
    }
    std::shared_ptr<SimObject> sunlight { SimObject::create<LightEmissionData>(
        {
            glm::vec4{glm::vec3(0.f), 1.f},
            glm::quat { glm::vec3 {
                glm::radians(-20.f), // pitch
                glm::radians(180.f), // yaw
                glm::radians(0.f) // roll
            }},
            glm::vec3{sqrt2, sqrt2, 1.f}
        },
        "sunlight",
        {
            LightEmissionData::MakeDirectionalLight(
                glm::vec3{20.f},
                glm::vec3{20.f},
                glm::vec3{.4f}
            )
        }
    )};
    sunlight->addAspect<Revolve>(); // make sunlight revolve


    ResourceDatabase::addResourceDescription({
        {"name", "boardPieceModel"},
        {"type", StaticModel::getResourceTypeName()},
        {"method", StaticModelFromFile::getResourceConstructorName()},
        {"parameters", {
            {"path", "data/models/Generic Board Piece.obj"},
        }}
    });

    std::shared_ptr<SimObject> boardPiecePrototype { SimObject::create<std::shared_ptr<StaticModel>>(
        Placement{{0.f, -1.f, -1.f, 1.f}},
        "board_piece",
        ResourceDatabase::getResource<StaticModel>("boardPieceModel")
    )};
    boardPiecePrototype->addAspect<BackAndForth>();

    std::vector<std::shared_ptr<SimObject>> boardPieces(21);
    boardPieces[0] = SimObject::copy(boardPiecePrototype);
    std::shared_ptr<SimObject> previousBoardPiece = boardPieces[0];
    for(std::size_t i{1}; i < 21; ++i) {
        boardPieces[i] = SimObject::copy(boardPiecePrototype);

        Placement boardPiecePlacement { boardPieces[i]->getComponent<Placement>() };
        boardPiecePlacement.mPosition = {0.f, 2.f, 0.f, 1.f};
        boardPiecePlacement.mOrientation = glm::rotate(boardPiecePlacement.mOrientation, glm::radians(360.f/(1+20)), {0.f, 0.f, -1.f});
        boardPieces[i]->updateComponent<Placement>(boardPiecePlacement);

        // each board piece is the parent of the board piece made in
        // the next iteration
        previousBoardPiece->addNode(boardPieces[i], "/");
        previousBoardPiece = boardPieces[i];
    }

    SimpleECS::getSystem<SceneSystem>()->addNode(camera, "/");
    SimpleECS::getSystem<SceneSystem>()->addNode(flashlight, "/camera/");
    SimpleECS::getSystem<SceneSystem>()->addNode(sunlight, "/");
    SimpleECS::getSystem<SceneSystem>()->addNode(boardPieces[0], "/");

    inputManager["Camera"].registerActionHandler("Rotate", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("ToggleControl", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("Move", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
    inputManager["Camera"].registerActionHandler("UpdateFOV", std::shared_ptr<FlyCamera>(camera, &(camera->getAspect<FlyCamera>())));
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

    std::size_t toRemove {21};

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

        // remove nodes 3 at a time every 5 seconds
        if(toRemove > 0 && currentTicks/5000 > (21 - toRemove)/3 && currentTicks/5000 < 10) {
            toRemove -= 3;
            boardPieces[toRemove]->removeNode("/");

        } else if(currentTicks/5000 > 2 + (21 + toRemove)/3 && toRemove < 21 && currentTicks/5000 >= 10) {

            if(toRemove == 0) {
                SimpleECS::getSystem<SceneSystem>()->addNode(boardPieces[toRemove],"/");
            }
            else {
                boardPieces[toRemove-3]->addNode(boardPieces[toRemove], "/board_piece/board_piece/");
            }
            toRemove += 3;
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
