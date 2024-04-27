#ifndef ZOWINDOWCONTEXTMANAGER_H
#define ZOWINDOWCONTEXTMANAGER_H

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <assimp/Importer.hpp>

class WindowContextManager {
    /*
    Initializes SDL and OpenGL contexts, creates an SDL window 
    and stores a reference to it
    */
    WindowContextManager(GLuint windowWidth, GLuint windowHeight);
    /* Uses default copy constructor */
    WindowContextManager(WindowContextManager& other) = default;
    /* Uses default copy assignment */
    WindowContextManager& operator=(WindowContextManager& other) = default;

    SDL_Window* mpSDLWindow;
    SDL_GLContext mpGLContext;
    Assimp::Importer* mpAssetImporter;

    GLuint mWindowWidth;
    GLuint mWindowHeight;

public:
    /* Deletes SDL and GL contexts */
    ~WindowContextManager();

    static WindowContextManager& getInstance(GLuint width=800, GLuint height=600);

    /*Removes default constructor*/
    WindowContextManager() = delete;
    /*Removes move constructor*/
    WindowContextManager(WindowContextManager&& other) = delete;
    /*Removes move assignment operator*/
    WindowContextManager& operator=(const WindowContextManager&& other) = delete;

    /* swap color buffers */
    void swapBuffers();

    /*Accessor for the OpenGL context pointer*/
    SDL_GLContext getGLContext() const;
    /*Accessor for the SDL Window pointer*/
    SDL_Window* getSDLWindow() const;
    /*Accessor for asset importer*/
    Assimp::Importer* getAssetImporter() const;
};

#endif
