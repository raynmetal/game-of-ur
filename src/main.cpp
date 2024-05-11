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
#include "engine/shader_program.hpp"
#include "engine/shapegen.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

void init();

int main(int argc, char* argv[]) {
    init();

    std::string tonemappingProgramJSONPath { "src/shader/tonemappingShader.json" };
    ShaderProgram tonemappingShader { tonemappingProgramJSONPath };
    if(!tonemappingShader.getBuildSuccess()) {
        std::cout << "Could not compile tonemapping shader!" << std::endl;
        return 1;
    }

    std::string gaussianblurProgramJSONPath { "src/shader/gaussianblurShader.json" };
    ShaderProgram gaussianblurShader { gaussianblurProgramJSONPath };
    if(!gaussianblurShader.getBuildSuccess()) {
        std::cout << "Could not compile gaussian blur shader!" << std::endl;
        return 1;
    }

    std::string geometryProgramJSONPath { "src/shader/geometryShader.json" };
    ShaderProgram geometryShader { geometryProgramJSONPath };
    if(!geometryShader.getBuildSuccess()) {
        std::cout << "Could not compile basic shader!" << std::endl;
        return 1;
    }

    std::string lightingProgramJSONPath { "src/shader/lightingShader.json" };
    ShaderProgram lightingShader { lightingProgramJSONPath };
    if(!lightingShader.getBuildSuccess()) {
        std::cout << "Could not compile lighting shader!" << std::endl;
        return 1;
    }

    // Create a framebuffer for storing geometrical information
    GLuint geometryFBO;
    glGenFramebuffers(1, &geometryFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, geometryFBO);       // Generate 3 color buffers for storing geometrical information
        GLenum geometryBufferColorFormat[3] { GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE };
        std::vector<TextureHandle> geometryBufferHandles {};
        for(int i{0}; i < 3; ++i) {
            geometryBufferHandles.push_back(
                TextureManager::getInstance().registerResource(
                    std::string("geometryBuffer") + std::to_string(i),
                    {
                        glm::vec2(gWindowWidth, gWindowHeight),
                        geometryBufferColorFormat[i],
                        GL_LINEAR,
                        GL_LINEAR,
                        GL_CLAMP_TO_EDGE,
                        GL_CLAMP_TO_EDGE
                    }
                )
            );
            geometryBufferHandles[i].getResource().attachToFramebuffer(i);
        }

        //Generate and attach a render buffer for storing depth and stencil values
        GLuint geometryRBO;
        glGenRenderbuffers(1, &geometryRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, geometryRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, gWindowWidth, gWindowHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, geometryRBO);

        // Specify that we'll be rendering to 3 colour attachments in this framebuffer
        const GLenum geometryColorAttachments[3] {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, geometryColorAttachments);

        // Check for framebuffer completeness
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR::FRAMEBUFFER: Framebuffer is not complete" << std::endl;
            return 1;
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint bloomFBO;
    glGenFramebuffers(1, &bloomFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
        std::vector<TextureHandle> bloomBufferHandles {};
        for(int i{0}; i < 2; ++i) {
            bloomBufferHandles.push_back(
                TextureManager::getInstance().registerResource(
                    std::string{"bloomBuffer"} + std::to_string(i),
                    {
                        glm::vec2(gWindowWidth, gWindowHeight),
                        GL_FLOAT,
                        GL_LINEAR,
                        GL_LINEAR,
                        GL_CLAMP_TO_EDGE,
                        GL_CLAMP_TO_EDGE
                    }
                )
            );
            bloomBufferHandles[i].getResource().attachToFramebuffer(i);
        }
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "FRAMEBUFFER::ERROR:: Bloom framebuffer is not complete" << std::endl;
            return 1;
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint lightingFBO;
    glGenFramebuffers(1, &lightingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lightingFBO);
        std::vector<TextureHandle> lightingBufferHandles {};
        for(int i {0}; i < 2; ++i) {
            lightingBufferHandles.push_back(
                TextureManager::getInstance().registerResource(
                    std::string("lightingBuffer") + std::to_string(i),
                    {
                        glm::vec2(gWindowWidth, gWindowHeight),
                        GL_FLOAT,
                        GL_LINEAR,
                        GL_LINEAR,
                        GL_CLAMP_TO_EDGE,
                        GL_CLAMP_TO_EDGE
                    }
                )
            );
            lightingBufferHandles[i].getResource().attachToFramebuffer(i);
        }

        GLuint lightingRBO;
        glGenRenderbuffers(1, &lightingRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, lightingRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, gWindowWidth, gWindowHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, lightingRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        const GLenum lightingColorAttachments[2] { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, lightingColorAttachments);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR::FRAMEBUFFER: Lighting framebuffer is incomplete" << std::endl;
            return 1;
        }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
    geometryShader.use();
    geometryShader.setUniformBlock("Matrices", uboBindingPoint);
    lightingShader.use();
    lightingShader.setUniformBlock("Matrices", uboBindingPoint);

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
    tonemappingShader.use();
    screenMesh.associateShaderProgram(tonemappingShader.getProgramID());
    gaussianblurShader.use();
    screenMesh.associateShaderProgram(gaussianblurShader.getProgramID());
    
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
    geometryShader.use();
    boardPieceModelHandle.getResource().associateShaderProgram(geometryShader.getProgramID());
    lightingShader.use();
    sceneLights.associateShaderProgram(lightingShader.getProgramID());

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
        geometryShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, geometryFBO);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            boardPieceModelHandle.getResource().draw(geometryShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        for(int i{0}; i < 3; ++i) {
            geometryBufferHandles[i].getResource().bind(i);
        }
        lightingShader.use();
        lightingShader.setUInt("uGeometryPositionMap", 0);
        lightingShader.setUInt("uGeometryNormalMap", 1);
        lightingShader.setUInt("uGeometryAlbedoSpecMap", 2);
        lightingShader.setUInt("uScreenWidth", gWindowWidth);
        lightingShader.setUInt("uScreenHeight", gWindowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, lightingFBO);
            // glEnable(GL_DEPTH_TEST);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glClear(GL_COLOR_BUFFER_BIT); //| GL_DEPTH_BUFFER_BIT);
            sceneLights.draw(lightingShader);
            glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        gaussianblurShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glClear(GL_COLOR_BUFFER_BIT);
            constexpr int nBlurPasses {6};
            for(int i{0}; i < nBlurPasses; ++i) {
                    // i? bloomBuffers[i%2]: lightingBuffers[1]
                if(!i) lightingBufferHandles[1].getResource().bind(0);
                else bloomBufferHandles[i%2].getResource().bind(0);
                glDrawBuffer(GL_COLOR_ATTACHMENT0 + (1+i)%2);
                gaussianblurShader.setUInt("uGenericTexture", 0);
                gaussianblurShader.setUBool("uHorizontal", i%2);
                screenMesh.draw(gaussianblurShader, 1);
            }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glClear(GL_COLOR_BUFFER_BIT);
        screenTextureHandles[currScreenTexture].getResource().bind(0);
        bloomBufferHandles[1].getResource().bind(1);
        tonemappingShader.use();
        tonemappingShader.setUInt("uGenericTexture", 0);
        tonemappingShader.setUInt("uGenericTexture1", 1);
        tonemappingShader.setUFloat("uExposure", exposure);
        tonemappingShader.setUFloat("uGamma", gamma);
        tonemappingShader.setUBool("uCombine", screenTextureHandles[currScreenTexture] == lightingBufferHandles[0]);
        screenMesh.draw(tonemappingShader, 1);

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
}
