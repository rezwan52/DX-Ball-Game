#include <GL/glut.h>
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <ctime>

#pragma comment(lib, "winmm.lib")

using namespace std;

struct Brick {
    float x, y, w, h;
    int hits;
    bool alive;
    float r, g, b;
};

struct DropItem {
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

void updateHighScore() {
    if (score > highScore) highScore = score;
}


// Initialize bricks
void initBricks() {
    bricks.clear();
    srand(time(0));
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 10; j++) {
            Brick b;
            b.x = 60 + j * 70.0f;
            b.y = 400 + i * 25.0f;
            b.w = 60;
            b.h = 20;
            b.hits = 1;
            b.alive = true;

            // DX Ball-like bright colors
            float colors[6][3] = {
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

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i++) {
        float rad = i * 3.14159f / 180.0f;
        glVertex2f(cx + cos(rad) * r, cy + sin(rad) * r);
    }
    glEnd();
}

void drawText(float x, float y, const string &text) {
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}


void drawBackgroundImage() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bgTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(windowWidth, 0);
    glTexCoord2f(1, 1); glVertex2f(windowWidth, windowHeight);
    glTexCoord2f(0, 1); glVertex2f(0, windowHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


void resetBall() {
    ballX = windowWidth / 2;
    ballY = 150;
    ballDX = 3.0f;
    ballDY = 3.0f;
    speedTimer = 0;
}

void resetGame() {
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
bool allBricksDestroyed() {
    for (auto &b : bricks) {
        if (b.alive) return false;
    }
    return true;
}

// Function to reset the current level
void resetLevel() {
    initBricks();     // নতুন level এর জন্য bricks reset করো
    resetBall();

}
// ===================================================================


void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBackgroundImage();


    if (gameState == MENU) {


    glColor3f(1, 1, 1);
    string title = "DX-BALL GAME";
    float titleX = 330;
    float titleY = 400;
    drawText(titleX, titleY, title);

    // ----- Start Button -----
    float startBtnX = 300, startBtnY = 300, startBtnW = 200, startBtnH = 50;
    float startBtnCenterX = startBtnX + startBtnW / 2;
    float startBtnCenterY = startBtnY + startBtnH / 2;

    // Draw button rectangle
    glColor3f(0, 0.7f, 0); // green
    drawRect(startBtnX, startBtnY, startBtnW, startBtnH);

    // Draw text centered
    string startText = "START GAME";
    glColor3f(1, 1, 1);
    drawText(startBtnCenterX - (startText.size() * 9) / 2, startBtnCenterY - 9 / 2, startText);

    // ----- Exit Button -----
    float exitBtnX = 300, exitBtnY = 220, exitBtnW = 200, exitBtnH = 50;
    float exitBtnCenterX = exitBtnX + exitBtnW / 2;
    float exitBtnCenterY = exitBtnY + exitBtnH / 2;

    // Draw button rectangle
    glColor3f(0.8f, 0, 0); // red
    drawRect(exitBtnX, exitBtnY, exitBtnW, exitBtnH);

    // Draw text centered
    string exitText = "EXIT";
    glColor3f(1, 1, 1);
    drawText(exitBtnCenterX - (exitText.size() * 9) / 2, exitBtnCenterY - 9 / 2, exitText);

    // ----- Help Button -----
    glColor3f(0.3f,0.3f,1.0f); // blue
    drawRect(300,150,200,50);  // same size/position style as other buttons
    glColor3f(1,1,1);
    drawText(375,175,"HELP");



    glColor3f(1, 1, 1); // White color

    string line1 = "Developed by";
    string line2 = "Rezwan Ahmed & Sabbir Hossain";

    // Approx width per character (for GLUT_BITMAP_HELVETICA_18)
    float charWidth = 9.0f;

    // Line 1 center
    float textWidth1 = line1.length() * charWidth;
    float textX1 = (windowWidth - textWidth1) / 2;
    float textY1 = 55; // distance from bottom

    drawText(textX1, textY1, line1);

    // Line 2 center
    float textWidth2 = line2.length() * charWidth;
    float textX2 = (windowWidth - textWidth2) / 2;
    float textY2 = 30; // below line1
    drawText(textX2, textY2, line2);

    }



    else if (gameState == WAIT_TO_START || gameState == PLAYING || gameState == PAUSED) {
        // Draw Paddle
        glColor3f(0.2f, 0.6f, 1.0f);
        drawRect(paddleX, paddleY, paddleW, paddleH);

        // Draw Ball
        glColor3f(1, 0, 0);
        drawCircle(ballX, ballY, ballR);

        // Draw Bricks
        for (auto &b : bricks) {
            if (b.alive) {
                glColor3f(b.r, b.g, b.b);
                drawRect(b.x, b.y, b.w, b.h);
            }
        }

        // HUD (Score, Lives, Level)
        glColor3f(1, 1, 1);
        drawText(10, 570, "Score: " + to_string(score) + "   Lives: " + to_string(lives));
        drawText(680, 570, "Level: " + to_string(level));

        if (gameState == PLAYING || gameState == PAUSED) {
            float btnX = 350; // text X position (center adjust)
            float btnY = 570; // text Y position
            float textWidth = btnText.length() * 9; // approx 9px per char
            float textHeight = 18; // bitmap font height

            if (gameState == PLAYING) {
                drawText(btnX, btnY, "PAUSE");
            } else {
                drawText(btnX, btnY, "RESUME");
            }
            // Exit text beside pause/resume
            float exitX = pauseX + pauseText.length() * 12 + 20; // 20 px gap
            float exitY = pauseY;
            drawText(exitX, exitY, "EXIT");
}





        // WAIT_TO_START text
        if (gameState == WAIT_TO_START) {
            glColor3f(1, 1, 1);
            drawText(320, 300, "CLICK TO START");
        }
       ;
    }

    else if (gameState == GAME_OVER) {
        // Game Over Screen
        glColor3f(0, 0, 0);
        drawRect(200, 200, 400, 250);
        glColor3f(1, 1, 1);
        drawText(355, 400,"High Score: " + to_string(highScore));
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
    else if (gameState == HELP) {
        glColor3f(1,1,1);
        drawText(100,550,"DX-Ball Game Controls & Perks:");
        drawText(100,510,"- Use LEFT/RIGHT arrows or mouse to move paddle");
        drawText(100,470,"- Break all bricks to earn score and advance levels");
        drawText(100,430,"Perks & Damages:");
        drawText(120,390,"Yellow = Extra Life (+1 life)");
        drawText(120,360,"Red = Fireball (increases ball speed)");
        drawText(120,330,"Blue = Shrink Paddle");
        drawText(120,300,"Green = Grow Paddle");
        drawText(120,270,"Orange = Immediate Death (lose game instantly)");


        glColor3f(0.3f,0.3f,1.0f); // blue rectangle
        drawRect(650, 30, 100, 40); // X,Y,Width,Height
        glColor3f(1,1,1); // text color
        drawText(675, 50, "BACK");
    }

    glutSwapBuffers();
}

// Mouse click for game over window
void mouseClick(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    int yInverted = 600 - y; // assuming window height = 600

    if (gameState == MENU) {
        // Start Game
        if (x >= 300 && x <= 500 && yInverted >= 300 && yInverted <= 350) {
            resetLevel();
            score = 0;
            lives = 3;
            level = 1;
            gameOver = false;
            gameState = WAIT_TO_START;

        }

        // Exit
        if (x >= 300 && x <= 500 && yInverted >= 220 && yInverted <= 270) {
            exit(0);
        }
        // Help button
        if (x >= 300 && x <= 500 && yInverted >= 150 && yInverted <= 200) {
            gameState = HELP;
        }
    }

    else if (gameState == WAIT_TO_START) {
        gameState = PLAYING; // first click starts game


    }

    else if (gameState == PLAYING || gameState == PAUSED) {
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
        yInverted >= textY && yInverted <= textY + textHeight) {
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
        yInverted >= exitY - exitHeight && yInverted <= exitY) {
        exit(0); // Game exits
    }
}



    else if (gameState == GAME_OVER) {
        // Restart button
        if (x >= 250 && x <= 370 && yInverted >= 230 && yInverted <= 270) {
            score = 0;
            lives = 3;
            level = 1;
            resetLevel();
            gameOver = false;
            gameState = WAIT_TO_START;

        }

        // Exit button
        if (x >= 430 && x <= 550 && yInverted >= 230 && yInverted <= 270) {
            exit(0);
        }

    }
    else if (gameState == HELP) {
    if(x >= 650 && x <= 750 && yInverted >= 30 && yInverted <= 70) {
        gameState = MENU; // return to main menu
    }
}

}


void updateBall() {
    if (gameOver) return;

    // Ball movement (skip movement during LEVEL UP display)
    if (!showLevelUp) {
        ballX += ballDX;
        ballY += ballDY;
    }

    // Speed timer
    if (!showLevelUp) {
        speedTimer += 1.0f / 60.0f; // approx seconds
        if (speedTimer >= speedIncreaseInterval) {
            if (fabs(ballDX) < 10.0f) { // speed limit
                ballDX *= 1.1f;
                ballDY *= 1.1f;
            }
            speedTimer = 0;
        }
    }

    // Wall collisions
    if (!showLevelUp) {
        if (ballX - ballR < 0 || ballX + ballR > windowWidth) ballDX = -ballDX;
        if (ballY + ballR > windowHeight) ballDY = -ballDY;
    }

    // Paddle collision
    if (!showLevelUp) {
        if (ballX > paddleX && ballX < paddleX + paddleW &&
            ballY - ballR < paddleY + paddleH && ballDY < 0) {
            ballDY = -ballDY;
        }
    }

    // Brick collisions
    if (!showLevelUp) {
        for (auto &b : bricks) {
            if (b.alive &&
                ballX > b.x && ballX < b.x + b.w &&
                ballY > b.y && ballY < b.y + b.h) {
                b.hits--;
                if (b.hits <= 0) {
                    b.alive = false;
                    score += 5;
                    PlaySound(TEXT("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/bricks-fall.wav"), NULL, SND_ASYNC | SND_FILENAME);
                    if(rand() % 5 == 0) { // 20% chance
                        DropItem item;
                        item.x = b.x + b.w/2;
                        item.y = b.y;
                        item.w = 15;
                        item.h = 15;
                        item.type = rand() % 5; // pick a perk type
                        item.active = true;
                        drops.push_back(item);
    }
                }
                if (ballX < b.x || ballX > b.x + b.w)
                    ballDX = -ballDX;

                else
                    ballDY = -ballDY;

                break;
            }
        }
    }

    for(auto &d : drops){
    if(d.active){
        d.y -= 3; // falling speed
        if(d.x + d.w > paddleX && d.x < paddleX + paddleW &&
           d.y <= paddleY + paddleH && d.y + d.h >= paddleY){

            switch(d.type){
                case 0: lives++; break; // extra life
                case 1: ballDX *= 1.5f; ballDY *= 1.5f; break; // fireball
                case 2: paddleW *= 0.7f; break; // shrink
                case 3: paddleW *= 1.3f; break; // grow
                case 4: gameOver = true; updateHighScore(); break; // immediate death
            }

            d.active = false; // collected
        }
    }
}

    // Ball falls
    if (!showLevelUp) {
        if (ballY - ballR < 0) {
            lives--;
            if (lives <= 0) {
                updateHighScore();
                gameOver = true;
                PlaySound(TEXT("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/gameover.wav"), NULL, SND_ASYNC | SND_FILENAME);
            } else {
                PlaySound(TEXT("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/error-fail.wav"), NULL, SND_ASYNC | SND_FILENAME);
                resetBall();
            }
        }
    }

    // Check if all bricks destroyed → next level
    bool allDestroyed = true;
    for (auto &b : bricks) {
        if (b.alive) {
            allDestroyed = false;
            break;
        }
    }

    if (allDestroyed && !showLevelUp) { // first detect level up
        level++;
        showLevelUp = true;
        levelUpTimer = 0.0f;
        PlaySound(TEXT("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/levelup.wav"), NULL, SND_ASYNC | SND_FILENAME);

    }

    // Update levelUpTimer
    if (showLevelUp) {
        levelUpTimer += 1.0f / 60.0f; // approx seconds
        if (levelUpTimer >= levelUpDuration) {   // 3 seconds show
            showLevelUp = false;
            initBricks();   // now generate new bricks
            resetBall();    // reset ball
        }
    }
}




// Keyboard movement
void keyboard(int key, int, int) {
    if (key == GLUT_KEY_LEFT && paddleX > 0)
        paddleX -= 20;
    if (key == GLUT_KEY_RIGHT && paddleX + paddleW < windowWidth)
        paddleX += 20;
}

// Mouse move (for paddle)
void mouseMove(int x, int y) {
    paddleX = x - paddleW / 2;
    if (paddleX < 0) paddleX = 0;
    if (paddleX + paddleW > windowWidth) paddleX = windowWidth - paddleW;
}

// Timer for update loop
void timer(int) {
    if (gameState == PLAYING) {
        updateBall();

        // Level complete check
        if (allBricksDestroyed()) {
            level++;
            resetLevel();
            PlaySound(TEXT("D:\\UNIVERSITY\\4.2 Semester\\Graphics_Lab\\DX-Ball-Game-Project\\levelup.wav"), NULL, SND_ASYNC | SND_FILENAME);
            gameState = WAIT_TO_START;
        }

        // Game over check
        if (lives <= 0) {
            gameState = GAME_OVER;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}
void init() {
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glEnable(GL_TEXTURE_2D); // Enable texture

    bgTexture = loadBMP("D:/UNIVERSITY/4.2 Semester/Graphics_Lab/DX-Ball-Game-Project/dx-ball-bg.bmp");

    initBricks();
}

GLuint loadBMP(const char* filename) {
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
    for (int i = 0; i < imageSize; i += 3) {
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



int main(int argc, char **argv) {
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
