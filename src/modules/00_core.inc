// AIUB Imposter Detection
// Module: 00_core.inc
// Shared declarations, world state, physics constants, drawing helpers, sky and common building details.

#include <GL/glut.h>
#include <GL/glu.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

#include <cstdlib>
#include <cctype>

namespace L3
{
    void init();
    void display();
    void step();
    void startLevel3(int lifeCount);
    void setLevel3Message(const char* msg, int timer = 260);
    void keyboardDown(unsigned char key, int x, int y);
    void keyboardUp(unsigned char key, int x, int y);
    void arrowDown(int key);
    void arrowUp(int key);
    void mouseMove(int x, int y);
    void reshape(int w, int h);
}

enum AppPhase
{
    PHASE_LEVEL2,
    PHASE_TRANSITION,
    PHASE_PANTA_LEVEL,
    PHASE_BOISHAKH_ENTRY,
    PHASE_LEVEL3
};

AppPhase appPhase = PHASE_LEVEL2;
int transitionStartTime = 0;

// phase control prototypes
void startPantaLevel();
void startBoishakhEntryLevel();
void tryPantaInteract();


// Level 3 originally updated every 40 ms.
// The merged Level 2 loop runs every 16 ms, so Level 3 is throttled
// to preserve the normal movement speed from the original Level 3 file.
const int LEVEL3_UPDATE_MS = 40;
int lastLevel3StepTime = 0;

// ======================================================
// EXTRA BOISHAKH ENTRY LEVEL
// ======================================================
float entryHeroX = 95.0f;
float entryHeroY = 105.0f;
float entryHeroVX = 0.0f;
float entryHeroVY = 0.0f;
bool entryHeroGround = true;
bool entryPassCollected = false;
int entryMsgTimer = 360;
char entryMsg[260] = "Extra Level: Collect the ENTRY PASS, then go to the decorated concert gate and press E.";

// ======================================================
// EXTRA PANTA ILISH LEVEL
// ======================================================
float pantaHeroX = 85.0f;
float pantaHeroY = 105.0f;
float pantaHeroVX = 0.0f;
float pantaHeroVY = 0.0f;
bool pantaHeroGround = true;
bool pantaEaten = false;
int pantaMsgTimer = 360;
char pantaMsg[260] = "Panta Level: Run and jump onto the Panta Ilish plate, then press E at CONCERT GATE.";


// ======================================================
// WINDOW / WORLD
// ======================================================
const int WIN_W = 1000;
const int WIN_H = 600;

// Real window size. This fixes black borders when the window is maximized/fullscreen.
int currentWindowW = WIN_W;
int currentWindowH = WIN_H;
extern float camX;  // declaration for projection helper

void forceFullWindowViewport()
{
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    if (w <= 0) w = WIN_W;
    if (h <= 0) h = WIN_H;

    currentWindowW = w;
    currentWindowH = h;

    // Always draw on the whole actual window, not only a small centered viewport.
    glViewport(0, 0, currentWindowW, currentWindowH);
}

void setLevel2Projection()
{
    forceFullWindowViewport();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Original Level 1 / Level 2 zoom kept. No extra minimize/zoom-out here.
    gluOrtho2D(camX, camX + WIN_W, 0, WIN_H);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void setScreenProjection()
{
    forceFullWindowViewport();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const float WORLD_W = 3400.0f;
const float PI = 3.1415926535f;

const float SHIFT = 1100.0f;

float O(float x)
{
    return x + SHIFT;
}

// ======================================================
// START / INFO STATE
// ======================================================
bool gameStarted = false;
bool infoMode = false;

// ======================================================
// DAY / NIGHT MODE
// ======================================================
bool isNightMode = false;
float cloudOffset = 0.0f;

// forward declaration needed by C Building section
void drawRoundedRect(float x, float y, float w, float h, float r);

// ======================================================
// PLAYER
// ======================================================
float px = 3200;
float py = 100;

float pw = 35;
float ph = 60;

float vx = 0;
float vy = 0;

bool onGround = false;
bool onLadder = false;
bool faceRight = false;

// ======================================================
// CAMERA
// ======================================================
float camX = WORLD_W - WIN_W + 0;

// ======================================================
// INPUT
// ======================================================
bool keyLeft = false;
bool keyRight = false;
bool keyUp = false;
bool keyDown = false;

// ======================================================
// PHYSICS
// ======================================================
const float SPEED = 4.2f;
const float GRAVITY = -0.42f;
const float JUMP_POWER = 10.8f;
const float CLIMB_SPEED = 3.2f;

// ======================================================
// GAME STATE
// ======================================================
// Phone + friend-call system replaces the old direct ID/laddder collection.
bool hasPhone = false;
bool phoneCallMade = false;

// Old ladder state is kept for structure compatibility, but disabled in gameplay.
bool hasLadder = false;
bool ladderPlaced = false;
bool hasID = false;
bool gateOpen = false;
bool levelComplete = false;

bool appleTaken = false;
int appleScore = 0;

bool hasBridgeBrick = false;
bool cBrickPlaced = false;

bool hasGun = false;
bool hasBullets = false;
bool hasKnife = false;

int lives = 3;

char messageText[260] =
    "Collect PHONE, call friend, collect ID, finish items. Then PANTA LEVEL will start before concert.";
int messageTimer = 420;

// ======================================================
// BASIC STRUCTS
// ======================================================
struct Rect
{
    float x;
    float y;
    float w;
    float h;
};

// Panta level is one single playable part: run + jump to eat, then enter concert gate.
Rect pantaJumpPlatform = {175, 238, 210, 18};
Rect pantaFoodTarget = {220, 272, 110, 46};

struct Platform
{
    float x;
    float y;
    float w;
    float h;
};

struct Spike
{
    float x;
    float y;
    float w;
    float h;
    int teeth;
};

// ======================================================
// PLATFORMS
// ======================================================
Platform platforms[] =
{
    {0, 40, WORLD_W, 60},

    {O(1790), 170, 260, 30},
    {O(1550), 270, 280, 30},

    // Old high ID-card bar removed after phone-call ID-drop logic
    {-1000, -1000, 0, 0},

    // New reachable jump bar beside the spike area for the life apple
    {O(520), 205, 120, 25},

    // C Building brick collection bar
    {820, 170, 220, 30},

    // Fixed brick bridge over water
    {400, 40, 130, 60}
};

int platformCount = sizeof(platforms) / sizeof(platforms[0]);

// ======================================================
// SPIKES
// ======================================================
Spike spikes[] =
{
    {O(1860), 200, 95, 35, 3},
    {O(1330), 100, 110, 35, 4},
    {O(655),  100, 135, 35, 4},

    {835, 200, 85, 35, 3},

    {85, 100, 80, 35, 3}
};

int spikeCount = sizeof(spikes) / sizeof(spikes[0]);

// ======================================================
// ITEMS
// ======================================================
// Old ladder position now holds the PHONE.
Rect ladderItem = {O(1675), 300, 45, 75};

// ID starts hidden. It drops from D Building window only after phone call.
Rect idCard     = {O(1005), 455, 120, 45};
bool idDropTriggered = false;
bool idLanded = false;
float idVelY = 0.0f;
float idWindowX = 0.0f;
float idWindowY = 0.0f;

Rect apple      = {O(555), 245, 45, 45};

// ======================================================
// LADDER PLACEMENT
// ======================================================
Rect ladderPlaceZone = {O(970), 100, 120, 25};

float placedLadderX = O(990);
float placedLadderY = 100;
float placedLadderW = 75;
float placedLadderH = 425;

// ======================================================
// GUARDS / GATE
// ======================================================
float guard1X = O(1600);
float guard1Y = 100;
float guard1W = 45;
float guard1H = 82;
float guard1VX = 1.8f;

float guard1Left = O(1475);
float guard1Right = O(1695);

Rect finalGuard = {O(280), 100, 50, 85};
Rect gateBlock  = {O(140), 100, 35, 150};

// ======================================================
// C BUILDING SECTION
// ======================================================
Rect waterZone = {280, 100, 420, 35};

Rect cBrickPlaceZone = {700, 100, 80, 25};

Rect cPlacedBrickPlatform = {530, 40, 150, 60};

Rect brickItem   = {970, 205, 55, 32};
Rect gunItem     = {430, 108, 80, 25};
Rect bulletsItem = {585, 105, 55, 48};
Rect knifeItem   = {150, 120, 42, 16};

// ======================================================
// BASIC DRAWING HELPERS
// ======================================================
void color(float r, float g, float b, float a = 1.0f)
{
    glColor4f(r, g, b, a);
}

void drawRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
}

void drawAbsRect(float x1, float y1, float x2, float y2)
{
    drawRect(x1, y1, x2 - x1, y2 - y1);
}

void drawQuad(float x1, float y1,
              float x2, float y2,
              float x3, float y3,
              float x4, float y4)
{
    glBegin(GL_QUADS);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glVertex2f(x3, y3);
        glVertex2f(x4, y4);
    glEnd();
}

void drawTriangle(float x1, float y1,
                  float x2, float y2,
                  float x3, float y3)
{
    glBegin(GL_TRIANGLES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glVertex2f(x3, y3);
    glEnd();
}

void drawLine(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    glEnd();
}

void thickLine(float x1, float y1, float x2, float y2, float width)
{
    glLineWidth(width);
    drawLine(x1, y1, x2, y2);
    glLineWidth(1);
}

void outlineRect(float x, float y, float w, float h)
{
    drawLine(x,     y,     x + w, y);
    drawLine(x + w, y,     x + w, y + h);
    drawLine(x + w, y + h, x,     y + h);
    drawLine(x,     y + h, x,     y);
}

void drawCircle(float cx, float cy, float r, int seg = 36)
{
    glBegin(GL_POLYGON);

    for (int i = 0; i < seg; i++)
    {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }

    glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, int seg = 48)
{
    glBegin(GL_POLYGON);

    for (int i = 0; i < seg; i++)
    {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + cos(a) * rx, cy + sin(a) * ry);
    }

    glEnd();
}

void drawText(float x, float y, const char* text)
{
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text[i]);
    }
}

void drawTextBig(float x, float y, const char* text)
{
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// ======================================================
// COLLISION
// ======================================================
Rect playerRect()
{
    Rect r = {px, py, pw, ph};
    return r;
}

bool collide(Rect a, Rect b)
{
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

void setMessage(const char* msg, int timer = 260)
{
    strcpy(messageText, msg);
    messageTimer = timer;
}

// ======================================================
// SKY / GROUND
// ======================================================

void drawSingleCloud(float x, float y, float s)
{
    if (!isNightMode)
        color(1.0f, 1.0f, 1.0f, 0.88f);
    else
        color(0.45f, 0.48f, 0.58f, 0.42f);

    drawEllipse(x, y, 34 * s, 15 * s, 28);
    drawCircle(x - 27 * s, y - 2 * s, 15 * s, 24);
    drawCircle(x - 10 * s, y + 9 * s, 19 * s, 24);
    drawCircle(x + 14 * s, y + 8 * s, 21 * s, 24);
    drawCircle(x + 35 * s, y - 1 * s, 14 * s, 24);

    if (!isNightMode)
        color(0.88f, 0.93f, 0.96f, 0.34f);
    else
        color(0.25f, 0.28f, 0.36f, 0.32f);

    drawEllipse(x + 5 * s, y - 8 * s, 46 * s, 8 * s, 24);
}

void drawMovingClouds()
{
    float baseX = camX - 300.0f + cloudOffset;

    while (baseX > camX - 420.0f)
        baseX -= 1250.0f;

    while (baseX < camX + WIN_W + 420.0f)
    {
        drawSingleCloud(baseX + 95,  505, 1.05f);
        drawSingleCloud(baseX + 380, 548, 0.78f);
        drawSingleCloud(baseX + 710, 520, 1.16f);
        drawSingleCloud(baseX + 1005,555, 0.88f);
        baseX += 1250.0f;
    }
}

void drawSkyAndGround()
{
    if (!isNightMode)
    {
        glBegin(GL_QUADS);
            glColor3f(0.84f, 0.93f, 1.0f);
            glVertex2f(camX, WIN_H);
            glVertex2f(camX + WIN_W, WIN_H);

            glColor3f(0.66f, 0.84f, 0.95f);
            glVertex2f(camX + WIN_W, 0);
            glVertex2f(camX, 0);
        glEnd();

        drawMovingClouds();

        // sun
        color(1.0f, 0.92f, 0.50f, 0.95f);
        drawCircle(camX + 120, WIN_H - 95, 34, 36);
        color(1.0f, 0.97f, 0.75f, 0.30f);
        drawCircle(camX + 120, WIN_H - 95, 52, 36);

        color(0.84f, 0.88f, 0.80f);
        drawRect(camX, 86, WIN_W, 46);

        color(0.02f, 0.025f, 0.02f);
        drawRect(camX, 0, WIN_W, 100);

        color(0.10f, 0.14f, 0.11f);
        thickLine(camX, 100, camX + WIN_W, 100, 3);
    }
    else
    {
        glBegin(GL_QUADS);
            glColor3f(0.03f, 0.06f, 0.15f);
            glVertex2f(camX, WIN_H);
            glVertex2f(camX + WIN_W, WIN_H);

            glColor3f(0.10f, 0.14f, 0.28f);
            glVertex2f(camX + WIN_W, 0);
            glVertex2f(camX, 0);
        glEnd();

        drawMovingClouds();

        // moon
        color(0.95f, 0.96f, 0.90f, 0.95f);
        drawCircle(camX + 140, WIN_H - 90, 30, 36);
        color(0.03f, 0.06f, 0.15f, 1.0f);
        drawCircle(camX + 152, WIN_H - 84, 25, 36);

        // stars
        color(0.98f, 0.98f, 0.92f, 0.95f);
        float starPos[][2] = {
            {70,540},{130,500},{210,560},{270,515},{345,545},{430,510},{520,565},
            {610,535},{690,550},{760,505},{830,555},{905,520},{965,545}
        };
        for (int i = 0; i < 13; i++)
        {
            float sx = camX + starPos[i][0];
            float sy = starPos[i][1];
            drawCircle(sx, sy, 2.3f, 10);
            thickLine(sx - 5, sy, sx + 5, sy, 1.2f);
            thickLine(sx, sy - 5, sx, sy + 5, 1.2f);
        }

        // night haze near ground
        color(0.18f, 0.21f, 0.28f, 0.60f);
        drawRect(camX, 86, WIN_W, 46);

        color(0.01f, 0.02f, 0.03f);
        drawRect(camX, 0, WIN_W, 100);

        color(0.20f, 0.26f, 0.24f);
        thickLine(camX, 100, camX + WIN_W, 100, 3);
    }
}

// ======================================================
// COMMON BUILDING DETAILS
// ======================================================
void drawPanelGrid(float x, float y, float w, float h, float gx, float gy)
{
    color(0.62f, 0.66f, 0.66f);

    for (float yy = y + gy; yy < y + h; yy += gy)
    {
        drawLine(x, yy, x + w, yy);
    }

    for (float xx = x + gx; xx < x + w; xx += gx)
    {
        drawLine(xx, y, xx, y + h);
    }
}

void drawSmallWindow(float x, float y, float w, float h)
{
    color(0.05f, 0.08f, 0.09f);
    drawRect(x, y, w, h);

    if (!isNightMode)
        color(0.63f, 0.82f, 0.90f);
    else
        color(0.95f, 0.86f, 0.48f);

    drawRect(x + 4, y + 4, w - 8, h - 8);

    if (!isNightMode)
        color(0.13f, 0.27f, 0.34f);
    else
        color(0.55f, 0.34f, 0.08f);

    for (float yy = y + 10; yy < y + h - 5; yy += 7)
    {
        drawLine(x + 6, yy, x + w - 6, yy);
    }

    if (!isNightMode)
        color(0.92f, 0.96f, 0.96f);
    else
        color(1.0f, 0.96f, 0.75f, 0.9f);

    thickLine(x + w / 2, y + 4, x + w / 2, y + h - 4, 1.5f);
}

void drawVent(float x, float y, float w, float h)
{
    color(0.06f, 0.07f, 0.08f);
    drawRect(x, y, w, h);

    color(0.0f, 0.0f, 0.0f);

    for (float yy = y + 6; yy < y + h - 3; yy += 6)
    {
        drawLine(x + 5, yy, x + w - 5, yy);
    }
}

// ======================================================
