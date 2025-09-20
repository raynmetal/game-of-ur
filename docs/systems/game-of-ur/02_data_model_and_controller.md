# Game of Ur: Data Model and Controller

## What is it?

![Game of Ur architecture diagram.](./Game_of_Ur_Architecture.png "Game of Ur architecture diagram.")

### Data model

The game of ur data model is a collection of classes and data structures responsible for representing the "real" state of a single game of Game of Ur.

### Controller

The ur controller is a ToyMaker::SimObjectAspect that acts as the engine level interface between the presentation layer of the game, and the game itself (i.e., the game's data model).

At the start of a game, it instantiates two player controllers.  Each player controller has methods representing game-actions each player can take.  Since player controllers aren't tied to the presentation layer directly, their methods may even be called by AI, or (theoretically) even a player on another machine.

## Important components

### Scenes and configs

- data/project.json -- The configuration file for the project, locating the first game scene and specifying the game's initial window dimensions.

- data/ur_prototype_cpu_vs.json -- The controllers responsible for a game between 1 human and 1 CPU player.

- data/ur_prototype_local_vs.json -- The controllers responsible for a game between 2 humans on the same computer.

### Classes

- GameOfUrModel -- The class at the center of the data model.  Provides methods for querying and advancing the state of the game.

- UrController -- The aspect class acting as game controller, governing a single instance of the game of ur.  Acts as interface between engine objects and the game data model.

- UrPlayerControls -- An object instantiated by UrController.  The controller creates 2 of these in its lifetime, one corresponding to each player of the game.  Gives access to methods to communicate with UrController, and advance the state of the game.

- UrRecords -- An aspect singleton class which collaborates with the UrController to save and load records of the results of past completed games.

- PlayerLocal -- An aspect representing a single human player on this platform.

- PlayerCPURandom -- An aspect representing a single AI player on this platform, which selects its moves at random.

## Why does it exist?

The game data model represents the state and logic of a game independently of its presentation to the user, its platform, or even the engine.  This makes it simple to adapt the game to different platforms, to add the ability to play the game in multiplayer, or even to just reason about the logic of the game itself.

The game controller provides the interface between the presentation layer of the game and the game data model.  This allows the game to be played even by players who do not require a UI, such as AI or online players.  

It also makes it so that the presentation later is somewhat modular; any interface that results in player controller methods being called, and capable of receiving and displaying game state changes, is a valid presentation layer.  The controller and data model aren't concerned with the source of game actions.
