# Raycaster Project

This is a simple raycasting project built using C and SDL2. The project simulates a 3D environment using raycasting, similar to early 3D games like *Wolfenstein 3D*. The player can move around the map, and the scene is rendered dynamically based on the player's position and field of view.

## How to Run

### Install SDL2

Before you can run the project, you need to install SDL2. If you're on macOS, you can easily install it using **Homebrew**:

```bash
brew install sdl2
```
### Compile the Code

Once SDL2 is installed, compile the project with the following command:

``` bash
clang -o raycaster raycaster.c -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lm
```

This will generate an executable named `raycaster`.

### Run the program

After the compilation, you can run the raycasting program with:

``` bash
./raycaster
```
The game window should open, and you can interact with it using the controls described below.

## Controls
- **W**: Move forward
- **S**: Move backward
- **A**: Rotate left
- **D**: Rotate right
- **Esc**: Exit the program

See it on [action](https://youtu.be/YLeauFlM1KY)
