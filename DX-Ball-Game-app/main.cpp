#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Brick {
    float x, y, w, h;
    bool alive;
};

int windowWidth = 800, windowHeight = 600;
float paddleX = 350, paddleY = 20, paddleW = 100, paddleH = 15;
float ballX = 400, ballY = 300, ballDX = 0.9f, ballDY = 0.9f, ballR = 8;
int lives = 3, score = 0;
vector<Brick> bricks;

// Initialize bricks
void initBricks() {
    bricks.clear();
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 10; j++) {
            Brick b = { 60 + j * 70.0f, 400 + i * 25.0f, 60, 20, true };
            bricks.push_back(b);
        }
    }
}

// Draw rectangle
void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// Draw circle for ball
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i++) {
        float rad = i * M_PI / 180.0f;
        glVertex2f(cx + std::cos(rad) * r, cy + std::sin(rad) * r);
    }
    glEnd();
}

// Display text
void drawText(float x, float y, const char* str) {
    glRasterPos2f(x, y);
    while (*str) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str++);
    }
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Paddle
    glColor3f(0, 0, 1);
    drawRect(paddleX, paddleY, paddleW, paddleH);

    // Ball
    glColor3f(1, 0, 0);
    drawCircle(ballX, ballY, ballR);

    // Bricks
    for (auto &b : bricks) {
        if (b.alive) {
            glColor3f(1, 0.5, 0);
            drawRect(b.x, b.y, b.w, b.h);
        }
    }

    // Score & Lives
    glColor3f(1, 1, 1);
    string hud = "Score: " + to_string(score) + "   Lives: " + to_string(lives);
    drawText(10, 570, hud.c_str());

    glutSwapBuffers();
}

// Ball + Paddle + Brick updates
void update(int value) {
    ballX += ballDX;
    ballY += ballDY;

    // Wall collisions
    if (ballX - ballR < 0 || ballX + ballR > windowWidth) ballDX = -ballDX;
    if (ballY + ballR > windowHeight) ballDY = -ballDY;

    // Ball falls below
    if (ballY - ballR < 0) {
        lives--;
        ballX = windowWidth / 2;
        ballY = windowHeight / 2;
        ballDX = 0.4f;
        ballDY = 0.4f;
    }

    // Paddle collision
    if (ballX > paddleX && ballX < paddleX + paddleW &&
        ballY - ballR < paddleY + paddleH) {
        ballDY = -ballDY;
    }

    // Brick collisions
    for (auto &b : bricks) {
        if (b.alive &&
            ballX > b.x && ballX < b.x + b.w &&
            ballY > b.y && ballY < b.y + b.h) {
            b.alive = false;
            score += 10;
            ballDY = -ballDY;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(10, update, 0);
}

// Keyboard input (still works as backup)
void keyboard(int key, int, int) {
    if (key == GLUT_KEY_LEFT && paddleX > 0) paddleX -= 20;
    if (key == GLUT_KEY_RIGHT && paddleX + paddleW < windowWidth) paddleX += 20;
}

// Mouse movement (sync paddle with mouse X)
void mouseMove(int x, int y) {
    paddleX = x - paddleW / 2;
    if (paddleX < 0) paddleX = 0;
    if (paddleX + paddleW > windowWidth) paddleX = windowWidth - paddleW;
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("DX Ball Game");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    initBricks();

    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutPassiveMotionFunc(mouseMove); // <-- Mouse handler added
    glutTimerFunc(10, update, 0);

    glutMainLoop();
    return 0;
}
