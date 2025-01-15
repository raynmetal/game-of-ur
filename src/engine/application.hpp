#ifndef ZOAPPLICATION_H
#define ZOAPPLICATION_H

#include <string>

#include "input_system/input_system.hpp"

class Application {
    template <typename TObject>
    class getByPath_Helper;

public:
    ~Application();

    static Application& getInstance();

    void execute();

    template <typename TObject>
    TObject getByPath(const std::string& path);

private:

    Application(const std::string& projectPath);

    template <typename TObject>
    class getByPath_Helper {
        static TObject get(const std::string& path);
    };

    void initialize();
    void cleanup();

    uint16_t mWindowWidth {800};
    uint16_t mWindowHeight {600};
    uint32_t mSimulationStep { 1000/30 }; // simulation stepsize in ms
    InputManager mInputManager {};

    static std::weak_ptr<Application> s_pInstance;
    static bool s_instantiated;

    static std::shared_ptr<Application> instantiate(const std::string& projectPath);
friend int main(int, char* []);
};

#endif
