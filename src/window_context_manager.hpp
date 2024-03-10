#ifndef ZOWINDOWCONTEXTMANAGER_H
#define ZOWINDOWCONTEXTMANAGER_H

#include <SDL2/SDL.h>
#include <GL/glew.h>

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

    SDL_Window* mSDLWindow;
    SDL_GLContext mGLContext;

    GLuint mWindowWidth;
    GLuint mWindowHeight;

public:

    /* Deletes SDL and GL contexts */
    ~WindowContextManager();

    static WindowContextManager& getInstance();

    //Removes default constructor
    WindowContextManager() = delete;
    //Removes move constructor
    WindowContextManager(WindowContextManager&& other) = delete;
    //Removes move assignment operator
    WindowContextManager& operator=(const WindowContextManager&& other) = delete;

    /*Accessor for the OpenGL context pointer*/
    const SDL_GLContext getGLContext() const;
    /*Accessor for the SDL Window pointer*/
    const SDL_Window* getSDLWindow() const;
};

#endif
