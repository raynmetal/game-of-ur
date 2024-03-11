#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "window_context_manager.hpp"
#include "shader_program.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

void init();

int main(int argc, char* argv[]) {
    init();

    ShaderProgram basicShader { "src/shader/basic/basic.vs", "src/shader/basic/basic.fs" };
    if(!basicShader.getBuildSuccess()) {
        std::cout << "Could not compile basic shader!" << std::endl;
        return 1;
    }

    std::vector<glm::vec4> vertices {
        {0.f, .25f, 0.f, 1.f}, // top
            {1.f, 0.f, 0.f, 1.f}, // red
        {-.25f, -.25f, 0.f, 1.f}, // bottom left
            {0.f, 1.f, 0.f, 1.f}, // green
        {.25f, -.25f, 0.f, 1.f}, // bottom right
            {0.f, 0.f, 1.f, 1.f} // blue
    };
    std::vector<GLuint> elements {
        {0}, {1}, {2}
    };

    GLuint triangleVAO;
    GLuint triangleVBO;
    GLuint triangleEBO;
    glGenBuffers(1, &triangleVBO);
    glGenBuffers(1, &triangleEBO);
    glGenVertexArrays(1, &triangleVAO);

    // Send data to GPU memory and bind related
    // attributes in the shader program
    basicShader.use();
    glBindVertexArray(triangleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(glm::vec4),
            vertices.data(),
            GL_STATIC_DRAW
        );
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            elements.size() * sizeof(GLuint),
            elements.data(),
            GL_STATIC_DRAW
        );
        basicShader.enableAttribArray("attrPosition");
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrPosition"),
            4,
            GL_FLOAT,
            GL_FALSE,
            2*sizeof(glm::vec4),
            reinterpret_cast<void*>(0)
        );
        basicShader.enableAttribArray("attrColor");
        glVertexAttribPointer(
            basicShader.getLocationAttribArray("attrColor"),
            4,
            GL_FLOAT,
            GL_FALSE,
            2*sizeof(glm::vec4),
            reinterpret_cast<void*>(sizeof(glm::vec4))
        );
    glBindVertexArray(0);

    glClearColor(.1f, .1f, .2f, 1.f);

    //Main event loop
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
        }
        if(quit) break;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        basicShader.use();
        glBindVertexArray(triangleVAO);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, static_cast<void*>(0));
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
