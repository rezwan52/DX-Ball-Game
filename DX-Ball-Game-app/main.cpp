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

int windowWidth = 800, windowHeight = 600;
float paddleX = 350, paddleY = 30, paddleW = 100, paddleH = 15;
float ballX = 400, ballY = 200, ballDX = 3.0f, ballDY = 3.0f, ballR = 8;
int lives = 2, score = 0;
bool gameOver = false;
vector<Brick> bricks;

float frameRate = 1000 / 60.0f; // 60 FPS
float speedTimer = 0;
float speedIncreaseInterval = 10.0f; // every 10 seconds
float speedIncrement = 0.5f; // increase speed by 0.5 every 10s

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
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

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

    // HUD
    glColor3f(1, 1, 1);
    drawText(10, 570, "Score: " + to_string(score) + "   Lives: " + to_string(lives));

    // Game Over popup
    if (gameOver) {
        glColor3f(0, 0, 0);
        drawRect(200, 200, 400, 200);
        glColor3f(1, 1, 1);
        drawText(330, 320, "GAME OVER");
        drawText(340, 290, "Score: " + to_string(score));

        // Restart button
        glColor3f(0, 0.7f, 0);
        drawRect(250, 230, 120, 40);
        glColor3f(1, 1, 1);
        drawText(280, 245, "Restart");

        // Exit button
        glColor3f(0.8f, 0, 0);
        drawRect(430, 230, 120, 40);
        glColor3f(1, 1, 1);
        drawText(465, 245, "Exit");
    }

    glutSwapBuffers();
}

// Mouse click for game over window
void mouseClick(int button, int state, int x, int y) {
    if (!gameOver) return;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int yInverted = windowHeight - y;
        // Restart button area
        if (x >= 250 && x <= 370 && yInverted >= 230 && yInverted <= 270) {
            resetGame();
        }
        // Exit button area
        if (x >= 430 && x <= 550 && yInverted >= 230 && yInverted <= 270) {
            exit(0);
        }
    }
}

void updateBall() {
    if (gameOver) return;

    // Ball movement
    ballX += ballDX;
    ballY += ballDY;

    // Speed timer
    speedTimer += 1.0f / 60.0f; // approx seconds
    if (speedTimer >= speedIncreaseInterval) {
        if (fabs(ballDX) < 10.0f) { // speed limit
            ballDX *= 1.1f;
            ballDY *= 1.1f;
        }
        speedTimer = 0;
    }

    // Wall collisions
    if (ballX - ballR < 0 || ballX + ballR > windowWidth) ballDX = -ballDX;
    if (ballY + ballR > windowHeight) ballDY = -ballDY;

    // Paddle collision
    if (ballX > paddleX && ballX < paddleX + paddleW &&
        ballY - ballR < paddleY + paddleH && ballDY < 0) {
        ballDY = -ballDY;
    }

    // Brick collisions
    for (auto &b : bricks) {
        if (b.alive &&
            ballX > b.x && ballX < b.x + b.w &&
            ballY > b.y && ballY < b.y + b.h) {
            b.hits--;
            if (b.hits <= 0) {
                b.alive = false;
                score += 5;
                PlaySound(TEXT("D:\\UNIVERSITY\\4.2 Semester\\Graphics_Lab\\DX-Ball-Game-Project\\brick.wav.wav"), NULL, SND_ASYNC | SND_FILENAME);
            }
            ballDY = -ballDY;
            break;
        }
    }

    // Ball falls
    if (ballY - ballR < 0) {
        lives--;
        if (lives <= 0) {
            gameOver = true;
            PlaySound(TEXT("D:\\UNIVERSITY\\4.2 Semester\\Graphics_Lab\\DX-Ball-Game-Project\\gameover.wav.wav"), NULL, SND_ASYNC | SND_FILENAME);
        } else {
            resetBall();
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
    updateBall();
    glutPostRedisplay();
    glutTimerFunc(frameRate, timer, 0);
}

void init() {
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    initBricks();
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
