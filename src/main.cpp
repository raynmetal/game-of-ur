#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

void init();
void cleanup();

glm::mat4 buildModelMatrix(glm::vec3 position, glm::quat orientation, glm::vec3 scale) {
    glm::mat4 rotateMatrix { glm::normalize(orientation) };
    glm::mat4 translateMatrix { glm::translate(glm::mat4(1.f), position) };
    glm::mat4 scaleMatrix { glm::scale(glm::mat4(1.f), scale) };
    return translateMatrix * rotateMatrix * scaleMatrix;
}

int main(int argc, char* argv[]) {
    init();

    GeometryRenderStage geometryRenderStage{"src/shader/geometryShader.json"};
    LightingRenderStage lightingRenderStage {"src/shader/lightingShader.json"};
    // BlurRenderStage blurRenderStage {"src/shader/gaussianblurShader.json"};
    // TonemappingRenderStage tonemappingRenderStage { "src/shader/tonemappingShader.json" };
    ScreenRenderStage screenRenderStage { "src/shader/screenShader.json" };
    geometryRenderStage.setup();
    lightingRenderStage.setup();
    // blurRenderStage.setup();
    // tonemappingRenderStage.setup();
    screenRenderStage.setup();

    // Set up a uniform buffer for shared matrices
    GLuint uboSharedMatrices;
    glGenBuffers(1, &uboSharedMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboSharedMatrices);
        glBufferData(
            GL_UNIFORM_BUFFER,
            2*sizeof(glm::mat4),
            NULL,
            GL_STATIC_DRAW
        );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // Let the shader(s) know where to find the shared matrices
    GLuint uboBindingPoint {0};

    // Bind the shared matrix uniform buffer to the same binding point
    glBindBufferRange(
        GL_UNIFORM_BUFFER,
        uboBindingPoint,
        uboSharedMatrices,
        0,
        2*sizeof(glm::mat4)
    );

    ModelHandle boardPieceModelHandle { 
        ModelManager::getInstance().registerResource("data/models/Generic Board Piece.obj", {"data/models/Generic Board Piece.obj"}) 
    };

    std::vector<LightData> lightData {};
    lightData.push_back(
        LightData::MakeSpotLight(
            glm::vec3(0.f),
            glm::vec3(0.f, 0.f, -1.f),
            4.f,
            13.f,
            glm::vec3(2.f),
            glm::vec3(4.f),
            glm::vec3(.3f),
            .07f,
            .03f
        )
    );

    lightData.push_back(
        LightData::MakePointLight(
            glm::vec3(0.f, 1.5f, -1.f),
            glm::vec3(2.f, 0.6f, 1.2f),
            glm::vec3(3.1f, 1.04f, .32f),
            glm::vec3(0.1f, 0.01f, 0.03f),
            .07f,
            .02f
        )
    );

    lightData.push_back(
        LightData::MakeDirectionalLight(
            glm::vec3(0.f, -1.f, 1.f),
            glm::vec3(20.f),
            glm::vec3(20.f),
            glm::vec3(0.2f)
        )
    );

    MaterialHandle lightMaterialHandle {
        MaterialManager::getInstance().registerResource(
            "lightMaterial",
            {}
        )
    };
    lightMaterialHandle.getResource().updateIntProperty("screenWidth", 800);
    lightMaterialHandle.getResource().updateIntProperty("screenHeight", 600);

    // geometryRenderStage.attachModel("boardPiece", boardPieceModelHandle);
    // lightingRenderStage.attachLightCollection("sceneLights", sceneLightsHandle);

    FlyCamera camera {
        glm::vec3(0.f), 0.f, 0.f, 0.f
    };

    // Send shared matrices to the uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, uboSharedMatrices);
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            0,
            sizeof(glm::mat4),
            glm::value_ptr(camera.getProjectionMatrix())
        );
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            sizeof(glm::mat4),
            sizeof(glm::mat4),
            glm::value_ptr(camera.getViewMatrix())
        );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    float exposure = 1.f;

    // Debug: list of screen textures that may be rendered
    constexpr GLuint nScreenTextures {5};
    GLuint currScreenTexture {2};
    const TextureHandle screenTextureHandles[nScreenTextures] {
        {geometryRenderStage.getRenderTarget("geometryPosition")}, {geometryRenderStage.getRenderTarget("geometryNormal")}, {geometryRenderStage.getRenderTarget("geometryAlbedoSpecular")},
        {lightingRenderStage.getRenderTarget("litScene")}, {lightingRenderStage.getRenderTarget("brightCutoff")},
        // {blurRenderStage.getRenderTarget("pingBuffer")},
        // {tonemappingRenderStage.getRenderTarget("tonemappedScene")}
    };
    float gamma { 2.2f };

    // Last pieces of pipeline setup, where we connect all the
    // render stages together
    lightingRenderStage.attachTexture("positionMap", geometryRenderStage.getRenderTarget("geometryPosition"));
    lightingRenderStage.attachTexture("normalMap", geometryRenderStage.getRenderTarget("geometryNormal"));
    lightingRenderStage.attachTexture("albedoSpecularMap", geometryRenderStage.getRenderTarget("geometryAlbedoSpecular"));
    // blurRenderStage.attachTexture("unblurredImage", lightingRenderStage.getRenderTarget("brightCutoff"));
    // tonemappingRenderStage.attachTexture("litScene", lightingRenderStage.getRenderTarget("litScene"));
    // tonemappingRenderStage.attachTexture("bloomEffect", blurRenderStage.getRenderTarget("pingBuffer"));
    // tonemappingRenderStage.updateFloatParameter("exposure", exposure);
    // tonemappingRenderStage.updateFloatParameter("gamma", gamma);
    // tonemappingRenderStage.updateIntParameter("combine", true);
    screenRenderStage.attachTexture("renderSource", screenTextureHandles[currScreenTexture]);
    geometryRenderStage.validate();
    lightingRenderStage.validate();
    // blurRenderStage.validate();
    // tonemappingRenderStage.validate();
    screenRenderStage.validate();

    MeshHandle sphereMesh { generateSphereMesh(10, 5) };

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
                currScreenTexture = (currScreenTexture + 1) % nScreenTextures;
                screenRenderStage.attachTexture("renderSource", screenTextureHandles[currScreenTexture]);
            }
            // if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_PERIOD) {
            //     exposure += .1f;
            //     tonemappingRenderStage.updateFloatParameter("exposure", exposure);
            // }
            // if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_COMMA) {
            //     if (exposure < .1f) exposure = 0.f;
            //     else exposure -= .1f; 
            //     tonemappingRenderStage.updateFloatParameter("exposure", exposure);
            // }
            // if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_j) {
            //     gamma -= .1f;
            //     if(gamma < 1.6f) gamma = 1.6f;
            //     tonemappingRenderStage.updateFloatParameter("gamma", gamma);
            // }
            // else if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_k) {
            //     gamma += .1f;
            //     if(gamma > 3.f) gamma = 3.f;
            //     tonemappingRenderStage.updateFloatParameter("gamma", gamma);
            // }
            camera.processInput(event);
        }
        if(quit) break;

        LightData& flashlightData { lightData[0] };

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

        // update objects according to calculated delta
        camera.update(deltaTime);
        flashlightData.mPosition = glm::vec4(camera.getPosition(), 1.f);
        flashlightData.mDirection = glm::vec4(camera.getForward(), 0.f);

        //Submit our various board piece meshes to the opaque render queue
        std::vector<MeshHandle> boardPieceMeshHandles {boardPieceModelHandle.getResource().getMeshHandles()};
        std::vector<MaterialHandle> boardPieceMaterialHandles {boardPieceModelHandle.getResource().getMaterialHandles()};
        for(std::size_t i{0}; i < boardPieceMeshHandles.size(); ++i) {
            geometryRenderStage.submitToRenderQueue(
                RenderUnit {
                    boardPieceMeshHandles[i],
                    boardPieceMaterialHandles[i],
                    buildModelMatrix(
                        {0.f, 0.f, -2.f}, {}, {1.f, 1.f, 1.f}
                    )
                }
            );
        }


        //Submit our light data to the lighting render queue
        for(const auto& light: lightData) {
            const float sqrt2 { sqrt(2.f) };
            lightingRenderStage.submitToRenderQueue(
                RenderLightUnit {
                    sphereMesh,
                    lightMaterialHandle,
                    buildModelMatrix(
                        light.mPosition,
                        {},
                        light.mType != LightData::directional?
                            glm::vec3{ light.mRadius }:
                            glm::vec3(sqrt2, sqrt2, 1.f)
                    ),
                    light
                }
            );
        }

        // Send shared matrices to the uniform buffer
        glBindBuffer(GL_UNIFORM_BUFFER, uboSharedMatrices);
            glBufferSubData(
                GL_UNIFORM_BUFFER,
                0,
                sizeof(glm::mat4),
                glm::value_ptr(camera.getProjectionMatrix())
            );
            glBufferSubData(
                GL_UNIFORM_BUFFER,
                sizeof(glm::mat4),
                sizeof(glm::mat4),
                glm::value_ptr(camera.getViewMatrix())
            );
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        GLenum error = glGetError();
        if(error!= GL_FALSE) {
            glewGetErrorString(error);
            std::cout << "Error occurred during mesh attribute setting: "  << error
                << ":" << glewGetErrorString(error) << std::endl;
            throw error;
        }
        //Render geometry to our geometry framebuffer
        geometryRenderStage.execute();
        lightingRenderStage.execute();
        // blurRenderStage.execute();
        // tonemappingRenderStage.execute();
        screenRenderStage.execute();

        WindowContextManager::getInstance().swapBuffers();
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
    Material::Clear();
}
