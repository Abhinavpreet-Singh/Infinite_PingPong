#include <iostream>
#include <raylib.h>
#include <string>
#include <cstring>
#include <cmath>

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
    float velocityY; // For smooth movement
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
    MAIN_MENU,
    DIFFICULTY_SELECT,
    READY_TO_START,
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
static GameState currentState = MAIN_MENU;
static DifficultyLevel currentDifficulty = MEDIUM;

// Player Name
static char playerName[32] = "Player";
static int letterCount = 0;

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

// Ball trail activation thresholds by difficulty
static int GetTrailThreshold() {
    switch(currentDifficulty) {
        case EASY: return 4;
        case MEDIUM: return 3;
        case HARD: return 2;
        case IMPOSSIBLE: return 1;
        default: return 3;
    }
}

//----------------------------------------------------------------------------------
// Main Entry Point
//----------------------------------------------------------------------------------
int main() {
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Enhanced Ping Pong Game");
    InitAudioDevice();

    // Initialize Paddles
    playerPaddle = {COURT_X + 20, SCREEN_HEIGHT / 2 - 60, 20, 120, 10, WHITE, 0};
    computerPaddle = {COURT_X + COURT_WIDTH - 40, SCREEN_HEIGHT / 2 - 60, 20, 120, 8, RED, 0};

    // Initialize Ball
    ball = { (float)COURT_X + COURT_WIDTH / 2, (float)COURT_Y + COURT_HEIGHT / 2, 7, 7, 15, WHITE, 1.0f, 0 };    // Initialize effects and background
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
    SetTargetFPS(60);  // Set to 60 FPS for web version
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);  // Set to consistent 60 FPS for smoother gameplay
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
    ball.impossibleSpeedMultiplier = 1.0f;    float initialSpeed = 0;
    switch(currentDifficulty) {
        case EASY: initialSpeed = 7.0f; break;     // Reduced for more manageable gameplay
        case MEDIUM: initialSpeed = 10.0f; break;  // Adjusted for proper medium challenge
        case HARD: initialSpeed = 14.0f; break;    // Adjusted for better game feel
        case IMPOSSIBLE: initialSpeed = 18.0f; break;  // Still challenging but more reasonable
    }

    if (direction == 0) {
        ball.speedX = (GetRandomValue(0, 1) == 0) ? -initialSpeed : initialSpeed;
    } else {
        ball.speedX = initialSpeed * direction;
    }
    ball.speedY = (GetRandomValue(0, 1) == 0) ? -initialSpeed : initialSpeed;
}

void UpdateDrawFrame(void)
{    // Update
    //----------------------------------------------------------------------------------
    // Check if window lost focus and automatically pause the game
    if (currentState == GAMEPLAY && !IsWindowFocused()) {
        currentState = PAUSED;
    }
    
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
        case MAIN_MENU: {
            // Handle name input
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (letterCount < 31)) {
                    playerName[letterCount] = (char)key;
                    playerName[letterCount+1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                letterCount--;
                if (letterCount < 0) letterCount = 0;
                playerName[letterCount] = '\0';
            }
            // Play button logic
            Rectangle playButton = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 60, 200, 50 };
            if ((CheckCollisionPointRec(GetMousePosition(), playButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ENTER)) {
                if (letterCount > 0) {
                    currentState = DIFFICULTY_SELECT;
                }
            }
            break;
        }        case DIFFICULTY_SELECT: {
            // Define button rectangles for collision detection
            Rectangle easyButton = { SCREEN_WIDTH/2 - 200, 180, 400, 75 };
            Rectangle mediumButton = { SCREEN_WIDTH/2 - 200, 180 + 100, 400, 75 };
            Rectangle hardButton = { SCREEN_WIDTH/2 - 200, 180 + 200, 400, 75 };
            Rectangle impossibleButton = { SCREEN_WIDTH/2 - 200, 180 + 300, 400, 75 };
              // Track mouse position and clicks
            Vector2 mousePos = GetMousePosition();
            bool mouseClicked = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
            bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
            
            // Use either pressed or released for better click reliability
            bool mouseAction = mouseClicked || mousePressed;
              // Check both keyboard and mouse selection for Easy difficulty
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1) || 
                (mouseAction && CheckCollisionPointRec(mousePos, easyButton))) {
                currentDifficulty = EASY;
                computerPaddle.speed = 8.5f;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
                currentState = READY_TO_START;
            }            // Check both keyboard and mouse selection for Medium difficulty
            else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2) || 
                     (mouseAction && CheckCollisionPointRec(mousePos, mediumButton))) {
                currentDifficulty = MEDIUM;
                computerPaddle.speed = 12.0f;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
                currentState = READY_TO_START;
            }            // Check both keyboard and mouse selection for Hard difficulty
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3) || 
                     (mouseAction && CheckCollisionPointRec(mousePos, hardButton))) {
                currentDifficulty = HARD;
                computerPaddle.speed = 15.0f;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
                currentState = READY_TO_START;
            }            // Check both keyboard and mouse selection for Impossible difficulty
            else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4) || 
                     (mouseAction && CheckCollisionPointRec(mousePos, impossibleButton))) {
                currentDifficulty = IMPOSSIBLE;
                computerPaddle.speed = 24.0f;
                ResetBall(0);
                playerScore = 0; computerScore = 0;
                currentState = READY_TO_START;
            }
            else if (IsKeyPressed(KEY_BACKSPACE)) {
                currentState = MAIN_MENU;
            }
            break;
        }
        case READY_TO_START: {
            if (IsKeyPressed(KEY_SPACE)) {
                currentState = GAMEPLAY;
            }
            break;
        }
        case GAMEPLAY: {
            if (IsKeyPressed(KEY_SPACE)) {
                currentState = PAUSED;
            }
            if (IsKeyPressed(KEY_M)) {
                currentState = MAIN_MENU;            }            // --- Perfect Arcade Feel Player Paddle Control ---
            const float acceleration = 7.0f;  // Very high acceleration for instant response
            const float friction = 0.5f;      // Lower friction for precise control and faster stops
            const float maxVelocity = 22.0f;  // Higher max velocity for lightning-fast movement
            const float directionChangeBoost = 1.5f; // Extra boost when changing directions
            
            // Apply acceleration based on key press for extremely responsive control
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
                // Instant direction change with extra boost for arcade-perfect feel
                if (playerPaddle.velocityY > 0) {
                    playerPaddle.velocityY = -acceleration * directionChangeBoost; // Boosted immediate direction change
                } else {
                    playerPaddle.velocityY -= acceleration; // Direct acceleration for responsive control
                }
                
                // Immediate boost to high speed for arcade feel
                if (fabs(playerPaddle.velocityY) < maxVelocity * 0.5f) {
                    playerPaddle.velocityY = -maxVelocity * 0.7f; // Quick ramp-up to 70% of max speed
                }
            } else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
                // Instant direction change with extra boost for arcade-perfect feel
                if (playerPaddle.velocityY < 0) {
                    playerPaddle.velocityY = acceleration * directionChangeBoost; // Boosted immediate direction change
                } else {
                    playerPaddle.velocityY += acceleration; // Direct acceleration for responsive control
                }
                
                // Immediate boost to high speed for arcade feel
                if (fabs(playerPaddle.velocityY) < maxVelocity * 0.5f) {
                    playerPaddle.velocityY = maxVelocity * 0.7f; // Quick ramp-up to 70% of max speed
                }
            } else {
                // Apply stronger friction for crisp stops - arcade machines stop quickly
                if (fabs(playerPaddle.velocityY) > 0.5f) {
                    playerPaddle.velocityY *= friction;
                } else {
                    playerPaddle.velocityY = 0; // Complete stop when near zero for crisp feel
                }
            }

            // Clamp velocity to max speed
            if (playerPaddle.velocityY > maxVelocity) playerPaddle.velocityY = maxVelocity;
            if (playerPaddle.velocityY < -maxVelocity) playerPaddle.velocityY = -maxVelocity;
            
            // Apply an aggressive deadzone to prevent tiny drifting movements
            if (fabs(playerPaddle.velocityY) < 0.3f) playerPaddle.velocityY = 0;
            
            // Update paddle position based on velocity
            playerPaddle.y += playerPaddle.velocityY;

            // Keep paddle within court bounds and reset velocity on collision
            if (playerPaddle.y < COURT_Y) {
                playerPaddle.y = COURT_Y;
                playerPaddle.velocityY = 0;
            }
            if (playerPaddle.y + playerPaddle.height > COURT_Y + COURT_HEIGHT) {
                playerPaddle.y = COURT_Y + COURT_HEIGHT - playerPaddle.height;
                playerPaddle.velocityY = 0;
            }
            // --- End Smooth Player Paddle Control ---
            
            // Computer AI based on difficulty
            {
                float computerPaddleCenter = computerPaddle.y + computerPaddle.height / 2;
                float ballTrackPosition = ball.y;
                
                // Adjust computer properties based on difficulty
                float aiAccuracy = 0.0f; // Percentage chance of moving correctly
                float aiReactionSpeed = 0.0f; // Speed multiplier
                float aiDeadZone = 0.0f; // Area where paddle won't react
                bool useAdvancedPrediction = false;
                
                // Set AI behavior based on difficulty
                switch(currentDifficulty) {
                    case EASY:
                        aiAccuracy = 0.5f;
                        aiReactionSpeed = 0.5f;
                        aiDeadZone = 35.0f;
                        useAdvancedPrediction = false;
                        break;
                    case MEDIUM:
                        aiAccuracy = 0.65f;
                        aiReactionSpeed = 0.55f;
                        aiDeadZone = 40.0f;
                        useAdvancedPrediction = false;
                        break;
                    case HARD:
                        aiAccuracy = 0.75f;
                        aiReactionSpeed = 0.75f;
                        aiDeadZone = 30.0f;
                        useAdvancedPrediction = false;
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
                        // Add prediction error for Hard mode to make it more human
                        if (currentDifficulty == HARD) {
                            ballTrackPosition += GetRandomValue(-20, 20);
                        }

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
                }                // Ball collision with player paddle
                if (ball.x - ball.radius <= playerPaddle.x + playerPaddle.width &&
                    ball.y >= playerPaddle.y && ball.y <= playerPaddle.y + playerPaddle.height &&
                    ball.speedX < 0) {
                    
                    // Track consecutive hits for IMPOSSIBLE difficulty
                    ball.hitCounter++;
                    
                    // Update speed multiplier in IMPOSSIBLE mode
                    if (currentDifficulty == IMPOSSIBLE && ball.hitCounter > 3) {
                        ball.impossibleSpeedMultiplier += 0.05f; // Reduced from 0.08f for more gradual increase
                        if (ball.impossibleSpeedMultiplier > 2.0f) ball.impossibleSpeedMultiplier = 2.0f; // Lower cap for more manageable gameplay
                    }
                    
                    // Speed increases with each hit, adjusted per difficulty level
                    float speedIncreaseFactor;
                    switch (currentDifficulty) {
                        case EASY:
                            speedIncreaseFactor = -1.02f; // Increased from -1.01f for better gameplay at 60 FPS
                            break;
                        case MEDIUM:
                            speedIncreaseFactor = -1.04f; // Increased from -1.02f for better gameplay at 60 FPS
                            break;
                        case HARD:
                            speedIncreaseFactor = -1.06f; // Increased from -1.03f for better gameplay at 60 FPS
                            break;
                        case IMPOSSIBLE:
                            speedIncreaseFactor = -1.08f * ball.impossibleSpeedMultiplier; // Increased from -1.06f for better gameplay at 60 FPS
                            break;
                        default:
                            speedIncreaseFactor = -1.05f; // Increased from -1.03f for better gameplay at 60 FPS
                    }                    ball.speedX *= speedIncreaseFactor;                    
                    
                    // Change Y speed based on where the ball hits the paddle                    
                    float hitPosition = (ball.y - (playerPaddle.y + playerPaddle.height / 2)) / (playerPaddle.height / 2);
                    ball.speedY = ball.speedY * 0.7f + hitPosition * 10; // Reduced for less aggressive angle changes
                    
                    if (paddleHit.frameCount > 0) PlaySound(paddleHit);                }                
                
                // Ball collision with computer paddle                
                if (ball.x + ball.radius >= computerPaddle.x &&
                    ball.y >= computerPaddle.y && ball.y <= computerPaddle.y + computerPaddle.height &&
                    ball.speedX > 0) {
                    
                    // Track consecutive hits for IMPOSSIBLE difficulty
                    ball.hitCounter++;
                    
                    // Update speed multiplier in IMPOSSIBLE mode
                    if (currentDifficulty == IMPOSSIBLE && ball.hitCounter > 3) {
                        ball.impossibleSpeedMultiplier += 0.05f; // Reduced from 0.08f for more gradual increase
                        if (ball.impossibleSpeedMultiplier > 2.0f) ball.impossibleSpeedMultiplier = 2.0f; // Lower cap for more manageable gameplay
                    }
                    
                    // Speed increases with each hit, adjusted per difficulty level
                    float speedIncreaseFactor;
                    switch (currentDifficulty) {
                        case EASY:
                            speedIncreaseFactor = -1.02f; // Increased from -1.01f for better gameplay at 60 FPS
                            break;
                        case MEDIUM:
                            speedIncreaseFactor = -1.04f; // Increased from -1.02f for better gameplay at 60 FPS
                            break;
                        case HARD:
                            speedIncreaseFactor = -1.06f; // Increased from -1.03f for better gameplay at 60 FPS
                            break;
                        case IMPOSSIBLE:
                            speedIncreaseFactor = -1.08f * ball.impossibleSpeedMultiplier; // Increased from -1.06f for better gameplay at 60 FPS
                            break;
                        default:
                            speedIncreaseFactor = -1.05f; // Increased from -1.03f for better gameplay at 60 FPS
                    }                    ball.speedX *= speedIncreaseFactor;                    
                    
                    // Change Y speed based on where the ball hits the paddle                    
                    float hitPosition = (ball.y - (computerPaddle.y + computerPaddle.height / 2)) / (computerPaddle.height / 2);
                    ball.speedY = ball.speedY * 0.7f + hitPosition * 10; // Reduced for less aggressive angle changes
                    
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
                }                // Cap ball speed - different caps for different difficulty levels
                float MAX_SPEED;
                switch (currentDifficulty) {                        case EASY:
                        MAX_SPEED = 18.0f;  // Reduced for more controllable gameplay
                        break;                        case MEDIUM:
                        MAX_SPEED = 24.0f;  // Reduced for more controllable gameplay
                        break;                        case HARD:
                        MAX_SPEED = 32.0f;  // Reduced for more controllable gameplay
                        break;                        case IMPOSSIBLE:
                        MAX_SPEED = 45.0f;  // Reduced but still very challenging
                        break;
                    default:
                        MAX_SPEED = 22.0f;  // Reduced for more controllable gameplay
                }
                
                if (ball.speedX > MAX_SPEED) ball.speedX = MAX_SPEED;
                if (ball.speedX < -MAX_SPEED) ball.speedX = -MAX_SPEED;
                if (ball.speedY > MAX_SPEED) ball.speedY = MAX_SPEED;
                if (ball.speedY < -MAX_SPEED) ball.speedY = -MAX_SPEED;
            }
            break;
        }        case PAUSED: {
            // Define button rectangles
            Rectangle resumeButton = { SCREEN_WIDTH/2 - 170, 410, 340, 65 };
            Rectangle menuButton = { SCREEN_WIDTH/2 - 170, 495, 340, 65 };

            // Check for button clicks - only respond when released within the same button
            Vector2 mousePos = GetMousePosition();
            static bool wasPressingResume = false;
            static bool wasPressingMenu = false;
            
            bool isOverResume = CheckCollisionPointRec(mousePos, resumeButton);
            bool isOverMenu = CheckCollisionPointRec(mousePos, menuButton);
            
            // Track when mouse is pressed on each button
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (isOverResume) wasPressingResume = true;
                if (isOverMenu) wasPressingMenu = true;
            }
            
            // Only trigger action when released on the same button
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (isOverResume && wasPressingResume) {
                    currentState = GAMEPLAY;
                }
                if (isOverMenu && wasPressingMenu) {
                    currentState = MAIN_MENU;
                }
                
                // Reset flags
                wasPressingResume = false;
                wasPressingMenu = false;
            }
            
            // Allow unpausing with SPACE key as well
            if (IsKeyPressed(KEY_SPACE)) currentState = GAMEPLAY;
            break;
        }
        case GAME_OVER: {
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
        
        switch (currentState) {            case MAIN_MENU: {
                // Background effect: animated stars with color variations
                for (int i = 0; i < numStars; i++) {
                    float starSize = (i % 4 == 0) ? 3.0f : ((i % 3 == 0) ? 2.0f : 1.2f);
                    Color starColor = (i % 5 == 0) ? YELLOW : ((i % 7 == 0) ? SKYBLUE : WHITE);
                    DrawCircle(stars[i].x, stars[i].y, starSize, ColorAlpha(starColor, 0.7f + 0.3f * sinf(GetTime() * 2 + i)));
                }
                
                // Semi-transparent overlay gradient for better readability
                DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                                     ColorAlpha(BLACK, 0.8f), ColorAlpha(DARKBLUE, 0.5f));
                
                // Draw dynamic retro-style title
                const char* title = "PING PONG";
                int titleSize = 105;
                float bounceHeight = 5.0f * sinf(GetTime() * 3.0f); // Subtle bounce effect
                int titleWidth = MeasureText(title, titleSize);
                
                // Draw colorful title with glowing effect and shadow
                DrawText(title, SCREEN_WIDTH/2 - titleWidth/2 - 3, 120 + 3, titleSize, BLACK); // Shadow
                
                // Main title with glow effect
                Color titleColor = ColorFromHSV(fmodf(GetTime() * 20, 360.0f), 0.7f, 1.0f); // Slow color cycle
                DrawText(title, SCREEN_WIDTH/2 - titleWidth/2, 120 + bounceHeight, titleSize, titleColor);
                
                // Subtitle with glow effect
                DrawText("ARCADE EDITION", 
                    SCREEN_WIDTH/2 - MeasureText("ARCADE EDITION", 32)/2, 
                    230, 
                    32, ORANGE);
                
                // Dynamic underline
                DrawLineEx(
                    (Vector2){SCREEN_WIDTH/2 - 180, 270},
                    (Vector2){SCREEN_WIDTH/2 + 180, 270},
                    3.0f, WHITE);
                
                // Name label with better styling
                DrawText("ENTER YOUR NAME:", 
                    SCREEN_WIDTH/2 - MeasureText("ENTER YOUR NAME:", 24)/2, 
                    290, 
                    24, WHITE);
                
                // Name input box with better visual style
                Rectangle nameBox = { SCREEN_WIDTH/2 - 200, 320, 400, 60 };
                DrawRectangleGradientH(nameBox.x, nameBox.y, nameBox.width, nameBox.height, 
                                     ColorAlpha(DARKBLUE, 0.7f), ColorAlpha(DARKPURPLE, 0.7f));
                DrawRectangleLines(nameBox.x, nameBox.y, nameBox.width, nameBox.height, WHITE);
                
                // Player name with better readability
                DrawText(playerName, 
                    nameBox.x + 20, 
                    nameBox.y + nameBox.height/2 - 15, 
                    40, WHITE);
                
                // Animated text cursor
                if ((int)(GetTime() * 2) % 2 == 0) {
                    DrawRectangle(
                        nameBox.x + 20 + MeasureText(playerName, 40), 
                        nameBox.y + 15, 
                        3, 30, 
                        ColorFromHSV(fmodf(GetTime() * 50, 360.0f), 0.8f, 1.0f));
                }
                
                // Better play button with glow effect
                Rectangle playButton = { SCREEN_WIDTH/2 - 150, 410, 300, 70 };
                bool playEnabled = (letterCount > 0);
                
                // Button with better hover effects
                bool playHover = playEnabled && CheckCollisionPointRec(GetMousePosition(), playButton);
                bool playPressed = playHover && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
                
                // Button style based on state
                Color buttonGradStart, buttonGradEnd;
                if (!playEnabled) {
                    buttonGradStart = ColorAlpha(DARKGRAY, 0.7f);
                    buttonGradEnd = ColorAlpha(GRAY, 0.6f);
                } else if (playPressed) {
                    buttonGradStart = ColorAlpha(ORANGE, 0.9f);
                    buttonGradEnd = ColorAlpha(RED, 0.9f);
                } else if (playHover) {
                    buttonGradStart = ColorAlpha(GOLD, 0.9f);
                    buttonGradEnd = ColorAlpha(ORANGE, 0.9f);
                } else {
                    buttonGradStart = ColorAlpha(ORANGE, 0.8f);
                    buttonGradEnd = ColorAlpha(RED, 0.8f);
                }
                
                DrawRectangleGradientV(playButton.x, playButton.y, playButton.width, playButton.height, 
                                     buttonGradStart, buttonGradEnd);
                DrawRectangleLines(playButton.x, playButton.y, playButton.width, playButton.height, WHITE);
                
                // Animated button text
                Color textColor = playEnabled ? WHITE : DARKGRAY;
                float textScale = playPressed ? 0.95f : (playHover ? 1.05f : 1.0f);
                int fontSize = 40 * textScale;
                
                DrawText("PLAY", 
                    playButton.x + playButton.width/2 - MeasureText("PLAY", fontSize)/2, 
                    playButton.y + playButton.height/2 - fontSize/2, 
                    fontSize, textColor);
                
                // Better keyboard hint with pulse effect
                if (playEnabled) {
                    float alpha = 0.5f + 0.5f * sinf(GetTime() * 4);
                    DrawText("Press ENTER to start", 
                        SCREEN_WIDTH/2 - MeasureText("Press ENTER to start", 22)/2, 
                        500, 
                        22, ColorAlpha(WHITE, alpha));
                }
            }
            break;
              case DIFFICULTY_SELECT: {
                // Background effect: animated stars with color variations
                for (int i = 0; i < numStars; i++) {
                    float starSize = (i % 4 == 0) ? 3.0f : ((i % 3 == 0) ? 2.0f : 1.2f);
                    Color starColor = (i % 5 == 0) ? YELLOW : ((i % 7 == 0) ? SKYBLUE : WHITE);
                    DrawCircle(stars[i].x, stars[i].y, starSize, ColorAlpha(starColor, 0.7f + 0.3f * sinf(GetTime() * 2 + i)));
                }
                
                // Semi-transparent overlay gradient for better readability
                DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                                     ColorAlpha(BLACK, 0.8f), ColorAlpha(DARKBLUE, 0.5f));
                
                // Draw animated title with retro style
                const char* title = "SELECT DIFFICULTY";
                int titleSize = 65;
                float bounceHeight = 3.0f * sinf(GetTime() * 2.5f); // Subtle bounce effect
                int titleWidth = MeasureText(title, titleSize);
                
                // Draw shadow for depth
                DrawText(title, SCREEN_WIDTH/2 - titleWidth/2 - 3, 90 + 3, titleSize, BLACK);
                
                // Main title with color animation
                Color titleColor = ColorFromHSV(fmodf(GetTime() * 20, 360.0f), 0.7f, 1.0f); // Slow color cycle
                DrawText(title, SCREEN_WIDTH/2 - titleWidth/2, 90 + bounceHeight, titleSize, titleColor);

                // Dynamic underline
                DrawLineEx(
                    (Vector2){SCREEN_WIDTH/2 - 280, 160},
                    (Vector2){SCREEN_WIDTH/2 + 280, 160},
                    3.0f, WHITE);
                  // Define difficulty option buttons with better spacing
                float buttonSpacing = 100;
                // Keep these button definitions identical to those in the update section for consistent click detection
                Rectangle easyButton = { SCREEN_WIDTH/2 - 200, 180, 400, 75 };
                Rectangle mediumButton = { SCREEN_WIDTH/2 - 200, 180 + buttonSpacing, 400, 75 };
                Rectangle hardButton = { SCREEN_WIDTH/2 - 200, 180 + buttonSpacing*2, 400, 75 };
                Rectangle impossibleButton = { SCREEN_WIDTH/2 - 200, 180 + buttonSpacing*3, 400, 75 };
                  Vector2 mousePos = GetMousePosition();
                bool easyHover = CheckCollisionPointRec(mousePos, easyButton);
                bool mediumHover = CheckCollisionPointRec(mousePos, mediumButton);
                bool hardHover = CheckCollisionPointRec(mousePos, hardButton);
                bool impossibleHover = CheckCollisionPointRec(mousePos, impossibleButton);
                
                // Add visual feedback for mouse clicks
                bool isMousePressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
                bool easyPressed = easyHover && isMousePressed;
                bool mediumPressed = mediumHover && isMousePressed;
                bool hardPressed = hardHover && isMousePressed;
                bool impossiblePressed = impossibleHover && isMousePressed;
                  // Draw difficulty buttons with retro arcade style and proper click feedback
                // 1. Easy Button - green gradient
                if (easyPressed) {
                    // Pressed effect - darker and slight offset
                    DrawRectangleGradientH(easyButton.x + 2, easyButton.y + 2, easyButton.width - 4, easyButton.height - 4, 
                                        DARKGREEN, GREEN);
                    DrawRectangleLines(easyButton.x + 2, easyButton.y + 2, easyButton.width - 4, easyButton.height - 4, WHITE);
                    DrawText("1 - EASY", easyButton.x + 22, easyButton.y + easyButton.height/2 - 13, 36, WHITE);
                } else {
                    DrawRectangleGradientH(easyButton.x, easyButton.y, easyButton.width, easyButton.height, 
                                        easyHover ? LIME : GREEN, easyHover ? GREEN : DARKGREEN);
                    DrawRectangleLines(easyButton.x, easyButton.y, easyButton.width, easyButton.height, WHITE);
                    DrawText("1 - EASY", easyButton.x + 20, easyButton.y + easyButton.height/2 - 15, 36, WHITE);
                }
                DrawCircle(easyButton.x + 380, easyButton.y + easyButton.height/2, 5, 
                          ColorAlpha(WHITE, 0.5f + 0.5f * sinf(GetTime() * 3))); // Indicator light
                  // 2. Medium Button - yellow/gold gradient with click feedback
                if (mediumPressed) {
                    // Pressed effect - darker and slight offset
                    DrawRectangleGradientH(mediumButton.x + 2, mediumButton.y + 2, mediumButton.width - 4, mediumButton.height - 4, 
                                        GOLD, ORANGE);
                    DrawRectangleLines(mediumButton.x + 2, mediumButton.y + 2, mediumButton.width - 4, mediumButton.height - 4, WHITE);
                    DrawText("2 - MEDIUM", mediumButton.x + 22, mediumButton.y + mediumButton.height/2 - 13, 36, BLACK);
                } else {
                    DrawRectangleGradientH(mediumButton.x, mediumButton.y, mediumButton.width, mediumButton.height, 
                                        mediumHover ? GOLD : YELLOW, mediumHover ? ORANGE : GOLD);
                    DrawRectangleLines(mediumButton.x, mediumButton.y, mediumButton.width, mediumButton.height, WHITE);
                    DrawText("2 - MEDIUM", mediumButton.x + 20, mediumButton.y + mediumButton.height/2 - 15, 36, BLACK);
                }
                DrawCircle(mediumButton.x + 380, mediumButton.y + mediumButton.height/2, 5, 
                          ColorAlpha(WHITE, 0.5f + 0.5f * sinf(GetTime() * 3 + 1))); // Indicator light
                  // 3. Hard Button - orange gradient with click feedback
                if (hardPressed) {
                    // Pressed effect - darker and slight offset
                    DrawRectangleGradientH(hardButton.x + 2, hardButton.y + 2, hardButton.width - 4, hardButton.height - 4, 
                                        ORANGE, (Color){200, 80, 0, 255});
                    DrawRectangleLines(hardButton.x + 2, hardButton.y + 2, hardButton.width - 4, hardButton.height - 4, WHITE);
                    DrawText("3 - HARD", hardButton.x + 22, hardButton.y + hardButton.height/2 - 13, 36, WHITE);
                } else {
                    DrawRectangleGradientH(hardButton.x, hardButton.y, hardButton.width, hardButton.height, 
                                        hardHover ? (Color){255, 180, 50, 255} : ORANGE, 
                                        hardHover ? RED : (Color){200, 80, 0, 255});
                    DrawRectangleLines(hardButton.x, hardButton.y, hardButton.width, hardButton.height, WHITE);
                    DrawText("3 - HARD", hardButton.x + 20, hardButton.y + hardButton.height/2 - 15, 36, WHITE);
                }
                DrawCircle(hardButton.x + 380, hardButton.y + hardButton.height/2, 5, 
                          ColorAlpha(WHITE, 0.5f + 0.5f * sinf(GetTime() * 3 + 2))); // Indicator light
                  // 4. Impossible Button - red gradient with warning effect and click feedback
                float warningPulse = impossibleHover ? (0.8f + 0.2f * sinf(GetTime() * 8)) : 1.0f;
                
                if (impossiblePressed) {
                    // Pressed effect - darker and slight offset with intense pulsing
                    DrawRectangleGradientH(impossibleButton.x + 2, impossibleButton.y + 2, impossibleButton.width - 4, impossibleButton.height - 4, 
                                        ColorAlpha(MAROON, warningPulse), 
                                        ColorAlpha((Color){100, 0, 0, 255}, warningPulse));
                    DrawRectangleLines(impossibleButton.x + 2, impossibleButton.y + 2, impossibleButton.width - 4, impossibleButton.height - 4, WHITE);
                    DrawText("4 - IMPOSSIBLE", impossibleButton.x + 22, impossibleButton.y + impossibleButton.height/2 - 13, 36, WHITE);
                } else {
                    DrawRectangleGradientH(impossibleButton.x, impossibleButton.y, impossibleButton.width, impossibleButton.height, 
                                        ColorAlpha(RED, warningPulse), 
                                        ColorAlpha(MAROON, warningPulse));
                    DrawRectangleLines(impossibleButton.x, impossibleButton.y, impossibleButton.width, impossibleButton.height, WHITE);
                    DrawText("4 - IMPOSSIBLE", impossibleButton.x + 20, impossibleButton.y + impossibleButton.height/2 - 15, 36, WHITE);
                }
                DrawCircle(impossibleButton.x + 380, impossibleButton.y + impossibleButton.height/2, 5, 
                          ColorAlpha(WHITE, 0.5f + 0.5f * sinf(GetTime() * 10))); // Fast pulsing indicator
                  // Better navigation info with animated effects
                float alpha1 = 0.7f + 0.3f * sinf(GetTime() * 3);
                DrawText("PRESS NUMBER KEY OR CLICK TO SELECT DIFFICULTY", 
                    SCREEN_WIDTH/2 - MeasureText("PRESS NUMBER KEY OR CLICK TO SELECT DIFFICULTY", 20)/2, 
                    585, 
                    20, ColorAlpha(WHITE, alpha1));
                
                float alpha2 = 0.6f + 0.4f * sinf(GetTime() * 2);    
                DrawText("PRESS BACKSPACE TO RETURN", 
                    SCREEN_WIDTH/2 - MeasureText("PRESS BACKSPACE TO RETURN", 20)/2, 
                    615, 
                    20, ColorAlpha(LIGHTGRAY, alpha2));
                    
                // Add some floating particles for effect
                for (int i = 0; i < 5; i++) {
                    float y = fmodf(GetTime() * (50 + i * 10) + i * 120, SCREEN_HEIGHT);
                    float x = SCREEN_WIDTH/2 + 250 * sinf(GetTime() * 0.5f + i);
                    DrawCircle(x, y, 2, ColorAlpha(WHITE, 0.5f));
                }
            }
            break;
                
            case READY_TO_START: {
                // Draw background and court as in gameplay
                DrawRectangleRounded((Rectangle){playerPaddle.x, playerPaddle.y, playerPaddle.width, playerPaddle.height}, 0.8f, 10, playerPaddle.color);
                DrawRectangleRounded((Rectangle){computerPaddle.x, computerPaddle.y, computerPaddle.width, computerPaddle.height}, 0.8f, 10, computerPaddle.color);
                DrawCircleGradient(ball.x, ball.y, ball.radius+4, ColorAlpha(WHITE, 0.3f), ColorAlpha(WHITE, 0.0f));
                DrawCircle(ball.x, ball.y, ball.radius, ball.color);
                // Draw Player Name and Score
                DrawText(playerName, COURT_X + COURT_WIDTH/4 - MeasureText(playerName, 20)/2, COURT_Y + 5, 20, WHITE);
                DrawText(TextFormat("%d", playerScore), COURT_X + COURT_WIDTH/4 - 15, COURT_Y + 30, 60, WHITE);
                DrawText("COMPUTER", COURT_X + COURT_WIDTH*3/4 - MeasureText("COMPUTER", 20)/2, COURT_Y + 5, 20, RED);
                DrawText(TextFormat("%d", computerScore), COURT_X + COURT_WIDTH*3/4 - 15, COURT_Y + 30, 60, RED);
                // Show difficulty
                const char* difficultyText = "";
                Color difficultyColor = WHITE;
                switch(currentDifficulty) {
                    case EASY: difficultyText = "EASY"; difficultyColor = GREEN; break;
                    case MEDIUM: difficultyText = "MEDIUM"; difficultyColor = YELLOW; break;
                    case HARD: difficultyText = "HARD"; difficultyColor = ORANGE; break;
                    case IMPOSSIBLE: difficultyText = "IMPOSSIBLE"; difficultyColor = RED; break;
                }
                DrawText(difficultyText, SCREEN_WIDTH / 2 - MeasureText(difficultyText, 30) / 2, 10, 30, difficultyColor);
                // Show 'Press SPACE to start' message
                float alpha = 0.6f + 0.4f * sinf(GetTime() * 3.0f);
                DrawText("Press SPACE to start", SCREEN_WIDTH/2 - MeasureText("Press SPACE to start", 40)/2, SCREEN_HEIGHT/2 - 20, 40, ColorAlpha(WHITE, alpha));
            }
            break;
                
            case GAMEPLAY:
            case PAUSED: {
                // Draw all common game elements
                DrawRectangleRounded((Rectangle){playerPaddle.x, playerPaddle.y, playerPaddle.width, playerPaddle.height}, 0.8f, 10, playerPaddle.color);
                DrawRectangleRounded((Rectangle){computerPaddle.x, computerPaddle.y, computerPaddle.width, computerPaddle.height}, 0.8f, 10, computerPaddle.color);
                
                // Only show trail after enough hits
                if (ball.hitCounter >= GetTrailThreshold()) {
                    for (int i = 0; i < TRAIL_LENGTH; i++) {
                        int current = (trailIndex - 1 - i + TRAIL_LENGTH) % TRAIL_LENGTH;
                        float alpha = 1.0f - ((float)i / TRAIL_LENGTH);
                        DrawCircle(ballTrail[current].x, ballTrail[current].y, ball.radius, ColorAlpha(ball.color, alpha * 0.3f));
                    }
                }
                
                DrawCircleGradient(ball.x, ball.y, ball.radius+4, ColorAlpha(WHITE, 0.3f), ColorAlpha(WHITE, 0.0f));
                DrawCircle(ball.x, ball.y, ball.radius, ball.color);
                
                // Draw Player Name and Score
                DrawText(playerName, COURT_X + COURT_WIDTH/4 - MeasureText(playerName, 20)/2, COURT_Y + 5, 20, WHITE);
                DrawText(TextFormat("%d", playerScore), COURT_X + COURT_WIDTH/4 - 15, COURT_Y + 30, 60, WHITE);

                // Draw Computer Score
                DrawText("COMPUTER", COURT_X + COURT_WIDTH*3/4 - MeasureText("COMPUTER", 20)/2, COURT_Y + 5, 20, RED);
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
                    DrawText("SPACE for Pause", SCREEN_WIDTH - MeasureText("SPACE for Pause", 20) - 20, 10, 20, LIGHTGRAY);
                    DrawText("M for Main Menu", SCREEN_WIDTH - MeasureText("M for Main Menu", 20) - 20, 35, 20, LIGHTGRAY);                } else if (currentState == PAUSED) {
                    // Semi-transparent overlay with radial gradient for dramatic pause effect
                    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                                         ColorAlpha(BLACK, 0.85f), ColorAlpha(DARKBLUE, 0.7f));
                    
                    // Create a glowing pause symbol
                    float glowIntensity = 0.6f + 0.4f * sinf(GetTime() * 3.0f);
                    float pauseWidth = 30;
                    float pauseHeight = 100;
                    float pauseSpacing = 40;
                    float pauseY = 180;
                    float pauseX = SCREEN_WIDTH/2 - pauseSpacing/2 - pauseWidth;
                    
                    Color pauseGlow = ColorAlpha(WHITE, glowIntensity);
                    DrawRectangleRounded((Rectangle){pauseX, pauseY, pauseWidth, pauseHeight}, 0.3f, 8, pauseGlow);
                    DrawRectangleRounded((Rectangle){pauseX + pauseWidth + pauseSpacing, pauseY, pauseWidth, pauseHeight}, 0.3f, 8, pauseGlow);
                    
                    // Draw animated "PAUSED" text
                    Color pauseTextColor = ColorFromHSV(fmodf(GetTime() * 15, 360.0f), 0.7f, 1.0f);
                    DrawText("PAUSED", SCREEN_WIDTH/2 - MeasureText("PAUSED", 80)/2, 300, 80, pauseTextColor);
                    
                    // Define button rectangles with better design
                    Rectangle resumeButton = { SCREEN_WIDTH/2 - 170, 410, 340, 65 };
                    Rectangle menuButton = { SCREEN_WIDTH/2 - 170, 495, 340, 65 };
                      // Get mouse position for hover effects
                    Vector2 mousePos = GetMousePosition();
                    bool resumeHover = CheckCollisionPointRec(mousePos, resumeButton);
                    bool menuHover = CheckCollisionPointRec(mousePos, menuButton);
                    
                    // Visual state based on mouse interaction
                    bool resumePressed = resumeHover && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
                    bool menuPressed = menuHover && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
                    
                    // Add click handling directly in the drawing phase as well for redundancy
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                        if (resumeHover) currentState = GAMEPLAY;
                        if (menuHover) currentState = MAIN_MENU;
                    }
                    
                    // --- Draw Resume Button with arcade style ---
                    Color resumeGradStart, resumeGradEnd;
                    if (resumePressed) {
                        resumeGradStart = (Color){20, 160, 20, 255};
                        resumeGradEnd = (Color){10, 100, 10, 255};
                    } else if (resumeHover) {
                        resumeGradStart = (Color){30, 220, 30, 255};
                        resumeGradEnd = (Color){0, 180, 0, 255};
                    } else {
                        resumeGradStart = (Color){20, 200, 20, 255};
                        resumeGradEnd = (Color){0, 140, 0, 255};
                    }
                    
                    DrawRectangleGradientH(resumeButton.x, resumeButton.y, resumeButton.width, resumeButton.height, 
                                         resumeGradStart, resumeGradEnd);
                    DrawRectangleLines(resumeButton.x, resumeButton.y, resumeButton.width, resumeButton.height, WHITE);
                    
                    // Play symbol inside resume button
                    DrawTriangle(
                        (Vector2){resumeButton.x + 30, resumeButton.y + resumeButton.height/2 - 15},
                        (Vector2){resumeButton.x + 30, resumeButton.y + resumeButton.height/2 + 15},
                        (Vector2){resumeButton.x + 60, resumeButton.y + resumeButton.height/2},
                        WHITE);
                    
                    // Draw button text with better style
                    DrawText("RESUME GAME", 
                        resumeButton.x + 75, 
                        resumeButton.y + resumeButton.height/2 - 15, 
                        30, WHITE);
                    
                    // --- Draw Main Menu Button with arcade style ---
                    Color menuGradStart, menuGradEnd;
                    if (menuPressed) {
                        menuGradStart = (Color){160, 20, 20, 255};
                        menuGradEnd = (Color){100, 10, 10, 255};
                    } else if (menuHover) {
                        menuGradStart = (Color){220, 30, 30, 255};
                        menuGradEnd = (Color){180, 0, 0, 255};
                    } else {
                        menuGradStart = (Color){200, 20, 20, 255};
                        menuGradEnd = (Color){140, 0, 0, 255};
                    }
                    
                    DrawRectangleGradientH(menuButton.x, menuButton.y, menuButton.width, menuButton.height, 
                                         menuGradStart, menuGradEnd);
                    DrawRectangleLines(menuButton.x, menuButton.y, menuButton.width, menuButton.height, WHITE);
                    
                    // Home symbol inside menu button
                    DrawRectangle(menuButton.x + 30, menuButton.y + resumeButton.height/2 - 8, 30, 17, WHITE);
                    DrawTriangle(
                        (Vector2){menuButton.x + 20, menuButton.y + resumeButton.height/2},
                        (Vector2){menuButton.x + 45, menuButton.y + resumeButton.height/2 - 20},
                        (Vector2){menuButton.x + 70, menuButton.y + resumeButton.height/2},
                        WHITE);
                    
                    // Draw button text with better style
                    DrawText("MAIN MENU", 
                        menuButton.x + 75, 
                        menuButton.y + menuButton.height/2 - 15, 
                        30, WHITE);
                    
                    // Animated keyboard controls reminder
                    float alpha = 0.6f + 0.4f * sinf(GetTime() * 4.0f);
                    DrawText("Press SPACE to resume", 
                        SCREEN_WIDTH/2 - MeasureText("Press SPACE to resume", 22)/2, 
                        590, 
                        22, ColorAlpha(WHITE, alpha));}
            }
            break;            case GAME_OVER: {
                // Background effect: animated stars with color variations
                for (int i = 0; i < numStars; i++) {
                    float starSize = (i % 4 == 0) ? 3.0f : ((i % 3 == 0) ? 2.0f : 1.2f);
                    Color starColor = (i % 5 == 0) ? YELLOW : ((i % 7 == 0) ? SKYBLUE : WHITE);
                    DrawCircle(stars[i].x, stars[i].y, starSize, ColorAlpha(starColor, 0.7f + 0.3f * sinf(GetTime() * 2 + i)));
                }
                  
                // Create a dynamic game over screen with gradient background
                DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                                    ColorAlpha(BLACK, 0.8f), ColorAlpha(DARKBLUE, 0.5f));
                  
                // Animated scanlines effect for retro feel
                for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
                    DrawRectangle(0, i, SCREEN_WIDTH, 1, ColorAlpha(BLACK, 0.15f));
                }
                
                // Game over title with pixel-like animated effects
                const char* gameOverText = "GAME OVER";
                float bounceHeight = 5.0f * sinf(GetTime() * 2.5f);
                Color titleGlow = ColorFromHSV(fmodf(GetTime() * 30, 360.0f), 0.8f, 1.0f);
                
                // Draw shadow for depth
                DrawText(gameOverText, 
                    SCREEN_WIDTH/2 - MeasureText(gameOverText, 80)/2 + 3, 
                    120 + 3, 
                    80, BLACK);
                
                // Main pulsing title
                DrawText(gameOverText, 
                    SCREEN_WIDTH/2 - MeasureText(gameOverText, 80)/2, 
                    120 + bounceHeight, 
                    80, titleGlow);
                
                // Winner announcement with animated effects
                if (playerScore > computerScore) {
                    // Player wins with celebration effects
                    const char* winText = TextFormat("%s WINS!", playerName);
                    
                    // Victory glow effect
                    float glowSize = 4.0f + 2.0f * sinf(GetTime() * 5.0f);
                    for (float i = glowSize; i > 0; i -= 1.0f) {
                        DrawText(winText, 
                            SCREEN_WIDTH/2 - MeasureText(winText, 60)/2, 
                            220, 
                            60, ColorAlpha(GREEN, 0.1f * i));
                    }
                    
                    DrawText(winText, 
                        SCREEN_WIDTH/2 - MeasureText(winText, 60)/2, 
                        220, 
                        60, GREEN);
                } else {
                    // Computer wins with intimidating effects
                    const char* loseText = "COMPUTER WINS!";
                    
                    // Danger/defeat effect
                    float glowSize = 3.0f + 2.0f * sinf(GetTime() * 8.0f);
                    for (float i = glowSize; i > 0; i -= 1.0f) {
                        DrawText(loseText, 
                            SCREEN_WIDTH/2 - MeasureText(loseText, 60)/2, 
                            220, 
                            60, ColorAlpha(RED, 0.15f * i));
                    }
                    
                    DrawText(loseText, 
                        SCREEN_WIDTH/2 - MeasureText(loseText, 60)/2, 
                        220, 
                        60, RED);
                }                // Retro-style arcade score display
                Rectangle scoreBox = { SCREEN_WIDTH/2 - 220, 310, 440, 130 };
                
                // Arcade-style score display with glowing borders
                DrawRectangleGradientV(scoreBox.x, scoreBox.y, scoreBox.width, scoreBox.height, 
                                     ColorAlpha(DARKBLUE, 0.7f), ColorAlpha(DARKPURPLE, 0.7f));
                
                // Animated border
                float borderGlow = 0.7f + 0.3f * sinf(GetTime() * 3.0f);
                Color borderColor = playerScore > computerScore ? 
                                  ColorAlpha(GREEN, borderGlow) : 
                                  ColorAlpha(RED, borderGlow);
                                  
                DrawRectangleLines(scoreBox.x, scoreBox.y, scoreBox.width, scoreBox.height, borderColor);
                
                // Draw digital-style separating line
                DrawLineEx(
                    (Vector2){scoreBox.x + 30, scoreBox.y + 65},
                    (Vector2){scoreBox.x + scoreBox.width - 30, scoreBox.y + 65},
                    2, ColorAlpha(LIGHTGRAY, 0.8f));
                
                // Show player name and score with arcade style
                DrawText(TextFormat("%s", playerName), 
                    scoreBox.x + 30, 
                    scoreBox.y + 20, 
                    30, WHITE);
                
                // Create a digital score display effect
                Color scoreColor = playerScore > computerScore ? GREEN : WHITE;
                DrawText(TextFormat("%d", playerScore), 
                    scoreBox.x + scoreBox.width - 90, 
                    scoreBox.y + 15, 
                    45, scoreColor);
                
                // Computer score display
                DrawText("COMPUTER", 
                    scoreBox.x + 30, 
                    scoreBox.y + 75, 
                    30, RED);
                    
                // Computer score with digital effect
                Color compScoreColor = computerScore > playerScore ? RED : WHITE;
                DrawText(TextFormat("%d", computerScore), 
                    scoreBox.x + scoreBox.width - 90, 
                    scoreBox.y + 75, 
                    45, compScoreColor);
                  // Arcade-style buttons for options
                Rectangle replayButton = { SCREEN_WIDTH/2 - 210, 465, 200, 55 };
                Rectangle diffButton = { SCREEN_WIDTH/2 + 10, 465, 200, 55 };
                
                Vector2 mousePos = GetMousePosition();
                bool replayHover = CheckCollisionPointRec(mousePos, replayButton);
                bool diffHover = CheckCollisionPointRec(mousePos, diffButton);
                
                // Draw "Play Again" button with retro style and hover effects
                DrawRectangleGradientH(replayButton.x, replayButton.y, replayButton.width, replayButton.height,
                                     ColorAlpha(replayHover ? LIME : GREEN, 0.9f),
                                     ColorAlpha(replayHover ? GREEN : DARKGREEN, 0.9f));
                DrawRectangleLines(replayButton.x, replayButton.y, replayButton.width, replayButton.height, WHITE);
                
                float replayScale = replayHover ? 1.05f : 1.0f;
                DrawText("PLAY AGAIN", 
                    replayButton.x + replayButton.width/2 - MeasureText("PLAY AGAIN", 22 * replayScale)/2, 
                    replayButton.y + replayButton.height/2 - 11 * replayScale, 
                    22 * replayScale, WHITE);
                
                // Draw key hint with animation
                float hintAlpha = 0.6f + 0.4f * sinf(GetTime() * 3.0f);
                DrawText("(R)", 
                    replayButton.x + replayButton.width/2 - MeasureText("(R)", 16)/2,
                    replayButton.y + 38,
                    16, ColorAlpha(WHITE, hintAlpha));
                
                // Draw "Change Difficulty" button
                DrawRectangleGradientH(diffButton.x, diffButton.y, diffButton.width, diffButton.height,                                     ColorAlpha(diffHover ? GOLD : ORANGE, 0.9f),
                                     ColorAlpha(diffHover ? ORANGE : (Color){180, 80, 0, 255}, 0.9f));
                DrawRectangleLines(diffButton.x, diffButton.y, diffButton.width, diffButton.height, WHITE);
                
                float diffScale = diffHover ? 1.05f : 1.0f;
                DrawText("DIFFICULTY", 
                    diffButton.x + diffButton.width/2 - MeasureText("DIFFICULTY", 22 * diffScale)/2, 
                    diffButton.y + diffButton.height/2 - 11 * diffScale, 
                    22 * diffScale, BLACK);
                
                // Draw key hint with animation
                DrawText("(SPACE)", 
                    diffButton.x + diffButton.width/2 - MeasureText("(SPACE)", 16)/2,
                    diffButton.y + 38,
                    16, ColorAlpha(BLACK, hintAlpha));
                  // Difficulty info with arcade cabinet style
                const char* gameOverDiffText = "";
                Color gameOverDiffColor;
                
                switch(currentDifficulty) {
                    case EASY:
                        gameOverDiffText = "EASY MODE";
                        gameOverDiffColor = GREEN;
                        break;
                    case MEDIUM:
                        gameOverDiffText = "MEDIUM MODE";
                        gameOverDiffColor = YELLOW;
                        break;
                    case HARD:
                        gameOverDiffText = "HARD MODE";
                        gameOverDiffColor = ORANGE;
                        break;
                    case IMPOSSIBLE:
                        gameOverDiffText = "IMPOSSIBLE MODE";
                        gameOverDiffColor = RED;
                        break;
                }
                
                // Show difficulty in a retro arcade style box
                Rectangle diffBox = { SCREEN_WIDTH / 2 - 170, 540, 340, 50 };                // Create a glowing arcade cabinet style difficulty display
                float glowIntensity = 0.7f + 0.3f * sinf(GetTime() * 2.0f);
                DrawRectangleGradientH(diffBox.x, diffBox.y, diffBox.width, diffBox.height, 
                                     ColorAlpha(BLACK, 0.7f), ColorAlpha(DARKBLUE, 0.7f));
                DrawRectangleLines(diffBox.x, diffBox.y, diffBox.width, diffBox.height, ColorAlpha(gameOverDiffColor, glowIntensity));
                
                // Pixelated decoration at edges
                for (int i = 0; i < 6; i++) {
                    DrawRectangle(diffBox.x + 10 + (i * 10), diffBox.y - 5, 5, 5, 
                                ColorAlpha(gameOverDiffColor, 0.7f));
                    DrawRectangle(diffBox.x + diffBox.width - 60 + (i * 10), diffBox.y - 5, 5, 5, 
                                ColorAlpha(gameOverDiffColor, 0.7f));
                }
                
                // Draw difficulty text with shadow for depth
                DrawText(gameOverDiffText,
                    diffBox.x + diffBox.width/2 - MeasureText(gameOverDiffText, 28) / 2 + 2, 
                    diffBox.y + 11 + 2, 
                    28, BLACK);
                    
                DrawText(gameOverDiffText,
                    diffBox.x + diffBox.width/2 - MeasureText(gameOverDiffText, 28) / 2, 
                    diffBox.y + 11, 
                    28, gameOverDiffColor);
                  // Credits with classic arcade style
                float creditsAlpha = 0.7f + 0.3f * sinf(GetTime() * 1.5f);
                DrawText("THANKS FOR PLAYING!",
                    SCREEN_WIDTH/2 - MeasureText("THANKS FOR PLAYING!", 24) / 2,
                    610, 
                    24, ColorAlpha(WHITE, creditsAlpha));
                  // Handle mouse clicks for buttons
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    if (replayHover) {
                        currentState = GAMEPLAY;
                        ResetBall(0);
                        playerScore = 0; computerScore = 0;
                    } else if (diffHover) {
                        currentState = DIFFICULTY_SELECT;
                    }
                }
            }
            break;
        }
        
        EndMode2D();
        
        // Draw FPS counter
        DrawFPS(10, 10);
        
    EndDrawing();
}
    