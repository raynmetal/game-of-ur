#include <iostream>
#include <windows.h>

#include <SDL2/SDL.h>

extern constexpr int gWindowWidth {800};
extern constexpr int gWindowHeight {600};

bool init(SDL_Window*& window);
void close();

int main(int argc, char* argv[]) {
    std::cout << "This program is running" << std::endl;
    SDL_Window* gameWindow;
    bool success { init(gameWindow) };

    if(!success) {
        std::cout << "Failed to create window" << std::endl;
        return -1;
    }
    std::cout << "Window successfully created!" << std::endl;

    // keep the window open for 4 seconds
    Sleep(4000);

    // ... and then die
    close();

    return 0;
}

bool init(SDL_Window*& window) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Game of Ur", 100, 100, gWindowWidth, gWindowHeight, 0);
    if(!window) return false;

    return true;
}

void close() {
    SDL_Quit();
}
