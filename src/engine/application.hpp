#ifndef FOOLSENGINE_APPLICATION_H
#define FOOLSENGINE_APPLICATION_H

#include <string>

#include "core/resource_database.hpp"
#include "core/ecs_world.hpp"

#include "signals.hpp"
#include "scene_system.hpp"
#include "render_system.hpp"
#include "input_system/input_system.hpp"
#include "window_context_manager.hpp"

namespace ToyMakersEngine {
    class Application {
        template <typename TObject, typename=void>
        class getByPath_Helper;

    public:
        ~Application();

        static Application& getInstance();

        void execute();

        template <typename TObject>
        TObject getObject(const std::string& path="");

        inline SignalTracker& getSignalTracker() { return mSignalTracker; }

    private:

        Application(const std::string& projectPath);

        template <typename TObject, typename Enable>
        class getByPath_Helper {
            getByPath_Helper(Application* application): mApplication { application } {}
            TObject get(const std::string& path="");
            Application* mApplication;
        friend class Application;
        };

        void initialize(const nlohmann::json& windowProperties);
        void cleanup();

        SignalTracker mSignalTracker{};
        uint32_t mSimulationStep { 1000/30 }; // simulation stepsize in ms
        InputManager mInputManager {};

        static std::weak_ptr<Application> s_pInstance;
        static bool s_instantiated;

        static std::shared_ptr<Application> instantiate(const std::string& projectPath);
        std::weak_ptr<SceneSystem> mSceneSystem {};
    friend int ::main(int, char* []);
    };

    template <typename TObject>
    TObject Application::getObject(const std::string& path) {
        return getByPath_Helper<TObject>{this}.get(path);
    }

    template <typename TObject, typename Enable>
    TObject Application::getByPath_Helper<TObject, Enable>::get(const std::string& path) {
        static_assert(false && "No getter for this object type is known.");
        return TObject {}; // prevent compiler warnings about no return type
    }

    template <typename TObject>
    struct Application::getByPath_Helper<
        TObject,
        typename std::enable_if_t<
            SceneNodeCore::getByPath_Helper<TObject>::s_valid
        >
    > {
        getByPath_Helper(Application* application): mApplication {application} {}
        TObject get(const std::string& path) {
            return mApplication->mSceneSystem.lock()->getByPath<TObject>(path);
        }
        Application* mApplication;
    };

    template <>
    inline InputManager& Application::getByPath_Helper<InputManager&, void>::get(const std::string& path){
        assert(path == "" && "Getter for InputManager does not accept any path parameter");
        return mApplication->mInputManager;
    }
}

#endif
