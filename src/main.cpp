#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vertex.hpp"
#include "model.hpp"
#include "fly_camera.hpp"
#include "window_context_manager.hpp"
#include "shader_program.hpp"
#include "shapegen.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

void init();
void handleInput(const SDL_Event& event);

int main(int argc, char* argv[]) {
    init();

    std::vector<std::string> basicVsSrc {
        {"src/shader/common/versionHeader.glsl"},
        {"src/shader/common/fragmentAttributes.vs"},
        {"src/shader/common/vertexAttributes.vs"},
        {"src/shader/basic/basic.vs"}
    };
    std::vector<std::string> basicFsSrc {
        {"src/shader/common/versionHeader.glsl"},
        {"src/shader/common/fragmentAttributes.fs"},
        {"src/shader/basic/basic.fs"}
    };
    ShaderProgram basicShader { basicVsSrc, basicFsSrc };
    if(!basicShader.getBuildSuccess()) {
        std::cout << "Could not compile basic shader!" << std::endl;
        return 1;
    }

    std::vector<std::string> geometryVsSrc {
        {"src/shader/common/versionHeader.glsl"},
        {"src/shader/common/vertexAttributes.vs"},
        {"src/shader/common/projectionViewMatrices.vs"},
        {"src/shader/common/modelNormalMatrices.vs"},
        {"src/shader/common/fragmentAttributes.vs"},
        {"src/shader/geometry/geometry.vs"}
    };
    std::vector<std::string> geometryFsSrc {
        {"src/shader/common/versionHeader.glsl"},
        {"src/shader/common/fragmentAttributes.fs"},
        {"src/shader/common/material.fs"},
        {"src/shader/geometry/geometry.fs"}
    };
    // load our geometry shader
    ShaderProgram geometryShader { geometryVsSrc, geometryFsSrc };
    if(!geometryShader.getBuildSuccess()) {
        std::cout << "Could not compile basic shader!" << std::endl;
        return 1;
    }

    // Create a framebuffer for storing geometrical information
    GLuint geometryFBO;
    glGenFramebuffers(1, &geometryFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, geometryFBO);       // Generate 3 color buffers for storing geometrical information
        GLuint geometryBuffers[3];
        GLenum geometryBufferColorInternalFormat[3] { GL_RGBA16F, GL_RGBA16F, GL_RGBA };
        GLenum geometryBufferColorFormat[3] { GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE };
        glGenTextures(3, geometryBuffers);
        for(int i{0}; i < 3; ++i) {
            glBindTexture(GL_TEXTURE_2D, geometryBuffers[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, geometryBufferColorInternalFormat[i], gWindowWidth, gWindowHeight, 0, GL_RGBA, geometryBufferColorFormat[i], nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, geometryBuffers[i], 0
                );
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        //Generate and attach a render buffer for storing depth and stencil values
        GLuint geometryRBO {};
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

    // Bind the shared matrix uniform buffer to the same binding point
    glBindBufferRange(
        GL_UNIFORM_BUFFER,
        uboBindingPoint,
        uboSharedMatrices,
        0,
        2*sizeof(glm::mat4)
    );

    std::vector<Vertex> screenVertices {
        {
            {-1.f, -1.f, 0.f, 1.f}, // bottom left
            {0.f, 0.f, 1.f, 0.f}, // facing +Z
            glm::vec4(0.f),
            glm::vec4(1.f), // color multiplier
            {0.f, 0.f}, // bottom left UV
        },
        { 
            {1.f, -1.f, 0.f, 1.f}, // bottom right
            {0.f, 0.f, 1.f, 0.f}, // facing +Z
            glm::vec4(0.f),
            glm::vec4(1.f),
            {1.f, 0.f},// bottom right UV
        },
        {
            {1.f, 1.f, 0.f, 1.f}, // top right
            {0.f, 0.f, 1.f, 0.f}, // facing +Z
            glm::vec4(0.f),
            glm::vec4(1.f),
            {1.f, 1.f}
        },
        {
            {-1.f, 1.f, 0.f, 1.f}, // top left
            {0.f, 0.f, 1.f, 0.f}, // facing +Z
            glm::vec4(0.f),
            glm::vec4(1.f), 
            {0.f, 1.f}
        }
    };
    std::vector<GLuint> screenElements {
        {0}, {1}, {2},
        {0}, {2}, {3}
    };

    // Generate a VAO for rendering to the default framebuffer
    GLuint screenVAO;
    GLuint screenVertexBuffer;
    GLuint screenElementBuffer;
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVertexBuffer);
    glGenBuffers(1, &screenElementBuffer);
    basicShader.use();
    glBindVertexArray(screenVAO);
        glBindBuffer(GL_ARRAY_BUFFER, screenVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenElementBuffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            screenVertices.size() * sizeof(Vertex),
            screenVertices.data(),
            GL_STATIC_DRAW
        );
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            screenElements.size() * sizeof(GLuint),
            screenElements.data(),
            GL_STATIC_DRAW
        );
        basicShader.enableAttribArray("attrPosition");
        basicShader.enableAttribArray("attrColor");
        basicShader.enableAttribArray("attrTextureCoordinates");
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrPosition"),
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, mPosition))
        );
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrColor"),
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, mColor))
        );
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrTextureCoordinates"),
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, mTextureCoordinates))
        );
    glBindVertexArray(0);

    Model boardPieceModel { "data/models/Generic Board Piece.obj" };
    boardPieceModel.addInstance(glm::vec3(0.f, 0.f, -2.f), glm::quat(glm::vec3(0.f, 0.f, 0.f)), glm::vec3(1.f));
    Model sphereModel { generateSphere(2, 2) };
    sphereModel.addInstance(glm::vec3(0.f, 2.f, -2.f), glm::quat(glm::vec3(0.f)), glm::vec3(1.f));

    geometryShader.use();
    boardPieceModel.associateShaderProgram(geometryShader.getProgramID());
    sphereModel.associateShaderProgram(geometryShader.getProgramID());

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
        glViewport(0, 0, gWindowWidth, gWindowHeight);
        geometryShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, geometryFBO);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            boardPieceModel.draw(geometryShader);
            sphereModel.draw(geometryShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, gWindowWidth, gWindowHeight);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, geometryBuffers[2]);
        glActiveTexture(GL_TEXTURE0);
        basicShader.use();
        basicShader.setUInt("uRenderTexture", 0);
        glBindVertexArray(screenVAO);
            glDrawElements(
                GL_TRIANGLES,
                screenElements.size(),
                GL_UNSIGNED_INT,
                reinterpret_cast<void*>(0)
            );
        glBindVertexArray(0);

        SDL_GL_SwapWindow(WindowContextManager::getInstance().getSDLWindow());
    }

    // ... and then die
    return 0;
}

void init() {
    std::cout << "This program is running" << std::endl;

    // call get instance, just to initialize the window
    WindowContextManager::getInstance(gWindowWidth, gWindowHeight);
}
