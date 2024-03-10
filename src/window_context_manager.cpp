#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "window_context_manager.hpp"

WindowContextManager::WindowContextManager(GLuint windowWidth, GLuint windowHeight) :
    mWindowWidth {windowWidth},
    mWindowHeight {windowHeight}
{
    SDL_Init(SDL_INIT_VIDEO);

    mSDLWindow = SDL_CreateWindow("Game of Ur", 100, 100, mWindowWidth, mWindowHeight, 0);
    if(mSDLWindow == nullptr){
        std::cout << "Window could not be created" << std::endl;
    }
    std::cout << "Window successfully created!" << std::endl;
    // mGLContext = SDL_GL_CreateContext(mSDLWindow);
}

WindowContextManager::~WindowContextManager() {
    std::cout << "Time to say goodbye" << std::endl;

    // SDL_GL_DeleteContext(mGLContext);
    SDL_Quit();
}

WindowContextManager& WindowContextManager::getInstance() {
    static WindowContextManager instance {800, 600};
    return instance;
}
