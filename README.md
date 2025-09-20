# Game of Ur Project

## Project Description

### Introduction

This is a computer adaptation of [Game of Ur](https://en.wikipedia.org/wiki/Royal_Game_of_Ur), written in C++ mainly using SDL and OpenGL.

Game of Ur is a competitive, two-player board game. The player who moves all 5 of their pieces to the end of the course first, wins the game.  The variant implemented in this adaptation is based on a paper by Irving Finkel.  See the [game design document](docs/game_design_doc.md) for more information.

### Motivation

I spent 2023-2024 studying C++, OpenGL, SDL, and 3D graphics by following the tutorials on [learncpp](https://www.learncpp.com/), [Lazy Foo](https://lazyfoo.net), and [Learn OpenGL](https://learnopengl.com/) among others. With this project, I hope to both cement and demonstrate my newly acquired skills.

I decided against making an original game because I wanted to focus on the technical aspects of game development, and not on game design. Adapting an existing game seemed like a good way to limit the scope of my first project.

## Goals

- [x] Stylized 3D graphics
- [x] Offline multiplayer
- [x] AI opponent
- ~~Music and sound effects~~ (I'm tired)
- ~~Tutorialization~~ (I'm very very tired)
- [x] Playable on Windows
- ~~Playable on Android~~ (Maybe some day)
- [ ] Itch.io release
- ~~Play Store release~~ (Maybe a day long after the other days)
- [ ] Code Documentation

## LICENSE

raynmetal/game-of-ur is distributed under the terms of the [MIT License](LICENSE.txt).

This program makes extensive use of the following libraries:

- [SDL](https://www.libsdl.org/)
- [SDL Image](https://github.com/libsdl-org/SDL_image)
- [SDL TTF](https://github.com/libsdl-org/SDL_ttf)
- [GLEW](https://github.com/nigels-com/glew)
- [Nlohmann JSON](https://json.nlohmann.me/)
- [GLM](https://github.com/g-truc/glm)
- [Assimp](https://github.com/assimp/assimp)
