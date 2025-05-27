#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/application.hpp"

int main(int argc, char* argv[]) {

    if(std::shared_ptr<ToyMakersEngine::Application> app { ToyMakersEngine::Application::instantiate("data/project.json") }) {
        app->execute();

    } else return 1;

    return 0;
}
