#include <iostream>
#include <raylib.h>
#include <string>

using namespace std;

// Window dimensions
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

// Constants for the court area - using the entire screen for gameplay (no border)
const int COURT_X = 0;
const int COURT_Y = 0;
const int COURT_WIDTH = SCREEN_WIDTH;    // Full screen width
const int COURT_HEIGHT = SCREEN_HEIGHT;  // Full screen height

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
    float impossibleSpeedMultiplier; // Speed multiplier for IMPOSSIBLE mode
    int hitCounter;                 // Track consecutive hits for IMPOSSIBLE mode
};

// Game state
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
    // Initialize game variables
    GameState currentState = MENU;
    DifficultyLevel currentDifficulty = MEDIUM;
    // Initialize paddle properties within court boundaries    Paddle playerPaddle = {
        COURT_X + 20,           // x position (inside court + margin)
        SCREEN_HEIGHT / 2 - 60, // y position
        20,                     // width
        120,                    // height
        10,                     // speed
        WHITE                   // color (changed from BLUE)
    };
    
    Paddle computerPaddle = {
        COURT_X + COURT_WIDTH - 40, // x position (right edge of court - paddle width - margin)
        SCREEN_HEIGHT / 2 - 60,     // y position
        20,                          // width
        120,                         // height
        8,                           // speed (will adjust based on difficulty)
        RED                          // color
    };      // Initialize ball properties - centered within the court
    Ball ball = {
        COURT_X + COURT_WIDTH / 2,  // x position (center of court)
        COURT_Y + COURT_HEIGHT / 2, // y position (center of court)
        7,                 // speed x
        7,                 // speed y
        15,                // radius
        WHITE,             // color
        1.0f,              // impossibleSpeedMultiplier (starts at 1.0)
        0                  // hitCounter
    };
    
    // Score tracking
    int playerScore = 0;
    int computerScore = 0;
    
    // Background elements
    const int numStars = 80;
    Vector2 stars[80];
    for (int i = 0; i < numStars; i++) {
        stars[i].x = GetRandomValue(0, SCREEN_WIDTH);
        stars[i].y = GetRandomValue(0, SCREEN_HEIGHT);
    }
    
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Enhanced Ping Pong Game");
    SetTargetFPS(60);
      // Initialize audio
    InitAudioDevice();
    
    // Initialize sounds with default empty values
    Sound paddleHit = { 0 };
    Sound wallHit = { 0 };
    Sound score = { 0 };
    
    // Try to load sounds if they exist, otherwise sounds will be silent
    if (FileExists("resources/paddle_hit.wav")) paddleHit = LoadSound("resources/paddle_hit.wav");
    if (FileExists("resources/wall_hit.wav")) wallHit = LoadSound("resources/wall_hit.wav"); 
    if (FileExists("resources/score.wav")) score = LoadSound("resources/score.wav");

    // Main game loop
    while (!WindowShouldClose()) {
        switch (currentState) {
            case MENU:
                // Check for space key press to go to difficulty selection
                if (IsKeyPressed(KEY_SPACE)) {
                    currentState = DIFFICULTY_SELECT;
                }
                break;
                
            case DIFFICULTY_SELECT:
                // Handle difficulty selection
                if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {                    currentDifficulty = EASY;
                    computerPaddle.speed = 5.0f;
                    currentState = GAMEPLAY;
                      // Reset game elements - ball positioned in center of court with slower speed for EASY
                    ball.x = COURT_X + COURT_WIDTH / 2;
                    ball.y = COURT_Y + COURT_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -5 : 5;  // Slower starting speed for EASY
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -5 : 5;  // Slower starting speed for EASY
                    ball.hitCounter = 0;
                    ball.impossibleSpeedMultiplier = 1.0f;
                    playerScore = 0;
                    computerScore = 0;
                }                else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {                    currentDifficulty = MEDIUM;
                    computerPaddle.speed = 7.0f;
                    currentState = GAMEPLAY;
                    
                    // Reset game elements
                    ball.x = COURT_X + COURT_WIDTH / 2;
                    ball.y = COURT_Y + COURT_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -6 : 6;  // Medium speed
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -6 : 6;  // Medium speed
                    ball.hitCounter = 0;
                    ball.impossibleSpeedMultiplier = 1.0f;
                    playerScore = 0;
                    computerScore = 0;
                }                else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {                    currentDifficulty = HARD;
                    computerPaddle.speed = 9.0f;
                    currentState = GAMEPLAY;
                    
                    // Reset game elements
                    ball.x = COURT_X + COURT_WIDTH / 2;
                    ball.y = COURT_Y + COURT_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -7 : 7;  // Fast speed
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;  // Fast speed
                    ball.hitCounter = 0;
                    ball.impossibleSpeedMultiplier = 1.0f;
                    playerScore = 0;
                    computerScore = 0;
                }                else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) {                    currentDifficulty = IMPOSSIBLE;
                    computerPaddle.speed = 11.0f;
                    currentState = GAMEPLAY;
                    
                    // Reset game elements
                    ball.x = COURT_X + COURT_WIDTH / 2;
                    ball.y = COURT_Y + COURT_HEIGHT / 2;
                    ball.speedX = (GetRandomValue(0, 1) == 0) ? -8 : 8;  // Very fast initial speed
                    ball.speedY = (GetRandomValue(0, 1) == 0) ? -8 : 8;  // Very fast initial speed
                    ball.hitCounter = 0;
                    ball.impossibleSpeedMultiplier = 1.0f;
                    playerScore = 0;
                    computerScore = 0;
                }
                else if (IsKeyPressed(KEY_BACKSPACE)) {
                    currentState = MENU;
                }
                break;
                
            case GAMEPLAY:                // Player paddle control - constrained to court boundaries
                if (IsKeyDown(KEY_W) && playerPaddle.y > COURT_Y) {
                    playerPaddle.y -= playerPaddle.speed;
                }
                if (IsKeyDown(KEY_S) && playerPaddle.y + playerPaddle.height < COURT_Y + COURT_HEIGHT) {
                    playerPaddle.y += playerPaddle.speed;
                }
                
                // Alternative controls (arrow keys) - constrained to court boundaries
                if (IsKeyDown(KEY_UP) && playerPaddle.y > COURT_Y) {
                    playerPaddle.y -= playerPaddle.speed;
                }
                if (IsKeyDown(KEY_DOWN) && playerPaddle.y + playerPaddle.height < COURT_Y + COURT_HEIGHT) {
                    playerPaddle.y += playerPaddle.speed;
                }
                
                // Computer AI based on difficulty
                {
                    float computerPaddleCenter = computerPaddle.y + computerPaddle.height / 2;
                    float ballTrackPosition = ball.y;
                    
                    // Adjust computer properties based on difficulty
                    float aiAccuracy; // Percentage chance of moving correctly
                    float aiReactionSpeed; // Speed multiplier
                    float aiDeadZone; // Area where paddle won't react
                    bool useAdvancedPrediction;
                    
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
                          // Keep the prediction within court bounds
                        if (useAdvancedPrediction) {
                            // Account for ball radius when calculating bounce
                            while (ballTrackPosition - ball.radius < COURT_Y || ballTrackPosition + ball.radius > COURT_Y + COURT_HEIGHT) {
                                if (ballTrackPosition - ball.radius < COURT_Y) 
                                    ballTrackPosition = 2 * (COURT_Y + ball.radius) - ballTrackPosition;
                                if (ballTrackPosition + ball.radius > COURT_Y + COURT_HEIGHT) 
                                    ballTrackPosition = 2 * (COURT_Y + COURT_HEIGHT - ball.radius) - ballTrackPosition;
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
                    }                    // Keep computer paddle within court bounds
                    if (computerPaddle.y < COURT_Y) computerPaddle.y = COURT_Y;
                    if (computerPaddle.y + computerPaddle.height > COURT_Y + COURT_HEIGHT) {
                        computerPaddle.y = COURT_Y + COURT_HEIGHT - computerPaddle.height;
                    }
                    
                    // Update ball position
                    ball.x += ball.speedX;
                    ball.y += ball.speedY;
                      // Ball collision with top and bottom court boundaries
                    if (ball.y - ball.radius <= COURT_Y || ball.y + ball.radius >= COURT_Y + COURT_HEIGHT) {
                        ball.speedY *= -1;
                        // Keep ball within court after collision
                        if (ball.y - ball.radius < COURT_Y) {
                            ball.y = COURT_Y + ball.radius;
                        }
                        if (ball.y + ball.radius > COURT_Y + COURT_HEIGHT) {
                            ball.y = COURT_Y + COURT_HEIGHT - ball.radius;
                        }
                        if (wallHit.frameCount > 0) PlaySound(wallHit);
                    }                        // Ball collision with player paddle
                    if (ball.x - ball.radius <= playerPaddle.x + playerPaddle.width &&
                        ball.y >= playerPaddle.y && ball.y <= playerPaddle.y + playerPaddle.height &&
                        ball.speedX < 0) {
                        
                        // Track consecutive hits for IMPOSSIBLE difficulty
                        ball.hitCounter++;
                        
                        // Update speed multiplier in IMPOSSIBLE mode
                        if (currentDifficulty == IMPOSSIBLE && ball.hitCounter > 3) {
                            ball.impossibleSpeedMultiplier += 0.08f;
                            if (ball.impossibleSpeedMultiplier > 2.5f) ball.impossibleSpeedMultiplier = 2.5f; // Higher cap for more challenge
                        }
                        
                        // Speed increases with each hit, adjusted per difficulty level
                        float speedIncreaseFactor;
                        switch (currentDifficulty) {
                            case EASY:
                                speedIncreaseFactor = -1.01f; // Very slight increase
                                break;
                            case MEDIUM:
                                speedIncreaseFactor = -1.03f; // Gentle increase
                                break;
                            case HARD:
                                speedIncreaseFactor = -1.05f; // Moderate increase
                                break;
                            case IMPOSSIBLE:
                                speedIncreaseFactor = -1.08f * ball.impossibleSpeedMultiplier; // Aggressive increase
                                break;
                            default:
                                speedIncreaseFactor = -1.03f;
                        }
                        ball.speedX *= speedIncreaseFactor;
                        
                        // Change Y speed based on where the ball hits the paddle
                        float hitPosition = (ball.y - (playerPaddle.y + playerPaddle.height / 2)) / (playerPaddle.height / 2);
                        ball.speedY = ball.speedY * 0.75f + hitPosition * 7;
                        
                        if (paddleHit.frameCount > 0) PlaySound(paddleHit);
                    }                      // Ball collision with computer paddle
                    if (ball.x + ball.radius >= computerPaddle.x &&
                        ball.y >= computerPaddle.y && ball.y <= computerPaddle.y + computerPaddle.height &&
                        ball.speedX > 0) {
                        
                        // Track consecutive hits for IMPOSSIBLE difficulty
                        ball.hitCounter++;
                        
                        // Update speed multiplier in IMPOSSIBLE mode
                        if (currentDifficulty == IMPOSSIBLE && ball.hitCounter > 3) {
                            ball.impossibleSpeedMultiplier += 0.08f;
                            if (ball.impossibleSpeedMultiplier > 2.5f) ball.impossibleSpeedMultiplier = 2.5f; // Higher cap for more challenge
                        }
                        
                        // Speed increases with each hit, adjusted per difficulty level
                        float speedIncreaseFactor;
                        switch (currentDifficulty) {
                            case EASY:
                                speedIncreaseFactor = -1.01f; // Very slight increase
                                break;
                            case MEDIUM:
                                speedIncreaseFactor = -1.03f; // Gentle increase
                                break;
                            case HARD:
                                speedIncreaseFactor = -1.05f; // Moderate increase
                                break;
                            case IMPOSSIBLE:
                                speedIncreaseFactor = -1.08f * ball.impossibleSpeedMultiplier; // Aggressive increase
                                break;
                            default:
                                speedIncreaseFactor = -1.03f;
                        }
                        ball.speedX *= speedIncreaseFactor;
                        
                        // Change Y speed based on where the ball hits the paddle
                        float hitPosition = (ball.y - (computerPaddle.y + computerPaddle.height / 2)) / (computerPaddle.height / 2);
                        ball.speedY = ball.speedY * 0.75f + hitPosition * 7;
                        
                        if (paddleHit.frameCount > 0) PlaySound(paddleHit);
                    }                      // Score points when ball passes paddles (using court boundaries)
                    if (ball.x - ball.radius < COURT_X) {                        // Computer scores
                        computerScore++;
                        ball.x = COURT_X + COURT_WIDTH / 2;
                        ball.y = COURT_Y + COURT_HEIGHT / 2;
                        
                        // Set ball speed based on difficulty
                        switch(currentDifficulty) {
                            case EASY:
                                ball.speedX = 5;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -5 : 5;
                                break;
                            case MEDIUM:
                                ball.speedX = 6;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -6 : 6;
                                break;
                            case HARD:
                                ball.speedX = 7;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                                break;
                            case IMPOSSIBLE:
                                ball.speedX = 8;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -8 : 8;
                                break;
                        }
                        
                        // Reset IMPOSSIBLE mode enhancements on score
                        ball.hitCounter = 0;
                        ball.impossibleSpeedMultiplier = 1.0f;
                        
                        if (score.frameCount > 0) PlaySound(score);
                        
                        // Check for game over
                        if (computerScore >= 10) {
                            currentState = GAME_OVER;
                        }
                    }
                      if (ball.x + ball.radius > COURT_X + COURT_WIDTH) {                        // Player scores
                        playerScore++;
                        ball.x = COURT_X + COURT_WIDTH / 2;
                        ball.y = COURT_Y + COURT_HEIGHT / 2;
                        
                        // Set ball speed based on difficulty (negative X direction)
                        switch(currentDifficulty) {
                            case EASY:
                                ball.speedX = -5;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -5 : 5;
                                break;
                            case MEDIUM:
                                ball.speedX = -6;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -6 : 6;
                                break;
                            case HARD:
                                ball.speedX = -7;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                                break;
                            case IMPOSSIBLE:
                                ball.speedX = -8;
                                ball.speedY = (GetRandomValue(0, 1) == 0) ? -8 : 8;
                                break;
                        }
                        
                        // Reset IMPOSSIBLE mode enhancements on score
                        ball.hitCounter = 0;
                        ball.impossibleSpeedMultiplier = 1.0f;
                        
                        if (score.frameCount > 0) PlaySound(score);
                        
                        // Check for game over
                        if (playerScore >= 10) {
                            currentState = GAME_OVER;
                        }
                    }                    // Cap ball speed - different caps for different difficulty levels
                    float MAX_SPEED;
                    switch (currentDifficulty) {
                        case EASY:
                            MAX_SPEED = 10.0f;  // Lower cap for easy mode
                            break;
                        case MEDIUM:
                            MAX_SPEED = 15.0f;  // Moderate cap for medium mode
                            break;
                        case HARD:
                            MAX_SPEED = 20.0f;  // Higher cap for hard mode
                            break;
                        case IMPOSSIBLE:
                            MAX_SPEED = 30.0f;  // No real cap for impossible mode
                            break;
                        default:
                            MAX_SPEED = 15.0f;
                    }
                    
                    if (ball.speedX > MAX_SPEED) ball.speedX = MAX_SPEED;
                    if (ball.speedX < -MAX_SPEED) ball.speedX = -MAX_SPEED;
                    if (ball.speedY > MAX_SPEED) ball.speedY = MAX_SPEED;
                    if (ball.speedY < -MAX_SPEED) ball.speedY = -MAX_SPEED;
                }
                break;
                
            case GAME_OVER:                // Check for R key to restart with same difficulty
                if (IsKeyPressed(KEY_R)) {
                    currentState = GAMEPLAY;                    // Reset game elements
                    ball.x = COURT_X + COURT_WIDTH / 2;
                    ball.y = COURT_Y + COURT_HEIGHT / 2;
                    
                    // Set appropriate speed based on difficulty level
                    switch(currentDifficulty) {
                        case EASY:
                            ball.speedX = (GetRandomValue(0, 1) == 0) ? -5 : 5;
                            ball.speedY = (GetRandomValue(0, 1) == 0) ? -5 : 5;
                            break;
                        case MEDIUM:
                            ball.speedX = (GetRandomValue(0, 1) == 0) ? -6 : 6;
                            ball.speedY = (GetRandomValue(0, 1) == 0) ? -6 : 6;
                            break;
                        case HARD:
                            ball.speedX = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                            ball.speedY = (GetRandomValue(0, 1) == 0) ? -7 : 7;
                            break;
                        case IMPOSSIBLE:
                            ball.speedX = (GetRandomValue(0, 1) == 0) ? -8 : 8;
                            ball.speedY = (GetRandomValue(0, 1) == 0) ? -8 : 8;
                            break;
                    }
                    ball.hitCounter = 0;
                    ball.impossibleSpeedMultiplier = 1.0f;
                    
                    playerScore = 0;
                    computerScore = 0;
                }
                // SPACE for difficulty selection
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
        }          // Drawing
        BeginDrawing();
            // Clear background with absolute black to remove any border
            ClearBackground(BLACK);
            
            // Force any possible border to be black
            DrawRectangleLinesEx((Rectangle){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, 5, BLACK);
            
            // Draw starfield background
            for (int i = 0; i < numStars; i++) {
                DrawCircle(stars[i].x, stars[i].y, 1.5f, GRAY);
            }// Draw center line (within court boundaries)
            float centerX = COURT_X + COURT_WIDTH / 2;
            for (int i = COURT_Y + 10; i < COURT_Y + COURT_HEIGHT - 10; i += 30) {
                DrawRectangle(centerX - 2, i, 4, 15, DARKGRAY);
            }
            
            switch (currentState) {
                case MENU:
                    // Draw title and instructions
                    DrawText("PING PONG", SCREEN_WIDTH / 2 - MeasureText("PING PONG", 80) / 2, SCREEN_HEIGHT / 4, 80, YELLOW);
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
                    break;
                    
                case GAMEPLAY:
                {
                    // Draw game elements
                    DrawRectangleRounded((Rectangle){playerPaddle.x, playerPaddle.y, playerPaddle.width, playerPaddle.height}, 0.3f, 6, playerPaddle.color);
                    DrawRectangleRounded((Rectangle){computerPaddle.x, computerPaddle.y, computerPaddle.width, computerPaddle.height}, 0.3f, 6, computerPaddle.color);
                    
                    // Draw ball with glow effect
                    DrawCircleGradient(ball.x, ball.y, ball.radius+4, ColorAlpha(WHITE, 0.3f), ColorAlpha(WHITE, 0.0f));                    DrawCircle(ball.x, ball.y, ball.radius, ball.color);                    // Draw scores (centered in each half of the court)
                    DrawText(TextFormat("%d", playerScore), COURT_X + COURT_WIDTH/4 - 15, COURT_Y + 30, 60, WHITE);
                    DrawText(TextFormat("%d", computerScore), COURT_X + COURT_WIDTH*3/4 - 15, COURT_Y + 30, 60, RED);
                    
                    // Draw difficulty level
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
                    
                    DrawText(difficultyText, SCREEN_WIDTH / 2 - MeasureText(difficultyText, 30) / 2, 10, 30, difficultyColor);
                    
                    // Display speed multiplier in IMPOSSIBLE mode
                    if (currentDifficulty == IMPOSSIBLE && ball.hitCounter > 3) {
                        char speedText[50];
                        sprintf(speedText, "SPEED: %.1fX", ball.impossibleSpeedMultiplier);
                        DrawText(speedText, SCREEN_WIDTH / 2 - MeasureText(speedText, 20) / 2, COURT_Y + COURT_HEIGHT - 25, 20, RED);
                    }
                }
                    break;
                    
                case GAME_OVER:
                {
                    // Draw game over screen
                    DrawText("GAME OVER", SCREEN_WIDTH / 2 - MeasureText("GAME OVER", 60) / 2, SCREEN_HEIGHT / 4, 60, RED);
                    
                    if (playerScore > computerScore) {
                        DrawText("YOU WIN!", SCREEN_WIDTH / 2 - MeasureText("YOU WIN!", 50) / 2, SCREEN_HEIGHT / 2 - 20, 50, GREEN);
                    } else {
                        DrawText("COMPUTER WINS!", SCREEN_WIDTH / 2 - MeasureText("COMPUTER WINS!", 50) / 2, SCREEN_HEIGHT / 2 - 20, 50, RED);
                    }
                    
                    // Show difficulty level
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
                    
                    DrawText(TextFormat("Difficulty: %s", gameOverDiffText),
                        SCREEN_WIDTH / 2 - MeasureText(TextFormat("Difficulty: %s", gameOverDiffText), 20) / 2, 
                        SCREEN_HEIGHT / 2 + 40, 20, gameOverDiffColor);
                    
                    DrawText(TextFormat("Final Score: %d - %d", playerScore, computerScore), 
                        SCREEN_WIDTH / 2 - MeasureText(TextFormat("Final Score: %d - %d", playerScore, computerScore), 30) / 2, 
                        SCREEN_HEIGHT / 2 + 70, 30, WHITE);
                        
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
    if (paddleHit.frameCount > 0) UnloadSound(paddleHit);
    if (wallHit.frameCount > 0) UnloadSound(wallHit);
    if (score.frameCount > 0) UnloadSound(score);
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}
