#include <iostream>
#include <raylib.h>
#include <string>

using namespace std;

// Structure for the paddles
struct Paddle {
    float x, y;
    float width, height;
    float speed;
    Color color;
};

// Structure for the ball
struct Ball {
    float x, y;
    float speedX, speedY;
    float radius;
    Color color;
};

// Gam                // Cap ball speed
                {
                    const float MAX_SPEED = 15.0f;
                    if (ball.speedX > MAX_SPEED) ball.speedX = MAX_SPEED;
                    if (ball.speedX < -MAX_SPEED) ball.speedX = -MAX_SPEED;
                    if (ball.speedY > MAX_SPEED) ball.speedY = MAX_SPEED;
                    if (ball.speedY < -MAX_SPEED) ball.speedY = -MAX_SPEED;
                }e
enum GameState {
    MENU,
    DIFFICULTY_SELECT,
    GAMEPLAY,
    GAME_OVER
};

// Difficulty levels
enum DifficultyLevel {
    EASY,
    MEDIUM,
    HARD,
    IMPOSSIBLE
};

int main() {
    // Window dimensions
    const int SCREEN_WIDTH = 1024;  // Larger screen
    const int SCREEN_HEIGHT = 768;  // Larger screen
    
    // Initialize game variables
    GameState currentState = MENU;
    DifficultyLevel currentDifficulty = MEDIUM;
    
    // Initialize paddle properties
    Paddle playerPaddle = {
        50,                     // x position
        SCREEN_HEIGHT / 2 - 50, // y position
        20,                     // width
        100,                    // height
        10,                     // speed
        BLUE                    // color
    };
    
    Paddle computerPaddle = {
        SCREEN_WIDTH - 50 - 20, // x position (right side - width)
        SCREEN_HEIGHT / 2 - 50, // y position
        20,                     // width
        100,                    // height
        8,                      // speed (slightly slower than player)
        RED                     // color
    };
    
    // Initialize ball properties
    Ball ball = {
        SCREEN_WIDTH / 2,  // x position (center)
        SCREEN_HEIGHT / 2, // y position (center)
        7,                 // speed x
        7,                 // speed y
        15,                // radius
        WHITE              // color
    };
    
    // Score tracking
    int playerScore = 0;
    int computerScore = 0;
    
    // Background elements
    const int numStars = 50;
    Vector2 stars[numStars];
    for (int i = 0; i < numStars; i++) {
        stars[i].x = GetRandomValue(0, SCREEN_WIDTH);
        stars[i].y = GetRandomValue(0, SCREEN_HEIGHT);
    }
    
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Enhanced Ping Pong Game");
    SetTargetFPS(60);
    
    // Load sounds
    InitAudioDevice();
    Sound paddleHit = LoadSound("resources/paddle_hit.wav");
    Sound wallHit = LoadSound("resources/wall_hit.wav");
    Sound score = LoadSound("resources/score.wav");
      // Create dummy sounds if needed (we'll assume they're loaded successfully)
    // For older versions of raylib that don't have IsSoundReady

    while (!WindowShouldClose()) {
        // Game state machine
        switch (currentState) {            case MENU:
                // Check for space key press to go to difficulty selection
                if (IsKeyPressed(KEY_SPACE)) {
                    currentState = DIFFICULTY_SELECT;
                }
                break;
                
            case DIFFICULTY_SELECT:
                // Handle difficulty selection
                if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {
                    currentDifficulty = EASY;
                    currentState = GAMEPLAY;
                    
                    // Reset game elements and start
                    ball.x = SCREEN_WIDTH / 2;
                    ball.y = SCREEN_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    
                    playerPaddle.y = SCREEN_HEIGHT / 2 - playerPaddle.height / 2;
                    computerPaddle.y = SCREEN_HEIGHT / 2 - computerPaddle.height / 2;
                    
                    // Adjust computer speed based on difficulty
                    if (currentDifficulty == EASY) computerPaddle.speed = 6.0f;
                    else if (currentDifficulty == MEDIUM) computerPaddle.speed = 8.0f;
                    else if (currentDifficulty == HARD) computerPaddle.speed = 10.0f;
                    else if (currentDifficulty == IMPOSSIBLE) computerPaddle.speed = 12.0f;
                    
                    playerScore = 0;
                    computerScore = 0;
                }
                else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
                    currentDifficulty = MEDIUM;
                    currentState = GAMEPLAY;
                    
                    // Reset game elements and start
                    ball.x = SCREEN_WIDTH / 2;
                    ball.y = SCREEN_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    
                    playerPaddle.y = SCREEN_HEIGHT / 2 - playerPaddle.height / 2;
                    computerPaddle.y = SCREEN_HEIGHT / 2 - computerPaddle.height / 2;
                    
                    // Adjust computer speed based on difficulty
                    if (currentDifficulty == EASY) computerPaddle.speed = 6.0f;
                    else if (currentDifficulty == MEDIUM) computerPaddle.speed = 8.0f;
                    else if (currentDifficulty == HARD) computerPaddle.speed = 10.0f;
                    else if (currentDifficulty == IMPOSSIBLE) computerPaddle.speed = 12.0f;
                    
                    playerScore = 0;
                    computerScore = 0;
                }
                else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {
                    currentDifficulty = HARD;
                    currentState = GAMEPLAY;
                    
                    // Reset game elements and start
                    ball.x = SCREEN_WIDTH / 2;
                    ball.y = SCREEN_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    
                    playerPaddle.y = SCREEN_HEIGHT / 2 - playerPaddle.height / 2;
                    computerPaddle.y = SCREEN_HEIGHT / 2 - computerPaddle.height / 2;
                    
                    // Adjust computer speed based on difficulty
                    if (currentDifficulty == EASY) computerPaddle.speed = 6.0f;
                    else if (currentDifficulty == MEDIUM) computerPaddle.speed = 8.0f;
                    else if (currentDifficulty == HARD) computerPaddle.speed = 10.0f;
                    else if (currentDifficulty == IMPOSSIBLE) computerPaddle.speed = 12.0f;
                    
                    playerScore = 0;
                    computerScore = 0;
                }
                else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) {
                    currentDifficulty = IMPOSSIBLE;
                    currentState = GAMEPLAY;
                    
                    // Reset game elements and start
                    ball.x = SCREEN_WIDTH / 2;
                    ball.y = SCREEN_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    
                    playerPaddle.y = SCREEN_HEIGHT / 2 - playerPaddle.height / 2;
                    computerPaddle.y = SCREEN_HEIGHT / 2 - computerPaddle.height / 2;
                    
                    // Adjust computer speed based on difficulty
                    if (currentDifficulty == EASY) computerPaddle.speed = 6.0f;
                    else if (currentDifficulty == MEDIUM) computerPaddle.speed = 8.0f;
                    else if (currentDifficulty == HARD) computerPaddle.speed = 10.0f;
                    else if (currentDifficulty == IMPOSSIBLE) computerPaddle.speed = 12.0f;
                    
                    playerScore = 0;
                    computerScore = 0;
                }
                else if (IsKeyPressed(KEY_BACK)) {
                    currentState = MENU;
                }
                break;
                
            case GAMEPLAY:
                // Player paddle control
                if (IsKeyDown(KEY_W) && playerPaddle.y > 0) {
                    playerPaddle.y -= playerPaddle.speed;
                }
                if (IsKeyDown(KEY_S) && playerPaddle.y + playerPaddle.height < SCREEN_HEIGHT) {
                    playerPaddle.y += playerPaddle.speed;
                }
                
                // Alternative controls (arrow keys)
                if (IsKeyDown(KEY_UP) && playerPaddle.y > 0) {
                    playerPaddle.y -= playerPaddle.speed;
                }
                if (IsKeyDown(KEY_DOWN) && playerPaddle.y + playerPaddle.height < SCREEN_HEIGHT) {
                    playerPaddle.y += playerPaddle.speed;
                }                // Computer AI based on difficulty
                float computerPaddleCenter = computerPaddle.y + computerPaddle.height / 2;
                float ballTrackPosition = ball.y;
                
                // Adjust computer properties based on difficulty
                float aiAccuracy = 0.75f; // Percentage chance of moving correctly
                float aiReactionSpeed = 0.8f; // Speed multiplier
                float aiDeadZone = 20.0f; // Area where paddle won't react
                bool useAdvancedPrediction = false; // Whether to predict bounces
                
                // Set AI behavior based on difficulty
                switch(currentDifficulty) {
                    case EASY:
                        aiAccuracy = 0.5f;
                        aiReactionSpeed = 0.6f;
                        aiDeadZone = 30.0f;
                        useAdvancedPrediction = false;
                        break;
                    case MEDIUM:
                        aiAccuracy = 0.75f;
                        aiReactionSpeed = 0.8f;
                        aiDeadZone = 20.0f;
                        useAdvancedPrediction = true;
                        break;
                    case HARD:
                        aiAccuracy = 0.9f;
                        aiReactionSpeed = 0.9f;
                        aiDeadZone = 10.0f;
                        useAdvancedPrediction = true;
                        break;
                    case IMPOSSIBLE:
                        aiAccuracy = 1.0f;
                        aiReactionSpeed = 1.0f;
                        aiDeadZone = 5.0f;
                        useAdvancedPrediction = true;
                        break;
                }
                
                // Add prediction based on difficulty
                if (ball.speedX > 0) {
                    // Calculate where the ball will be when it reaches the computer's x position
                    float timeToReach = (computerPaddle.x - ball.x) / ball.speedX;
                    ballTrackPosition = ball.y + ball.speedY * timeToReach;
                    
                    // Keep the prediction within screen bounds (only for medium and above)
                    if (useAdvancedPrediction) {
                        while (ballTrackPosition < 0 || ballTrackPosition > SCREEN_HEIGHT) {
                            if (ballTrackPosition < 0) ballTrackPosition = -ballTrackPosition;
                            if (ballTrackPosition > SCREEN_HEIGHT) ballTrackPosition = 2 * SCREEN_HEIGHT - ballTrackPosition;
                        }
                    }
                }
                
                // Add difficulty-based movement
                if (GetRandomValue(0, 100) < aiAccuracy * 100) {  // Chance to react based on accuracy
                    if (computerPaddleCenter < ballTrackPosition - aiDeadZone) {
                        computerPaddle.y += computerPaddle.speed * aiReactionSpeed;
                    } else if (computerPaddleCenter > ballTrackPosition + aiDeadZone) {
                        computerPaddle.y -= computerPaddle.speed * aiReactionSpeed;
                    }
                }
                
                // Keep computer paddle within screen bounds
                if (computerPaddle.y < 0) computerPaddle.y = 0;
                if (computerPaddle.y + computerPaddle.height > SCREEN_HEIGHT) computerPaddle.y = SCREEN_HEIGHT - computerPaddle.height;
                
                // Update ball position
                ball.x += ball.speedX;
                ball.y += ball.speedY;
                
                // Ball collision with top and bottom walls
                if (ball.y - ball.radius <= 0 || ball.y + ball.radius >= SCREEN_HEIGHT) {
                    ball.speedY *= -1;
                    PlaySound(wallHit);
                }
                
                // Ball collision with player paddle
                if (ball.x - ball.radius <= playerPaddle.x + playerPaddle.width &&
                    ball.y >= playerPaddle.y && ball.y <= playerPaddle.y + playerPaddle.height &&
                    ball.speedX < 0) {
                    
                    ball.speedX *= -1.05f;  // Speed increases slightly each hit
                    
                    // Change Y speed based on where the ball hits the paddle
                    float hitPosition = (ball.y - (playerPaddle.y + playerPaddle.height / 2)) / (playerPaddle.height / 2);
                    ball.speedY = ball.speedY * 0.75f + hitPosition * 7;
                    
                    PlaySound(paddleHit);
                }
                
                // Ball collision with computer paddle
                if (ball.x + ball.radius >= computerPaddle.x &&
                    ball.y >= computerPaddle.y && ball.y <= computerPaddle.y + computerPaddle.height &&
                    ball.speedX > 0) {
                    
                    ball.speedX *= -1.05f;  // Speed increases slightly each hit
                    
                    // Change Y speed based on where the ball hits the paddle
                    float hitPosition = (ball.y - (computerPaddle.y + computerPaddle.height / 2)) / (computerPaddle.height / 2);
                    ball.speedY = ball.speedY * 0.75f + hitPosition * 7;
                    
                    PlaySound(paddleHit);
                }
                
                // Score points when ball passes paddles
                if (ball.x < 0) {
                    // Computer scores
                    computerScore++;
                    ball.x = SCREEN_WIDTH / 2;
                    ball.y = SCREEN_HEIGHT / 2;
                    ball.speedX = 7;
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    PlaySound(score);
                    
                    // Check for game over
                    if (computerScore >= 10) {
                        currentState = GAME_OVER;
                    }
                }
                
                if (ball.x > SCREEN_WIDTH) {
                    // Player scores
                    playerScore++;
                    ball.x = SCREEN_WIDTH / 2;
                    ball.y = SCREEN_HEIGHT / 2;
                    ball.speedX = -7;
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    PlaySound(score);
                    
                    // Check for game over
                    if (playerScore >= 10) {
                        currentState = GAME_OVER;
                    }
                }
                
                // Cap the ball speed
                const float MAX_SPEED = 15.0f;
                if (ball.speedX > MAX_SPEED) ball.speedX = MAX_SPEED;
                if (ball.speedX < -MAX_SPEED) ball.speedX = -MAX_SPEED;
                if (ball.speedY > MAX_SPEED) ball.speedY = MAX_SPEED;
                if (ball.speedY < -MAX_SPEED) ball.speedY = -MAX_SPEED;
                
                break;
                  case GAME_OVER:
                // Check for R key to restart with same difficulty
                if (IsKeyPressed(KEY_R)) {
                    currentState = GAMEPLAY;
                    
                    // Reset game elements
                    ball.x = SCREEN_WIDTH / 2;
                    ball.y = SCREEN_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                    
                    playerPaddle.y = SCREEN_HEIGHT / 2 - playerPaddle.height / 2;
                    computerPaddle.y = SCREEN_HEIGHT / 2 - computerPaddle.height / 2;
                    
                    playerScore = 0;
                    computerScore = 0;
                }
                // Space key for difficulty selection
                else if (IsKeyPressed(KEY_SPACE)) {
                    currentState = DIFFICULTY_SELECT;
                }
                break;
        }
        
        // Animation for background stars
        for (int i = 0; i < numStars; i++) {
            stars[i].x -= 0.5f;
            if (stars[i].x < 0) {
                stars[i].x = SCREEN_WIDTH;
                stars[i].y = GetRandomValue(0, SCREEN_HEIGHT);
            }
        }
        
        // Drawing
        BeginDrawing();
            // Clear background
            ClearBackground(BLACK);
            
            // Draw starfield background
            for (int i = 0; i < numStars; i++) {
                DrawCircle(stars[i].x, stars[i].y, 1.5f, GRAY);
            }
            
            // Draw center line
            for (int i = 0; i < SCREEN_HEIGHT; i += 30) {
                DrawRectangle(SCREEN_WIDTH / 2 - 2, i, 4, 15, DARKGRAY);
            }
              switch (currentState) {
                case MENU:
                    // Draw title and instructions
                    DrawText("PING PONG", SCREEN_WIDTH / 2 - MeasureText("PING PONG", 60) / 2, SCREEN_HEIGHT / 4, 60, YELLOW);
                    DrawText("Press SPACE to select difficulty", SCREEN_WIDTH / 2 - MeasureText("Press SPACE to select difficulty", 30) / 2, SCREEN_HEIGHT / 2, 30, WHITE);
                    DrawText("Controls: W/S or UP/DOWN to move paddle", SCREEN_WIDTH / 2 - MeasureText("Controls: W/S or UP/DOWN to move paddle", 20) / 2, SCREEN_HEIGHT * 3 / 4, 20, LIGHTGRAY);
                    break;
                
                case DIFFICULTY_SELECT:
                    // Draw difficulty selection screen
                    DrawText("SELECT DIFFICULTY", SCREEN_WIDTH / 2 - MeasureText("SELECT DIFFICULTY", 60) / 2, SCREEN_HEIGHT / 6, 60, YELLOW);
                    
                    // Draw difficulty options
                    DrawText("1 - EASY", SCREEN_WIDTH / 2 - MeasureText("1 - EASY", 40) / 2, SCREEN_HEIGHT / 2 - 100, 40, GREEN);
                    DrawText("2 - MEDIUM", SCREEN_WIDTH / 2 - MeasureText("2 - MEDIUM", 40) / 2, SCREEN_HEIGHT / 2 - 30, 40, YELLOW);
                    DrawText("3 - HARD", SCREEN_WIDTH / 2 - MeasureText("3 - HARD", 40) / 2, SCREEN_HEIGHT / 2 + 40, 40, ORANGE);
                    DrawText("4 - IMPOSSIBLE", SCREEN_WIDTH / 2 - MeasureText("4 - IMPOSSIBLE", 40) / 2, SCREEN_HEIGHT / 2 + 110, 40, RED);
                    
                    DrawText("Press number to select difficulty and start", SCREEN_WIDTH / 2 - MeasureText("Press number to select difficulty and start", 20) / 2, SCREEN_HEIGHT * 3 / 4 + 30, 20, WHITE);
                    DrawText("Press BACKSPACE to return to main menu", SCREEN_WIDTH / 2 - MeasureText("Press BACKSPACE to return to main menu", 20) / 2, SCREEN_HEIGHT * 3 / 4 + 60, 20, LIGHTGRAY);
                    break;                case GAMEPLAY:
                {
                    // Draw game elements
                    Rectangle playerRect = { playerPaddle.x, playerPaddle.y, playerPaddle.width, playerPaddle.height };
                    Rectangle computerRect = { computerPaddle.x, computerPaddle.y, computerPaddle.width, computerPaddle.height };
                    DrawRectangleRec(playerRect, playerPaddle.color);
                    DrawRectangleRec(computerRect, computerPaddle.color);
                    
                    // Draw glow effect for ball
                    DrawCircleGradient(ball.x, ball.y, ball.radius+4, ColorAlpha(WHITE, 0.3f), ColorAlpha(WHITE, 0.0f));
                    DrawCircle(ball.x, ball.y, ball.radius, ball.color);
                    
                    // Draw scores
                    DrawText(TextFormat("%d", playerScore), SCREEN_WIDTH / 4, 50, 50, BLUE);                    DrawText(TextFormat("%d", computerScore), 3 * SCREEN_WIDTH / 4 - 20, 50, 50, RED);
                    
                    // Draw difficulty level
                    {
                    const char* difficultyText;
                    Color difficultyColor;
                    
                    switch(currentDifficulty) {
                        case EASY:
                            difficultyText = "EASY";
                            difficultyColor = GREEN;
                            break;
                        case MEDIUM:
                            difficultyText = "MEDIUM";
                            difficultyColor = YELLOW;
                            break;
                        case HARD:
                            difficultyText = "HARD";
                            difficultyColor = ORANGE;
                            break;
                        case IMPOSSIBLE:
                            difficultyText = "IMPOSSIBLE";
                            difficultyColor = RED;
                            break;
                    }
                      DrawText(difficultyText, SCREEN_WIDTH / 2 - MeasureText(difficultyText, 30) / 2, 20, 30, difficultyColor);
                    }
                }
                    break;                case GAME_OVER:
                {
                    // Draw game over screen
                    DrawText("GAME OVER", SCREEN_WIDTH / 2 - MeasureText("GAME OVER", 60) / 2, SCREEN_HEIGHT / 4, 60, RED);
                    
                    if (playerScore > computerScore) {
                        DrawText("YOU WIN!", SCREEN_WIDTH / 2 - MeasureText("YOU WIN!", 40) / 2, SCREEN_HEIGHT / 2 - 20, 40, GREEN);
                    } else {
                        DrawText("COMPUTER WINS!", SCREEN_WIDTH / 2 - MeasureText("COMPUTER WINS!", 40) / 2, SCREEN_HEIGHT / 2 - 20, 40, RED);
                    }
                    
                    // Show difficulty level that was played
                    const char* gameOverDiffText;
                    Color gameOverDiffColor;
                      switch(currentDifficulty) {
                        case EASY:
                            gameOverDiffText = "EASY";
                            gameOverDiffColor = GREEN;
                            break;
                        case MEDIUM:
                            gameOverDiffText = "MEDIUM";
                            gameOverDiffColor = YELLOW;
                            break;
                        case HARD:
                            gameOverDiffText = "HARD";
                            gameOverDiffColor = ORANGE;
                            break;
                        case IMPOSSIBLE:
                            gameOverDiffText = "IMPOSSIBLE";
                            gameOverDiffColor = RED;
                            break;
                    }
                    
                    DrawText(TextFormat("Difficulty: %s", gameOverDiffText),                        SCREEN_WIDTH / 2 - MeasureText(TextFormat("Difficulty: %s", gameOverDiffText), 20) / 2, 
                        SCREEN_HEIGHT / 2 + 20, 20, gameOverDiffColor);
                    
                    DrawText(TextFormat("Final Score: %d - %d", playerScore, computerScore), 
                        SCREEN_WIDTH / 2 - MeasureText(TextFormat("Final Score: %d - %d", playerScore, computerScore), 30) / 2, 
                        SCREEN_HEIGHT / 2 + 50, 30, WHITE);
                        
                    DrawText("Press R to play again with same difficulty", 
                        SCREEN_WIDTH / 2 - MeasureText("Press R to play again with same difficulty", 20) / 2, 
                        SCREEN_HEIGHT * 3 / 4, 20, WHITE);
                          DrawText("Press SPACE for difficulty selection", 
                        SCREEN_WIDTH / 2 - MeasureText("Press SPACE for difficulty selection", 20) / 2, 
                        SCREEN_HEIGHT * 3 / 4 + 30, 20, LIGHTGRAY);
                }
                    break;
            }
            
            // Draw FPS counter
            DrawFPS(10, 10);
        EndDrawing();
    }
    
    // Clean up resources
    UnloadSound(paddleHit);
    UnloadSound(wallHit);
    UnloadSound(score);
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}