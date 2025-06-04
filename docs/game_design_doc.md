# Game of Ur

## About

Game of Ur is the oldest board game known to date.  It is informally named after the ruins the first board of its kind was excavated from, the Royal Cemetary at Ur, an ancient Sumer burial site.  The other, more formal name for the Game of Ur is the "Game of Twenty Squares."

It is thought to be a race game, with some variants possibly using additional rules for complex betting.  Two players, each with 5 (or 7, or 9) playing pieces, attempt to move all their pieces from the start of the board to the end and off the board.  The one to do so first, wins.

The version of the game used as the basis for this document (and prototype) is the one described by Irving Finkel in [Ancient Board Games in Perspective](https://archive.org/details/ancientboardgame0000unse), in the chapter titled *On the Rules for the Royal Game of Ur*.  It is based on a cuneiform manuscript written in ancient Babylon, and appears to describe a special variant of the game played there.

### Modifications

There are a few differences from the rules given here and the ones layed down by Irving Finkel.  Some of these modifications make decisions about rules that were left open to interpretation.  Others attempt to make the game more interesting by giving each player more tactical freedom.

- game pieces need not be launched in any specific order
- all counters in the common pool go the the winner, rather than just those counters won by them during play by moving their pieces
- the Swallow can be moved to any square before a rosette from the very beginning of the game
- two of a player's pieces may not occupy a single rosette square

## Technical notes

### Camera

Fixed camera positioned in 3/4ths view above the board, close to the player's side. (orthogonal or frustum with narrow FOV?).  The camera may move around slightly to focus on action taking place on or around the board.

### Menus and screens

- A main menu at the start of the game, with buttons to play the game and to see credits
- A game selection screen, where the player may choose whether to play against the computer or play against a local player
- The game screen, where the game proper is played, and which the bulk of this document pertains to

### Controls, interactions

The game should be playable entirely by mouse.  Hovering over a game element provides previews, tooltips, and the like.  Clicking on an element performs an action.  For example, clicking on a die causes it to be rolled, and clicking on a game piece once the die (or dice) has been rolled, causes it to move along the track.

If feasible, perhaps some keyboard shortcuts may also be provided.  Space, for example, might end one's turn, while arrow keys change which interaction is selected, with enter performing the action associated with said interactable.

### UI

The following information should be communicated by the UI:

- The player whose turn it presently is
- The no. of counters held by each player
- The no. of counters currently in the pool
- (optional) the total exchange value of each pile of counters
- The rolls produced by the `1`-`4` die and the `yes`-`no` die (and their combined resultant score) in the current turn
- (after a roll) A preview of the result of moving a particular piece, when it is hovered over
- The pieces on the board for each player
- Each player's unlaunched pieces
- The pieces that have reached the end for each player

### Assets

#### Game paraphernalia

3D models of the following objects:

- Game pieces for each player, one set white and the other black (or other similarly contrasting colours).  Each piece may be a 3D model of the corresponding bird's head, on a flat round pebble-like base.
  - Swallow (glossy)
  - Storm-bird (glossy)
  - Raven (glossy)
  - Rooster (glossy)
  - Eagle (matte)
- The Game of Ur board, in its [2nd and 1st millenia BC variants](https://www.metmuseum.org/exhibitions/listings/2014/assyria-to-iberia/blog/posts/twenty-squares) as described in the paper
- [Astragals](https://www.tumblr.com/hellenic-reconstructionism/188676486607/astragaloi-what-are-they-basically-theyre)
  - One smaller, where each side corresponds to a number between `1` and `4`
  - A broader one, with a greater likelyhood of landing on the two broader faces of the bone, each face corresponding either to `yes` or `no`
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

The goal of the game is to move all of ones pieces to the end and off the board per the route prescribed by the rules.  The first player to do so of the 2 playing, wins the game and collects all the counters in the common pool (in addition to the counters already in their possession over the course of play).

The more important goal, perhaps over the course of several games, is to maximize the number of counters won from the opponent if one is winning.  Otherwise, it is to minimize the loss of one's own counters.

### Setup

Each player purchases 25 counters at some mutually agreed upon rate.  At the start of the game, each player places 10 of their counters into a common pool.  The number of counters in the pool represents the prize for victory.

Then, to determine which player goes first, each player rolls both astragals and notes the resulting score.  The player with the higher score goes first.  Here, the yes-no die merely determines whether the roll is read as its primary score, or as the converted one.

See the *Dice roll* sub-heading in the next section for instructions on interpreting a roll.

### The anatomy of a turn

#### Dice roll

Roll the smaller astragal to receive a primary roll between `1` and `4`.

1. If the roll can't be applied to any of the current player's pieces, or otherwise if the player would like to try for a bigger roll, roll the larger astragal for a `yes` or a `no`
    1. If `no`, the player must forfeit their turn
    1. If `yes`, the player must apply the converted roll to a valid piece (or skip their turn if no such piece is present).  The conversion takes place in the manner specified below:
        - `1` &rarr; `5`
        - `2` &rarr; `6`
        - `3` &rarr; `7`
        - `4` &rarr; `10`
1. Otherwise, apply the roll to any valid piece

#### Movement

There are two kinds of moves one can make after the roll, depending on whether the piece one wishes to move is off the board, or on it.  Note that once a piece has reached the end of the route, it is no longer eligible for *any* move.

##### Launch

To launch one of the unlaunched pieces, take note of the launch score and starting house corresponding to the piece in the table below.

| Piece      | Launch roll   | Starting house                |
| :-------   | ------------: | ----------------------------: |
| Swallow    | `2`           | before any rosette            |
| Storm-bird | `5`           | 5<sup>th</sup> house          |
| Raven      | `6`           | 6<sup>th</sup> house          |
| Rooster    | `7`           | 7<sup>th</sup> house          |
| Eagle      | `10`          | 10<sup>th</sup> house         |

If the player succeeds in throwing a launch roll, the piece may be moved to its starting house, taking the opponent's piece there, if any, off the board.

If the starting house is occupied by one of the current player's pieces, another valid move must be played, and if no such move exists, the player's turn must be skipped.

##### Movement on the board, and exchanging tokens

Any piece on the board may be moved up along the route by the quantity represented by the roll.  If a valid move is possible, then it must be made before the start of the next turn.  Otherwise, the current turn must be skipped.

The results of moving a piece depend on the house the piece lands on, and the piece already occupying that house, if one is present.  The effects and restrictions on making a move are as follows:

1. If the target house is unoccupied, the player may successfully move there.

    - If the target house has a rosette, the player must collect counters from the common pool.  The number of counters to collect depends on the piece moved onto the rosette, per the table below:

      | Piece         | No. of counters |
      | :------------ | --------------: |
      | Swallow       | 3               |
      | Storm-bird    | 4               |
      | Raven         | 4               |
      | Rooster       | 4               |
      | Eagle         | 5               |

    - If the piece passes over a rosette before landing on a non-rosette house, the player must pay into the common pool the same number of counters as shown in the table above
    - If the piece does not pass over a rosette before landing on a non-rosette house, no counters are exchanged

1. If the target house is occupied, the results are as follows

    - If the target house is a rosette, the move cannot be made, and another valid move must be found.  If no other move exists, the current turn is skipped
    - If the target house is *not* a rosette, then:
      - If the house has a piece belonging to the player, the move cannot be made, and another valid move must be found.  If no such move exists, the current turn is skipped
      - If the house is occupied by the opponent's piece, the opponent's piece is taken off the board and replaced with the piece moved by the player.  The player must exchange counters in accordance with point 1.

1. A roll of `1 + the number of remaining houses in the route relative to a piece` allows a piece to complete the route.  Once moved in this way, the piece is no longer in play and may not be re-launched.

### Victory

The player who completes the route with all of their pieces first wins the game.  All counters in the common pool go to the victor, and the game ends.  The players may now exchange their counters for money or points.  They may also play another round of the game with counters valued at the same rate, or agree upon a new one.
