#include <iostream>
#include <windows.h>

#include <SDL2/SDL.h>

#include "window_context_manager.hpp"

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

int main(int argc, char* argv[]) {
    std::cout << "This program is running" << std::endl;

    WindowContextManager::getInstance();

    // keep the window open for 4 seconds
    Sleep(4000);

    // ... and then die
    return 0;
}
