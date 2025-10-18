#include <GL/glut.h>
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <ctime>
#include <algorithm>



#pragma comment(lib, "winmm.lib")

using namespace std;

struct Brick
{
    float x, y, w, h;
    int hits;
    bool alive;
    float r, g, b;
};

struct DropItem
{
    float x, y, w, h;
    int type; // 0=extra life, 1=fireball, 2=shrink paddle, 3=grow paddle, 4=immediate death
    bool active;
};
vector<DropItem> drops;


int level = 1;
int highScore = 0; // global


int windowWidth = 800, windowHeight = 600;
float paddleX = 350, paddleY = 30, paddleW = 100, paddleH = 15;
float ballX = 400, ballY = 200, ballDX = 3.0f, ballDY = 3.0f, ballR = 8;
int lives = 2, score = 0;
bool gameOver = false;
vector<Brick> bricks;
GLuint bgTexture;

bool showLevelUp = false;
float levelUpTimer = 0.0f;
float levelUpDuration = 4.0f;

// ---- New feature variables ----
bool ballThroughBricks = false;     // Fireball বা through-brick power সক্রিয় কিনা
bool enableShooting = false;        // Paddle কি bullet shoot করতে পারবে?
float fireballTimer = 0.0f;         // Fireball power এর টাইমার
float fireballDuration = 5.0f;      // Fireball power কত সেকেন্ড থাকবে

// Bullets data
std::vector<float> bulletsX;        // প্রতিটি bullet এর X অবস্থান
std::vector<float> bulletsY;        // প্রতিটি bullet এর Y অবস্থান
float bulletSpeed = 10.0f;          // Bullet এর গতি

float frameRate = 1000 / 60.0f; // 60 FPS
float speedTimer = 0;
float speedIncreaseInterval = 10.0f; // every 10 seconds
float speedIncrement = 0.5f; // increase speed by 0.5 every 10s

GLuint loadBMP(const char* filename);

float btnX = 350; // text X position (center adjust)
float btnY = 570; // text Y position
string btnText = "PAUSE";
float textHeight = 18; // bitmap font height


float pauseX = 350;
float pauseY = 570;
float pauseWidth = 50;   // initial approx, later update
float pauseHeight = 18;
string pauseText = "PAUSE";

float exitX, exitY, exitWidth, exitHeight;
string exitText = "EXIT";




bool bgMusicPlaying = false;





void playEffectSound(const char* file)
{
    PlaySound(TEXT(file), NULL, SND_ASYNC | SND_NODEFAULT | SND_FILENAME);
}



void updateHighScore()
{
    if (score > highScore) highScore = score;
}


// Initialize bricks
void initBricks()
{
    bricks.clear();
    srand(time(0));
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            Brick b;
            b.x = 60 + j * 70.0f;
            b.y = 400 + i * 25.0f;
            b.w = 60;
            b.h = 20;
            b.hits = 1;
            b.alive = true;

            // DX Ball-like bright colors
            float colors[6][3] =
            {
                {1.0f, 0.3f, 0.3f}, // red
                {0.3f, 0.6f, 1.0f}, // blue
                {0.3f, 1.0f, 0.4f}, // green
                {1.0f, 1.0f, 0.3f}, // yellow
                {1.0f, 0.5f, 0.9f}, // pink
                {0.8f, 0.4f, 1.0f}  // purple
            };
            int ci = rand() % 6;
            b.r = colors[ci][0];
            b.g = colors[ci][1];
            b.b = colors[ci][2];

            bricks.push_back(b);
        }
    }
}

void drawRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawCircle(float cx, float cy, float r)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i++)
    {
        float rad = i * 3.14159f / 180.0f;
        glVertex2f(cx + cos(rad) * r, cy + sin(rad) * r);
    }
    glEnd();
}

void drawText(float x, float y, const string &text)
{
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}


void drawBackgroundImage()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bgTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(windowWidth, 0);
    glTexCoord2f(1, 1);
    glVertex2f(windowWidth, windowHeight);
    glTexCoord2f(0, 1);
    glVertex2f(0, windowHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


void resetBall()
{
    ballX = windowWidth / 2;
    ballY = 150;
    ballDX = 3.0f;
    ballDY = 3.0f;
    speedTimer = 0;
}

void resetGame()
{
    score = 0;
    lives = 2;
    gameOver = false;
    initBricks();
    resetBall();
}

// Draw everything
enum GameState { MENU, WAIT_TO_START, PLAYING, PAUSED, GAME_OVER,HELP };
GameState gameState = MENU;

// Function to check if all bricks are destroyed
bool allBricksDestroyed()
{
    for (auto &b : bricks)
    {
        if (b.alive) return false;
    }
    return true;
}

// Function to reset the current level
void resetLevel()
{
    initBricks();     // নতুন level এর জন্য bricks reset করো
    resetBall();

}
// ===================================================================


void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    drawBackgroundImage();

    // ===== MENU STATE =====
    if (gameState == MENU)
    {

        glColor3f(1, 1, 1);
        string title = "DX-BALL GAME";
        drawText(330, 400, title);

        // Start Button
        glColor3f(0, 0.7f, 0);
        drawRect(300, 300, 200, 50);
        glColor3f(1, 1, 1);
        drawText(340, 320, "START GAME");

        // Exit Button
        glColor3f(0.8f, 0, 0);
        drawRect(300, 220, 200, 50);
        glColor3f(1, 1, 1);
        drawText(372, 240, "EXIT");

        // Help Button
        glColor3f(0.3f, 0.3f, 1.0f);
        drawRect(300, 150, 200, 50);
        glColor3f(1, 1, 1);
        drawText(370, 170, "HELP");

        // Developer credits
        glColor3f(1, 1, 1);
        string line1 = "Developed by";
        string line2 = "Rezwan Ahmed & Sabbir Hossain";

        float charWidth = 9.0f;
        float textWidth1 = line1.length() * charWidth;
        float textX1 = (windowWidth - textWidth1) / 2;
        float textY1 = 55;
        drawText(textX1, textY1, line1);

        float textWidth2 = line2.length() * charWidth;
        float textX2 = (windowWidth - textWidth2) / 2;
        float textY2 = 30;
        drawText(textX2, textY2, line2);
    }

    // ===== GAME STATES =====
    else if (gameState == WAIT_TO_START || gameState == PLAYING || gameState == PAUSED)
    {

        // --- Paddle ---
        glColor3f(0.2f, 0.6f, 1.0f);
        drawRect(paddleX, paddleY, paddleW, paddleH);

        // --- Ball ---
        if (ballThroughBricks) glColor3f(1.0f, 0.4f, 0.0f); // fireball = orange
        else glColor3f(1, 0, 0); // normal = red
        drawCircle(ballX, ballY, ballR);

        // --- Bricks ---
        for (auto &b : bricks)
        {
            if (b.alive)
            {
                glColor3f(b.r, b.g, b.b);
                drawRect(b.x, b.y, b.w, b.h);
            }
        }

        // --- Falling Drops ---
        for (auto &d : drops)
        {
            if (d.active)
            {
                switch (d.type)
                {
                case 0:
                    glColor3f(1, 1, 0);
                    break; // yellow - extra life
                case 1:
                    glColor3f(1, 0, 0);
                    break; // red - fireball
                case 2:
                    glColor3f(0, 0, 1);
                    break; // blue - shrink
                case 3:
                    glColor3f(0, 1, 0);
                    break; // green - grow
                case 4:
                    glColor3f(1, 0.5f, 0);
                    break; // orange - death
                case 5:
                    glColor3f(0.5f, 0.2f, 1.0f);
                    break; // purple - shooting
                case 6:
                    glColor3f(1, 0.2f, 0.8f);
                    break; // pink - speed
                }
                drawRect(d.x, d.y, d.w, d.h);
            }
        }

        // --- Bullets (if shooting active) ---
        if (enableShooting)
        {
            glColor3f(1, 1, 0); // yellow bullets
            for (int i = 0; i < bulletsX.size(); i++)
            {
                drawRect(bulletsX[i] - 2, bulletsY[i], 4, 10);
            }
        }

        // --- HUD (Score, Lives, Level) ---
        glColor3f(1, 1, 1);
        drawText(10, 570, "Score: " + to_string(score) + "   Lives: " + to_string(lives));
        drawText(680, 570, "Level: " + to_string(level));

        // --- Pause / Resume / Exit ---
        if (gameState == PLAYING || gameState == PAUSED)
        {
            float btnX = 350;
            float btnY = 570;

            if (gameState == PLAYING)
                drawText(btnX, btnY, "PAUSE");
            else
                drawText(btnX, btnY, "RESUME");

            drawText(440, btnY, "EXIT");
        }

        // --- Wait to Start message ---
        if (gameState == WAIT_TO_START)
        {
            glColor3f(1, 1, 1);
            drawText(320, 300, "CLICK TO START");
        }

        // --- LEVEL UP animation ---
        if (showLevelUp)
        {
            glColor3f(1, 1, 0);
            drawText(350, 320, "LEVEL UP!");
        }
    }

    // ===== GAME OVER =====
    else if (gameState == GAME_OVER)
    {
        glColor3f(0, 0, 0);
        drawRect(200, 200, 400, 250);
        glColor3f(1, 1, 1);
        drawText(325, 400, "High Score: " + to_string(highScore));
        drawText(355, 360, "Level: " + to_string(level));
        drawText(330, 320, "GAME OVER");
        drawText(340, 290, "Score: " + to_string(score));

        // Restart Button
        glColor3f(0, 0.7f, 0);
        drawRect(250, 230, 120, 40);
        glColor3f(1, 1, 1);
        drawText(280, 245, "Restart");

        // Exit Button
        glColor3f(0.8f, 0, 0);
        drawRect(430, 230, 120, 40);
        glColor3f(1, 1, 1);
        drawText(465, 245, "Exit");
    }

    // ===== HELP SCREEN =====
    else if (gameState == HELP)
    {
        glColor3f(1, 1, 1);
        drawText(100, 550, "DX-Ball Game Controls & Perks:");
        drawText(100, 510, "- Use LEFT/RIGHT arrows or mouse to move paddle");
        drawText(100, 470, "- Break all bricks to earn score and advance levels");
        drawText(100, 430, "Perks & Damages:");
        drawText(120, 390, "Yellow = Extra Life (+1 life)");
        drawText(120, 360, "Red = Fireball (pass through bricks)");
        drawText(120, 330, "Blue = Shrink Paddle");
        drawText(120, 300, "Green = Grow Paddle");
        drawText(120, 270, "Orange = Immediate Death");
        drawText(120, 240, "Purple = Shooting Paddle");
        drawText(120, 210, "Pink = Speed Boost");

        // Back Button
        glColor3f(0.3f, 0.3f, 1.0f);
        drawRect(650, 30, 100, 40);
        glColor3f(1, 1, 1);
        drawText(675, 50, "BACK");
    }

    glutSwapBuffers();
}

// Mouse click for game over window
void mouseClick(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    int yInverted = 600 - y; // assuming window height = 600

    if (gameState == MENU)
    {
        // Start Game
        if (x >= 300 && x <= 500 && yInverted >= 300 && yInverted <= 350)
        {
            resetLevel();
            score = 0;
            lives = 3;
            level = 1;
            gameOver = false;
            gameState = WAIT_TO_START;


        }

        // Exit
        if (x >= 300 && x <= 500 && yInverted >= 220 && yInverted <= 270)
        {
            exit(0);
        }
        // Help button
        if (x >= 300 && x <= 500 && yInverted >= 150 && yInverted <= 200)
        {
            gameState = HELP;
        }
    }

    else if (gameState == WAIT_TO_START)
    {
        gameState = PLAYING; // first click starts game


    }

    else if (gameState == PLAYING || gameState == PAUSED)
    {
        string btnText = (gameState == PLAYING) ? "PAUSE" : "RESUME";

        // Approx text width & height for GLUT_BITMAP_HELVETICA_18
        float charWidth = 9.0f;
        float textWidth = btnText.length() * charWidth;
        float textHeight = 18.0f;

        // Center the text horizontally
        float textX = (windowWidth - textWidth) / 2;
        float textY = 570; // bottom of text

        // Convert mouse y to OpenGL coordinates
        float yInverted = windowHeight - y;

        // Check if mouse click is inside text bounding box
        if (x >= textX && x <= textX + textWidth &&
                yInverted >= textY && yInverted <= textY + textHeight)
        {
            // Toggle game state
            if (gameState == PLAYING) gameState = PAUSED;
            else gameState = PLAYING;
        }
        // Exit click area
        float exitX = pauseX + pauseWidth + 20;
        float exitY = pauseY;
        float exitWidth = 4 * 12; // "EXIT" ~ 4 chars
        float exitHeight = 18;

        if (x >= exitX && x <= exitX + exitWidth &&
                yInverted >= exitY - exitHeight && yInverted <= exitY)
        {
            exit(0); // Game exits
        }
    }



    else if (gameState == GAME_OVER)
    {
        // Restart button
        if (x >= 250 && x <= 370 && yInverted >= 230 && yInverted <= 270)
        {
            score = 0;
            lives = 3;
            level = 1;
            resetLevel();
            gameOver = false;
            gameState = WAIT_TO_START;

        }

        // Exit button
        if (x >= 430 && x <= 550 && yInverted >= 230 && yInverted <= 270)
        {
            exit(0);
        }

    }
    else if (gameState == HELP)
    {
        if(x >= 650 && x <= 750 && yInverted >= 30 && yInverted <= 70)
        {
            gameState = MENU; // return to main menu
        }
    }

}


void updateBall()
{
    if (gameOver) return;

    // Ball movement (skip movement during LEVEL UP display)
    if (!showLevelUp)
    {
        ballX += ballDX;
        ballY += ballDY;
    }

    // Speed timer
    if (!showLevelUp)
    {
        speedTimer += 1.0f / 60.0f; // approx seconds
        if (speedTimer >= speedIncreaseInterval)
        {
            if (fabs(ballDX) < 10.0f)   // speed limit
            {
                ballDX *= 1.1f;
                ballDY *= 1.1f;
            }
            speedTimer = 0;
        }
    }

    // Wall collisions
    if (!showLevelUp)
    {
        if (ballX - ballR < 0 || ballX + ballR > windowWidth) ballDX = -ballDX;
        if (ballY + ballR > windowHeight) ballDY = -ballDY;
    }

    // Paddle collision
    if (!showLevelUp)
    {
        if (ballX > paddleX && ballX < paddleX + paddleW &&
                ballY - ballR < paddleY + paddleH && ballDY < 0)
        {
            ballDY = -ballDY;
        }
    }

    // 🔥 Brick collisions (with Fireball mode)
    if (!showLevelUp)
    {
        for (auto &b : bricks)
        {
            if (b.alive &&
                    ballX > b.x && ballX < b.x + b.w &&
                    ballY > b.y && ballY < b.y + b.h)
            {

                b.hits--;
                if (b.hits <= 0)
                {
                    b.alive = false;
                    score += 5;

                    playEffectSound("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/bricks-fall.wav");

                    if (rand() % 5 == 0)   // 20% chance of drop
                    {
                        DropItem item;
                        item.x = b.x + b.w / 2;
                        item.y = b.y;
                        item.w = 15;
                        item.h = 15;
                        item.type = rand() % 7; // 0–6 → more perk types
                        item.active = true;
                        drops.push_back(item);
                    }
                }

                // If Fireball active, pass through bricks (no bounce)
                if (!ballThroughBricks)
                {
                    if (ballX < b.x || ballX > b.x + b.w)
                        ballDX = -ballDX;
                    else
                        ballDY = -ballDY;
                }

                break;
            }
        }
    }

    // 🧩 Drops collision and effects
    for (auto &d : drops)
    {
        if (d.active)
        {
            d.y -= 3; // falling speed
            if (d.x + d.w > paddleX && d.x < paddleX + paddleW &&
                    d.y <= paddleY + paddleH && d.y + d.h >= paddleY)
            {

                switch (d.type)
                {
                case 0:
                    lives++;
                    break; // extra life
                case 1: // fireball
                    ballThroughBricks = true;
                    fireballTimer = 0;
                    break;
                case 2:
                    paddleW *= 0.7f;
                    break; // shrink
                case 3:
                    paddleW *= 1.3f;
                    break; // grow

                // ⚡ Fixed part: Instant death now reduces life instead of full game over
                case 4:
                    lives--;
                    playEffectSound("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/error-fail.wav");
                    if (lives <= 0)
                    {
                        updateHighScore();
                        gameOver = true;

                        playEffectSound("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/gameover.wav");
                    }
                    else
                    {
                        resetBall();
                    }
                    break;

                case 5:
                    enableShooting = true;
                    break; // enable shooting
                case 6: // super speed temporarily
                    ballDX *= 1.5f;
                    ballDY *= 1.5f;
                    break;
                }
                d.active = false; // collected
            }
        }
    }

    // 🔥 Fireball timer (auto-off after few seconds)
    if (ballThroughBricks)
    {
        fireballTimer += 1.0f / 60.0f;
        if (fireballTimer > 10.0f)   // active for 10 sec
        {
            ballThroughBricks = false;
        }
    }

    // 🔫 Shooting logic (bullets)
    if (enableShooting)
    {
        for (int i = 0; i < bulletsX.size(); i++)
        {
            bulletsY[i] += bulletSpeed;
            for (auto &b : bricks)
            {
                if (b.alive && bulletsX[i] > b.x && bulletsX[i] < b.x + b.w &&
                        bulletsY[i] > b.y && bulletsY[i] < b.y + b.h)
                {
                    b.hits--;
                    if (b.hits <= 0)
                    {
                        b.alive = false;
                        score += 5;
                        playEffectSound("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/bricks-fall.wav");
                    }
                    bulletsY[i] = windowHeight + 20; // remove bullet
                }
            }
        }
    }

    // Remove bullets that left the screen
    bulletsX.erase(
        remove_if(bulletsX.begin(), bulletsX.end(),
                  [&](float x, int idx = 0)
    {
        return bulletsY[idx++] > windowHeight;
    }),
    bulletsX.end());

    bulletsY.erase(
        remove_if(bulletsY.begin(), bulletsY.end(),
                  [&](float y)
    {
        return y > windowHeight;
    }),
    bulletsY.end());

    // Ball falls below screen
    if (!showLevelUp)
    {
        if (ballY - ballR < 0)
        {
            lives--;
            if (lives <= 0)
            {
                updateHighScore();
                gameOver = true;
                playEffectSound("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/gameover.wav");
            }
            else
            {
                playEffectSound("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/error-fail.wav");
                resetBall();
            }
        }
    }

    // Check if all bricks destroyed → next level
    bool allDestroyed = true;
    for (auto &b : bricks)
    {
        if (b.alive)
        {
            allDestroyed = false;
            break;
        }
    }

    if (allDestroyed && !showLevelUp)
    {
        showLevelUp = true;
        levelUpTimer = 0.0f;
        playEffectSound("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/levelup.wav");
    }

    // Update levelUpTimer
    if (showLevelUp)
    {
        levelUpTimer += 1.0f / 60.0f; // approx seconds
        if (levelUpTimer >= levelUpDuration)
        {
            showLevelUp = false;
            initBricks();
            resetBall();
        }
    }
}


// Keyboard movement
void keyboard(int key, int, int)
{
    if (key == GLUT_KEY_LEFT && paddleX > 0)
        paddleX -= 20;
    if (key == GLUT_KEY_RIGHT && paddleX + paddleW < windowWidth)
        paddleX += 20;
}

// Mouse move (for paddle)
void mouseMove(int x, int y)
{
    paddleX = x - paddleW / 2;
    if (paddleX < 0) paddleX = 0;
    if (paddleX + paddleW > windowWidth) paddleX = windowWidth - paddleW;
}

// Timer for update loop
void timer(int)
{
    if (gameState == PLAYING)
    {
        updateBall();

        // Level complete check
        if (allBricksDestroyed())
        {
            level++;
            resetLevel();
            playEffectSound("D:\\UNIVERSITY\\4.2 Semester\\Graphics_Lab\\DX-Ball-Game-Project\\levelup.wav");
            gameState = WAIT_TO_START;
        }

        // Game over check
        if (lives <= 0)
        {
            gameState = GAME_OVER;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}
void init()
{
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glEnable(GL_TEXTURE_2D); // Enable texture

    bgTexture = loadBMP("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/dx-ball-bg.bmp");

    initBricks();
}

GLuint loadBMP(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (!file) return 0;

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int imageSize = 3 * width * height;
    unsigned char* data = new unsigned char[imageSize];
    fread(data, sizeof(unsigned char), imageSize, file);
    fclose(file);

    // BGR → RGB
    for (int i = 0; i < imageSize; i += 3)
    {
        std::swap(data[i], data[i + 2]);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;
    return texture;
}



int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("DX Ball - Modern OpenGL Version");

    init();

    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutPassiveMotionFunc(mouseMove);
    glutMouseFunc(mouseClick);
    glutTimerFunc(frameRate, timer, 0);

    glutMainLoop();
    return 0;
}
