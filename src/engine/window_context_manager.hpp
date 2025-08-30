#ifndef FOOLSENGINE_WINDOWCONTEXT_H
#define FOOLSENGINE_WINDOWCONTEXT_H

#include <memory>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <assimp/Importer.hpp>

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>

#include "signals.hpp"

namespace ToyMaker {
    class WindowContext {
        SignalTracker mSignalTracker {};
    public:
        /* Deletes SDL and GL contexts */
        ~WindowContext();

        static WindowContext& getInstance();

        /*Removes default constructor*/
        WindowContext() = delete;

        /*Accessor for the OpenGL context pointer*/
        SDL_GLContext getGLContext() const;
        /*Accessor for the SDL Window pointer*/
        SDL_Window* getSDLWindow() const;
        /*Accessor for asset importer*/
        Assimp::Importer* getAssetImporter() const;

        void handleWindowEvent(const SDL_WindowEvent& windowEvent);
        static WindowContext& initialize(const nlohmann::json& initialWindowConfiguration);
        static void clear();
        /* swap color buffers */
        void swapBuffers();

        const std::string& getTitle() const;
        bool isMaximized() const;
        bool isMinimized() const;
        bool isResizable() const;
        bool isHidden() const;
        bool isShown() const;
        bool hasKeyFocus() const;
        bool hasMouseFocus() const;
        bool hasCapturedMouse() const;
        bool isFullscreen() const;
        bool isExclusiveFullscreen() const;
        bool isBorderless() const;
        int getDisplayID() const;
        const glm::ivec2 getPosition() const;
        const glm::uvec2 getDimensions() const;
        const glm::uvec2 getDimensionsMinimum() const;
        const glm::uvec2 getDimensionsMaximum() const;

        void setPosition(const glm::uvec2& position);
        void setDimensions(const glm::uvec2& dimensions);
        void setResizeAllowed(bool allowed);
        void setBorder(bool state);
        void setHidden(bool hide);
        void setTitle(const std::string& newTitle);
        void setDimensionsMinimum(const glm::uvec2& dimensions);
        void setDimensionsMaximum(const glm::uvec2& dimensions);
        void setFullscreen(bool fullscreen, bool exclusive=false);
        void maximize();
        void minimize();
        void restore();

        Signal<> mSigWindowResized { mSignalTracker, "WindowResized" };
        Signal<> mSigWindowMaximized { mSignalTracker, "WindowMaximized" };
        Signal<> mSigWindowMinimized { mSignalTracker, "WindowMinimized" };
        Signal<> mSigWindowMoved { mSignalTracker, "WindowMoved" };
        Signal<> mSigWindowMouseEntered { mSignalTracker, "WindowMouseEntered" };
        Signal<> mSigWindowMouseExited { mSignalTracker, "WindowMouseExited" };
        Signal<> mSigWindowShown { mSignalTracker, "WindowShown" };
        Signal<> mSigWindowHidden { mSignalTracker, "WindowHidden" };
        Signal<> mSigWindowExposed { mSignalTracker, "WindowExposed" };
        Signal<> mSigWindowSizeChanged { mSignalTracker, "WindowSizeChanged" };
        Signal<> mSigWindowRestored { mSignalTracker, "WindowRestored" };
        Signal<> mSigWindowKeyFocusGained { mSignalTracker, "WindowKeyFocusGained" };
        Signal<> mSigWindowKeyFocusLost { mSignalTracker, "WindowKeyFocusLost" };
        Signal<> mSigWindowCloseRequested { mSignalTracker, "WindowClosed" };
        Signal<> mSigWindowKeyFocusOffered { mSignalTracker, "WindowKeyFocusOffered" };
        Signal<> mSigWindowICCProfileChanged { mSignalTracker, "WindowICCProfileChanged" };
        Signal<> mSigWindowDisplayChanged { mSignalTracker, "WindowDisplayChanged" };

    private:
        void refreshWindowProperties();
        /*
        Initializes SDL and OpenGL contexts, creates an SDL window 
        and stores a reference to it
        */
        WindowContext(const nlohmann::json& initialWindowConfiguration);

        // non copyable
        WindowContext(const WindowContext& other) = delete;
        // non moveable
        WindowContext(WindowContext&& other) = delete;
        // non copy-assignable
        WindowContext& operator=(const WindowContext& other) = delete;
        // non move-assignable
        WindowContext& operator=(WindowContext&& other) = delete;

        SDL_Window* mpSDLWindow;
        // TODO: move this out of the window context manager, or move window 
        // management itself into a class with a smaller scope
        SDL_GLContext mpGLContext;
        // TODO: move this out of the window context manager, or move window 
        // management itself into a class with a smaller scope
        Assimp::Importer* mpAssetImporter;

        SDL_WindowFlags mCachedWindowFlags {};
        glm::i16vec2 mCachedWindowPosition {};
        glm::u16vec2 mCachedWindowDimensions {};
        glm::u16vec2 mCachedWindowMinimumDimensions {};
        glm::u16vec2 mCachedWindowMaximumDimensions {};
        int mCachedDisplayID {0};
        std::string mCachedTitle {};
        inline static std::unique_ptr<WindowContext> s_windowContextManager { nullptr };
    };
}

#endif
