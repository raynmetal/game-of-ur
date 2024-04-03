#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <assimp/Importer.hpp>

#include "window_context_manager.hpp"

WindowContextManager::WindowContextManager(GLuint windowWidth, GLuint windowHeight) :
    mWindowWidth {windowWidth},
    mWindowHeight {windowHeight}
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    mpSDLWindow = SDL_CreateWindow("Game of Ur", 100, 100, mWindowWidth, mWindowHeight, SDL_WINDOW_OPENGL);
    if(mpSDLWindow == nullptr){
        std::cout << "Window could not be created" << std::endl;
    }
    std::cout << "Window successfully created!" << std::endl;

    //Specify that we want a forward compatible OpenGL 4.5 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); // creating a stencil buffer

    mpGLContext = SDL_GL_CreateContext(mpSDLWindow);
    if(mpGLContext == nullptr) {
        std::cout << "OpenGL context could not be initialized" << std::endl;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // Set up viewport
    glViewport(0, 0, 800, 600);

    // Apply color correction, converting SRGB values to linear space values
    // when in the shader context
    glEnable(GL_FRAMEBUFFER_SRGB);

    std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;

    mpAssetImporter = new Assimp::Importer();
}

WindowContextManager::~WindowContextManager() {
    std::cout << "Time to say goodbye" << std::endl;
    SDL_GL_DeleteContext(mpGLContext);
    SDL_Quit();
    delete mpAssetImporter;
}

WindowContextManager& WindowContextManager::getInstance(GLuint width, GLuint height) {
    static WindowContextManager instance { width, height };
    return instance;
}

SDL_GLContext WindowContextManager::getGLContext() const {
    return mpGLContext;
}

SDL_Window* WindowContextManager::getSDLWindow() const {
    return mpSDLWindow;
}

Assimp::Importer* WindowContextManager::getAssetImporter() const {
    return mpAssetImporter;
}
