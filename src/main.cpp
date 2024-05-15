#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/light.hpp"
#include "engine/fly_camera.hpp"
#include "engine/window_context_manager.hpp"
#include "engine/texture_manager.hpp"
#include "engine/model_manager.hpp"
#include "engine/shader_program_manager.hpp"
#include "engine/framebuffer_manager.hpp"
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
    Mesh screenMesh{ generateRectangleMesh() };
    tonemappingShaderHandle.getResource().use();
    screenMesh.associateShaderProgram(tonemappingShaderHandle);
    gaussianblurShaderHandle.getResource().use();
    screenMesh.associateShaderProgram(gaussianblurShaderHandle);

    ModelHandle boardPieceModelHandle { 
        ModelManager::getInstance().registerResource("data/models/Generic Board Piece.obj", {"data/models/Generic Board Piece.obj"}) 
    };

    boardPieceModelHandle.getResource().addInstance(glm::vec3(0.f, 0.f, -2.f), glm::quat(glm::vec3(0.f, 0.f, 0.f)), glm::vec3(1.f));
    LightCollection sceneLights {};
    GLuint flashlight { sceneLights.addLight(
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

    sceneLights.addLight(
        Light::MakePointLight(
            glm::vec3(0.f, 1.5f, -1.f),
            glm::vec3(2.f, 0.6f, 1.2f),
            glm::vec3(3.1f, 1.04f, .32f),
            glm::vec3(0.1f, 0.01f, 0.03f),
            .07f,
            .02f
        )
    );
    sceneLights.addLight(Light::MakeDirectionalLight(
        glm::vec3(0.f, -1.f, 1.f),
        glm::vec3(20.f),
        glm::vec3(20.f),
        glm::vec3(0.2f)
    ));
    geometryShaderHandle.getResource().use();
    boardPieceModelHandle.getResource().associateShaderProgram(geometryShaderHandle);
    lightingShaderHandle.getResource().use();
    sceneLights.associateShaderProgram(lightingShaderHandle);

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
    constexpr GLuint nScreenTextures {6};
    GLuint currScreenTexture {3};
    const std::vector<TextureHandle> geometryBufferHandles {
        geometryFramebufferHandle.getResource().getColorBufferHandles()
    };
    const std::vector<TextureHandle> bloomBufferHandles {
        bloomFramebufferHandle.getResource().getColorBufferHandles()
    };
    const std::vector<TextureHandle> lightingBufferHandles {
        lightingFramebufferHandle.getResource().getColorBufferHandles()
    };
    const TextureHandle screenTextureHandles[nScreenTextures] {
        {geometryBufferHandles[0]}, {geometryBufferHandles[1]}, {geometryBufferHandles[2]},
        {lightingBufferHandles[0]}, {lightingBufferHandles[1]},
        {bloomBufferHandles[1]}
    };
    float gamma = 2.2f;

    //Timing related variables
    GLuint previousTicks { SDL_GetTicks() };
    float framerate {0.f};
    const float frameratePoll {1.f};
    float framerateCounter {0.f};

    //Main event loop
    glClearColor(0.f, 0.f, 0.f, 1.f);
    SDL_Event event;
    bool quit {false};
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
            }
            if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_PERIOD) {
                exposure += .1f;
            }
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_COMMA) {
                if (exposure < .1f) exposure = 0.f;
                else exposure -= .1f; 
            }
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_k) {
                gamma -= .1f;
                if(gamma < 1.6f) gamma = 1.6f;
            }
            else if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_l) {
                gamma += .1f;
                if(gamma > 3.f) gamma = 3.f;
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
        Light flashlightAttributes { sceneLights.getLight(flashlight) };
        flashlightAttributes.mPosition = glm::vec4(camera.getPosition(), 1.f);
        flashlightAttributes.mDirection = glm::vec4(camera.getForward(), 0.f);
        sceneLights.updateLight(flashlight, flashlightAttributes);

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
        geometryShaderHandle.getResource().use();
        geometryFramebufferHandle.getResource().bind();
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            boardPieceModelHandle.getResource().draw(geometryShaderHandle);
        geometryFramebufferHandle.getResource().unbind();

        for(int i{0}; i < 3; ++i) {
            geometryBufferHandles[i].getResource().bind(i);
        }
        lightingShaderHandle.getResource().use();
        lightingShaderHandle.getResource().setUInt("uGeometryPositionMap", 0);
        lightingShaderHandle.getResource().setUInt("uGeometryNormalMap", 1);
        lightingShaderHandle.getResource().setUInt("uGeometryAlbedoSpecMap", 2);
        lightingShaderHandle.getResource().setUInt("uScreenWidth", gWindowWidth);
        lightingShaderHandle.getResource().setUInt("uScreenHeight", gWindowHeight);
        lightingFramebufferHandle.getResource().bind();
            // glEnable(GL_DEPTH_TEST);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glClear(GL_COLOR_BUFFER_BIT); //| GL_DEPTH_BUFFER_BIT);
            sceneLights.draw(lightingShaderHandle);
            glDisable(GL_BLEND);
        lightingFramebufferHandle.getResource().unbind();

        gaussianblurShaderHandle.getResource().use();
        bloomFramebufferHandle.getResource().bind();
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glClear(GL_COLOR_BUFFER_BIT);
            constexpr int nBlurPasses {6};
            for(int i{0}; i < nBlurPasses; ++i) {
                    // i? bloomBuffers[i%2]: lightingBuffers[1]
                if(!i) lightingBufferHandles[1].getResource().bind(0);
                else bloomBufferHandles[i%2].getResource().bind(0);
                glDrawBuffer(GL_COLOR_ATTACHMENT0 + (1+i)%2);
                gaussianblurShaderHandle.getResource().setUInt("uGenericTexture", 0);
                gaussianblurShaderHandle.getResource().setUBool("uHorizontal", i%2);
                screenMesh.draw(gaussianblurShaderHandle, 1);
            }
        bloomFramebufferHandle.getResource().unbind();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glClear(GL_COLOR_BUFFER_BIT);
        screenTextureHandles[currScreenTexture].getResource().bind(0);
        bloomBufferHandles[1].getResource().bind(1);
        tonemappingShaderHandle.getResource().use();
        tonemappingShaderHandle.getResource().setUInt("uGenericTexture", 0);
        tonemappingShaderHandle.getResource().setUInt("uGenericTexture1", 1);
        tonemappingShaderHandle.getResource().setUFloat("uExposure", exposure);
        tonemappingShaderHandle.getResource().setUFloat("uGamma", gamma);
        tonemappingShaderHandle.getResource().setUBool("uCombine", screenTextureHandles[currScreenTexture] == lightingBufferHandles[0]);
        screenMesh.draw(tonemappingShaderHandle, 1);

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
    ModelManager::getInstance();
    FramebufferManager::getInstance();
}
