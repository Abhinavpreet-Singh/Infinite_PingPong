# Ping Pong Game

This is a classic Ping Pong game with several enhancements, built using the C++ and the raylib library. It features multiple difficulty levels, special effects, and a dynamic scoring system.

## Features

*   **Four Difficulty Levels**:
    *   **EASY**: A relaxed pace for beginners.
    *   **MEDIUM**: A balanced challenge for most players.
    *   **HARD**: A tougher opponent with faster reactions.
    *   **IMPOSSIBLE**: A nearly unbeatable AI with increasing speed.
*   **Dynamic Visuals**:
    *   A "comet trail" effect for the ball.
    *   Screen shake on scoring.
    *   A scrolling starfield background.
*   **Sound Effects**: Audio feedback for paddle hits, wall bounces, and scoring.
*   **Widescreen Play Area**: A modern, rectangular court for a cinematic feel.

## Controls

*   **Player Paddle**: Use `W`/`S` keys or the `UP`/`DOWN` arrow keys to move your paddle.
*   **Menu Navigation**: Use `SPACE`, `R`, and the number keys (`1-4`) to navigate the menus and start the game.

## How to Build and Run

This project uses `make` to build. Ensure you have `mingw32-make` (on Windows) or `make` (on Linux/macOS) and a C++ compiler installed.

1.  **Clone the repository**:
    ```sh
    git clone <repository-url>
    cd <repository-directory>
    ```

2.  **Configure raylib path**:
    Open the `Makefile` and ensure the `RAYLIB_PATH` variable points to your raylib installation directory. By default, it's set to `C:/raylib`.

3.  **Build the game**:
    From the project root, run the following command:
    ```sh
    mingw32-make PLATFORM=PLATFORM_DESKTOP PROJECT_NAME=pong_levels OBJS=pong_levels.cpp
    ```

4.  **Run the game**:
    After a successful build, an executable named `pong_levels.exe` (on Windows) will be created. Run it from your terminal:
    ```sh
    ./pong_levels.exe
    ```

## License

This project is licensed under the MIT License - see the `LICENSE.txt` file for details.
