# Game of Ur

## About

Game of Ur is the oldest board game known to date.  It is informally named after the ruins the first board of its kind was excavated from, the Royal Cemetary at Ur, an ancient Sumer burial site.  The other, more formal name for the Game of Ur is the "Game of Twenty Squares."

It is hypothesized to be a race game, with some variants possibly using additional rules for complex betting.  Two players, each with 5 (or 7, or 9) playing pieces, attempt to move all their pieces from the start of the board to the end and off the board.  The one to do so first, wins.

The version of the game used as the basis for this document (and prototype) is the one described by Irving Finkel in [Ancient Board Games in Perspective](https://archive.org/details/ancientboardgame0000unse), in the chapter titled *On the Rules for the Royal Game of Ur*.  It is based on a cuneiform manuscript written in ancient Babylon, and appears to describe a special variant of the game played there.

## Technical notes

### Camera

Fixed camera positioned in 3/4ths view above the board, close to the player's side. (orthogonal or frustum with narrow FOV?)

### UI

The following pieces of information should be communicated by the UI:

- The player whose turn it presently is
- The no. of counters held by each player
- The no. of counters currently in the pool
- The rolls produced by the 1-4 die and the yes-no die (and their combined resultant score) in the current turn
- (after a roll) A preview of the result of moving a particular piece, when it is hovered over
- The pieces on the board for each player
- The pieces that have reached the end for each player
- The pieces off the board for each player

### Assets

#### Game paraphernalia

3D models of the following objects:

- Game pieces for each player, one set white and the other black (or other similarly contrasting colours).  Each piece may be a 3D model of the corresponding bird's head, on a flat round pebble-like base.
  - Swallow (glossy)
  - Storm-bird (glossy)
  - Raven (glossy)
  - Rooster (glossy)
  - Eagle (matte)
- The Game of Ur board, in its 2nd and 1st century BC variants as described in the paper
- [Astragals](https://www.tumblr.com/hellenic-reconstructionism/188676486607/astragaloi-what-are-they-basically-theyre)
  - One smaller, where each side corresponds to a number b/w 1-4
  - A broader one, with a greater likelyhood of landing on the two broader faces of the bone, each face corresponding either to "yes" or "no"
- A simple counter, probably in the shape of a coin, to act as currency for bets

#### Environment

The following assets will make up the environment:

- A table model.  Simply to give the place a sense of location
- A model of a chair
- A cubemap of a desert at night, to show through the windows
- A model of a room, with appropriately textured walls and floors and a door
- A model of a lamp, to act as a plausible source for point lights

## Gameplay

### Goal

The goal of the game is to move all of ones pieces to the end and off the board per the route prescribed by the rules.  The first player to do so of the 2 playing, wins the game and collects all the counters remaining in the common pool.

The more important goal, perhaps over the course of multiple rounds of the game, is to maximize the number of counters won from the opponent if one is winning.  Otherwise, it is to minimize the loss of one's own counters.

### Setup

At the start of the game, each player places 10 of their counters into a common pool.  Then, to determine which player goes first, each player rolls both astragals and notes the resulting score.  The player with the higher score goes first.

### The anatomy of a turn

#### Dice roll

Roll the smaller astragal for a number between 1 and 4.

1. If the roll can't be applied to any piece, or if otherwise you would like to try for a larger number, roll the larger astragal for a "yes" or a "no"
    1. If "no," you must forfeit your turn
    1. If "yes," apply the converted roll to a valid piece.  The conversion takes place in the manner specified below:
        - 1 &rarr; 5
        - 2 &rarr; 6
        - 3 &rarr; 7
        - 4 &rarr; 10
1. Otherwise, apply the roll to any valid piece

#### Movement

There are two kinds of moves one can make after the roll, depending on whether the piece one wishes to move is off the board, or on it.  Note that one cannot relaunch a piece that has already reached the end of the route.

##### Launch

To launch one of the pieces that are off the board (but haven't reached the goal), take note of the launch score and starting house corresponding to the piece in the table below.

| Piece      | Launch roll | Starting house     |
| :-------   | ----------: | -----------------: |
| Swallow    | 2           | before any rosette |
| Storm-bird | 5           | 5th house          |
| Raven      | 6           | 6th house          |
| Rooster    | 7           | 7th house          |
| Eagle      | 10          | 10th house         |

If you succeed in throwing a launch roll, your piece may be moved to its starting house, taking the opponent's piece there, if any, out of play.

If there is no space in the starting house unoccupied by one of your own pieces, another valid move must be played, and if no such move exists, your turn must be skipped.

##### Movement on the board, and exchanging tokens
