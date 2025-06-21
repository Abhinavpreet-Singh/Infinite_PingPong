#include <iostream>
#include <raylib.h>
#include <string>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

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
    float impossibleSpeedMultiplier; // Speed multiplier for IMPOSSIBLE mode
    int hitCounter;                 // Track consecutive hits for IMPOSSIBLE mode
};

// Game state
enum GameState {
    MENU,
    DIFFICULTY_SELECT,
    GAMEPLAY,
    PAUSED,
    GAME_OVER
};

// Difficulty levels
enum DifficultyLevel {
    EASY,
    MEDIUM,
    HARD,
    IMPOSSIBLE
};

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int SCREEN_WIDTH = 1024;
static const int SCREEN_HEIGHT = 768;

// Constants for the court area - with a border
static const int COURT_BORDER_X = 5;
static const int COURT_BORDER_Y = 75;
static const int COURT_X = COURT_BORDER_X;
static const int COURT_Y = COURT_BORDER_Y;
static const int COURT_WIDTH = SCREEN_WIDTH - (2 * COURT_BORDER_X);
static const int COURT_HEIGHT = SCREEN_HEIGHT - (2 * COURT_BORDER_Y);

// Game state and difficulty
static GameState currentState = MENU;
static DifficultyLevel currentDifficulty = MEDIUM;

// Game elements
static Paddle playerPaddle;
static Paddle computerPaddle;
static Ball ball;
static Camera2D camera = { 0 };

// Score and effects
static int playerScore = 0;
static int computerScore = 0;
static float screenShake = 0.0f;

// Trail and stars
static const int TRAIL_LENGTH = 15;
static Vector2 ballTrail[TRAIL_LENGTH] = { 0 };
static int trailIndex = 0;
static const int numStars = 80;
static Vector2 stars[numStars];

// Sounds
static Sound paddleHit, wallHit, score;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame
void ResetBall(int direction);  // Reset ball position and speed

//----------------------------------------------------------------------------------
// Main Entry Point
//----------------------------------------------------------------------------------
int main() {
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Enhanced Ping Pong Game");
    InitAudioDevice();

    // Initialize Paddles
    playerPaddle = {COURT_X + 20, SCREEN_HEIGHT / 2 - 60, 20, 120, 10, WHITE};
    computerPaddle = {COURT_X + COURT_WIDTH - 40, SCREEN_HEIGHT / 2 - 60, 20, 120, 8, RED};

    // Initialize Ball
    ball = { (float)COURT_X + COURT_WIDTH / 2, (float)COURT_Y + COURT_HEIGHT / 2, 7, 7, 15, WHITE, 1.0f, 0 };

    // Initialize effects and background
    camera.zoom = 1.0f;
    for (int i = 0; i < TRAIL_LENGTH; i++) ballTrail[i] = (Vector2){ ball.x, ball.y };
    for (int i = 0; i < numStars; i++) {
        stars[i].x = GetRandomValue(0, SCREEN_WIDTH);
        stars[i].y = GetRandomValue(0, SCREEN_HEIGHT);
    }

    // Load sounds
    if (FileExists("resources/paddle_hit.wav")) paddleHit = LoadSound("resources/paddle_hit.wav");
    if (FileExists("resources/wall_hit.wav")) wallHit = LoadSound("resources/wall_hit.wav"); 
    if (FileExists("resources/score.wav")) score = LoadSound("resources/score.wav");

    ResetBall(0); // Set initial ball state

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    // Main game loop
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    if (paddleHit.frameCount > 0) UnloadSound(paddleHit);
    if (wallHit.frameCount > 0) UnloadSound(wallHit);
    if (score.frameCount > 0) UnloadSound(score);
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}

void ResetBall(int direction) // direction: 0 = random, 1 = to player, -1 = to computer
{
    ball.x = (float)COURT_X + COURT_WIDTH / 2;
    ball.y = (float)COURT_Y + COURT_HEIGHT / 2;
    ball.hitCounter = 0;
    ball.impossibleSpeedMultiplier = 1.0f;

    float initialSpeed = 0;
    switch(currentDifficulty) {
        case EASY: initialSpeed = 2.5f; break;
        case MEDIUM: initialSpeed = 4.0f; break;
        case HARD: initialSpeed = 5.5f; break;
        case IMPOSSIBLE: initialSpeed = 7.0f; break;
    }

    if (direction == 0) {
        ball.speedX = (GetRandomValue(0, 1) == 0) ? -initialSpeed : initialSpeed;
    } else {
        ball.speedX = initialSpeed * direction;
    }
    ball.speedY = (GetRandomValue(0, 1) == 0) ? -initialSpeed : initialSpeed;
}

void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // Update screen shake
    if (screenShake > 0) {
        camera.offset.x = GetRandomValue(-screenShake, screenShake);
        camera.offset.y = GetRandomValue(-screenShake, screenShake);
        screenShake -= 0.5f; // Reduce shake intensity
    } else {
        screenShake = 0;
        camera.offset = (Vector2){ 0, 0 };
    }
    
    // Update ball trail
    ballTrail[trailIndex] = (Vector2){ ball.x, ball.y };
    trailIndex = (trailIndex + 1) % TRAIL_LENGTH;
    
    switch (currentState) {
        case MENU:
            if (IsKeyPressed(KEY_SPACE)) {
                currentState = DIFFICULTY_SELECT;
            }
            break;
            
        case DIFFICULTY_SELECT:
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {
                currentDifficulty = EASY;
                computerPaddle.speed = 5.0f;
                currentState = GAMEPLAY;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
            } else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
                currentDifficulty = MEDIUM;
                computerPaddle.speed = 7.0f;
                currentState = GAMEPLAY;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
            } else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {
                currentDifficulty = HARD;
                computerPaddle.speed = 9.0f;
                currentState = GAMEPLAY;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
            } else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) {
                currentDifficulty = IMPOSSIBLE;
                computerPaddle.speed = 11.0f;
                currentState = GAMEPLAY;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
            }
            else if (IsKeyPressed(KEY_BACKSPACE)) {
                currentState = MENU;
            }
            break;
            
        case GAMEPLAY:
            if (IsKeyPressed(KEY_P)) {
                currentState = PAUSED;
            }

            if (IsKeyDown(KEY_W) && playerPaddle.y > COURT_Y) playerPaddle.y -= playerPaddle.speed;
            if (IsKeyDown(KEY_S) && playerPaddle.y + playerPaddle.height < COURT_Y + COURT_HEIGHT) playerPaddle.y += playerPaddle.speed;
            if (IsKeyDown(KEY_UP) && playerPaddle.y > COURT_Y) playerPaddle.y -= playerPaddle.speed;
            if (IsKeyDown(KEY_DOWN) && playerPaddle.y + playerPaddle.height < COURT_Y + COURT_HEIGHT) playerPaddle.y += playerPaddle.speed;
            
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
                    screenShake = 8.0f; // Trigger screen shake
                    ResetBall(1); // Serve to player
                    
                    if (score.frameCount > 0) PlaySound(score);
                    
                    // Check for game over
                    if (computerScore >= 10) {
                        currentState = GAME_OVER;
                    }
                }
                  if (ball.x + ball.radius > COURT_X + COURT_WIDTH) {                        // Player scores
                    playerScore++;
                    screenShake = 8.0f; // Trigger screen shake
                    ResetBall(-1); // Serve to computer
                    
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
            
        case PAUSED:
            if (IsKeyPressed(KEY_P)) currentState = GAMEPLAY;
            if (IsKeyPressed(KEY_R)) {
                currentState = GAMEPLAY;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
            }
            break;
            
        case GAME_OVER:
            if (IsKeyPressed(KEY_R)) {
                currentState = GAMEPLAY;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
            }
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
    
    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

            // Draw starfield background
            for (int i = 0; i < numStars; i++) {
                DrawCircle(stars[i].x, stars[i].y, 1.5f, GRAY);
            }
            // Draw court border
            DrawRectangleLinesEx((Rectangle){COURT_X, COURT_Y, COURT_WIDTH, COURT_HEIGHT}, 2, DARKGRAY);
            
            // Draw center line (within court boundaries)
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
                case PAUSED:
                {
                    // Draw all common game elements
                    DrawRectangleRounded((Rectangle){playerPaddle.x, playerPaddle.y, playerPaddle.width, playerPaddle.height}, 0.8f, 10, playerPaddle.color);
                    DrawRectangleRounded((Rectangle){computerPaddle.x, computerPaddle.y, computerPaddle.width, computerPaddle.height}, 0.8f, 10, computerPaddle.color);
                    
                    for (int i = 0; i < TRAIL_LENGTH; i++) {
                        int current = (trailIndex - 1 - i + TRAIL_LENGTH) % TRAIL_LENGTH;
                        float alpha = 1.0f - ((float)i / TRAIL_LENGTH);
                        DrawCircle(ballTrail[current].x, ballTrail[current].y, ball.radius, ColorAlpha(ball.color, alpha * 0.3f));
                    }
                    
                    DrawCircleGradient(ball.x, ball.y, ball.radius+4, ColorAlpha(WHITE, 0.3f), ColorAlpha(WHITE, 0.0f));
                    DrawCircle(ball.x, ball.y, ball.radius, ball.color);
                    DrawText(TextFormat("%d", playerScore), COURT_X + COURT_WIDTH/4 - 15, COURT_Y + 30, 60, WHITE);
                    DrawText(TextFormat("%d", computerScore), COURT_X + COURT_WIDTH*3/4 - 15, COURT_Y + 30, 60, RED);
                    
                    const char* difficultyText = "";
                    Color difficultyColor = WHITE;
                    
                    switch(currentDifficulty) {
                        case EASY: difficultyText = "EASY"; difficultyColor = GREEN; break;
                        case MEDIUM: difficultyText = "MEDIUM"; difficultyColor = YELLOW; break;
                        case HARD: difficultyText = "HARD"; difficultyColor = ORANGE; break;
                        case IMPOSSIBLE: difficultyText = "IMPOSSIBLE"; difficultyColor = RED; break;
                    }
                    DrawText(difficultyText, SCREEN_WIDTH / 2 - MeasureText(difficultyText, 30) / 2, 10, 30, difficultyColor);
                    
                    if (currentDifficulty == IMPOSSIBLE && ball.hitCounter > 3) {
                        char speedText[50];
                        sprintf(speedText, "SPEED: %.1fX", ball.impossibleSpeedMultiplier);
                        DrawText(speedText, SCREEN_WIDTH / 2 - MeasureText(speedText, 20) / 2, COURT_Y + COURT_HEIGHT - 25, 20, RED);
                    }
                    
                    // State-specific drawing
                    if (currentState == GAMEPLAY) {
                        DrawText("P for Pause", SCREEN_WIDTH - MeasureText("P for Pause", 20) - 20, 10, 20, LIGHTGRAY);
                    } else if (currentState == PAUSED) {
                        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.7f));
                        DrawText("PAUSED", SCREEN_WIDTH / 2 - MeasureText("PAUSED", 60) / 2, SCREEN_HEIGHT / 3, 60, WHITE);
                        DrawText("Press P to Resume", SCREEN_WIDTH / 2 - MeasureText("Press P to Resume", 30) / 2, SCREEN_HEIGHT / 2, 30, WHITE);
                        DrawText("Press R to Restart", SCREEN_WIDTH / 2 - MeasureText("Press R to Restart", 30) / 2, SCREEN_HEIGHT / 2 + 50, 30, WHITE);
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
            
            EndMode2D();
            
            // Draw FPS counter
            DrawFPS(10, 10);
            
        EndDrawing();
}
