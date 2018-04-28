# Combatris - A Tetris clone

![screenshots](screenshots/combatris-demo-1.gif)
![screenshots](screenshots/combatris-demo-2.png)

A modern C++ implementation of Tetris. Combatris has been tested and works under Windows 10 (x64 only),
OSX 10.12/10.13, Raspian GNU/Linux 9 (stretch) and Ubuntu 16.04

Combatris allows up to 9 players to play against each other. Any line clear over 1 will send lines over
to your opponents. Using combo’s and special moves you can send even more lines and secure the victory.

You can also play against your self in the single player campaign.

I have tested the game with four players, three on MacBook Pro and one Raspberry Pi 3 B+ all players where
running over 2.4 Wi-Fi. The game works well but there can be lag up to one second, since heartbeats are sent
every second and status updates every 0.5 second (if something has happened). Heartbeats are suppressed
if other messages has been sent within the heartbeat interval (to keep network congestion down)

Feature | Support
--- | ------
Hold Piece | Yes
Next Pieces shown | 3
Lock Delay with cancel | Yes
Wall kicks | SRS
Infinity limit | 15
Bag randomiser | 7-bag
Top out | Line at 22
Levels | 15
Level advancement | 10 lines
T-Spin recognition | 3-corner with kicks
Back-to-back Recognition | Tetris / T-spin
Platform | PC / OSX / Linux / Raspian

**Status**
- [x] Rotate left / right
- [x] Soft / Hard drop
- [x] BPS Random Generator
- [x] Wall kicks (Super Rotating System)
- [X] Hold piece
- [x] T-Spin (scoring)
- [x] T-Spin mini (scoring)
- [x] B2B T-Spin (scoring)
- [x] B2B Tetris (scoring)
- [x] Combos (scoring)
- [X] Perfect clear (cleared matrix after game started)
- [X] Hard Drop & Soft Drop (scoring)
- [X] Panes for score / next tetrominos / hold piece / level and high score
- [X] Pane showing the last clearing move (B2B / Combo / T-Spin / Single / Double /etc.)
- [X] Joystick support (PS3 joystick and Logitech F310 tested)
- [X] Joystick Mapping to enable support for different brands
- [X] Hot add / remove joystick device
- [X] Tetromino Locked animations
- [X] Level up animations
- [X] Score animation
- [X] Lines Cleared Animation
- [X] Count down before game start animations
- [X] Game Over Screen
- [X] Pause Screen
- [X] Splash Screen
- [X] Battle Mode (P2P based on UDP)
- [ ] Introduce KO in Battle mode
- [ ] Introduce Marathon mode
- [ ] More unit tests
- [ ] Sound

**Improvements**
- [ ] Remove the pane for showing combo/latest move and display combo / last move direct on the frame
- [ ] Add animation for combo/last clearing move (expanding text)
- [ ] Change font to https://www.dafont.com/obelixpro.font
- [ ] Configure which joystick to use (if many available)
- [ ] Move joystick mapping and other settings to a config-file
- [ ] Auto discovery of broadcast address (e.g. 192.168.1.255)
- [ ] Make the network protocol more robust (multicast / ack)
- [ ] Wait for all in State waiting to got to Play before starting the fame

**Keyboard Commands**

Key | Action
--- | ------
N  | New game
P | Pause
F1 | Toggle Single Player / Battle mode
Z | Rotate counter clockwise
Up / X | Rotate clockwise
Left | Move Left
Right | Move Right
Down | Soft Drop
Space | Hard Drop
Shift | Hold piece

**Joystick Commands (PS3 Controller)**

Key | Action
--- | ------
Start  | New game
Select | Pause
Square | Rotate counter clockwise
Circle | Rotate clockwise
Left | Move Left
Right | Move Right
Down | Soft Drop
Triangle | Hold piece
X | Hard Drop

**Joystick Commands (Logitech F310 Gamepad)**

Key | Action
--- | ------
Start  | New game
Select | Pause
X | Rotate counter clockwise
B | Rotate clockwise
Left | Move Left
Right | Move Right
Down | Soft Drop
Y | Hold piece
A | Hard Drop

## Network Considerations

The default port is 11000 and default broadcast IP are 192.168.1.255
Use the environment variables COMBATRIS_BROADCAST_PORT and COMBATRIS_BROADCAST_IP to
change the port and broadcast IP accordingly.

The network protocol is UDP based and uses a sliding window for handling lost and out of order
packages.

## Build Combatris

**Dependencies:**
* C++17 compliant compiler (e.g. clang 6/7, clang-9.0.0, Visual Studio 2017.3, GCC 7.x.x)
* cmake 3.10.0 or later (Windows) or 3.5.0 or later (OSX/Linux)
* git
* SDL2 (x64 only)
* SDL2_ttf (x64 only)

The test suit requires catch - Catch will be automatically downloaded and installed
as part of the build process

**OSX / Linux**

Install the required libraries with apt/brew or any other package manager.

Builds the project, default is a release build. Ninja is used if available:

```bash
make
```

Starts the game:
```bash
make run
```

Runs the test suit:

```bash
make test
```

Run cppcheck (if installed) on the codebase with all checks turned-on:

```bash
make cppcheck
```

**Raspian GNU/Linux**

You need to activate the OpenGL driver otherwise the game will be far to slow.

The code builds cleanly and has been tested with GCC 7.3.0, instructions how
to install the compiler can be found here:

https://solarianprogrammer.com/2017/12/08/raspberry-pi-raspbian-install-gcc-compile-cpp-17-programs/

You need to set these flag before running make:

```bash
export PATH=/usr/local/gcc-7.3.0/bin:$PATH
export CXX=gcc-7.3.0
```

**Ubuntu**

The code builds cleanly and has been tested with Clang 5.0.0.

**64-bit Windows 10**

The code has been tested with Visual Studio Community Edition 15.6.1. Using the following settings:

Download the latest versions of the development libraries at https://www.libsdl.org.

Set the following environment variables (see System Properties/Environment Variables...):

SDL2DIR
SDL2TTFDIR

*Example:*
SDL2DIR C:\SDL2-2.0.5

The PATH environmen variable should include both libraries lib\x64 directories

*Example:*
PATH C:\SDL2-2.0.5\lib\x64;C:\SDL2_ttf-2.0.14\lib\x64

Generate Visual Studio project files with CMakeSetup.exe (Choose Visual Studio 15 2017 Win64 as generator)

## Credits

Some sources has there own license and is not covered by the source codes license.

* Fonts https://www.dafont.com/cabin.font
* Fonts https://www.dafont.com/obelixpro.font
* Wall kick tables from https://harddrop.com/wiki/SRS
* Tetris info from http://tetris.wikia.com and https://tetris.wiki
* Tetrominos from https://commons.wikimedia.org/wiki/File:Tetrominoes_IJLO_STZ_Worlds.svg
* Original FindSDL2_mixer https://github.com/rlsosborne/doom
* DeltaTimer adapted from http://headerphile.com/sdl2/sdl2-part-9-no-more-delays/
* https://github.com/philsquared/Catch
