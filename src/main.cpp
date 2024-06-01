#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/fly_camera.hpp"
#include "engine/window_context_manager.hpp"
#include "engine/material_manager.hpp"
#include "engine/texture_manager.hpp"
#include "engine/model_manager.hpp"
#include "engine/light_manager.hpp"
#include "engine/mesh_manager.hpp"
#include "engine/shader_program_manager.hpp"
#include "engine/framebuffer_manager.hpp"
#include "engine/render_stage.hpp"
#include "engine/shapegen.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

void init();

int main(int argc, char* argv[]) {
    init();

    ShaderProgramHandle tonemappingShaderHandle { ShaderProgramManager::getInstance().registerResource(
        "src/shader/tonemappingShader.json",
        {"src/shader/tonemappingShader.json"}
    )};
    if(!tonemappingShaderHandle.getResource().getBuildSuccess()) {
        std::cout << "Could not compile " << tonemappingShaderHandle.getName() <<"!" << std::endl;
        return 1;
    }

    ShaderProgramHandle gaussianblurShaderHandle { ShaderProgramManager::getInstance().registerResource(
        "src/shader/gaussianblurShader.json",
        {"src/shader/gaussianblurShader.json"}
    )};
    if(!gaussianblurShaderHandle.getResource().getBuildSuccess()) {
        std::cout << "Could not compile " << gaussianblurShaderHandle.getName() << "!" << std::endl;
        return 1;
    }

    ShaderProgramHandle geometryShaderHandle { ShaderProgramManager::getInstance().registerResource(
        "src/shader/geometryShader.json",
        {"src/shader/geometryShader.json"}
    )};
    if(!geometryShaderHandle.getResource().getBuildSuccess()) {
        std::cout << "Could not compile basic shader!" << std::endl;
        return 1;
    }

    ShaderProgramHandle lightingShaderHandle {ShaderProgramManager::getInstance().registerResource(
        "src/shader/lightingShader.json",
        {"src/shader/lightingShader.json"}
    )};
    if(!lightingShaderHandle.getResource().getBuildSuccess()) {
        std::cout << "Could not compile lighting shader!" << std::endl;
        return 1;
    }

    ShaderProgramHandle screenShaderHandle {
        ShaderProgramManager::getInstance().registerResource(
            "src/shader/screenShader.json",
            {"src/shader/screenShader.json"}
        )
    };

    // Create a framebuffer for storing geometrical information
    FramebufferHandle geometryFramebufferHandle {
        FramebufferManager::getInstance().registerResource(
            "geometryFramebuffer",
            {
                {gWindowWidth, gWindowHeight},
                3,
                {
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Float},
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Float},
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Byte}
                },
                true
            }
        )
    };
    FramebufferHandle bloomFramebufferHandle {
        FramebufferManager::getInstance().registerResource(
            "bloomFramebuffer",
            {
                {gWindowWidth, gWindowHeight},
                2,
                {
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Float},
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Float}
                },
                false
            }
        )
    };
    FramebufferHandle lightingFramebufferHandle {
        FramebufferManager::getInstance().registerResource(
            "lightingFramebuffer",
            {
                {gWindowWidth, gWindowHeight},
                2,
                {
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Float},
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Float}
                },
                true
            }
        )
    };
    FramebufferHandle tonemappingFramebufferHandle {
        FramebufferManager::getInstance().registerResource(
            "tonemappingFramebuffer",
            {
                {gWindowWidth, gWindowHeight},
                1,
                {
                    {ColorBufferDefinition::ComponentCount::Four, ColorBufferDefinition::DataType::Byte}
                },
                true
            }
        )
    };

    GeometryRenderStage geometryRenderStage {geometryShaderHandle, geometryFramebufferHandle};
    geometryRenderStage.declareRenderTarget("geometryPosition", 0);
    geometryRenderStage.declareRenderTarget("geometryNormal", 1);
    geometryRenderStage.declareRenderTarget("geometryAlbedoSpecular", 2);
    LightingRenderStage lightingRenderStage {lightingShaderHandle, lightingFramebufferHandle};
    lightingRenderStage.declareRenderTarget("litScene", 0);
    lightingRenderStage.declareRenderTarget("brightCutoff", 1);
    BlurRenderStage blurRenderStage {gaussianblurShaderHandle, bloomFramebufferHandle};
    blurRenderStage.declareRenderTarget("pingBuffer", 0);
    blurRenderStage.declareRenderTarget("pongBuffer", 1);
    TonemappingRenderStage tonemappingRenderStage {tonemappingShaderHandle, tonemappingFramebufferHandle };
    tonemappingRenderStage.declareRenderTarget("tonemappedScene", 0);
    ScreenRenderStage screenRenderStage {screenShaderHandle};

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
    geometryShaderHandle.getResource().use();
    geometryShaderHandle.getResource().setUniformBlock("Matrices", uboBindingPoint);
    lightingShaderHandle.getResource().use();
    lightingShaderHandle.getResource().setUniformBlock("Matrices", uboBindingPoint);
    // Bind the shared matrix uniform buffer to the same binding point
    glBindBufferRange(
        GL_UNIFORM_BUFFER,
        uboBindingPoint,
        uboSharedMatrices,
        0,
        2*sizeof(glm::mat4)
    );

    // Generate a VAO for rendering to the default framebuffer
    MeshHandle screenMeshHandle{ generateRectangleMesh() };
    tonemappingRenderStage.attachMesh("screenMesh", screenMeshHandle);
    blurRenderStage.attachMesh("screenMesh", screenMeshHandle);

    ModelHandle boardPieceModelHandle { 
        ModelManager::getInstance().registerResource("data/models/Generic Board Piece.obj", {"data/models/Generic Board Piece.obj"}) 
    };

    boardPieceModelHandle.getResource().addInstance(glm::vec3(0.f, 0.f, -2.f), glm::quat(glm::vec3(0.f, 0.f, 0.f)), glm::vec3(1.f));
    LightCollectionHandle sceneLightsHandle {
        LightCollectionManager::getInstance().registerResource("sceneLights",
            {}
        )
    };
    GLuint flashlight { sceneLightsHandle.getResource().addLight(
        Light::MakeSpotLight(
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
    )};

    sceneLightsHandle.getResource().addLight(
        Light::MakePointLight(
            glm::vec3(0.f, 1.5f, -1.f),
            glm::vec3(2.f, 0.6f, 1.2f),
            glm::vec3(3.1f, 1.04f, .32f),
            glm::vec3(0.1f, 0.01f, 0.03f),
            .07f,
            .02f
        )
    );
    sceneLightsHandle.getResource().addLight(Light::MakeDirectionalLight(
        glm::vec3(0.f, -1.f, 1.f),
        glm::vec3(20.f),
        glm::vec3(20.f),
        glm::vec3(0.2f)
    ));
    geometryRenderStage.attachModel("boardPiece", boardPieceModelHandle);
    lightingRenderStage.attachLightCollection("sceneLights", sceneLightsHandle);

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
    constexpr GLuint nScreenTextures {7};
    GLuint currScreenTexture {3};
    const TextureHandle screenTextureHandles[nScreenTextures] {
        {geometryRenderStage.getRenderTarget("geometryPosition")}, {geometryRenderStage.getRenderTarget("geometryNormal")}, {geometryRenderStage.getRenderTarget("geometryAlbedoSpecular")},
        {lightingRenderStage.getRenderTarget("litScene")}, {lightingRenderStage.getRenderTarget("brightCutoff")},
        {blurRenderStage.getRenderTarget("pingBuffer")}, 
        {tonemappingRenderStage.getRenderTarget("tonemappedScene")}
    };
    float gamma = 2.2f;

    // Last pieces of pipeline setup
    lightingRenderStage.attachTexture("positionMap", geometryRenderStage.getRenderTarget("geometryPosition"));
    lightingRenderStage.attachTexture("normalMap", geometryRenderStage.getRenderTarget("geometryNormal"));
    lightingRenderStage.attachTexture("albedoSpecularMap", geometryRenderStage.getRenderTarget("geometryAlbedoSpecular"));
    lightingRenderStage.updateIntParameter("screenWidth", gWindowWidth);
    lightingRenderStage.updateIntParameter("screenHeight", gWindowHeight);
    blurRenderStage.attachMesh("screenMesh", screenMeshHandle);
    blurRenderStage.attachTexture("unblurredImage", lightingRenderStage.getRenderTarget("brightCutoff"));
    blurRenderStage.updateIntParameter("nPasses", 12);
    tonemappingRenderStage.attachMesh("screenMesh", screenMeshHandle);
    tonemappingRenderStage.attachTexture("litScene", lightingRenderStage.getRenderTarget("litScene"));
    tonemappingRenderStage.attachTexture("bloomEffect", blurRenderStage.getRenderTarget("pingBuffer"));
    tonemappingRenderStage.updateFloatParameter("exposure", exposure);
    tonemappingRenderStage.updateFloatParameter("gamma", gamma);
    tonemappingRenderStage.updateIntParameter("combine", true);
    screenRenderStage.attachMesh("screenMesh", screenMeshHandle);
    screenRenderStage.attachTexture("renderSource", screenTextureHandles[currScreenTexture]);
    geometryRenderStage.validate();
    lightingRenderStage.validate();
    blurRenderStage.validate();
    tonemappingRenderStage.validate();
    screenRenderStage.validate();

    //Timing related variables
    GLuint previousTicks { SDL_GetTicks() };
    float framerate {0.f};
    const float frameratePoll {1.f};
    float framerateCounter {0.f};

    //Main event loop
    glClearColor(0.f, 0.f, 0.f, 1.f);
    SDL_Event event;
    bool quit {false};
    glDisable(GL_FRAMEBUFFER_SRGB);
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
            if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_PERIOD) {
                exposure += .1f;
                tonemappingRenderStage.updateFloatParameter("exposure", exposure);
            }
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_COMMA) {
                if (exposure < .1f) exposure = 0.f;
                else exposure -= .1f; 
                tonemappingRenderStage.updateFloatParameter("exposure", exposure);
            }
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_j) {
                gamma -= .1f;
                if(gamma < 1.6f) gamma = 1.6f;
                tonemappingRenderStage.updateFloatParameter("gamma", gamma);
            }
            else if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_k) {
                gamma += .1f;
                if(gamma > 3.f) gamma = 3.f;
                tonemappingRenderStage.updateFloatParameter("gamma", gamma);
            }
            camera.processInput(event);
        }
        if(quit) break;

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
        Light flashlightAttributes { sceneLightsHandle.getResource().getLight(flashlight) };
        flashlightAttributes.mPosition = glm::vec4(camera.getPosition(), 1.f);
        flashlightAttributes.mDirection = glm::vec4(camera.getForward(), 0.f);
        sceneLightsHandle.getResource().updateLight(flashlight, flashlightAttributes);

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

        //Render geometry to our geometry framebuffer
        geometryRenderStage.execute();

        lightingRenderStage.execute();

        blurRenderStage.execute();

        tonemappingRenderStage.execute();

        screenRenderStage.execute();

        WindowContextManager::getInstance().swapBuffers();
    }

    // ... and then die
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
    MeshManager::getInstance();
    LightCollectionManager::getInstance();
    FramebufferManager::getInstance();
    MaterialManager::getInstance();
    ModelManager::getInstance();
}
