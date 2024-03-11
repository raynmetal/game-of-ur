#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "window_context_manager.hpp"

WindowContextManager::WindowContextManager(GLuint windowWidth, GLuint windowHeight) :
    mWindowWidth {windowWidth},
    mWindowHeight {windowHeight}
{
    SDL_Init(SDL_INIT_VIDEO);

    mSDLWindow = SDL_CreateWindow("Game of Ur", 100, 100, mWindowWidth, mWindowHeight, SDL_WINDOW_OPENGL);
    if(mSDLWindow == nullptr){
        std::cout << "Window could not be created" << std::endl;
    }
    std::cout << "Window successfully created!" << std::endl;

    //Specify that we want a forward compatible OpenGL 4.5 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); // creating a stencil buffer

    mGLContext = SDL_GL_CreateContext(mSDLWindow);
    if(mGLContext == nullptr) {
        std::cout << "OpenGL context could not be initialized" << std::endl;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    //Set up viewport
    glViewport(0, 0, 800, 600);

    std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
}

WindowContextManager::~WindowContextManager() {
    std::cout << "Time to say goodbye" << std::endl;
    SDL_GL_DeleteContext(mGLContext);
    SDL_Quit();
}

WindowContextManager& WindowContextManager::getInstance(GLuint width, GLuint height) {
    static WindowContextManager instance { width, height };
    return instance;
}

SDL_GLContext WindowContextManager::getGLContext() const {
    return mGLContext;
}

SDL_Window* WindowContextManager::getSDLWindow() const {
    return mSDLWindow;
}
