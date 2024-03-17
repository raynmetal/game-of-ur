#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "fly_camera.hpp"
#include "window_context_manager.hpp"
#include "shader_program.hpp"
#include "texture.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

void init();
void handleInput(const SDL_Event& event);

int main(int argc, char* argv[]) {
    init();

    ShaderProgram basicShader { "src/shader/basic/basic.vs", "src/shader/basic/basic.fs" };
    if(!basicShader.getBuildSuccess()) {
        std::cout << "Could not compile basic shader!" << std::endl;
        return 1;
    }
    ShaderProgram geometryShader { "src/shader/geometry/geometry.vs", "src/shader/geometry/geometry.fs" };
    if(!geometryShader.getBuildSuccess()) {
        std::cout << "Could not compile basic shader!" << std::endl;
        return 1;
    }

    Texture textureRock {"data/textures/pixel_rock_moss.png", Texture::Type::TextureAlbedo };

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

    std::vector<glm::vec4> screenVertices {
        {-1.f, -1.f, 0.f, 1.f}, // bottom left
            {glm::vec4(1.f)}, // color multiplier
            {0.f, 0.f, 0.f, 0.f}, // bottom left UV
        {1.f, -1.f, 0.f, 1.f}, // bottom right
            {glm::vec4(1.f)},
            {1.f, 0.f, 0.f, 0.f},// bottom right UV
        {1.f, 1.f, 0.f, 1.f}, // top right
            {glm::vec4(1.f)},
            {1.f, 1.f, 0.f, 0.f},
        {-1.f, 1.f, 0.f, 1.f}, // top left
            {glm::vec4(1.f)}, 
            {0.f, 1.f, 0.f, 0.f}
    };
    std::vector<GLuint> screenElements {
        {0}, {1}, {2},
        {0}, {2}, {3}
    };
    std::vector<glm::vec4> pyramidVertices {
        {0.f, .5f, 0.f, 1.f}, // top
            glm::vec4(1.f), // white
            {.5f, 0.f, 0.f, 0.f}, // texture top middle
        {-.25f, -.5f, .25f, 1.f}, // bottom left front
            glm::vec4(1.f),
            {0.f, 0.f, 0.f, 0.f}, // texture top left
        {.25f, -.5f, .25f, 1.f}, // bottom right front
            glm::vec4(1.f),
            {0.f, 1.f, 0.f, 0.f}, // texture bottom left 
        {.25f, -.5f, -.25f, 1.f}, // bottom right back
            glm::vec4(1.f),
            {1.f, 1.f, 0.f, 0.f}, // texture bottom right
        {-.25f, -.5f, -.25f, 1.f}, // bottom left back
            glm::vec4(1.f),
            {1.f, 0.f, 0.f, 0.f} // texture top right
    };
    std::vector<GLuint> pyramidElements {
        {0}, {1}, {2}, // front
        {0}, {2}, {3}, // right
        {0}, {3}, {4}, // back
        {0}, {4}, {1}, // left
        {1}, {3}, {2}, // base triangle 1
        {1}, {4}, {3} // base triangle 2
    };

    //Create 1 model and 1 normal matrix for this pyramid
    std::vector<glm::mat4> pyramidMatrices {
        {
            glm::rotate(
                glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -2.f)),
                glm::radians(45.f),
                glm::vec3(0.f, 1.f, 0.f)
            )
        },
        {
            glm::mat4(1.f)
        }
    };
    pyramidMatrices[1] = glm::transpose(glm::inverse(pyramidMatrices[1]));

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
            screenVertices.size() * sizeof(glm::vec4),
            screenVertices.data(),
            GL_STATIC_DRAW
        );
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            screenElements.size() * sizeof(GLuint),
            screenElements.data(),
            GL_STATIC_DRAW
        );
        basicShader.enableAttribArray(basicShader.getLocationAttribArray("attrPosition"));
        basicShader.enableAttribArray(basicShader.getLocationAttribArray("attrColor"));
        basicShader.enableAttribArray(basicShader.getLocationAttribArray("attrTextureCoordinates"));
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrPosition"),
            4,
            GL_FLOAT,
            GL_FALSE,
            3 * sizeof(glm::vec4),
            reinterpret_cast<void*>(0)
        );
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrColor"),
            4,
            GL_FLOAT,
            GL_FALSE,
            3*sizeof(glm::vec4),
            reinterpret_cast<void*>(sizeof(glm::vec4))
        );
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrTextureCoordinates"),
            2,
            GL_FLOAT,
            GL_FALSE,
            3*sizeof(glm::vec4),
            reinterpret_cast<void*>(2*sizeof(glm::vec4))
        );
    glBindVertexArray(0);

    //Generate a vertex array for use in the gBuffer renderer
    GLuint pyramidVAO;
    GLuint pyramidVertexBuffer;
    GLuint pyramidMatrixBuffer;
    GLuint pyramidElementBuffer;
    glGenBuffers(1, &pyramidVertexBuffer);
    glGenBuffers(1, &pyramidElementBuffer);
    glGenBuffers(1, &pyramidMatrixBuffer);
    glGenVertexArrays(1, &pyramidVAO);
    // Send data to GPU memory and bind related
    // attributes in the shader program
    geometryShader.use();
    glBindVertexArray(pyramidVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pyramidVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidElementBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                pyramidVertices.size() * sizeof(glm::vec4),
                pyramidVertices.data(),
                GL_STATIC_DRAW
            );
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                pyramidElements.size() * sizeof(GLuint),
                pyramidElements.data(),
                GL_STATIC_DRAW
            );
            geometryShader.enableAttribArray("attrPosition");
            glVertexAttribPointer(
                geometryShader.getLocationAttribArray("attrPosition"),
                4,
                GL_FLOAT,
                GL_FALSE,
                3*sizeof(glm::vec4),
                reinterpret_cast<void*>(0)
            );
            geometryShader.enableAttribArray("attrColor");
            glVertexAttribPointer(
                geometryShader.getLocationAttribArray("attrColor"),
                4,
                GL_FLOAT,
                GL_FALSE,
                3*sizeof(glm::vec4),
                reinterpret_cast<void*>(sizeof(glm::vec4))
            );
            geometryShader.enableAttribArray("attrTextureCoordinates");
            glVertexAttribPointer(
                geometryShader.getLocationAttribArray("attrTextureCoordinates"),
                2,
                GL_FLOAT,
                GL_FALSE,
                3*sizeof(glm::vec4),
                reinterpret_cast<void*>(2 * sizeof(glm::vec4))
            );
        //Send instanced matrix data to the GPU
        glBindBuffer(GL_ARRAY_BUFFER, pyramidMatrixBuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                pyramidMatrices.size() * sizeof(glm::mat4),
                pyramidMatrices.data(),
                GL_STATIC_DRAW
            );
            //Enable model and normal matrix arrays, set their pointers
            GLint locationModelMatrix {geometryShader.getLocationAttribArray("attrModelMatrix")};
            GLint locationNormalMatrix {geometryShader.getLocationAttribArray("attrNormalMatrix")};
            for(int i{0}; i < 4; ++i) {
                geometryShader.enableAttribArray(locationModelMatrix+i);
                geometryShader.enableAttribArray(locationNormalMatrix+i);
                glVertexAttribPointer(
                    locationModelMatrix+i,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    2*sizeof(glm::mat4),
                    reinterpret_cast<void*>(0 + i*sizeof(glm::vec4))
                );
                glVertexAttribDivisor(locationModelMatrix+i, 1);
                glVertexAttribPointer(
                    locationNormalMatrix+i,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    2*sizeof(glm::mat4),
                    reinterpret_cast<void*>(sizeof(glm::mat4) + i*sizeof(glm::vec4))
                );
                glVertexAttribDivisor(locationNormalMatrix+i, 1);
            }
    glBindVertexArray(0);
    
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


    //Main event loop
    glClearColor(0.f, 0.f, 0.f, 1.f);
    SDL_Event event;
    GLuint previousTicks { SDL_GetTicks() };
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
        glBindFramebuffer(GL_FRAMEBUFFER, geometryFBO);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureRock.getTextureID());
            geometryShader.use();
            geometryShader.setUBool("uUsingNormalMap", false);
            geometryShader.setUBool("uUsingSpecularMap", false);
            geometryShader.setUBool("uUsingAlbedoMap", true);
            geometryShader.setUInt("uMaterial.textureAlbedo", 0);
            glBindVertexArray(pyramidVAO);
                glDrawElementsInstanced(
                    GL_TRIANGLES,
                    pyramidElements.size(),
                    GL_UNSIGNED_INT,
                    reinterpret_cast<void*>(0),
                    1
                );
            glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
