#include <iostream>
#include <raylib.h>
#include <string>

using namespace std;

// Enhanced implementation of Pong with multiple difficulty levels and better visuals
int main() 
{
    // Initialize window and game variables
    const int screenWidth = 1024;  // Larger board area
    const int screenHeight = 768;  // Larger board area
    
    // Define difficulty levels
    enum DifficultyLevel { EASY, MEDIUM, HARD, IMPOSSIBLE };
    DifficultyLevel currentDifficulty = MEDIUM;  // Default to medium
    
    // Define theme colors
    Color bgColor = BLACK;
    Color accentColor = DARKBLUE;
    Color playerColor = Color{41, 128, 185, 255}; // Bright blue
    Color computerColor = Color{231, 76, 60, 255}; // Bright red
    Color ballColor = Color{255, 255, 255, 255};   // White
    Color lineColor = Color{52, 73, 94, 255};      // Dark blue-gray
      // Initialize player paddle
    float playerPaddleX = 50;
    float playerPaddleY = screenHeight / 2 - 60;  // Centered
    float playerPaddleWidth = 20;
    float playerPaddleHeight = 120;    // Slightly taller paddle
    float playerPaddleSpeed = 10.0f;   // Player speed remains fast// Initialize computer paddle
    float computerPaddleX = screenWidth - 50 - 20;
    float computerPaddleY = screenHeight / 2 - 50;
    float computerPaddleWidth = 20;
    float computerPaddleHeight = 120;  // Slightly taller paddle
    
    // Computer AI properties (will be set based on difficulty)
    float computerPaddleSpeed = 6.5f;
    int computerAccuracy = 80;        // Percentage of time computer reacts
    float computerReactionSpeed = 0.7f; // Multiplier for speed
    int computerDeadZone = 20;        // Pixels of "dead zone" where computer won't react
    
    // Initialize ball
    float ballX = screenWidth / 2;
    float ballY = screenHeight / 2;
    float ballRadius = 15;
    float ballSpeedX = 7.0f;
    float ballSpeedY = 7.0f;
      // Score variables
    int playerScore = 0;
    int computerScore = 0;
      // Function to update computer AI based on difficulty level
    auto updateDifficulty = [&]() {
        switch(currentDifficulty) {
            case EASY:
                computerPaddleSpeed = 5.0f;
                computerAccuracy = 50;        // 50% accuracy
                computerReactionSpeed = 0.5f;
                computerDeadZone = 30;
                break;
            case MEDIUM:
                computerPaddleSpeed = 6.5f;
                computerAccuracy = 70;        // 70% accuracy 
                computerReactionSpeed = 0.7f;
                computerDeadZone = 20;
                break;
            case HARD:
                computerPaddleSpeed = 8.5f;
                computerAccuracy = 85;        // 85% accuracy
                computerReactionSpeed = 0.9f;
                computerDeadZone = 10;
                break;
            case IMPOSSIBLE:
                computerPaddleSpeed = 11.0f;
                computerAccuracy = 100;       // 100% accuracy
                computerReactionSpeed = 1.0f;
                computerDeadZone = 5;
                break;
        }
    };
    
    // Set initial difficulty
    updateDifficulty();
    
    // Create window with initial resolution and title
    InitWindow(screenWidth, screenHeight, "Enhanced Ping Pong");
    SetTargetFPS(60);  // Set target frames-per-second
    
    // Load game textures and assets
    Font gameFont = GetFontDefault();
    
    // Game state
    bool showMenu = true;
    bool gameOver = false;
      // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Handle difficulty change at any time with number keys
        if (IsKeyPressed(KEY_ONE)) {
            currentDifficulty = EASY;
            updateDifficulty();
        } else if (IsKeyPressed(KEY_TWO)) {
            currentDifficulty = MEDIUM;
            updateDifficulty();
        } else if (IsKeyPressed(KEY_THREE)) {
            currentDifficulty = HARD;
            updateDifficulty();
        } else if (IsKeyPressed(KEY_FOUR)) {
            currentDifficulty = IMPOSSIBLE;
            updateDifficulty();
        }
        
        // Toggle menu with Escape
        if (IsKeyPressed(KEY_ESCAPE)) {
            showMenu = !showMenu;
            if (!showMenu && gameOver) {
                // Reset game when exiting menu after game over
                playerScore = 0;
                computerScore = 0;
                gameOver = false;
                ballX = screenWidth / 2;
                ballY = screenHeight / 2;
                ballSpeedX = (GetRandomValue(0, 1) == 0) ? -7.0f : 7.0f;
                ballSpeedY = (GetRandomValue(0, 1) == 0) ? -7.0f : 7.0f;
            }
        }
        
        if (!showMenu && !gameOver) {
            // Update player paddle position based on input
            if (IsKeyDown(KEY_W) && playerPaddleY > 0) 
                playerPaddleY -= playerPaddleSpeed;
                
            if (IsKeyDown(KEY_S) && playerPaddleY + playerPaddleHeight < screenHeight) 
                playerPaddleY += playerPaddleSpeed;
                
            // Alternate controls with arrow keys
            if (IsKeyDown(KEY_UP) && playerPaddleY > 0) 
                playerPaddleY -= playerPaddleSpeed;
                
            if (IsKeyDown(KEY_DOWN) && playerPaddleY + playerPaddleHeight < screenHeight) 
                playerPaddleY += playerPaddleSpeed;
          // Update computer paddle position (based on difficulty)
        if (!showMenu && !gameOver)
        {// Calculate where the ball might go
            float targetY = ballY;
            
            // Computer AI based on current difficulty level
            if (ballSpeedX > 0) // Only move when ball is coming toward computer
            {
                // Calculate ball's predicted Y position when it reaches the paddle
                if (currentDifficulty >= MEDIUM) {
                    float timeToReach = (computerPaddleX - ballX) / ballSpeedX;
                    targetY = ballY + ballSpeedY * timeToReach;
                    
                    // Account for bounces on more difficult levels
                    if (currentDifficulty >= HARD) {
                        while (targetY < 0 || targetY > screenHeight) {
                            if (targetY < 0) targetY = -targetY;
                            if (targetY > screenHeight) targetY = 2 * screenHeight - targetY;
                        }
                    }
                }
                
                float computerPaddleCenter = computerPaddleY + computerPaddleHeight / 2;
                
                // Add reaction delay and occasional mistakes based on difficulty
                if (GetRandomValue(0, 100) < computerAccuracy) 
                {
                    // Move toward the target with reaction speed based on difficulty
                    if (computerPaddleCenter < targetY - computerDeadZone)
                        computerPaddleY += computerPaddleSpeed * computerReactionSpeed;
                    else if (computerPaddleCenter > targetY + computerDeadZone)
                        computerPaddleY -= computerPaddleSpeed * computerReactionSpeed;
                }
            }
            
            // Keep computer paddle within screen bounds
            if (computerPaddleY < 0) 
                computerPaddleY = 0;
                
            if (computerPaddleY + computerPaddleHeight > screenHeight) 
                computerPaddleY = screenHeight - computerPaddleHeight;
                
            // Update ball position
            ballX += ballSpeedX;
            ballY += ballSpeedY;
            
            // Check for collisions with top and bottom walls
            if (ballY - ballRadius <= 0 || ballY + ballRadius >= screenHeight)
            {
                ballSpeedY *= -1;
            }
            
            // Check for collisions with player paddle
            if (ballX - ballRadius <= playerPaddleX + playerPaddleWidth &&
                ballY >= playerPaddleY && ballY <= playerPaddleY + playerPaddleHeight &&
                ballSpeedX < 0)
            {
                ballSpeedX *= -1.05f; // Speed increases slightly
                
                // Change Y speed based on where the ball hits the paddle
                float hitPosition = (ballY - (playerPaddleY + playerPaddleHeight/2)) / (playerPaddleHeight/2);
                ballSpeedY = hitPosition * 7.0f;
            }
            
            // Check for collisions with computer paddle
            if (ballX + ballRadius >= computerPaddleX &&
                ballY >= computerPaddleY && ballY <= computerPaddleY + computerPaddleHeight &&
                ballSpeedX > 0)
            {
                ballSpeedX *= -1.05f; // Speed increases slightly
                
                // Change Y speed based on where the ball hits the paddle
                float hitPosition = (ballY - (computerPaddleY + computerPaddleHeight/2)) / (computerPaddleHeight/2);
                ballSpeedY = hitPosition * 7.0f;
            }
            
            // Check for scoring
            if (ballX < 0)
            {
                // Computer scores
                computerScore++;
                ballX = screenWidth / 2;
                ballY = screenHeight / 2;
                ballSpeedX = 7.0f; // Reset ball speed
                ballSpeedY = (GetRandomValue(0, 1) == 0) ? -7.0f : 7.0f;
                
                if (computerScore >= 10) gameOver = true;
            }
            
            if (ballX > screenWidth)
            {
                // Player scores
                playerScore++;
                ballX = screenWidth / 2;
                ballY = screenHeight / 2;
                ballSpeedX = -7.0f; // Reset ball speed
                ballSpeedY = (GetRandomValue(0, 1) == 0) ? -7.0f : 7.0f;
                
                if (playerScore >= 10) gameOver = true;
            }
            
            // Cap ball speed
            const float MAX_SPEED = 15.0f;
            if (ballSpeedX > MAX_SPEED) ballSpeedX = MAX_SPEED;
            if (ballSpeedX < -MAX_SPEED) ballSpeedX = -MAX_SPEED;
            if (ballSpeedY > MAX_SPEED) ballSpeedY = MAX_SPEED;
            if (ballSpeedY < -MAX_SPEED) ballSpeedY = -MAX_SPEED;
        }
          // Restart the game if R is pressed
        if (gameOver && !showMenu && IsKeyPressed(KEY_R))
        {
            playerScore = 0;
            computerScore = 0;
            gameOver = false;
            ballX = screenWidth / 2;
            ballY = screenHeight / 2;
            ballSpeedX = (GetRandomValue(0, 1) == 0) ? -7.0f : 7.0f;
            ballSpeedY = (GetRandomValue(0, 1) == 0) ? -7.0f : 7.0f;
        }
          // Drawing
        BeginDrawing();
            ClearBackground(bgColor);
            
            // Draw court outline
            DrawRectangleLinesEx((Rectangle){40, 40, screenWidth-80, screenHeight-80}, 2, accentColor);
            
            // Draw center line
            for (int i = 40; i < screenHeight-40; i += 30)
            {
                DrawRectangle(screenWidth / 2 - 2, i, 4, 15, lineColor);
            }
            
            // Draw paddles with rounded corners
            DrawRectangleRounded((Rectangle){playerPaddleX, playerPaddleY, playerPaddleWidth, playerPaddleHeight}, 0.3f, 6, playerColor);
            DrawRectangleRounded((Rectangle){computerPaddleX, computerPaddleY, computerPaddleWidth, computerPaddleHeight}, 0.3f, 6, computerColor);
            
            // Draw ball with a small glow effect
            DrawCircleGradient(ballX, ballY, ballRadius+4, 
                ColorAlpha(ballColor, 0.3f), ColorAlpha(ballColor, 0.0f));
            DrawCircle(ballX, ballY, ballRadius, ballColor);
            
            // Draw scores
            DrawText(TextFormat("%d", playerScore), screenWidth / 4, 50, 60, playerColor);
            DrawText(TextFormat("%d", computerScore), 3 * screenWidth / 4 - 40, 50, 60, computerColor);
            
            // Draw difficulty level indicator
            const char* diffText;
            Color diffColor;
            
            switch(currentDifficulty) {
                case EASY: diffText = "EASY"; diffColor = GREEN; break;
                case MEDIUM: diffText = "MEDIUM"; diffColor = ORANGE; break;
                case HARD: diffText = "HARD"; diffColor = RED; break;
                case IMPOSSIBLE: diffText = "IMPOSSIBLE"; diffColor = PURPLE; break;
                default: diffText = "UNKNOWN"; diffColor = WHITE;
            }
            
            DrawText(diffText, screenWidth / 2 - MeasureText(diffText, 30) / 2, 20, 30, diffColor);
            
            // Draw menu or game over screen
            if (showMenu)
            {
                // Semi-transparent overlay
                DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));
                
                DrawText("PING PONG", screenWidth / 2 - MeasureText("PING PONG", 80) / 2, 100, 80, WHITE);
                
                DrawText("SELECT DIFFICULTY:", screenWidth / 2 - MeasureText("SELECT DIFFICULTY:", 40) / 2, 250, 40, WHITE);
                DrawText("1 - EASY", screenWidth / 2 - MeasureText("1 - EASY", 30) / 2, 310, 30, GREEN);
                DrawText("2 - MEDIUM", screenWidth / 2 - MeasureText("2 - MEDIUM", 30) / 2, 350, 30, ORANGE);
                DrawText("3 - HARD", screenWidth / 2 - MeasureText("3 - HARD", 30) / 2, 390, 30, RED);
                DrawText("4 - IMPOSSIBLE", screenWidth / 2 - MeasureText("4 - IMPOSSIBLE", 30) / 2, 430, 30, PURPLE);
                
                DrawText("PRESS [ESC] TO PLAY", screenWidth / 2 - MeasureText("PRESS [ESC] TO PLAY", 30) / 2, 500, 30, WHITE);
                DrawText("CONTROLS: W/S OR ARROW KEYS", screenWidth / 2 - MeasureText("CONTROLS: W/S OR ARROW KEYS", 20) / 2, 550, 20, LIGHTGRAY);
            }
            else if (gameOver)
            {
                // Semi-transparent overlay
                DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));
                
                DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 60) / 2, screenHeight / 4, 60, RED);
                
                if (playerScore > computerScore) {
                    DrawText("YOU WIN!", screenWidth / 2 - MeasureText("YOU WIN!", 50) / 2, screenHeight / 2 - 25, 50, GREEN);
                } else {
                    DrawText("COMPUTER WINS!", screenWidth / 2 - MeasureText("COMPUTER WINS!", 50) / 2, screenHeight / 2 - 25, 50, RED);
                }
                
                DrawText(TextFormat("FINAL SCORE: %d - %d", playerScore, computerScore), 
                    screenWidth / 2 - MeasureText(TextFormat("FINAL SCORE: %d - %d", playerScore, computerScore), 30) / 2, 
                    screenHeight / 2 + 50, 30, WHITE);
                    
                DrawText("PRESS [R] TO PLAY AGAIN", screenWidth / 2 - MeasureText("PRESS [R] TO PLAY AGAIN", 20) / 2, screenHeight * 3 / 4, 20, WHITE);
                DrawText("PRESS [ESC] FOR MENU", screenWidth / 2 - MeasureText("PRESS [ESC] FOR MENU", 20) / 2, screenHeight * 3 / 4 + 30, 20, LIGHTGRAY);
            }
            else {
                // Draw controls info during gameplay
                DrawText("Change Difficulty: 1-4", 10, screenHeight - 50, 20, LIGHTGRAY);
                DrawText("Controls: W/S or UP/DOWN", 10, screenHeight - 25, 20, LIGHTGRAY);
                DrawText("Menu: ESC", screenWidth - MeasureText("Menu: ESC", 20) - 10, screenHeight - 25, 20, LIGHTGRAY);
            }
              // Draw FPS info
            DrawFPS(10, 10);
            
        EndDrawing();
    }

    CloseWindow();  // Close window and OpenGL context
    
    return 0;
}
            
        EndDrawing();
    }

    CloseWindow();  // Close window and OpenGL context
    
    return 0;
}
