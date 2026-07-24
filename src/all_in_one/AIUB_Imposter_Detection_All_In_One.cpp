/*
 * AIUB Imposter Detection — All-in-One Source
 * Generated from the uploaded game source.
 * Modular edition: ../main.cpp + ../modules/*.inc
 */

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
// OLD LEFT BUILDING ART, OFFSET VERSION
// ======================================================
void drawLeftGlassTowerArt(float ox)
{
    color(0.11f, 0.20f, 0.23f);
    drawRect(ox + 310, 105, 160, 345);

    color(0.22f, 0.38f, 0.43f);
    drawQuad(ox + 470, 105,
             ox + 515, 130,
             ox + 515, 425,
             ox + 470, 450);

    for (float y = 120; y < 420; y += 50)
    {
        color(0.34f, 0.55f, 0.60f);
        drawRect(ox + 325, y, 50, 38);
        drawRect(ox + 385, y, 72, 38);

        color(0.25f, 0.45f, 0.50f);
        drawQuad(ox + 480, y + 6,
                 ox + 505, y + 18,
                 ox + 505, y + 48,
                 ox + 480, y + 38);
    }

    color(0.88f, 0.93f, 0.93f);
    thickLine(ox + 310, 105, ox + 310, 450, 3);
    thickLine(ox + 380, 105, ox + 380, 450, 2);
    thickLine(ox + 470, 105, ox + 470, 450, 3);
    thickLine(ox + 515, 130, ox + 515, 425, 3);

    for (float y = 165; y < 440; y += 50)
    {
        thickLine(ox + 310, y, ox + 470, y, 2);
        thickLine(ox + 470, y, ox + 515, y + 22, 2);
    }

    color(0.86f, 0.86f, 0.81f);
    drawRect(ox + 300, 450, 190, 18);

    color(0.72f, 0.74f, 0.70f);
    drawRect(ox + 345, 468, 13, 18);
    drawRect(ox + 395, 468, 13, 18);
}

void drawLeftBuildingArt(float ox)
{
    color(0.82f, 0.85f, 0.82f);
    drawRect(ox + 45, 105, 265, 270);

    color(0.82f, 0.85f, 0.82f);
    drawRect(ox + 515, 105, 190, 270);

    drawPanelGrid(ox + 45, 105, 265, 270, 45, 35);
    drawPanelGrid(ox + 515, 105, 190, 270, 45, 35);

    drawVent(ox + 85, 315, 50, 32);
    drawSmallWindow(ox + 175, 315, 62, 45);
    drawSmallWindow(ox + 170, 230, 70, 48);
    drawSmallWindow(ox + 170, 145, 70, 48);

    drawVent(ox + 535, 315, 50, 32);
    drawSmallWindow(ox + 620, 315, 58, 45);
    drawSmallWindow(ox + 585, 225, 65, 48);
    drawSmallWindow(ox + 575, 135, 70, 48);
    drawSmallWindow(ox + 655, 255, 42, 38);

    drawLeftGlassTowerArt(ox);

    color(0.78f, 0.03f, 0.02f);
    drawRect(ox + 220, 105, 295, 18);

    color(0.95f, 0.05f, 0.03f);
    drawRect(ox + 220, 123, 295, 10);

    color(0.72f, 0.03f, 0.02f);
    drawRect(ox + 230, 75, 15, 55);
    drawRect(ox + 495, 75, 15, 55);

    color(0.15f, 0.28f, 0.30f);
    drawRect(ox + 340, 80, 105, 25);

    color(0.75f, 0.90f, 0.93f);
    thickLine(ox + 392, 80, ox + 392, 105, 2);

    color(0.23f, 0.26f, 0.25f);
    drawRect(ox + 45, 80, 660, 25);
}

// ======================================================
// D BUILDING CODE ADAPTED INTO GAME WORLD
// ======================================================
const float D_SCALE = 0.56f;

// Moved slightly right for better spacing
const float D_ORIGIN_X = SHIFT + 680.0f;

const float D_ORIGIN_Y = 43.0f;

float DX(float x)
{
    return D_ORIGIN_X + x * D_SCALE;
}

float DY(float y)
{
    return D_ORIGIN_Y + y * D_SCALE;
}

void dRect(float x1, float y1, float x2, float y2)
{
    drawAbsRect(DX(x1), DY(y1), DX(x2), DY(y2));
}

void dQuad(float x1, float y1, float x2, float y2,
           float x3, float y3, float x4, float y4)
{
    drawQuad(DX(x1), DY(y1),
             DX(x2), DY(y2),
             DX(x3), DY(y3),
             DX(x4), DY(y4));
}

void dLine(float x1, float y1, float x2, float y2)
{
    drawLine(DX(x1), DY(y1), DX(x2), DY(y2));
}

void dCircle(float cx, float cy, float r, int seg = 40)
{
    drawCircle(DX(cx), DY(cy), r * D_SCALE, seg);
}

void drawDGlassBand(float x1, float y1, float x2, float y2, int cols)
{
    glBegin(GL_QUADS);
        color(0.56f, 0.73f, 0.82f);
        glVertex2f(DX(x1), DY(y2));
        glVertex2f(DX(x2), DY(y2));

        color(0.38f, 0.58f, 0.68f);
        glVertex2f(DX(x2), DY(y1));
        glVertex2f(DX(x1), DY(y1));
    glEnd();

    color(0.22f, 0.39f, 0.48f, 0.35f);
    dRect(x1, y1, x2, y1 + 8);

    float step = (x2 - x1) / cols;

    for (int i = 0; i < cols; i++)
    {
        float px1 = x1 + i * step + 2;
        float px2 = x1 + (i + 1) * step - 2;

        color(0.50f, 0.70f, 0.80f, 0.45f);
        dRect(px1, y1 + 4, px2, y2 - 4);

        color(0.82f, 0.95f, 1.0f, 0.22f);
        dQuad(px1 + 3, y2 - 12,
              px2 - 3, y2 - 12,
              px2 - 8, y2 - 4,
              px1 + 8, y2 - 4);
    }

    color(0.90f, 0.98f, 1.0f, 0.18f);
    dRect(x1 + 20, y1 + 9, x1 + (x2 - x1) * 0.36f, y2 - 9);

    color(0.86f, 0.96f, 1.0f, 0.13f);
    dRect(x1 + (x2 - x1) * 0.58f, y1 + 10,
          x1 + (x2 - x1) * 0.82f, y2 - 10);

    color(0.12f, 0.25f, 0.31f, 0.65f);
    glLineWidth(1.2f);

    for (int i = 1; i < cols; i++)
    {
        float x = x1 + i * step;
        dLine(x, y1, x, y2);
    }

    color(0.14f, 0.28f, 0.34f, 0.45f);
    dLine(x1, (y1 + y2) / 2.0f, x2, (y1 + y2) / 2.0f);

    color(0.08f, 0.16f, 0.20f, 0.75f);
    glLineWidth(1.6f);
    dLine(x1, y1, x2, y1);
    dLine(x1, y2, x2, y2);
    glLineWidth(1.0f);
}

void drawDCladdingBand(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
        color(0.86f, 0.85f, 0.81f);
        glVertex2f(DX(x1), DY(y2));
        glVertex2f(DX(x2), DY(y2));

        color(0.78f, 0.77f, 0.73f);
        glVertex2f(DX(x2), DY(y1));
        glVertex2f(DX(x1), DY(y1));
    glEnd();

    color(0.62f, 0.61f, 0.57f, 0.25f);

    for (float y = y1 + 8; y < y2; y += 10)
    {
        dLine(x1, y, x2, y);
    }
}

void drawDPlanterStrip(float x1, float y1, float x2, float y2)
{
    color(0.75f, 0.74f, 0.70f);
    dRect(x1, y1, x2, y2);

    color(0.25f, 0.54f, 0.23f);
    dRect(x1, y2 - 8, x2, y2);

    color(0.35f, 0.62f, 0.30f, 0.70f);

    for (float x = x1 + 20; x < x2 - 20; x += 35)
    {
        dLine(x, y2 - 8, x - 3, y2 - 24);
    }
}

void drawDSmallTree(float x, float y, float s = 1.0f)
{
    color(0.44f, 0.30f, 0.18f);
    dRect(x - 2 * s, y - 15 * s, x + 2 * s, y);

    color(0.34f, 0.58f, 0.28f);
    dCircle(x, y + 8 * s, 10 * s, 30);
    dCircle(x - 8 * s, y + 5 * s, 8 * s, 30);
    dCircle(x + 8 * s, y + 5 * s, 8 * s, 30);
}

void drawDTerraceTrees()
{
    drawDSmallTree(690, 620, 1.2f);
    drawDSmallTree(780, 620, 1.2f);
    drawDSmallTree(870, 620, 1.2f);
    drawDSmallTree(960, 620, 1.2f);
    drawDSmallTree(1050, 620, 1.2f);
}

void drawDLeftCurvedTower()
{
    glBegin(GL_POLYGON);

        color(0.42f, 0.58f, 0.68f);

        for (int i = 0; i <= 90; i++)
        {
            float t = i / 90.0f;
            float y = 160 + t * 600;
            float x = 185 + 25 * sin(t * 1.57f);
            glVertex2f(DX(x), DY(y));
        }

        color(0.30f, 0.48f, 0.58f);
        glVertex2f(DX(360), DY(760));
        glVertex2f(DX(360), DY(160));

    glEnd();

    glBegin(GL_POLYGON);

        color(0.42f, 0.58f, 0.68f);
        glVertex2f(DX(360), DY(160));
        glVertex2f(DX(455), DY(160));
        glVertex2f(DX(455), DY(635));
        glVertex2f(DX(360), DY(635));

    glEnd();

    color(0.82f, 0.94f, 1.0f, 0.13f);
    dQuad(220, 170, 250, 170, 244, 745, 215, 745);

    color(0.78f, 0.92f, 1.0f, 0.10f);
    dQuad(285, 170, 318, 170, 312, 745, 280, 745);

    color(0.82f, 0.94f, 1.0f, 0.12f);
    dQuad(382, 175, 405, 175, 405, 625, 378, 635);

    color(0.70f, 0.88f, 0.96f, 0.10f);
    dQuad(425, 190, 445, 190, 445, 615, 420, 630);

    color(0.16f, 0.25f, 0.30f, 0.42f);

    for (float x = 205; x <= 345; x += 18)
    {
        dLine(x, 165, x, 755);
    }

    for (float y = 185; y <= 735; y += 22)
    {
        dLine(200, y, 350, y);
    }

    for (float x = 375; x <= 445; x += 15)
    {
        dLine(x, 170, x, 630);
    }

    for (float y = 185; y <= 630; y += 22)
    {
        dLine(365, y, 452, y);
    }

    color(0.08f, 0.15f, 0.18f);
    dRect(330, 420, 350, 445);

    color(0.70f, 0.74f, 0.73f);
    dQuad(330, 420, 350, 420, 345, 407, 326, 407);

    color(0.28f, 0.52f, 0.22f);

    for (float x = 205; x <= 360; x += 10)
    {
        dRect(x, 760, x + 6, 766);
    }
}

void drawDRightServiceBlock()
{
    glBegin(GL_QUADS);

        color(0.29f, 0.39f, 0.56f);
        glVertex2f(DX(1120), DY(780));
        glVertex2f(DX(1185), DY(780));

        color(0.18f, 0.28f, 0.44f);
        glVertex2f(DX(1185), DY(170));
        glVertex2f(DX(1120), DY(170));

    glEnd();

    color(0.66f, 0.77f, 0.84f);

    for (float y = 220; y <= 700; y += 52)
    {
        dRect(1145, y, 1160, y + 12);
    }
}

void drawDRooftop()
{
    color(0.32f, 0.55f, 0.24f);
    dRect(346, 784, 1118, 792);

    color(0.22f, 0.25f, 0.28f);
    dRect(520, 705, 1115, 748);

    color(0.14f, 0.14f, 0.14f);

    for (float x = 530; x < 1108; x += 5)
    {
        dRect(x, 705, x + 2.0f, 748);
    }

    color(0.42f, 0.46f, 0.47f);
    dQuad(710, 800, 900, 800, 940, 860, 670, 860);
    dQuad(980, 800, 1085, 800, 1115, 835, 960, 835);

    color(0.74f, 0.73f, 0.69f);
    dRect(735, 792, 746, 858);
    dRect(810, 792, 821, 858);
    dRect(885, 792, 896, 858);
    dRect(1015, 792, 1026, 833);
}

void drawDMainBlock()
{
    color(0.84f, 0.83f, 0.79f);
    dRect(345, 170, 1120, 785);

    drawDCladdingBand(350, 190, 1110, 225);
    drawDGlassBand(350, 225, 1110, 265, 24);

    drawDCladdingBand(350, 275, 1110, 310);
    drawDGlassBand(350, 310, 1110, 350, 24);

    drawDCladdingBand(350, 360, 1110, 395);
    drawDGlassBand(350, 395, 1110, 438, 24);

    drawDPlanterStrip(350, 448, 1110, 466);
    drawDGlassBand(350, 468, 1110, 515, 24);

    drawDCladdingBand(350, 525, 1110, 560);
    drawDGlassBand(350, 560, 1110, 605, 24);

    drawDPlanterStrip(350, 615, 1110, 635);
    drawDGlassBand(350, 637, 1110, 682, 24);

    drawDCladdingBand(350, 690, 1110, 705);

    color(0.68f, 0.67f, 0.63f, 0.35f);

    for (float x = 355; x <= 1105; x += 16)
    {
        dLine(x, 170, x, 785);
    }

    color(0.42f, 0.41f, 0.38f, 0.55f);
    dLine(345, 170, 1120, 170);
    dLine(345, 785, 1120, 785);
    dLine(345, 170, 345, 785);
    dLine(1120, 170, 1120, 785);

    drawDTerraceTrees();
}

void drawDForegroundTrees()
{
    for (float x = 290; x <= 1040; x += 35)
    {
        color(0.26f, 0.46f, 0.24f);
        dCircle(x, 155, 16, 28);

        color(0.20f, 0.35f, 0.18f);
        dCircle(x + 10, 150, 10, 28);
    }
}

void drawDBuilding()
{
    drawDMainBlock();
    drawDLeftCurvedTower();
    drawDRightServiceBlock();
    drawDRooftop();
    drawDForegroundTrees();

    // Lowered and cleaned D Building label
    color(0.08f, 0.10f, 0.12f);
    drawRect(DX(585), DY(685), 165 * D_SCALE, 34 * D_SCALE);

    color(0.95f, 0.95f, 0.90f);
    drawText(DX(618), DY(698), "D BUILDING");
}


void drawIDThrowPerson()
{
    float wx = DX(340);
    float wy = DY(435);

    if (!hasID && hasPhone)
    {
        // Small normal friend in the D Building open window.
        color(0.04f, 0.06f, 0.07f);
        drawRoundedRect(wx - 7, wy - 6, 25, 31, 3);

        color(0.02f, 0.03f, 0.04f);
        drawRect(wx - 4, wy - 3, 19, 25);

        // small front-facing head
        color(1.0f, 0.75f, 0.58f);
        drawCircle(wx + 5, wy + 15, 5, 22);

        // hair only on top so the face clearly looks forward, not backward
        color(0.05f, 0.035f, 0.02f);
        drawEllipse(wx + 5, wy + 19, 5.2f, 2.2f, 18);
        drawEllipse(wx + 2.2f, wy + 17.5f, 2.0f, 1.4f, 12);
        drawEllipse(wx + 7.8f, wy + 17.5f, 2.0f, 1.4f, 12);

        // small body
        color(0.18f, 0.35f, 0.72f);
        drawRoundedRect(wx, wy + 1, 11, 11, 3);

        // neck
        color(1.0f, 0.75f, 0.58f);
        drawRect(wx + 3.5f, wy + 9, 3, 4);

        // small arm: before call he waits, after call he drops ID
        color(1.0f, 0.75f, 0.58f);
        if (phoneCallMade || idDropTriggered || idLanded)
        {
            thickLine(wx + 10, wy + 8, wx + 21, wy + 3, 2.5f);
        }
        else
        {
            thickLine(wx, wy + 8, wx - 10, wy + 13, 2.5f);
        }

        // front-facing face: two eyes, nose and mouth at the center
        color(0.0f, 0.0f, 0.0f);
        drawCircle(wx + 3.2f, wy + 15.5f, 0.75f, 8);
        drawCircle(wx + 6.8f, wy + 15.5f, 0.75f, 8);
        thickLine(wx + 5.0f, wy + 14.7f, wx + 5.0f, wy + 13.3f, 1.0f);
        thickLine(wx + 3.8f, wy + 12.6f, wx + 6.2f, wy + 12.6f, 1.0f);

        if (!phoneCallMade)
        {
            drawText(wx - 28, wy + 33, "Call");
        }
        else if (!idLanded)
        {
            drawText(wx - 30, wy + 33, "ID");
        }
    }
}

// ======================================================
// BUSH
// ======================================================
void drawBush(float x, float y, float s)
{
    color(0.05f, 0.16f, 0.06f);
    drawEllipse(x, y - 6 * s, 70 * s, 16 * s);

    color(0.20f, 0.55f, 0.18f);
    drawCircle(x - 48 * s, y + 5 * s, 32 * s);
    drawCircle(x - 22 * s, y + 18 * s, 42 * s);
    drawCircle(x + 18 * s, y + 20 * s, 46 * s);
    drawCircle(x + 55 * s, y + 4 * s, 35 * s);

    color(0.32f, 0.68f, 0.26f);
    drawCircle(x - 60 * s, y - 2 * s, 25 * s);
    drawCircle(x - 35 * s, y + 25 * s, 28 * s);
    drawCircle(x, y + 30 * s, 34 * s);
    drawCircle(x + 35 * s, y + 25 * s, 30 * s);
    drawCircle(x + 65 * s, y - 1 * s, 25 * s);

    color(0.55f, 0.82f, 0.40f);
    drawCircle(x - 40 * s, y + 28 * s, 12 * s);
    drawCircle(x - 5 * s, y + 42 * s, 14 * s);
    drawCircle(x + 32 * s, y + 34 * s, 12 * s);
    drawCircle(x + 62 * s, y + 14 * s, 10 * s);
}


// ======================================================
// C BUILDING (FROM UPLOADED ROUND BUILDING FILE, ADAPTED)
// ======================================================
const float C_ART_SCALE = 0.45f;
const float C_ART_ORIGIN_X = 155.0f;
const float C_ART_ORIGIN_Y = 45.0f;

const float C_ART_CX = 500.0f;
const float C_ART_CY = 575.0f;
const float C_ART_RX = 455.0f;
const float C_ART_RY = 520.0f;
const float C_ART_BODY_TOP = C_ART_CY + C_ART_RY;
const float C_ART_BODY_BOTTOM = 90.0f;

float CBX(float x) { return C_ART_ORIGIN_X + x * C_ART_SCALE; }
float CBY(float y) { return C_ART_ORIGIN_Y + y * C_ART_SCALE; }

void cbRect(float x1, float y1, float x2, float y2)
{
    drawAbsRect(CBX(x1), CBY(y1), CBX(x2), CBY(y2));
}

void cbLine(float x1, float y1, float x2, float y2)
{
    drawLine(CBX(x1), CBY(y1), CBX(x2), CBY(y2));
}

void cbThickLine(float x1, float y1, float x2, float y2, float w)
{
    glLineWidth(w);
    cbLine(x1, y1, x2, y2);
    glLineWidth(1);
}

void cbEllipse(float cx, float cy, float rx, float ry, int seg = 50)
{
    drawEllipse(CBX(cx), CBY(cy), rx * C_ART_SCALE, ry * C_ART_SCALE, seg);
}

float cHalfWidthArt(float y)
{
    float t = (y - C_ART_CY) / C_ART_RY;
    if (fabs(t) >= 1.0f) return 0.0f;
    return C_ART_RX * sqrt(1.0f - t * t);
}

void cbCurvedLine(float yBase, float sag, float inset = 0.0f)
{
    float maxW = cHalfWidthArt(yBase) - inset;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 220; i++)
    {
        float u = -1.0f + 2.0f * i / 220.0f;
        float x = C_ART_CX + u * maxW;
        float y = yBase - sag * u * u;

        if (fabs(x - C_ART_CX) <= cHalfWidthArt(y))
            glVertex2f(CBX(x), CBY(y));
    }
    glEnd();
}

void cbCurvedStrip(float topY, float bottomY, float sag, float r, float g, float b)
{
    float maxW = std::min(cHalfWidthArt(topY), cHalfWidthArt(bottomY)) - 6.0f;

    color(r, g, b);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 220; i++)
    {
        float u = -1.0f + 2.0f * i / 220.0f;
        float x = C_ART_CX + u * maxW;
        float curve = sag * u * u;

        glVertex2f(CBX(x), CBY(topY - curve));
        glVertex2f(CBX(x), CBY(bottomY - curve));
    }
    glEnd();
}

void drawCBodyArt()
{
    color(0.72f, 0.69f, 0.61f);
    glBegin(GL_POLYGON);
    for (float y = C_ART_BODY_TOP; y >= C_ART_BODY_BOTTOM; y -= 4)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX - hw), CBY(y));
    }
    for (float y = C_ART_BODY_BOTTOM; y <= C_ART_BODY_TOP; y += 4)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX + hw), CBY(y));
    }
    glEnd();

    color(0.88f, 0.85f, 0.78f, 0.28f);
    glBegin(GL_POLYGON);
    for (float y = 650; y <= 1085; y += 4)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX - hw * 0.92f), CBY(y));
    }
    for (float y = 1085; y >= 650; y -= 4)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX + hw * 0.92f), CBY(y));
    }
    glEnd();

    color(0.50f, 0.47f, 0.40f, 0.20f);
    glBegin(GL_POLYGON);
    for (float y = C_ART_BODY_BOTTOM; y <= 420; y += 4)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX - hw), CBY(y));
    }
    for (float y = 420; y >= C_ART_BODY_BOTTOM; y -= 4)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX + hw), CBY(y));
    }
    glEnd();
}

void drawCGlassBandsArt()
{
    cbCurvedStrip(835, 775, 45, 0.15f, 0.29f, 0.33f);
    cbCurvedStrip(710, 620, 42, 0.13f, 0.27f, 0.30f);
    cbCurvedStrip(565, 485, 36, 0.10f, 0.23f, 0.25f);
    cbCurvedStrip(420, 335, 31, 0.09f, 0.21f, 0.23f);
    cbCurvedStrip(285, 205, 22, 0.10f, 0.22f, 0.24f);

    glLineWidth(2.5f);
    color(0.04f, 0.06f, 0.06f, 0.95f);
    cbCurvedLine(835, 45, 5); cbCurvedLine(775, 45, 5);
    cbCurvedLine(710, 42, 5); cbCurvedLine(620, 42, 5);
    cbCurvedLine(565, 36, 5); cbCurvedLine(485, 36, 5);
    cbCurvedLine(420, 31, 5); cbCurvedLine(335, 31, 5);
    cbCurvedLine(285, 22, 5); cbCurvedLine(205, 22, 5);
    glLineWidth(1);
}

void drawCPanelGridArt()
{
    color(0.19f, 0.18f, 0.15f, 0.35f);
    for (float x = 95; x <= 905; x += 38)
    {
        glBegin(GL_LINE_STRIP);
        for (float y = C_ART_BODY_BOTTOM + 10; y <= C_ART_BODY_TOP - 10; y += 5)
        {
            if (fabs(x - C_ART_CX) <= cHalfWidthArt(y))
                glVertex2f(CBX(x), CBY(y));
        }
        glEnd();
    }

    color(0.21f, 0.20f, 0.17f, 0.32f);
    for (float y = 1035; y >= 880; y -= 38) cbCurvedLine(y, 22, 10);
    cbCurvedLine(750, 36, 10);
    cbCurvedLine(600, 32, 10);
    cbCurvedLine(455, 28, 10);
    cbCurvedLine(315, 22, 10);
    cbCurvedLine(190, 16, 10);

    glLineWidth(2);
    color(0.08f, 0.08f, 0.07f, 0.45f);
    cbLine(C_ART_CX, 130, C_ART_CX, 1080);
    glLineWidth(1);

    color(0.02f, 0.05f, 0.06f, 0.45f);
    for (float x = 130; x <= 870; x += 58)
    {
        cbLine(x, 785, x, 825);
        cbLine(x, 635, x, 695);
        cbLine(x, 500, x, 555);
        cbLine(x, 350, x, 410);
        cbLine(x, 220, x, 275);
    }
}

void drawCGlassReflectionsArt()
{
    color(0.50f, 0.68f, 0.72f, 0.22f);
    cbRect(270, 785, 390, 825);
    cbRect(465, 780, 560, 830);
    cbRect(620, 785, 735, 825);

    color(0.54f, 0.72f, 0.76f, 0.18f);
    cbRect(250, 635, 355, 690);
    cbRect(455, 630, 535, 700);
    cbRect(610, 635, 730, 692);

    color(0.72f, 0.82f, 0.84f, 0.15f);
    cbRect(280, 500, 390, 555);
    cbRect(600, 505, 710, 555);

    color(0.02f, 0.04f, 0.04f, 0.22f);
    cbRect(170, 350, 275, 410);
    cbRect(725, 350, 835, 410);
    cbRect(360, 220, 450, 275);
    cbRect(600, 220, 700, 275);
}

void drawCAwningWindowArt(float x, float y, float w, float h, bool openDown)
{
    color(0.02f, 0.03f, 0.03f);
    cbRect(x - 3, y - 3, x + w + 3, y + h + 3);

    color(0.40f, 0.59f, 0.64f);
    cbRect(x, y, x + w, y + h);

    color(0.66f, 0.80f, 0.83f, 0.55f);
    cbRect(x + 4, y + 4, x + w / 2 - 2, y + h - 4);

    glLineWidth(1.6f);
    color(0.82f, 0.86f, 0.84f);
    cbLine(x + w / 2, y, x + w / 2, y + h);
    glLineWidth(1);

    if (openDown)
    {
        color(0.78f, 0.80f, 0.76f);
        glBegin(GL_QUADS);
        glVertex2f(CBX(x + 5),     CBY(y));
        glVertex2f(CBX(x + w - 5), CBY(y));
        glVertex2f(CBX(x + w - 14),CBY(y - 24));
        glVertex2f(CBX(x + 14),    CBY(y - 24));
        glEnd();

        color(0.04f, 0.05f, 0.05f);
        cbLine(x + 14, y - 24, x + w - 14, y - 24);
    }
}

void drawCSmallWindowsArt()
{
    drawCAwningWindowArt(280, 795, 112, 38, true);
    drawCAwningWindowArt(615, 795, 110, 38, false);

    drawCAwningWindowArt(82, 648, 60, 34, true);
    drawCAwningWindowArt(295, 648, 82, 42, true);
    drawCAwningWindowArt(635, 650, 92, 42, true);
    drawCAwningWindowArt(845, 646, 52, 34, true);

    drawCAwningWindowArt(66, 512, 64, 34, true);
    drawCAwningWindowArt(255, 520, 100, 42, false);
    drawCAwningWindowArt(600, 522, 100, 40, false);
    drawCAwningWindowArt(855, 515, 55, 34, true);

    drawCAwningWindowArt(225, 365, 88, 40, false);
    drawCAwningWindowArt(608, 365, 103, 42, false);
    drawCAwningWindowArt(783, 360, 76, 35, false);

    drawCAwningWindowArt(335, 235, 78, 42, true);
    drawCAwningWindowArt(585, 235, 90, 42, true);
    drawCAwningWindowArt(790, 232, 58, 34, true);
}

void drawCNeckBaseArt()
{
    color(0.08f, 0.17f, 0.18f);
    cbRect(305, 38, 695, 145);

    color(0.14f, 0.28f, 0.30f);
    cbRect(320, 48, 395, 135);
    cbRect(410, 48, 485, 135);
    cbRect(505, 48, 580, 135);
    cbRect(598, 48, 680, 135);

    color(0.75f, 0.73f, 0.67f);
    cbRect(260, 145, 740, 198);

    color(0.58f, 0.56f, 0.51f);
    cbRect(260, 145, 740, 153);
    cbRect(260, 190, 740, 198);

    color(0.17f, 0.16f, 0.14f, 0.35f);
    for (float x = 285; x <= 715; x += 35)
    {
        cbLine(x, 145, x, 198);
        cbLine(x, 38, x, 145);
    }

    glLineWidth(2.0f);
    color(0.04f, 0.05f, 0.05f);
    cbLine(260, 145, 740, 145);
    cbLine(260, 198, 740, 198);
    glLineWidth(1);
}

void drawCAntennaArt()
{
    glLineWidth(2.5f);
    color(0.17f, 0.18f, 0.18f);
    cbLine(640, 1085, 640, 1190);

    glLineWidth(1.5f);
    cbLine(640, 1138, 655, 1138);
    cbLine(640, 1105, 657, 1105);

    color(0.52f, 0.52f, 0.50f);
    cbRect(626, 1110, 638, 1145);
    cbRect(642, 1140, 655, 1165);

    color(0.12f, 0.12f, 0.12f);
    cbRect(637, 1083, 644, 1094);
    glLineWidth(1);
}

void drawCOuterEdgeArt()
{
    glLineWidth(3);
    color(0.04f, 0.04f, 0.035f, 0.75f);

    glBegin(GL_LINE_STRIP);
    for (float y = C_ART_BODY_TOP - 10; y >= C_ART_BODY_BOTTOM + 10; y -= 5)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX - hw), CBY(y));
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (float y = C_ART_BODY_BOTTOM + 10; y <= C_ART_BODY_TOP - 10; y += 5)
    {
        float hw = cHalfWidthArt(y);
        glVertex2f(CBX(C_ART_CX + hw), CBY(y));
    }
    glEnd();

    glLineWidth(1);
}

void drawCBuilding()
{
    color(0.10f, 0.12f, 0.13f);
    drawRoundedRect(245, 543, 180, 28, 6);
    color(0.95f, 0.95f, 0.90f);
    drawText(283, 555, "C BUILDING");

    drawCAntennaArt();
    drawCBodyArt();
    drawCGlassBandsArt();
    drawCGlassReflectionsArt();
    drawCPanelGridArt();
    drawCSmallWindowsArt();
    drawCOuterEdgeArt();
    drawCNeckBaseArt();
}

void drawWater()

{
    if (!isNightMode)
    {
        color(0.04f, 0.40f, 0.65f);
        drawRect(waterZone.x, 73, waterZone.w, 35);

        color(0.18f, 0.68f, 0.95f);
        drawRect(waterZone.x, 95, waterZone.w, 10);

        color(0.38f, 0.78f, 0.96f);
    }
    else
    {
        color(0.03f, 0.12f, 0.24f);
        drawRect(waterZone.x, 73, waterZone.w, 35);

        color(0.28f, 0.45f, 0.68f);
        drawRect(waterZone.x, 95, waterZone.w, 10);

        color(0.72f, 0.82f, 0.96f, 0.60f);
    }

    for (float x = 295; x <= 735; x += 85)
    {
        drawCircle(x, 96, 17);
        drawCircle(x + 18, 90, 14);
        drawCircle(x - 15, 90, 12);

        drawTriangle(x - 20, 92, x - 5, 128, x + 10, 92);
        drawTriangle(x + 5, 92, x + 20, 132, x + 35, 92);
    }
}

// ======================================================
// PLATFORM STYLE
// ======================================================
void drawBrickVisual(float x, float y, float w, float h)
{
    color(0.72f, 0.40f, 0.19f);
    drawRect(x, y, w, h);

    color(0.50f, 0.24f, 0.10f);
    outlineRect(x, y, w, h);

    color(0.38f, 0.16f, 0.07f);
    drawLine(x + 10, y + h / 2, x + w - 10, y + h / 2);
    drawLine(x + w / 2, y + 3, x + w / 2, y + h - 3);

    drawCircle(x + 18, y + h - 12, 3);
    drawCircle(x + 42, y + h - 16, 3);
    drawCircle(x + 78, y + h - 12, 3);
}

void drawPlatformBlock(float x, float y, float w, float h)
{
    color(0.00f, 0.00f, 0.00f);
    drawRect(x + 6, y - 6, w, h);

    color(0.12f, 0.16f, 0.18f);
    drawRect(x, y, w, h);

    color(0.30f, 0.38f, 0.42f);
    drawRect(x, y + h - 7, w, 7);

    color(0.88f, 0.62f, 0.20f);
    glLineWidth(3);
    outlineRect(x, y, w, h);
    glLineWidth(1);
}

void drawPlatforms()
{
    for (int i = 0; i < platformCount; i++)
    {
        if (platforms[i].w <= 0 || platforms[i].h <= 0)
        {
            continue;
        }

        if (i == 0)
        {
            color(0.02f, 0.025f, 0.02f);
            drawRect(platforms[i].x, platforms[i].y, platforms[i].w, platforms[i].h);
        }
        else if (i == 6)
        {
            drawBrickVisual(platforms[i].x, 70, platforms[i].w, 30);
        }
        else
        {
            drawPlatformBlock(platforms[i].x, platforms[i].y, platforms[i].w, platforms[i].h);

            if (i == 4)
            {
                color(1.0f, 0.88f, 0.15f);
                drawText(platforms[i].x + 18, platforms[i].y + platforms[i].h + 16, "JUMP BAR");
            }
        }
    }

    if (cBrickPlaced)
    {
        drawBrickVisual(cPlacedBrickPlatform.x, 70, cPlacedBrickPlatform.w, 30);
    }
}

// ======================================================
// SPIKES
// ======================================================
void drawSpike(Spike s)
{
    float toothW = s.w / s.teeth;

    for (int i = 0; i < s.teeth; i++)
    {
        float x1 = s.x + i * toothW;
        float x2 = x1 + toothW;
        float mid = x1 + toothW / 2.0f;

        color(0.0f, 0.0f, 0.0f);
        drawTriangle(x1, s.y, x2, s.y, mid, s.y + s.h);

        color(0.22f, 0.22f, 0.22f);
        drawLine(x1, s.y, mid, s.y + s.h);
        drawLine(mid, s.y + s.h, x2, s.y);
    }
}

void drawSpikes()
{
    for (int i = 0; i < spikeCount; i++)
    {
        drawSpike(spikes[i]);
    }
}

// ======================================================
// LADDER / ITEMS
// ======================================================
void drawLadderGraphic(float x, float y, float w, float h)
{
    glLineWidth(6);

    color(0.58f, 0.35f, 0.09f);
    drawLine(x, y, x + 12, y + h);
    drawLine(x + w, y, x + w + 12, y + h);

    glLineWidth(4);

    color(0.72f, 0.48f, 0.14f);

    for (float yy = y + 20; yy < y + h - 10; yy += 38)
    {
        drawLine(x + 6, yy, x + w + 6, yy + 5);
    }

    glLineWidth(1);
}

void drawPhoneGraphic(float x, float y, float w, float h)
{
    // shadow
    color(0.0f, 0.0f, 0.0f, 0.25f);
    drawEllipse(x + w / 2, y - 5, 25, 7);

    // phone body
    color(0.04f, 0.05f, 0.06f);
    drawRoundedRect(x + 4, y, w - 8, h, 7);

    // screen
    color(0.12f, 0.55f, 0.80f);
    drawRoundedRect(x + 9, y + 11, w - 18, h - 22, 4);

    // screen shine
    color(0.75f, 0.95f, 1.0f, 0.35f);
    drawQuad(x + 12, y + h - 16,
             x + w - 14, y + h - 16,
             x + w - 24, y + h - 31,
             x + 20, y + h - 31);

    // speaker and home button
    color(0.70f, 0.72f, 0.74f);
    drawRect(x + w / 2 - 8, y + h - 8, 16, 2);
    drawCircle(x + w / 2, y + 6, 3, 16);

    // call sign
    color(0.0f, 0.85f, 0.18f);
    thickLine(x + 17, y + 30, x + 25, y + 38, 3);
    thickLine(x + 25, y + 38, x + 35, y + 32, 3);
}

void drawLadderItem()
{
    // Function name kept for compatibility, but it now draws PHONE at old ladder position.
    if (!hasPhone)
    {
        drawPhoneGraphic(ladderItem.x, ladderItem.y, ladderItem.w, ladderItem.h);

        color(1.0f, 0.88f, 0.05f);
        drawCircle(ladderItem.x + ladderItem.w / 2, ladderItem.y + ladderItem.h + 42, 20, 32);

        color(0.0f, 0.0f, 0.0f);
        drawText(ladderItem.x - 35, ladderItem.y + ladderItem.h + 18, "1) COLLECT PHONE");
    }
}

void drawPlacedLadder()
{
    // This is no longer a ladder marker. It is now the green CALL ZONE.
    if (!hasID && !idDropTriggered)
    {
        color(0.0f, 0.70f, 0.20f);
        drawRect(ladderPlaceZone.x - 18, ladderPlaceZone.y, ladderPlaceZone.w + 36, ladderPlaceZone.h + 10);

        color(1.0f, 1.0f, 0.0f);
        glLineWidth(4);
        outlineRect(ladderPlaceZone.x - 18, ladderPlaceZone.y, ladderPlaceZone.w + 36, ladderPlaceZone.h + 10);
        glLineWidth(1);

        color(0.0f, 0.0f, 0.0f);
        if (!hasPhone)
        {
            drawText(ladderPlaceZone.x - 28, ladderPlaceZone.y + 48, "2) CALL ZONE - NEED PHONE");
        }
        else
        {
            drawText(ladderPlaceZone.x - 38, ladderPlaceZone.y + 48, "2) STAND HERE + PRESS C");
        }
    }
}

void drawIDCard()
{
    if (!hasID && (idDropTriggered || idLanded))
    {
        color(0.82f, 0.92f, 0.97f);
        drawRect(idCard.x, idCard.y, idCard.w, idCard.h);

        color(0.15f, 0.25f, 0.30f);
        glLineWidth(2);
        outlineRect(idCard.x, idCard.y, idCard.w, idCard.h);
        glLineWidth(1);

        color(0.0f, 0.0f, 0.0f);
        drawText(idCard.x + 20, idCard.y + 26, "ID CARD");

        color(1.0f, 0.65f, 0.35f);
        drawRect(idCard.x + 8, idCard.y + 7, 15, 20);

        color(0.2f, 0.2f, 0.2f);
        drawLine(idCard.x + 30, idCard.y + 12, idCard.x + idCard.w - 8, idCard.y + 12);
        drawLine(idCard.x + 30, idCard.y + 20, idCard.x + idCard.w - 8, idCard.y + 20);

        if (!idLanded)
        {
            color(0.0f, 0.0f, 0.0f);
            drawText(idCard.x - 8, idCard.y + idCard.h + 15, "ID FALLING");
        }
    }
}

void drawApple()
{
    if (!appleTaken)
    {
        color(0.85f, 0.03f, 0.04f);
        drawCircle(apple.x + 18, apple.y + 18, 18);
        drawCircle(apple.x + 30, apple.y + 18, 18);

        color(0.92f, 0.14f, 0.12f);
        drawCircle(apple.x + 20, apple.y + 25, 8);

        color(0.18f, 0.42f, 0.08f);
        thickLine(apple.x + 25, apple.y + 38, apple.x + 33, apple.y + 52, 3);

        color(0.20f, 0.65f, 0.16f);
        drawEllipse(apple.x + 42, apple.y + 50, 13, 6);

        color(0.0f, 0.0f, 0.0f);
        drawText(apple.x - 8, apple.y + 62, "LIFE +1");
    }
}

// ======================================================
// C BUILDING ITEMS
// ======================================================
void drawBrickItem()
{
    if (!hasBridgeBrick)
    {
        drawBrickVisual(brickItem.x, brickItem.y, brickItem.w, brickItem.h);

        color(0.0f, 0.0f, 0.0f);
        drawText(brickItem.x - 6, brickItem.y + 44, "BRICK");
    }
}

void drawBrickPlaceZone()
{
    if (!cBrickPlaced)
    {
        color(0.36f, 0.36f, 0.38f);
        drawRect(cBrickPlaceZone.x, cBrickPlaceZone.y, cBrickPlaceZone.w, cBrickPlaceZone.h);

        color(1.0f, 0.88f, 0.30f);
        glLineWidth(2);
        outlineRect(cBrickPlaceZone.x, cBrickPlaceZone.y, cBrickPlaceZone.w, cBrickPlaceZone.h);
        glLineWidth(1);

        color(0.0f, 0.0f, 0.0f);
        drawText(cBrickPlaceZone.x - 5, cBrickPlaceZone.y + 35, "Press B");
    }
}

void drawGun()
{
    if (cBrickPlaced && !hasGun)
    {
        float x = gunItem.x;
        float y = gunItem.y;

        color(0.35f, 0.23f, 0.12f);
        drawRect(x + 42, y, 14, 18);

        color(0.35f, 0.35f, 0.35f);
        drawRect(x, y + 10, 54, 8);
        drawRect(x + 46, y + 16, 8, 9);
        drawQuad(x + 10, y + 18,
                 x + 38, y + 18,
                 x + 48, y + 28,
                 x + 18, y + 28);

        color(0.0f, 0.0f, 0.0f);
        drawText(x + 8, y + 35, "GUN");
    }
}

void drawBullets()
{
    if (cBrickPlaced && !hasBullets)
    {
        float x = bulletsItem.x;
        float y = bulletsItem.y;

        for (int i = 0; i < 3; i++)
        {
            float bx = x + i * 16;

            color(0.10f, 0.15f, 0.85f);
            drawRect(bx, y, 10, 28);

            color(0.14f, 0.14f, 0.95f);
            drawTriangle(bx, y + 28, bx + 5, y + 38, bx + 10, y + 28);

            color(0.86f, 0.90f, 1.0f);
            drawRect(bx + 2, y + 18, 6, 8);
        }

        color(0.0f, 0.0f, 0.0f);
        drawText(x - 4, y + 48, "BULLETS");
    }
}

void drawKnife()
{
    if (!hasKnife)
    {
        float x = knifeItem.x;
        float y = knifeItem.y;

        color(0.78f, 0.78f, 0.82f);
        drawQuad(x, y + 5,
                 x + 20, y + 14,
                 x + 34, y + 8,
                 x + 14, y + 1);

        color(0.28f, 0.28f, 0.28f);
        drawRect(x + 30, y + 3, 12, 6);

        color(0.0f, 0.0f, 0.0f);
        drawText(x - 4, y + 24, "KNIFE");
    }
}

// ======================================================
// PLAYER / POLICE / REALISTIC GATE
// ======================================================
void drawRoundedRect(float x, float y, float w, float h, float r)
{
    if (r < 1.0f)
    {
        drawRect(x, y, w, h);
        return;
    }

    drawRect(x + r, y, w - 2 * r, h);
    drawRect(x, y + r, w, h - 2 * r);
    drawCircle(x + r, y + r, r, 28);
    drawCircle(x + w - r, y + r, r, 28);
    drawCircle(x + r, y + h - r, r, 28);
    drawCircle(x + w - r, y + h - r, r, 28);
}

void drawShoe(float x, float y, bool right)
{
    color(0.02f, 0.025f, 0.03f);
    if (right)
    {
        drawRoundedRect(x, y, 20, 7, 3);
        drawRect(x + 12, y + 3, 10, 3);
    }
    else
    {
        drawRoundedRect(x - 8, y, 20, 7, 3);
        drawRect(x - 12, y + 3, 10, 3);
    }
}

void drawPlayer()
{
    float x = px + pw / 2.0f;
    float y = py;
    float walk = sin(px * 0.08f) * 2.0f;
    float s = 0.70f; // smaller than before, so starting view is not too zoomed

    // shadow
    color(0.0f, 0.0f, 0.0f, 0.20f);
    drawEllipse(x, y - 3, 22 * s, 6 * s, 24);

    // white pajama
    color(0.96f, 0.95f, 0.90f);
    drawRoundedRect(x - 11 * s, y + 4, 7 * s, 32 * s + walk, 3 * s);
    drawRoundedRect(x + 4 * s, y + 4, 7 * s, 32 * s - walk, 3 * s);

    // shoes
    color(0.02f, 0.02f, 0.025f);
    drawRoundedRect(x - 15 * s, y, 13 * s, 5 * s, 2 * s);
    drawRoundedRect(x + 2 * s, y, 14 * s, 5 * s, 2 * s);

    // blue panjabi back
    color(0.05f, 0.21f, 0.70f);
    drawRoundedRect(x - 18 * s, y + 31 * s, 36 * s, 48 * s, 6 * s);
    color(0.16f, 0.42f, 0.94f);
    drawRoundedRect(x - 13 * s, y + 38 * s, 26 * s, 32 * s, 5 * s);

    // back seam + Boishakh uttorio on back
    color(0.95f, 0.84f, 0.28f);
    drawRect(x - 2 * s, y + 34 * s, 4 * s, 41 * s);
    color(0.95f, 0.05f, 0.04f);
    drawRect(x - 17 * s, y + 64 * s, 34 * s, 5 * s);
    color(1.0f, 0.88f, 0.15f);
    drawRect(x - 3 * s, y + 34 * s, 6 * s, 35 * s);

    // arms from back
    color(0.05f, 0.21f, 0.70f);
    thickLine(x - 18 * s, y + 61 * s, x - 29 * s, y + 40 * s, 5 * s);
    thickLine(x + 18 * s, y + 61 * s, x + 29 * s, y + 40 * s, 5 * s);
    color(0.95f, 0.67f, 0.49f);
    drawCircle(x - 30 * s, y + 38 * s, 4 * s, 18);
    drawCircle(x + 30 * s, y + 38 * s, 4 * s, 18);

    // neck, back head, hair/cap. No eyes, no nose, no mouth.
    color(0.95f, 0.67f, 0.49f);
    drawRoundedRect(x - 6 * s, y + 78 * s, 12 * s, 7 * s, 2 * s);
    drawCircle(x, y + 94 * s, 15 * s, 32);

    // dark back hair covers front-face features; this makes face clearly piser dike.
    color(0.04f, 0.025f, 0.015f);
    drawEllipse(x, y + 99 * s, 16 * s, 13 * s, 32);
    drawRect(x - 14 * s, y + 88 * s, 28 * s, 11 * s);

    // hero blue cap/topi from the back
    color(0.07f, 0.16f, 0.52f);
    drawRoundedRect(x - 15 * s, y + 105 * s, 30 * s, 7 * s, 3 * s);
    color(0.95f, 0.82f, 0.24f);
    drawRect(x - 4 * s, y + 106 * s, 8 * s, 5 * s);

    if (hasGun)
    {
        color(0.06f, 0.06f, 0.08f);
        drawRect(x + 23 * s, y + 39 * s, 22 * s, 5 * s);
        drawRect(x + 39 * s, y + 36 * s, 7 * s, 3 * s);
    }
}

void drawPatrolMan(float x, float y, bool moving)
{
    float step = moving ? sin(x * 0.05f) * 2.8f : 0.0f;

    color(0.0f, 0.0f, 0.0f, 0.24f);
    drawEllipse(x + 24, y - 4, 31, 7);

    // ash color patrol uniform
    color(0.46f, 0.48f, 0.50f);
    drawRoundedRect(x + 8, y + 5, 11, 37 + step, 4);
    drawRoundedRect(x + 27, y + 5, 11, 37 - step, 4);

    color(0.12f, 0.13f, 0.15f);
    drawRoundedRect(x + 4, y + 1, 18, 7, 3);
    drawRoundedRect(x + 24, y + 1, 20, 7, 3);

    color(0.54f, 0.56f, 0.59f);
    drawRoundedRect(x + 4, y + 39, 40, 47, 8);
    color(0.68f, 0.70f, 0.73f);
    drawRoundedRect(x + 8, y + 45, 32, 34, 6);

    // belt, pocket line and badge
    color(0.13f, 0.13f, 0.14f);
    drawRect(x + 5, y + 42, 38, 6);
    drawRect(x + 11, y + 58, 10, 2);
    drawRect(x + 27, y + 58, 10, 2);
    color(0.90f, 0.78f, 0.25f);
    drawCircle(x + 32, y + 67, 4, 18);
    color(0.24f, 0.24f, 0.26f);
    drawTriangle(x + 21, y + 81, x + 27, y + 81, x + 24, y + 55);

    // shoulders / arms
    color(0.60f, 0.62f, 0.65f);
    thickLine(x + 7, y + 73, x - 7, y + 55, 7);
    thickLine(x + 41, y + 73, x + 55, y + 56, 7);

    color(1.0f, 0.76f, 0.60f);
    drawCircle(x - 8, y + 53, 4, 18);
    drawCircle(x + 56, y + 54, 4, 18);

    // baton / radio
    color(0.16f, 0.12f, 0.08f);
    if (moving)
        thickLine(x + 54, y + 53, x + 68, y + 28, 4);
    else
        thickLine(x - 8, y + 53, x - 20, y + 30, 4);

    color(0.20f, 0.20f, 0.22f);
    drawRoundedRect(x + 8, y + 55, 5, 10, 2);

    // head
    color(1.0f, 0.77f, 0.60f);
    drawEllipse(x + 24, y + 101, 14, 15, 34);

    // ash cap tied / fitted on head
    color(0.33f, 0.35f, 0.38f);
    drawRoundedRect(x + 11, y + 112, 27, 8, 4);
    drawQuad(x + 8, y + 111, x + 40, y + 111, x + 36, y + 105, x + 12, y + 105);
    color(0.22f, 0.23f, 0.25f);
    drawRect(x + 14, y + 108, 20, 3);
    color(0.90f, 0.78f, 0.25f);
    drawCircle(x + 24, y + 116, 2.8f, 16);

    // face details
    color(0.02f, 0.02f, 0.02f);
    drawCircle(x + 18, y + 101, 1.5f, 12);
    drawCircle(x + 29, y + 101, 1.5f, 12);
    thickLine(x + 18, y + 93, x + 30, y + 93, 2);

    color(0.02f, 0.02f, 0.02f);
    if (moving)
        drawText(x - 12, y + 130, "PATROL MAN");
    else
        drawText(x - 16, y + 130, "SECURITY PATROL");
}

void drawGuard(float x, float y, bool moving)
{
    drawPatrolMan(x, y, moving);
}

void drawGatePanel(float x, float y, float w, float h, bool leftPanel)
{
    // iron frame
    color(0.05f, 0.06f, 0.07f);
    drawRoundedRect(x, y, w, h, 3);

    color(0.22f, 0.24f, 0.25f);
    drawRect(x + 5, y + 8, w - 10, h - 16);

    // vertical bars
    color(0.02f, 0.025f, 0.03f);
    for (float bx = x + 12; bx < x + w - 8; bx += 14)
    {
        drawRoundedRect(bx, y + 12, 5, h - 24, 2);
        drawCircle(bx + 2.5f, y + h - 9, 4, 16);
    }

    // diagonal cross braces
    color(0.45f, 0.34f, 0.16f);
    thickLine(x + 8, y + 18, x + w - 8, y + h - 20, 4);
    thickLine(x + w - 8, y + 18, x + 8, y + h - 20, 4);

    // red access stripe
    color(0.70f, 0.04f, 0.03f);
    drawRect(x + 4, y + h - 22, w - 8, 7);
}

void drawGate()
{
    float gx = gateBlock.x;
    float gy = gateBlock.y;
    float postW = 34;
    float gateH = 165;
    float gapW = 120;
    float rightPostX = gx + gapW + postW;

    // concrete base / ground rail
    color(0.10f, 0.10f, 0.10f);
    drawRect(gx - 12, gy - 5, gapW + postW * 2 + 24, 10);

    color(0.55f, 0.55f, 0.52f);
    drawRect(gx - 4, gy - 1, gapW + postW * 2 + 8, 4);

    // brick pillars
    color(0.62f, 0.22f, 0.10f);
    drawRoundedRect(gx, gy, postW, gateH, 5);
    drawRoundedRect(rightPostX, gy, postW, gateH, 5);

    color(0.82f, 0.42f, 0.22f);
    for (float yy = gy + 14; yy < gy + gateH - 10; yy += 22)
    {
        drawLine(gx + 4, yy, gx + postW - 4, yy);
        drawLine(rightPostX + 4, yy, rightPostX + postW - 4, yy);
    }

    color(0.18f, 0.06f, 0.03f);
    glLineWidth(2);
    outlineRect(gx, gy, postW, gateH);
    outlineRect(rightPostX, gy, postW, gateH);
    glLineWidth(1);

    // caps and lamps
    color(0.18f, 0.18f, 0.17f);
    drawQuad(gx - 7, gy + gateH, gx + postW + 7, gy + gateH,
             gx + postW + 1, gy + gateH + 15, gx - 1, gy + gateH + 15);
    drawQuad(rightPostX - 7, gy + gateH, rightPostX + postW + 7, gy + gateH,
             rightPostX + postW + 1, gy + gateH + 15, rightPostX - 1, gy + gateH + 15);

    color(1.0f, 0.78f, 0.22f, 0.80f);
    drawCircle(gx + postW / 2, gy + gateH + 23, 8, 22);
    drawCircle(rightPostX + postW / 2, gy + gateH + 23, 8, 22);

    // top arch sign
    color(0.12f, 0.12f, 0.13f);
    drawRoundedRect(gx + 10, gy + gateH + 42, gapW + postW - 20, 18, 6);
    color(0.95f, 0.92f, 0.78f);
    drawText(gx + 28, gy + gateH + 55, "SECURITY GATE");

    if (!gateOpen)
    {
        // closed double gate
        drawGatePanel(gx + postW, gy + 8, gapW / 2 + 5, gateH - 18, true);
        drawGatePanel(gx + postW + gapW / 2 - 5, gy + 8, gapW / 2 + 5, gateH - 18, false);

        // lock box / scanner
        color(0.08f, 0.09f, 0.09f);
        drawRoundedRect(gx + postW + gapW / 2 - 12, gy + 74, 24, 28, 4);
        color(0.90f, 0.10f, 0.08f);
        drawCircle(gx + postW + gapW / 2, gy + 92, 4, 16);

        color(0.0f, 0.0f, 0.0f);
        drawText(gx - 25, gy + gateH + 75, "SHOW ID - PRESS E");
    }
    else
    {
        // opened panels sliding into sides
        drawGatePanel(gx + postW - 48, gy + 8, 50, gateH - 18, true);
        drawGatePanel(rightPostX + postW - 2, gy + 8, 50, gateH - 18, false);

        color(0.10f, 0.58f, 0.15f);
        drawRoundedRect(gx + postW + 25, gy + 72, gapW - 50, 35, 8);

        color(0.92f, 1.0f, 0.90f);
        drawText(gx + postW + 40, gy + 94, "OPEN");
    }
}

// ======================================================
// LEVEL 2 INSTRUCTION BAR + MESSAGE
// ======================================================
const char* getLevel2Objective()
{
    if (!hasPhone)
        return "Objective: Move left and collect the PHONE.";

    if (!phoneCallMade)
        return "Objective: Stand on green CALL ZONE and press C to call your friend.";

    if (!idLanded && !hasID)
        return "Objective: Wait for the ID CARD to drop from D Building.";

    if (!hasID)
        return "Objective: Collect the dropped ID CARD.";

    if (!gateOpen)
        return "Objective: Stand near the security guard and press E to show the ID.";

    if (!hasBridgeBrick)
        return "Objective: Enter C Building area and collect the BRICK.";

    if (!cBrickPlaced)
        return "Objective: Stand near the water marker and press B to place the brick.";

    if (!hasGun || !hasBullets || !hasKnife)
        return "Objective: Cross safely and collect GUN, BULLETS, and KNIFE.";

    return "Objective: Go to the far-left end to enter Level 3.";
}

void drawLevel2InstructionBar()
{
    color(0.02f, 0.02f, 0.04f, 0.82f);
    drawRoundedRect(camX + 14, WIN_H - 48, 972, 36, 8);

    color(0.95f, 0.66f, 0.12f, 0.95f);
    drawRect(camX + 14, WIN_H - 48, 972, 4);

    char status[260];
    sprintf(status, "LEVEL 2 | Lives: %d | Apple Life: %d | Phone:%s | ID:%s | Gate:%s",
            lives,
            appleScore,
            hasPhone ? "YES" : "NO",
            hasID ? "YES" : "NO",
            gateOpen ? "OPEN" : "LOCKED");

    color(1.0f, 1.0f, 1.0f);
    drawText(camX + 26, WIN_H - 27, status);

    color(0.78f, 0.90f, 1.0f);
    drawText(camX + 565, WIN_H - 27, getLevel2Objective());
}

void drawGameMessage()
{
    // Level 2 navbar/message bar restored exactly like the original style.
    // No top HUD is drawn in Level 2.
    if (messageTimer > 0)
    {
        color(0.0f, 0.0f, 0.0f, 0.72f);
        drawRect(camX + 15, 14, 760, 32);

        color(1.0f, 1.0f, 1.0f);
        drawText(camX + 25, 25, messageText);
    }

    if (levelComplete)
    {
        color(0.0f, 0.45f, 0.0f);
        drawText(camX + 390, 300, "MISSION COMPLETE!");
    }
}

// ======================================================
// START / INFO SCREENS
// ======================================================
void drawStartScreen()
{
    setScreenProjection();

    glBegin(GL_QUADS);
        color(0.08f, 0.12f, 0.18f);
        glVertex2f(0, WIN_H);
        glVertex2f(WIN_W, WIN_H);

        color(0.16f, 0.25f, 0.34f);
        glVertex2f(WIN_W, 0);
        glVertex2f(0, 0);
    glEnd();

    color(0.95f, 0.95f, 0.90f);
    drawTextBig(345, 420, "AIUB IMPOSTER DETECTION");

    color(0.45f, 0.75f, 1.0f);
    drawTextBig(395, 350, "PRESS  S  TO START");

    color(1.0f, 0.86f, 0.35f);
    drawTextBig(405, 310, "PRESS  I  FOR INFO");

    color(0.85f, 0.90f, 0.95f);
    drawText(225, 250, "Mission: Phone -> ID -> C Building items -> Panta Ilish Run-Jump Level -> Concert.");
}

void drawInfoScreen()
{
    setScreenProjection();

    color(0.05f, 0.08f, 0.12f);
    drawRect(0, 0, WIN_W, WIN_H);

    color(0.95f, 0.95f, 0.90f);
    drawTextBig(415, 510, "GAME INFO");

    color(0.75f, 0.90f, 1.0f);
    drawText(220, 440, "A / D or Arrow Keys : Move left and right");
    drawText(220, 410, "W / UP / Space      : Jump");
    drawText(220, 380, "C                   : Call friend from green CALL ZONE");
    drawText(220, 350, "E                   : Show ID to guard");
    drawText(220, 320, "B                   : Place brick in front of water");
    drawText(220, 290, "R                   : Reset game");
    drawText(220, 260, "N                   : Toggle day / night mode in every level | P: test Panta level");

    color(1.0f, 0.86f, 0.35f);
    drawTextBig(340, 170, "PRESS  S  TO START");
    drawTextBig(330, 130, "PRESS  I  TO GO BACK");
}

// ======================================================
// BACKGROUND COMPOSITION
// ======================================================
void drawBackgroundScenario()
{
    drawSkyAndGround();

    drawCBuilding();
    drawWater();

    drawBush(45, 92, 0.86f);
    drawBush(130, 90, 0.74f);

    drawLeftBuildingArt(SHIFT);
    drawDBuilding();
    drawIDThrowPerson();

    drawBush(O(40), 92, 0.70f);
    drawBush(O(135), 90, 0.68f);
    drawBush(O(880), 92, 0.70f);

    if (isNightMode)
    {
        color(0.03f, 0.05f, 0.12f, 0.20f);
        drawRect(camX, 100, WIN_W, WIN_H - 100);
    }
}

// ======================================================
// RESET
// ======================================================
void resetPlayerOnly()
{
    px = 3200;
    py = 100;

    vx = 0;
    vy = 0;

    onGround = false;
    onLadder = false;

    faceRight = false;

    camX = WORLD_W - WIN_W;
}

void fullReset()
{
    resetPlayerOnly();

    lives = 3;

    hasPhone = false;
    phoneCallMade = false;

    hasLadder = false;
    ladderPlaced = false;
    hasID = false;

    idDropTriggered = false;
    idLanded = false;
    idVelY = 0.0f;
    idWindowX = 0.0f;
    idWindowY = 0.0f;
    idCard.x = O(1005);
    idCard.y = 455;
    idCard.w = 120;
    idCard.h = 45;

    gateOpen = false;
    levelComplete = false;

    appleTaken = false;
    appleScore = 0;

    hasBridgeBrick = false;
    cBrickPlaced = false;

    hasGun = false;
    hasBullets = false;
    hasKnife = false;

    guard1X = O(1600);
    guard1VX = 1.8f;

    setMessage("Game restarted. Collect phone, go to CALL ZONE, press C, then collect the dropped ID.");
}

void playerDeath(const char* reason)
{
    lives--;

    if (lives <= 0)
    {
        fullReset();
        setMessage("Game over. Game restarted. Press R anytime to restart manually.");
    }
    else
    {
        resetPlayerOnly();
        char deathMsg[260];
        snprintf(deathMsg, sizeof(deathMsg), "%s Press R to restart anytime.", reason);
        setMessage(deathMsg);
    }
}

// ======================================================
// LOGIC HELPERS
// ======================================================
bool nearPlacedLadder()
{
    // Ladder gameplay is disabled. ID collection is now controlled by phone + CALL ZONE.
    return false;
}

bool nearPlaceZone()
{
    Rect zone =
    {
        ladderPlaceZone.x - 40,
        ladderPlaceZone.y,
        ladderPlaceZone.w + 80,
        ladderPlaceZone.h + 80
    };

    return collide(playerRect(), zone);
}

bool nearFinalGuard()
{
    Rect area =
    {
        finalGuard.x - 90,
        finalGuard.y,
        finalGuard.w + 150,
        finalGuard.h + 80
    };

    return collide(playerRect(), area);
}

bool nearCBrickPlaceZone()
{
    Rect zone =
    {
        cBrickPlaceZone.x - 35,
        cBrickPlaceZone.y,
        cBrickPlaceZone.w + 70,
        cBrickPlaceZone.h + 80
    };

    return collide(playerRect(), zone);
}

// ======================================================
// ACTIONS
// ======================================================
void tryJump()
{
    if (onGround && !nearPlacedLadder())
    {
        vy = JUMP_POWER;
        onGround = false;
        onLadder = false;
    }
}

void tryCallFriend()
{
    if (phoneCallMade || idDropTriggered)
    {
        setMessage("Already called. Wait for the ID to fall, then collect it.");
        return;
    }

    if (!hasPhone)
    {
        setMessage("Collect the PHONE first.");
        return;
    }

    if (!nearPlaceZone())
    {
        setMessage("Stand on the green CALL ZONE and press C.");
        return;
    }

    phoneCallMade = true;
    idDropTriggered = true;
    idLanded = false;

    idWindowX = DX(350);
    idWindowY = DY(430);

    idCard.x = idWindowX;
    idCard.y = idWindowY;
    idCard.w = 82;
    idCard.h = 34;

    idVelY = -1.5f;

    setMessage("Call done. Friend is dropping the ID card from D Building window.");
}

void tryPlaceLadder()
{
    setMessage("Ladder is removed in this version. Use PHONE, stand on CALL ZONE, then press C.");
}

void tryShowID()
{
    if (!nearFinalGuard())
    {
        setMessage("Go near the final guard first.");
        return;
    }

    if (!hasID)
    {
        setMessage("Guard: You cannot enter without ID card.");
        return;
    }

    gateOpen = true;
    setMessage("Gate opened. Continue left to C Building.");
}

void tryPlaceBridgeBrick()
{
    if (cBrickPlaced)
    {
        setMessage("Brick is already placed.");
        return;
    }

    if (!hasBridgeBrick)
    {
        setMessage("Collect the BRICK from the C Building bar first.");
        return;
    }

    if (!nearCBrickPlaceZone())
    {
        setMessage("Stand near the water-side marker and press B.");
        return;
    }

    cBrickPlaced = true;
    setMessage("Brick placed. Cross water and collect gun, bullets, and knife.");
}

// ======================================================
// COLLISION HELPERS
// ======================================================
void collideWithPlatform(Rect p, Rect& pr)
{
    if (collide(pr, p))
    {
        if (vx > 0)
        {
            px = p.x - pw;
        }

        if (vx < 0)
        {
            px = p.x + p.w;
        }

        pr = playerRect();
    }
}

void verticalResolvePlatform(Rect p, Rect& pr, float oldY)
{
    if (collide(pr, p))
    {
        float platformTop = p.y + p.h;
        float playerOldBottom = oldY;
        float playerOldTop = oldY + ph;

        if (vy <= 0 && playerOldBottom >= platformTop)
        {
            py = platformTop;
            vy = 0;
            onGround = true;
        }
        else if (vy > 0 && playerOldTop <= p.y)
        {
            py = p.y - ph;
            vy = 0;
        }

        pr = playerRect();
    }
}

// ======================================================
// UPDATE FUNCTIONS
// ======================================================
void updateGuard()
{
    guard1X += guard1VX;

    if (guard1X < guard1Left)
    {
        guard1X = guard1Left;
        guard1VX = -guard1VX;
    }

    if (guard1X > guard1Right)
    {
        guard1X = guard1Right;
        guard1VX = -guard1VX;
    }

    Rect guardRect =
    {
        guard1X,
        guard1Y,
        guard1W,
        guard1H
    };

    if (collide(playerRect(), guardRect))
    {
        playerDeath("Patrol guard caught you.");
    }
}

void horizontalCollision()
{
    Rect pr = playerRect();

    for (int i = 0; i < platformCount; i++)
    {
        if (platforms[i].w <= 0 || platforms[i].h <= 0)
        {
            continue;
        }

        Rect p =
        {
            platforms[i].x,
            platforms[i].y,
            platforms[i].w,
            platforms[i].h
        };

        collideWithPlatform(p, pr);
    }

    if (cBrickPlaced)
    {
        Rect p = cPlacedBrickPlatform;
        collideWithPlatform(p, pr);
    }

    if (!gateOpen)
    {
        Rect gate = gateBlock;

        if (collide(pr, gate))
        {
            if (vx < 0)
            {
                px = gate.x + gate.w;
                setMessage("Gate locked. Show ID to the guard using E.");
            }

            if (vx > 0)
            {
                px = gate.x - pw;
            }
        }
    }
}

void verticalCollision(float oldY)
{
    onGround = false;

    Rect pr = playerRect();

    for (int i = 0; i < platformCount; i++)
    {
        if (platforms[i].w <= 0 || platforms[i].h <= 0)
        {
            continue;
        }

        Rect p =
        {
            platforms[i].x,
            platforms[i].y,
            platforms[i].w,
            platforms[i].h
        };

        verticalResolvePlatform(p, pr, oldY);
    }

    if (cBrickPlaced)
    {
        Rect p = cPlacedBrickPlatform;
        verticalResolvePlatform(p, pr, oldY);
    }
}

void updatePlayer()
{
    if (levelComplete)
    {
        return;
    }

    vx = 0;

    if (keyLeft)
    {
        vx = -SPEED;
        faceRight = false;
    }

    if (keyRight)
    {
        vx = SPEED;
        faceRight = true;
    }

    px += vx;

    if (px < 0)
    {
        px = 0;
    }

    if (px + pw > WORLD_W)
    {
        px = WORLD_W - pw;
    }

    horizontalCollision();

    bool nearLadderNow = nearPlacedLadder();

    if (nearLadderNow && (keyUp || keyDown || onLadder))
    {
        onLadder = true;
        vy = 0;

        px = placedLadderX + placedLadderW / 2.0f - pw / 2.0f;

        if (keyUp)
        {
            py += CLIMB_SPEED;
        }

        if (keyDown)
        {
            py -= CLIMB_SPEED;
        }

        if (py < placedLadderY)
        {
            py = placedLadderY;
            onLadder = false;
            onGround = true;
        }

        if (py > 505)
        {
            py = 505;
            onLadder = false;
            onGround = true;
        }
    }
    else
    {
        onLadder = false;

        float oldY = py;

        vy += GRAVITY;

        if (vy < -15)
        {
            vy = -15;
        }

        py += vy;

        verticalCollision(oldY);
    }

    if (py < -100)
    {
        playerDeath("You fell down.");
    }
}

void updateIDDrop()
{
    if (hasID)
    {
        return;
    }

    if (idDropTriggered && !idLanded)
    {
        idVelY += GRAVITY * 0.45f;
        idCard.y += idVelY;

        if (idCard.y <= 125)
        {
            idCard.y = 125;
            idVelY = 0.0f;
            idLanded = true;

            setMessage("ID card dropped. Collect it, then show it to the guard.");
        }
    }
}

void updateItems()
{
    if (!hasPhone && collide(playerRect(), ladderItem))
    {
        hasPhone = true;
        setMessage("Phone collected. Go to the green CALL ZONE and press C to call your friend.");
    }

    if (!hasID && idLanded && collide(playerRect(), idCard))
    {
        hasID = true;
        setMessage("ID card collected. Show it to the guard.");
    }

    if (!appleTaken && collide(playerRect(), apple))
    {
        appleTaken = true;
        appleScore++;
        setMessage("Life apple collected. +1 extra retry for Level 3.");
    }

    if (!hasBridgeBrick && collide(playerRect(), brickItem))
    {
        hasBridgeBrick = true;
        setMessage("Brick collected. Go to water marker and press B.");
    }

    if (cBrickPlaced && !hasGun && collide(playerRect(), gunItem))
    {
        hasGun = true;
        setMessage("Gun collected.");
    }

    if (cBrickPlaced && !hasBullets && collide(playerRect(), bulletsItem))
    {
        hasBullets = true;
        setMessage("Bullets collected.");
    }

    if (!hasKnife && collide(playerRect(), knifeItem))
    {
        hasKnife = true;
        setMessage("Knife collected.");
    }
}

void updateSpikes()
{
    for (int i = 0; i < spikeCount; i++)
    {
        Rect sp =
        {
            spikes[i].x + 12,
            spikes[i].y + 5,
            spikes[i].w - 24,
            spikes[i].h - 10
        };

        if (collide(playerRect(), sp))
        {
            playerDeath("Spikes hit you.");
            return;
        }
    }
}

void updateWater()
{
    Rect pr = playerRect();

    bool inWaterX =
        pr.x + pr.w > waterZone.x &&
        pr.x < waterZone.x + waterZone.w;

    if (inWaterX && py < 110)
    {
        bool safe = false;

        Rect fixedBrick =
        {
            400,
            40,
            130,
            60
        };

        if (pr.x + pr.w > fixedBrick.x && pr.x < fixedBrick.x + fixedBrick.w)
        {
            safe = true;
        }

        if (cBrickPlaced)
        {
            if (pr.x + pr.w > cPlacedBrickPlatform.x &&
                pr.x < cPlacedBrickPlatform.x + cPlacedBrickPlatform.w)
            {
                safe = true;
            }
        }

        Rect safeBank =
        {
            cBrickPlaceZone.x - 25,
            cBrickPlaceZone.y,
            cBrickPlaceZone.w + 50,
            cBrickPlaceZone.h + 80
        };

        if (collide(pr, safeBank))
        {
            safe = true;
        }

        if (!safe)
        {
            playerDeath("You fell into the water. Place the brick first.");
            return;
        }
    }
}

void updateCamera()
{
    camX = px - WIN_W / 2.0f;

    if (camX < 0)
    {
        camX = 0;
    }

    if (camX > WORLD_W - WIN_W)
    {
        camX = WORLD_W - WIN_W;
    }
}

void checkFinish()
{
    if (hasGun && hasBullets && hasKnife && px < 60 && appPhase == PHASE_LEVEL2)
    {
        levelComplete = true;
        setMessage("Level 2 complete. Panta Ilish Break starts now.");
        startPantaLevel();
    }
}

void updateGame()
{
    if (!gameStarted || infoMode)
    {
        return;
    }

    if (messageTimer > 0)
    {
        messageTimer--;
    }

    updateGuard();
    updatePlayer();
    updateIDDrop();
    updateItems();
    updateSpikes();
    updateWater();
    checkFinish();
    updateCamera();
}


// ======================================================
// LEVEL TRANSITION SCREEN
// ======================================================
void drawTransitionScreen()
{
    setScreenProjection();

    glBegin(GL_QUADS);
        color(0.02f, 0.03f, 0.06f);
        glVertex2f(0, WIN_H);
        glVertex2f(WIN_W, WIN_H);
        color(0.10f, 0.13f, 0.20f);
        glVertex2f(WIN_W, 0);
        glVertex2f(0, 0);
    glEnd();

    int now = glutGet(GLUT_ELAPSED_TIME);
    float t = (now - transitionStartTime) / 1800.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    color(0.95f, 0.95f, 0.90f);
    drawTextBig(365, 350, "LEVEL 2 COMPLETE");

    color(0.45f, 0.75f, 1.0f);
    drawTextBig(245, 300, "NEXT: PANTA ILISH RUN-JUMP LEVEL...");

    color(0.90f, 0.70f, 0.22f);
    drawRect(300, 250, 400, 18);
    color(0.20f, 0.85f, 0.35f);
    drawRect(300, 250, 400 * t, 18);

    char buf[120];
    sprintf(buf, "Eat panta, then concert starts. Lives later: %d", lives + appleScore);
    color(0.85f, 0.90f, 0.95f);
    drawText(392, 215, buf);
}


// ======================================================
// EXTRA BOISHAKH ENTRY LEVEL DRAWING / LOGIC
// ======================================================
void setEntryMessage(const char* msg, int timer = 360)
{
    strcpy(entryMsg, msg);
    entryMsgTimer = timer;
}

void startBoishakhEntryLevel()
{
    appPhase = PHASE_BOISHAKH_ENTRY;
    entryHeroX = 95.0f;
    entryHeroY = 105.0f;
    entryHeroVX = 0.0f;
    entryHeroVY = 0.0f;
    entryHeroGround = true;
    entryPassCollected = false;
    keyLeft = keyRight = keyUp = keyDown = false;
    setEntryMessage("Extra Level: Boishakh gate-e dhukar age ENTRY PASS collect koro.");
}

void drawBackPerson2D(float x, float y, float s, float r, float g, float b, bool hero = false)
{
    // Shadow
    color(0.0f, 0.0f, 0.0f, 0.22f);
    drawEllipse(x, y - 4 * s, 23 * s, 6 * s, 24);

    // Legs and shoes
    color(0.02f, 0.03f, 0.06f);
    drawRoundedRect(x - 10 * s, y, 6 * s, 27 * s, 2 * s);
    drawRoundedRect(x + 4 * s, y, 6 * s, 27 * s, 2 * s);
    color(0.0f, 0.0f, 0.0f);
    drawRoundedRect(x - 13 * s, y - 4 * s, 11 * s, 5 * s, 2 * s);
    drawRoundedRect(x + 3 * s, y - 4 * s, 12 * s, 5 * s, 2 * s);

    // Body - back side, no face
    color(r, g, b);
    drawRoundedRect(x - 16 * s, y + 25 * s, 32 * s, 42 * s, 5 * s);
    color(r * 0.80f, g * 0.80f, b * 0.80f);
    drawRect(x - 13 * s, y + 29 * s, 26 * s, 10 * s);

    // Boishakh uttorio/scarf on back
    color(0.95f, 0.05f, 0.04f);
    drawRect(x - 15 * s, y + 58 * s, 30 * s, 5 * s);
    color(1.0f, 0.86f, 0.15f);
    drawRect(x - 3 * s, y + 35 * s, 6 * s, 28 * s);

    // Arms
    color(0.95f, 0.69f, 0.50f);
    drawRoundedRect(x - 25 * s, y + 30 * s, 7 * s, 29 * s, 3 * s);
    drawRoundedRect(x + 18 * s, y + 30 * s, 7 * s, 29 * s, 3 * s);

    // Neck and back of head - face is turned away from camera.
    drawRect(x - 6 * s, y + 66 * s, 12 * s, 7 * s);
    color(0.94f, 0.68f, 0.50f);
    drawCircle(x, y + 84 * s, 15 * s, 32);

    // Dark back hair covers the facial side. No eyes/nose/mouth anywhere.
    color(0.025f, 0.018f, 0.012f);
    drawEllipse(x, y + 89 * s, 18 * s, 15 * s, 32);
    drawRect(x - 16 * s, y + 77 * s, 32 * s, 13 * s);
    color(0.06f, 0.035f, 0.02f);
    drawEllipse(x, y + 78 * s, 13 * s, 5 * s, 20);

    if (hero)
    {
        color(0.07f, 0.18f, 0.55f);
        drawRoundedRect(x - 16 * s, y + 91 * s, 32 * s, 8 * s, 4 * s);
        color(0.95f, 0.82f, 0.24f);
        drawRect(x - 4 * s, y + 92 * s, 8 * s, 6 * s);
    }
}


void setPantaMessage(const char* msg, int timer = 360)
{
    strcpy(pantaMsg, msg);
    pantaMsgTimer = timer;
}

void startPantaLevel()
{
    appPhase = PHASE_PANTA_LEVEL;
    pantaHeroX = 85.0f;
    pantaHeroY = 105.0f;
    pantaHeroVX = 0.0f;
    pantaHeroVY = 0.0f;
    pantaHeroGround = true;
    pantaEaten = false;
    keyLeft = keyRight = keyUp = keyDown = false;
    setPantaMessage("Panta Level: Run right, jump onto the Panta Ilish plate, then go to CONCERT GATE.");
}

void drawPantaPlate(float x, float y, bool large=false)
{
    float s = large ? 1.25f : 1.0f;
    color(0.95f, 0.94f, 0.86f);
    drawEllipse(x, y, 32*s, 13*s, 32);
    color(0.88f, 0.88f, 0.78f);
    drawEllipse(x, y+1*s, 24*s, 8*s, 28);
    color(1.0f, 1.0f, 0.93f);
    drawCircle(x-8*s, y+3*s, 8*s, 18);
    drawCircle(x+4*s, y+4*s, 7*s, 18);
    color(0.72f, 0.34f, 0.07f);
    drawEllipse(x+18*s, y+3*s, 13*s, 5*s, 20);
    color(0.95f, 0.65f, 0.10f);
    drawTriangle(x+7*s, y+4*s, x+25*s, y+8*s, x+27*s, y-2*s);
    color(0.05f, 0.45f, 0.10f);
    drawCircle(x-22*s, y+6*s, 4*s, 12);
}


void drawScreenSkyNoBlack()
{
    if (!isNightMode)
    {
        glBegin(GL_QUADS);
            glColor3f(0.78f, 0.93f, 1.0f); glVertex2f(0, WIN_H); glVertex2f(WIN_W, WIN_H);
            glColor3f(0.96f, 0.99f, 1.0f); glVertex2f(WIN_W, 0); glVertex2f(0, 0);
        glEnd();

        color(1.0f, 0.90f, 0.35f, 0.96f);
        drawCircle(855, 515, 38, 36);
        color(1.0f, 0.96f, 0.60f, 0.25f);
        drawCircle(855, 515, 62, 36);
    }
    else
    {
        glBegin(GL_QUADS);
            glColor3f(0.03f, 0.06f, 0.15f); glVertex2f(0, WIN_H); glVertex2f(WIN_W, WIN_H);
            glColor3f(0.10f, 0.13f, 0.25f); glVertex2f(WIN_W, 0); glVertex2f(0, 0);
        glEnd();

        color(0.94f, 0.95f, 0.86f, 0.95f);
        drawCircle(858, 515, 32, 36);
        color(0.03f, 0.06f, 0.15f, 1.0f);
        drawCircle(871, 523, 27, 36);

        color(1.0f, 1.0f, 0.88f, 0.92f);
        float st[][2] = {{70,520},{145,555},{235,500},{330,545},{430,510},{540,555},{650,522},{735,560},{930,490}};
        for (int i = 0; i < 9; i++)
        {
            drawCircle(st[i][0], st[i][1], 2.2f, 10);
            thickLine(st[i][0]-4, st[i][1], st[i][0]+4, st[i][1], 1.0f);
            thickLine(st[i][0], st[i][1]-4, st[i][0], st[i][1]+4, 1.0f);
        }
    }

    float cx = fmod(cloudOffset, 900.0f) - 160.0f;
    for (int i = 0; i < 4; i++) drawSingleCloud(cx + i * 310.0f, 505 + (i % 2) * 26, 0.72f + 0.10f * (i % 2));
}

void drawBoishakhBanglaStyleBanner(float x, float y, float w, const char* subtitle)
{
    // GLUT bitmap fonts do not reliably draw Bengali glyphs in Code::Blocks,
    // so the banner uses romanized Bangla while the decoration follows Noboborsho style.
    color(0.82f, 0.04f, 0.03f);
    drawRoundedRect(x, y, w, 68, 10);
    color(0.05f, 0.42f, 0.12f);
    drawRect(x + 10, y + 8, w - 20, 7);
    drawRect(x + 10, y + 53, w - 20, 7);
    color(1.0f, 0.88f, 0.16f);
    drawTextBig(x + 75, y + 40, "SHUBHO NOBOBORSHO");
    color(1.0f, 0.96f, 0.70f);
    drawText(x + 105, y + 20, subtitle);

    // small alpana corner flowers
    for (int k = 0; k < 2; k++)
    {
        float cx = x + (k == 0 ? 40 : w - 40);
        float cy = y + 34;
        color(1.0f, 0.96f, 0.82f);
        drawCircle(cx, cy, 10, 20);
        color(0.05f, 0.42f, 0.12f);
        drawCircle(cx, cy, 4, 14);
        color(1.0f, 0.96f, 0.82f);
        for (int j = 0; j < 8; j++)
        {
            float a = 2 * PI * j / 8.0f;
            drawEllipse(cx + cos(a) * 17, cy + sin(a) * 11, 5, 3, 12);
        }
    }
}

void drawPantaSteam(float x, float y)
{
    color(1.0f, 1.0f, 1.0f, isNightMode ? 0.35f : 0.50f);
    for (int i = 0; i < 3; i++)
    {
        float sx = x + i * 13;
        glBegin(GL_LINE_STRIP);
        for (int t = 0; t <= 12; t++)
        {
            float yy = y + t * 3.0f;
            float xx = sx + sin((t + i) * 0.7f + cloudOffset * 0.02f) * 4.0f;
            glVertex2f(xx, yy);
        }
        glEnd();
    }
}

void drawClayPot(float x, float y, float s)
{
    color(0.49f, 0.20f, 0.07f);
    drawEllipse(x, y + 25*s, 25*s, 32*s, 30);
    color(0.70f, 0.34f, 0.12f);
    drawEllipse(x, y + 51*s, 18*s, 7*s, 24);
    color(0.33f, 0.12f, 0.04f);
    drawRect(x - 13*s, y + 49*s, 26*s, 4*s);
    color(0.78f, 0.45f, 0.18f);
    drawEllipse(x, y + 22*s, 15*s, 20*s, 24);
}

void drawBananaLeafPlate(float x, float y, float s)
{
    color(0.07f, 0.48f, 0.14f);
    drawEllipse(x, y, 48*s, 16*s, 32);
    color(0.30f, 0.72f, 0.20f);
    drawEllipse(x, y+2*s, 36*s, 10*s, 28);
    color(0.92f, 0.92f, 0.82f);
    drawCircle(x - 8*s, y + 4*s, 10*s, 18);
    drawCircle(x + 5*s, y + 5*s, 8*s, 18);
    color(0.72f, 0.32f, 0.06f);
    drawEllipse(x + 25*s, y + 4*s, 17*s, 6*s, 18);
    color(0.95f, 0.65f, 0.10f);
    drawTriangle(x + 12*s, y + 5*s, x + 38*s, y + 12*s, x + 40*s, y - 3*s);
    drawPantaSteam(x - 8*s, y + 14*s);
}


void drawPantaDBuildingBackdrop()
{
    // D Building placed behind the Panta Ilish level as background scenery.
    float x = 505.0f;
    float y = 112.0f;
    float w = 335.0f;
    float h = 315.0f;

    // shadow/base
    color(0.05f, 0.07f, 0.08f, 0.20f);
    drawRoundedRect(x - 8, y - 8, w + 16, 18, 4);

    // main block
    if (!isNightMode)
        color(0.79f, 0.79f, 0.74f);
    else
        color(0.25f, 0.27f, 0.32f);
    drawRoundedRect(x, y, w, h, 5);

    // left glass tower
    if (!isNightMode)
        color(0.34f, 0.55f, 0.66f, 0.82f);
    else
        color(0.13f, 0.25f, 0.36f, 0.90f);
    drawRect(x - 66, y, 68, h + 12);

    color(0.08f, 0.15f, 0.18f, 0.35f);
    for (float gx = x - 54; gx < x + 2; gx += 13) drawLine(gx, y + 8, gx, y + h + 5);
    for (float gy = y + 18; gy < y + h + 8; gy += 18) drawLine(x - 64, gy, x + 2, gy);

    // service block right
    color(isNightMode ? 0.18f : 0.25f, isNightMode ? 0.25f : 0.34f, isNightMode ? 0.38f : 0.50f);
    drawRect(x + w, y, 42, h + 8);
    color(isNightMode ? 0.85f : 0.65f, isNightMode ? 0.78f : 0.78f, isNightMode ? 0.45f : 0.86f);
    for (float wy = y + 35; wy < y + h - 10; wy += 40) drawRect(x + w + 15, wy, 9, 10);

    // roof panels
    color(0.30f, 0.55f, 0.22f);
    drawRect(x - 4, y + h + 4, w + 50, 6);
    color(0.40f, 0.43f, 0.43f);
    drawQuad(x + 95, y + h + 18, x + 220, y + h + 18, x + 238, y + h + 54, x + 78, y + h + 54);
    drawQuad(x + 245, y + h + 18, x + 315, y + h + 18, x + 330, y + h + 42, x + 235, y + h + 42);

    // bands/windows
    for (int row = 0; row < 6; row++)
    {
        float by = y + 28 + row * 45;
        if (row % 2 == 0)
        {
            color(isNightMode ? 0.55f : 0.54f, isNightMode ? 0.49f : 0.74f, isNightMode ? 0.26f : 0.84f);
            drawRect(x + 18, by, w - 36, 26);
            color(0.10f, 0.22f, 0.28f, 0.45f);
            for (float gx = x + 25; gx < x + w - 24; gx += 21) drawLine(gx, by, gx, by + 26);
            drawLine(x + 18, by + 13, x + w - 18, by + 13);
        }
        else
        {
            color(isNightMode ? 0.18f : 0.72f, isNightMode ? 0.20f : 0.72f, isNightMode ? 0.23f : 0.68f);
            drawRect(x + 18, by + 5, w - 36, 16);
        }
    }

    // D BUILDING sign
    color(0.06f, 0.08f, 0.10f, 0.88f);
    drawRoundedRect(x + 96, y + h - 36, 132, 22, 5);
    color(1.0f, 0.96f, 0.78f);
    drawText(x + 128, y + h - 21, "D BUILDING");

    // foreground bush line in front of building but behind panta gameplay
    for (float bx = x - 72; bx < x + w + 70; bx += 28)
    {
        color(0.15f, 0.38f, 0.14f);
        drawCircle(bx, y - 2, 14, 18);
        color(0.24f, 0.52f, 0.20f);
        drawCircle(bx + 8, y + 4, 10, 18);
    }
}

void drawPantaJumpFoodPlatform()
{
    // Raised task platform: hero must run and jump here to eat panta ilish.
    color(0.0f, 0.0f, 0.0f, 0.28f);
    drawRoundedRect(pantaJumpPlatform.x + 5, pantaJumpPlatform.y - 6, pantaJumpPlatform.w, pantaJumpPlatform.h, 4);
    color(0.38f, 0.18f, 0.06f);
    drawRoundedRect(pantaJumpPlatform.x, pantaJumpPlatform.y, pantaJumpPlatform.w, pantaJumpPlatform.h, 5);
    color(0.78f, 0.42f, 0.14f);
    drawRect(pantaJumpPlatform.x, pantaJumpPlatform.y + pantaJumpPlatform.h - 5, pantaJumpPlatform.w, 5);

    // legs for platform/table
    color(0.28f, 0.12f, 0.04f);
    drawRect(pantaJumpPlatform.x + 18, 115, 12, pantaJumpPlatform.y - 115);
    drawRect(pantaJumpPlatform.x + pantaJumpPlatform.w - 30, 115, 12, pantaJumpPlatform.y - 115);

    if (!pantaEaten)
    {
        drawBananaLeafPlate(275, 278, 1.08f);
        color(1.0f, 0.88f, 0.08f);
        drawCircle(277, 334, 27, 32);
        color(0.0f, 0.0f, 0.0f);
        drawText(252, 330, "JUMP");
        drawText(207, 312, "RUN + JUMP TO EAT");
    }
    else
    {
        color(0.0f, 0.60f, 0.15f);
        drawCircle(277, 284, 20, 28);
        color(1.0f, 1.0f, 1.0f);
        drawText(267, 280, "OK");
    }
}

void drawNagordola(float cx, float cy, float r)
{
    // Nagordola / ferris wheel for Noboborsho mela
    color(0.23f, 0.20f, 0.18f);
    thickLine(cx - r * 0.45f, cy - r - 42, cx, cy, 5);
    thickLine(cx + r * 0.45f, cy - r - 42, cx, cy, 5);
    thickLine(cx - r * 0.58f, cy - r - 42, cx + r * 0.58f, cy - r - 42, 5);

    // outer wheel
    color(0.95f, 0.08f, 0.05f);
    glLineWidth(4);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 72; i++)
    {
        float a = 2.0f * PI * i / 72.0f;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
    glLineWidth(1);

    // inner wheel
    color(1.0f, 0.82f, 0.10f);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 72; i++)
    {
        float a = 2.0f * PI * i / 72.0f;
        glVertex2f(cx + cos(a) * (r * 0.68f), cy + sin(a) * (r * 0.68f));
    }
    glEnd();
    glLineWidth(1);

    // spokes + cabins
    for (int i = 0; i < 8; i++)
    {
        float a = 2.0f * PI * i / 8.0f + cloudOffset * 0.006f;
        float x = cx + cos(a) * r;
        float y = cy + sin(a) * r;

        color(0.10f, 0.12f, 0.14f);
        thickLine(cx, cy, x, y, 2);

        if (i % 3 == 0) color(0.10f, 0.62f, 0.95f);
        else if (i % 3 == 1) color(0.95f, 0.20f, 0.15f);
        else color(1.0f, 0.82f, 0.12f);

        drawRoundedRect(x - 13, y - 11, 26, 17, 4);
        color(0.06f, 0.06f, 0.07f);
        drawLine(x - 10, y + 6, x + 10, y + 6);
    }

    color(0.95f, 0.86f, 0.18f);
    drawCircle(cx, cy, 9, 24);

    // small ticket counter/base
    color(0.55f, 0.24f, 0.07f);
    drawRoundedRect(cx - 48, cy - r - 70, 96, 28, 6);
    color(1.0f, 0.92f, 0.22f);
    drawText(cx - 34, cy - r - 52, "NAGORDOLA");
}

void drawPantaLevelExtras()
{
    // clean mela ground decoration instead of pond/boat
    color(isNightMode ? 0.12f : 0.22f, isNightMode ? 0.30f : 0.56f, isNightMode ? 0.12f : 0.20f);
    drawRoundedRect(392, 45, 335, 42, 16);
    color(1.0f, 0.95f, 0.82f, 0.78f);
    for (int i = 0; i < 4; i++)
    {
        float ax = 455 + i * 70;
        drawCircle(ax, 66, 15, 24);
        color(0.95f, 0.05f, 0.04f, 0.78f);
        drawCircle(ax, 66, 6, 18);
        color(1.0f, 0.95f, 0.82f, 0.78f);
    }

    // bigger eating table with banana leaf plates
    color(0.38f, 0.16f, 0.05f);
    drawRoundedRect(116, 114, 228, 25, 6);
    drawBananaLeafPlate(175, 151, 0.86f);
    drawBananaLeafPlate(268, 151, 0.72f);

    // cook / vendor behind stall
    drawBackPerson2D(220, 214, 0.58f, 0.95f, 0.75f, 0.18f, false);
    color(1.0f, 1.0f, 1.0f);
    drawRoundedRect(207, 267, 26, 10, 4);

    // Nagordola replaces the floating-looking person/dhol corner
    drawNagordola(720, 258, 58);

    // dhol kept as ground decoration, not floating person
    color(0.55f, 0.25f, 0.05f);
    drawEllipse(690, 132, 28, 18, 28);
    color(0.95f, 0.80f, 0.45f);
    drawCircle(664, 132, 11, 18);
    drawCircle(716, 132, 11, 18);
    color(0.30f, 0.16f, 0.06f);
    thickLine(655, 158, 713, 108, 3);
    thickLine(722, 158, 664, 108, 3);

    // Top-right Noboborsho side sign removed as requested.
}


void drawPantaLevel()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // day/night full-screen sky for this separate level
    drawScreenSkyNoBlack();
    drawPantaDBuildingBackdrop();

    color(isNightMode ? 0.13f : 0.24f, isNightMode ? 0.30f : 0.58f, isNightMode ? 0.15f : 0.22f); drawRect(0, 0, WIN_W, 115);
    color(isNightMode ? 0.42f : 0.70f, isNightMode ? 0.35f : 0.52f, isNightMode ? 0.22f : 0.30f); drawQuad(250, 115, 710, 115, 840, 0, 120, 0);

    // Bangla-style Noboborsho banner and flags
    drawBoishakhBanglaStyleBanner(245, 482, 510, "PANTA ILISH UTSHOB");
    color(0.72f, 0.50f, 0.14f); thickLine(85, 455, 920, 455, 2);
    for (int i = 0; i < 22; i++)
    {
        float fx = 90 + i * 38;
        if (i % 3 == 0) color(0.95f, 0.05f, 0.04f);
        else if (i % 3 == 1) color(1.0f, 0.86f, 0.10f);
        else color(0.05f, 0.55f, 0.15f);
        drawTriangle(fx, 455, fx + 24, 455, fx + 12, 425);
    }

    // panta stall with table
    color(0.78f, 0.35f, 0.10f); drawRoundedRect(105, 125, 230, 118, 8);
    color(0.95f, 0.06f, 0.04f); drawTriangle(80, 243, 360, 243, 220, 318);
    color(1.0f, 0.95f, 0.60f); drawText(138, 200, "PANTA ILISH - NOBOBORSHO SPECIAL");
    color(0.48f, 0.22f, 0.08f); drawRoundedRect(125, 116, 190, 22, 5);
    drawPantaPlate(200, 158, true);
    drawPantaPlate(270, 158, false);
    drawPantaLevelExtras();

    drawPantaJumpFoodPlatform();

    // water pot and clay glass
    color(0.55f, 0.25f, 0.08f); drawEllipse(390, 158, 30, 40, 32);
    color(0.73f, 0.38f, 0.12f); drawEllipse(390, 192, 21, 8, 22);
    color(0.75f, 0.38f, 0.12f); drawRoundedRect(430, 130, 20, 38, 4);

    // alpana path
    color(1.0f, 0.95f, 0.82f, 0.86f);
    for (int k = 0; k < 4; k++)
    {
        float ax = 485 + k * 80;
        drawCircle(ax, 90, 20, 28);
        color(0.95f, 0.05f, 0.04f, 0.85f); drawCircle(ax, 90, 8, 20);
        color(1.0f, 0.95f, 0.82f, 0.86f);
        for (int j = 0; j < 8; j++)
        {
            float a = 2 * PI * j / 8.0f;
            drawEllipse(ax + cos(a) * 29, 90 + sin(a) * 14, 7, 4, 14);
        }
    }

    // final concert gate after eating panta
    color(0.48f, 0.18f, 0.05f); drawRoundedRect(850, 105, 24, 142, 5);
    color(0.48f, 0.18f, 0.05f); drawRoundedRect(935, 105, 24, 142, 5);
    color(0.86f, 0.04f, 0.03f); drawRoundedRect(825, 245, 160, 34, 8);
    color(1.0f, 0.88f, 0.16f); drawText(842, 259, "CONCERT GATE");
    if (pantaEaten)
    {
        color(1.0f, 0.88f, 0.08f);
        drawCircle(904, 305, 25, 32);
        color(0.0f, 0.0f, 0.0f);
        drawText(891, 301, "E");
    }

    // all people from back, looking toward food/gate area
    drawBackPerson2D(460, 122, 0.55f, 0.95f, 0.25f, 0.18f, false);
    drawBackPerson2D(535, 118, 0.55f, 0.20f, 0.65f, 0.90f, false);
    drawBackPerson2D(625, 122, 0.55f, 0.95f, 0.75f, 0.18f, false);
    drawBackPerson2D(710, 116, 0.55f, 0.12f, 0.60f, 0.28f, false);

    if (!pantaEaten)
    {
        color(1.0f, 0.86f, 0.12f);
        drawText(130, 365, "Run, jump onto the panta plate, then go to gate.");
    }
    else
    {
        color(0.0f, 0.55f, 0.12f);
        drawText(150, 365, "Panta eaten! Go to CONCERT GATE and press E.");
    }

    drawBackPerson2D(pantaHeroX, pantaHeroY, 0.64f, 0.05f, 0.22f, 0.74f, true);

    color(0.0f, 0.0f, 0.0f, 0.78f);
    drawRoundedRect(15, 15, 930, 34, 8);
    color(1.0f, 1.0f, 1.0f);
    if (pantaMsgTimer > 0) drawText(28, 28, pantaMsg);
    else drawText(28, 28, "Panta Level: Run + jump to eat Panta Ilish, then press E at CONCERT GATE.");
}

void updatePantaLevel()
{
    if (pantaMsgTimer > 0) pantaMsgTimer--;

    pantaHeroVX = 0.0f;
    if (keyLeft) pantaHeroVX = -5.2f;
    if (keyRight) pantaHeroVX = 5.2f;
    if (keyUp && pantaHeroGround)
    {
        pantaHeroVY = 12.4f;
        pantaHeroGround = false;
    }

    float oldY = pantaHeroY;
    pantaHeroX += pantaHeroVX;
    pantaHeroVY -= 0.55f;
    pantaHeroY += pantaHeroVY;

    pantaHeroGround = false;

    // ground collision
    if (pantaHeroY <= 105.0f)
    {
        pantaHeroY = 105.0f;
        pantaHeroVY = 0.0f;
        pantaHeroGround = true;
    }

    // raised food platform collision: land on it by jumping/running
    float heroLeft = pantaHeroX - 18;
    float heroRight = pantaHeroX + 18;
    float platTop = pantaJumpPlatform.y + pantaJumpPlatform.h;
    bool overPlatform = heroRight > pantaJumpPlatform.x && heroLeft < pantaJumpPlatform.x + pantaJumpPlatform.w;
    if (overPlatform && pantaHeroVY <= 0.0f && oldY >= platTop && pantaHeroY <= platTop)
    {
        pantaHeroY = platTop;
        pantaHeroVY = 0.0f;
        pantaHeroGround = true;
    }

    if (pantaHeroX < 50) pantaHeroX = 50;
    if (pantaHeroX > 950) pantaHeroX = 950;

    Rect hero = {pantaHeroX - 18, pantaHeroY, 36, 82};
    if (!pantaEaten && collide(hero, pantaFoodTarget))
    {
        pantaEaten = true;
        setPantaMessage("Panta Ilish eaten by jumping! Now go to CONCERT GATE and press E.");
    }
}

void tryPantaInteract()
{
    if (appPhase != PHASE_PANTA_LEVEL) return;

    Rect hero = {pantaHeroX - 18, pantaHeroY, 36, 82};
    Rect gate = {790, 90, 205, 205};

    if (!pantaEaten)
    {
        setPantaMessage("Ekhane E lagbe na. Run kore jump diye panta plate-er upor utho.");
        return;
    }

    if (collide(hero, gate))
    {
        int carriedLives = lives + appleScore;
        if (carriedLives < 1) carriedLives = 1;
        L3::startLevel3(carriedLives);
        lastLevel3StepTime = glutGet(GLUT_ELAPSED_TIME);
        appPhase = PHASE_LEVEL3;
    }
    else
    {
        setPantaMessage("CONCERT GATE-er shamne giye E press koro.");
    }
}

void drawBoishakhEntryDecor()
{
    // sky
    drawScreenSkyNoBlack();

    // ground and path
    color(isNightMode ? 0.13f : 0.25f, isNightMode ? 0.30f : 0.58f, isNightMode ? 0.15f : 0.22f); drawRect(0, 0, WIN_W, 110);
    color(isNightMode ? 0.42f : 0.70f, isNightMode ? 0.35f : 0.55f, isNightMode ? 0.22f : 0.36f); drawQuad(330, 110, 630, 110, 760, 0, 190, 0);

    // Bangla-style Boishakh entrance banner
    drawBoishakhBanglaStyleBanner(235, 480, 530, "POHELA BOISHAKH ENTRY");

    // flag strings
    color(0.75f, 0.55f, 0.18f); thickLine(75, 455, 925, 455, 2);
    for (int i = 0; i < 22; i++)
    {
        float fx = 90 + i * 38;
        if (i % 3 == 0) color(0.95f, 0.05f, 0.04f);
        else if (i % 3 == 1) color(1.0f, 0.86f, 0.10f);
        else color(0.05f, 0.55f, 0.15f);
        drawTriangle(fx, 455, fx + 24, 455, fx + 12, 425);
    }

    // mela stalls
    color(0.86f, 0.38f, 0.12f); drawRoundedRect(80, 130, 170, 92, 6);
    color(0.95f, 0.06f, 0.04f); drawTriangle(60, 222, 270, 222, 165, 285);
    color(1.0f, 0.95f, 0.60f); drawText(118, 178, "PANTA ILISH");
    color(0.82f, 0.35f, 0.08f); drawRoundedRect(720, 130, 190, 92, 6);
    color(0.06f, 0.55f, 0.14f); drawTriangle(700, 222, 930, 222, 815, 285);
    color(1.0f, 0.95f, 0.60f); drawText(765, 178, "NOKSHI STALL");

    // alpana on ground
    color(1.0f, 0.95f, 0.82f, 0.82f);
    for (int k = 0; k < 3; k++)
    {
        float ax = 390 + k * 100;
        drawCircle(ax, 88, 25, 32);
        color(0.95f, 0.05f, 0.04f, 0.85f); drawCircle(ax, 88, 10, 24);
        color(1.0f, 0.95f, 0.82f, 0.82f);
        for (int j = 0; j < 8; j++)
        {
            float a = 2 * PI * j / 8.0f;
            drawEllipse(ax + cos(a) * 36, 88 + sin(a) * 18, 9, 5, 18);
        }
    }

    // decorated concert gate
    color(0.48f, 0.18f, 0.05f); drawRoundedRect(845, 105, 24, 150, 5);
    color(0.48f, 0.18f, 0.05f); drawRoundedRect(940, 105, 24, 150, 5);
    color(0.86f, 0.04f, 0.03f); drawRoundedRect(820, 250, 170, 34, 8);
    color(1.0f, 0.88f, 0.16f); drawText(860, 264, "CONCERT GATE");
    color(0.08f, 0.45f, 0.12f); drawRect(835, 228, 140, 12);

    // dhol drums
    color(0.55f, 0.25f, 0.05f); drawEllipse(300, 150, 32, 22, 30);
    color(0.95f, 0.80f, 0.45f); drawCircle(270, 150, 14, 20); drawCircle(330, 150, 14, 20);
    color(0.55f, 0.25f, 0.05f); drawEllipse(650, 150, 32, 22, 30);
    color(0.95f, 0.80f, 0.45f); drawCircle(620, 150, 14, 20); drawCircle(680, 150, 14, 20);

    // pass item
    if (!entryPassCollected)
    {
        color(0.20f, 0.80f, 0.95f); drawRoundedRect(545, 145, 72, 42, 6);
        color(0.02f, 0.05f, 0.08f); outlineRect(545, 145, 72, 42);
        drawText(551, 170, "PASS");
        color(1.0f, 0.9f, 0.1f); drawText(520, 200, "COLLECT ENTRY PASS");
    }

    // back-facing crowd/public
    drawBackPerson2D(410, 122, 0.55f, 0.95f, 0.25f, 0.18f, false);
    drawBackPerson2D(470, 118, 0.55f, 0.20f, 0.65f, 0.90f, false);
    drawBackPerson2D(690, 122, 0.55f, 0.95f, 0.75f, 0.18f, false);
    drawBackPerson2D(760, 116, 0.55f, 0.12f, 0.60f, 0.28f, false);
    drawBackPerson2D(165, 114, 0.50f, 0.85f, 0.30f, 0.70f, false);
}

void drawBoishakhEntryLevel()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawBoishakhEntryDecor();
    drawBackPerson2D(entryHeroX, entryHeroY, 0.66f, 0.05f, 0.22f, 0.74f, true);

    color(0.0f, 0.0f, 0.0f, 0.78f);
    drawRoundedRect(15, 15, 920, 34, 8);
    color(1.0f, 1.0f, 1.0f);
    if (entryMsgTimer > 0) drawText(28, 28, entryMsg);
    else drawText(28, 28, "Extra Level: A/D move, W jump. Collect PASS, go to CONCERT GATE and press E.");
}

void updateBoishakhEntryLevel()
{
    if (entryMsgTimer > 0) entryMsgTimer--;

    entryHeroVX = 0.0f;
    if (keyLeft) entryHeroVX = -5.2f;
    if (keyRight) entryHeroVX = 5.2f;
    if (keyUp && entryHeroGround)
    {
        entryHeroVY = 12.0f;
        entryHeroGround = false;
    }

    entryHeroX += entryHeroVX;
    entryHeroVY -= 0.55f;
    entryHeroY += entryHeroVY;

    if (entryHeroY <= 105.0f)
    {
        entryHeroY = 105.0f;
        entryHeroVY = 0.0f;
        entryHeroGround = true;
    }
    if (entryHeroX < 50) entryHeroX = 50;
    if (entryHeroX > 950) entryHeroX = 950;

    Rect hero = {entryHeroX - 18, entryHeroY, 36, 80};
    Rect pass = {545, 145, 72, 42};
    if (!entryPassCollected && collide(hero, pass))
    {
        entryPassCollected = true;
        setEntryMessage("Entry pass collected. Ebar decorated CONCERT GATE-er kache giye E press koro.");
    }
}

void tryEnterConcertFromBoishakhLevel()
{
    if (appPhase != PHASE_BOISHAKH_ENTRY) return;

    Rect hero = {entryHeroX - 18, entryHeroY, 36, 80};
    Rect gate = {790, 90, 205, 205};
    if (!entryPassCollected)
    {
        setEntryMessage("Age ENTRY PASS collect korte hobe.");
        return;
    }
    if (!collide(hero, gate))
    {
        setEntryMessage("Gate-er shamne giye E press koro.");
        return;
    }

    int carriedLives = lives + appleScore;
    if (carriedLives < 1) carriedLives = 1;
    L3::startLevel3(carriedLives);
    lastLevel3StepTime = glutGet(GLUT_ELAPSED_TIME);
    appPhase = PHASE_LEVEL3;
}

// ======================================================
// DISPLAY
// ======================================================
void display()
{
    forceFullWindowViewport();

    if (appPhase == PHASE_LEVEL3)
    {
        L3::display();
        glutSwapBuffers();
        return;
    }

    if (appPhase == PHASE_TRANSITION)
    {
        glClearColor(0.02f, 0.03f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawTransitionScreen();
        glutSwapBuffers();
        return;
    }

    if (appPhase == PHASE_PANTA_LEVEL)
    {
        glClearColor(0.78f, 0.93f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawPantaLevel();
        glutSwapBuffers();
        return;
    }

    if (appPhase == PHASE_BOISHAKH_ENTRY)
    {
        glClearColor(0.78f, 0.92f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawBoishakhEntryLevel();
        glutSwapBuffers();
        return;
    }

    glClearColor(0.84f, 0.93f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!gameStarted && !infoMode)
    {
        drawStartScreen();
        glutSwapBuffers();
        return;
    }

    if (infoMode)
    {
        drawInfoScreen();
        glutSwapBuffers();
        return;
    }

    setLevel2Projection();

    drawBackgroundScenario();

    drawPlatforms();
    drawSpikes();

    drawLadderItem();
    drawPlacedLadder();

    drawIDCard();
    drawApple();

    drawBrickItem();
    drawBrickPlaceZone();
    drawGun();
    drawBullets();
    drawKnife();

    drawGuard(guard1X, guard1Y, true);
    drawGate();
    drawGuard(finalGuard.x, finalGuard.y, false);

    drawPlayer();

    drawGameMessage();

    glutSwapBuffers();
}

// ======================================================
// TIMER
// ======================================================
void timer(int)
{
    if (appPhase == PHASE_TRANSITION)
    {
        int now = glutGet(GLUT_ELAPSED_TIME);

        if (now - transitionStartTime > 1800)
        {
            int carriedLives = lives + appleScore;
            if (carriedLives < 1)
                carriedLives = 1;

            startPantaLevel();
        }
    }
    else if (appPhase == PHASE_PANTA_LEVEL)
    {
        cloudOffset += 0.45f;
        if (cloudOffset > 1250.0f) cloudOffset = 0.0f;
        updatePantaLevel();
    }
    else if (appPhase == PHASE_BOISHAKH_ENTRY)
    {
        cloudOffset += 0.45f;
        if (cloudOffset > 1250.0f) cloudOffset = 0.0f;
        updateBoishakhEntryLevel();
    }
    else if (appPhase == PHASE_LEVEL3)
    {
        int now = glutGet(GLUT_ELAPSED_TIME);

        if (lastLevel3StepTime == 0)
        {
            lastLevel3StepTime = now;
        }

        int loops = 0;

        while (now - lastLevel3StepTime >= LEVEL3_UPDATE_MS && loops < 3)
        {
            L3::step();
            lastLevel3StepTime += LEVEL3_UPDATE_MS;
            loops++;
        }
    }
    else
    {
        cloudOffset += 0.35f;
        if (cloudOffset > 1250.0f)
            cloudOffset = 0.0f;

        updateGame();
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ======================================================
// KEYBOARD
// ======================================================
void normalKeyDown(unsigned char key, int, int)
{
    if (appPhase == PHASE_LEVEL3)
    {
        if (key == 'n' || key == 'N')
        {
            isNightMode = !isNightMode;
            if (isNightMode)
                L3::setLevel3Message("Final Concert Night mode enabled. Press N for day.", 260);
            else
                L3::setLevel3Message("Final Concert Day mode enabled. Press N for night.", 260);
            glutPostRedisplay();
            return;
        }
        L3::keyboardDown(key, 0, 0);
        return;
    }

    if (appPhase == PHASE_PANTA_LEVEL)
    {
        if (key == 'n' || key == 'N')
        {
            isNightMode = !isNightMode;
            setPantaMessage(isNightMode ? "Night mode: Noboborsho lights on." : "Day mode: morning Noboborsho vibe.");
            return;
        }
        if (key == 'a' || key == 'A' || key == '4') keyLeft = true;
        if (key == 'd' || key == 'D' || key == '6') keyRight = true;
        if (key == 'w' || key == 'W' || key == ' ' || key == '2') keyUp = true;
        if (key == 'e' || key == 'E' || key == '0') tryPantaInteract();
        if (key == 'r' || key == 'R') startPantaLevel();
        return;
    }

    if (appPhase == PHASE_BOISHAKH_ENTRY)
    {
        if (key == 'n' || key == 'N')
        {
            isNightMode = !isNightMode;
            setEntryMessage(isNightMode ? "Night mode: entry lights on." : "Day mode: Boishakh entry fair.");
            return;
        }
        if (key == 'a' || key == 'A' || key == '4') keyLeft = true;
        if (key == 'd' || key == 'D' || key == '6') keyRight = true;
        if (key == 'w' || key == 'W' || key == ' ' || key == '2') keyUp = true;
        if (key == 'e' || key == 'E' || key == '0') tryEnterConcertFromBoishakhLevel();
        if (key == 'r' || key == 'R') startBoishakhEntryLevel();
        return;
    }

    if (appPhase == PHASE_TRANSITION)
    {
        return;
    }

    // debug/test shortcut: press P anytime in Level 2 to confirm the Panta level is added
    if (appPhase == PHASE_LEVEL2 && gameStarted && (key == 'p' || key == 'P'))
    {
        startPantaLevel();
        return;
    }
    if (!gameStarted)
    {
        if (key == 'n' || key == 'N')
        {
            isNightMode = !isNightMode;
            setMessage(isNightMode ? "Night mode enabled. Press N again for day mode." : "Day mode enabled. Press N again for night mode.");
            return;
        }

        if (key == 's' || key == 'S')
        {
            gameStarted = true;
            infoMode = false;
            setMessage("Game started. Collect phone, stand on CALL ZONE, press C to call friend.");
            return;
        }

        if (key == 'i' || key == 'I')
        {
            infoMode = !infoMode;
            return;
        }

        return;
    }

    if (infoMode)
    {
        if (key == 'n' || key == 'N')
        {
            isNightMode = !isNightMode;
            return;
        }

        if (key == 's' || key == 'S')
        {
            infoMode = false;
            gameStarted = true;
            return;
        }

        if (key == 'i' || key == 'I')
        {
            infoMode = false;
            return;
        }

        return;
    }

    if (key == 'i' || key == 'I')
    {
        infoMode = true;
        return;
    }

    if (key == 'n' || key == 'N')
    {
        isNightMode = !isNightMode;
        setMessage(isNightMode ? "Night mode enabled." : "Day mode enabled.");
        return;
    }

    if (key == 'a' || key == 'A' || key == '4')
    {
        keyLeft = true;
    }

    if (key == 'd' || key == 'D' || key == '6')
    {
        keyRight = true;
    }

    if (key == 'w' || key == 'W' || key == ' ' || key == '2')
    {
        keyUp = true;

        if (!nearPlacedLadder())
        {
            tryJump();
        }
    }

    if (key == 's' || key == 'S' || key == '8')
    {
        keyDown = true;
    }

    if (key == 'l' || key == 'L' || key == '5')
    {
        tryPlaceLadder();
    }

    if (key == 'c' || key == 'C')
    {
        tryCallFriend();
    }

    if (key == 'e' || key == 'E' || key == '0')
    {
        tryShowID();
    }

    if (key == 'b' || key == 'B')
    {
        tryPlaceBridgeBrick();
    }

    if (key == 'r' || key == 'R')
    {
        fullReset();
    }
}

void normalKeyUp(unsigned char key, int, int)
{
    if (appPhase == PHASE_LEVEL3)
    {
        L3::keyboardUp(key, 0, 0);
        return;
    }
    if (appPhase == PHASE_PANTA_LEVEL || appPhase == PHASE_BOISHAKH_ENTRY)
    {
        if (key == 'a' || key == 'A' || key == '4') keyLeft = false;
        if (key == 'd' || key == 'D' || key == '6') keyRight = false;
        if (key == 'w' || key == 'W' || key == ' ' || key == '2') keyUp = false;
        if (key == 's' || key == 'S' || key == '8') keyDown = false;
        return;
    }
    if (key == 'a' || key == 'A' || key == '4')
    {
        keyLeft = false;
    }

    if (key == 'd' || key == 'D' || key == '6')
    {
        keyRight = false;
    }

    if (key == 'w' || key == 'W' || key == ' ' || key == '2')
    {
        keyUp = false;
    }

    if (key == 's' || key == 'S' || key == '8')
    {
        keyDown = false;
    }
}

void specialKeyDown(int key, int, int)
{
    if (appPhase == PHASE_LEVEL3)
    {
        L3::arrowDown(key);
        return;
    }
    if (appPhase == PHASE_PANTA_LEVEL || appPhase == PHASE_BOISHAKH_ENTRY)
    {
        if (key == GLUT_KEY_LEFT) keyLeft = true;
        if (key == GLUT_KEY_RIGHT) keyRight = true;
        if (key == GLUT_KEY_UP) keyUp = true;
        if (key == GLUT_KEY_DOWN) keyDown = true;
        return;
    }
    if (!gameStarted || infoMode)
    {
        return;
    }

    if (key == GLUT_KEY_LEFT)
    {
        keyLeft = true;
    }

    if (key == GLUT_KEY_RIGHT)
    {
        keyRight = true;
    }

    if (key == GLUT_KEY_UP)
    {
        keyUp = true;

        if (!nearPlacedLadder())
        {
            tryJump();
        }
    }

    if (key == GLUT_KEY_DOWN)
    {
        keyDown = true;
    }
}

void specialKeyUp(int key, int, int)
{
    if (appPhase == PHASE_LEVEL3)
    {
        L3::arrowUp(key);
        return;
    }
    if (appPhase == PHASE_PANTA_LEVEL || appPhase == PHASE_BOISHAKH_ENTRY)
    {
        if (key == GLUT_KEY_LEFT) keyLeft = false;
        if (key == GLUT_KEY_RIGHT) keyRight = false;
        if (key == GLUT_KEY_UP) keyUp = false;
        if (key == GLUT_KEY_DOWN) keyDown = false;
        return;
    }
    if (key == GLUT_KEY_LEFT)
    {
        keyLeft = false;
    }

    if (key == GLUT_KEY_RIGHT)
    {
        keyRight = false;
    }

    if (key == GLUT_KEY_UP)
    {
        keyUp = false;
    }

    if (key == GLUT_KEY_DOWN)
    {
        keyDown = false;
    }
}

// ======================================================
// INIT / MAIN
// ======================================================
void init()
{
    glClearColor(0.84f, 0.93f, 1.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
}


void reshapeCombined(int w, int h)
{
    if (w <= 0) w = WIN_W;
    if (h <= 0) h = WIN_H;

    currentWindowW = w;
    currentWindowH = h;

    // Use the full window area in every level. This removes the black border.
    glViewport(0, 0, currentWindowW, currentWindowH);

    if (appPhase == PHASE_LEVEL3)
    {
        L3::reshape(w, h);
    }

    glutPostRedisplay();
}

void mouseMoveCombined(int x, int y)
{
    if (appPhase == PHASE_LEVEL3)
    {
        L3::mouseMove(x, y);
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("AIUB Imposter Detection");

// Start in fullscreen so the scenario fills the screen instead of staying centered with black border.
// Press ESC to exit as before.
glutFullScreen();  // make the game fill the monitor

    init();
    L3::init();

    glutDisplayFunc(display);
    glutKeyboardFunc(normalKeyDown);
    glutKeyboardUpFunc(normalKeyUp);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutPassiveMotionFunc(mouseMoveCombined);
    glutMotionFunc(mouseMoveCombined);
    glutReshapeFunc(reshapeCombined);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}


// ======================================================
// LEVEL 3 CODE WRAPPED IN NAMESPACE
// ======================================================
namespace L3 {
/* ================= ANIMATION VARIABLES ================= */

float waveAngle = 0.0f;
float lightMove = 0.0f;
float singerMove = 0.0f;
float cloudMove = 0.0f;

/* ================= GAME VARIABLES ================= */

float playerX = 0.0f;
float playerY = -0.82f;
float playerSpeed = 0.010f;

int keyState[256] = {0};

float camX = 0.0f;
float camY = -0.55f;

const float VIEW_HALF_W = 1.05f;
const float VIEW_HALF_H = 0.62f;

const float WORLD_MIN_X = -1.28f;
const float WORLD_MAX_X = 1.28f;
const float WORLD_MIN_Y = -1.22f;
const float WORLD_MAX_Y = 0.56f;

int winW = 1000;
int winH = 600;

float aimX = 0.0f;
float aimY = 0.0f;
int mouseScreenX = 500;
int mouseScreenY = 400;

int missionStartTime = 0;

enum GameState {
    PLAYING,
    MAIN_ATTACK,
    WON,
    LOST
};

GameState gameState = PLAYING;

#define MAX_NPCS 280
#define MAX_EFFECTS 250
#define IMPOSTER_TOTAL 5

struct NPC {
    float x, y;
    float homeX, homeY;
    float scale;
    int type;
    int colorIndex;
    int imposter;
    int active;
};

struct EffectParticle {
    float x, y;
    float dx, dy;
    float life;
    int active;
};

NPC npcs[MAX_NPCS];
int npcCount = 0;

EffectParticle effects[MAX_EFFECTS];

int impostersStopped = 0;
int bullets = 2;

float mainImpX = 0.62f;
float mainImpY = 0.08f;
int mainImpActive = 0;

int level3Lives = 1;
char level3Message[320] = "Boishakh Concert Level: Everyone faces the stage. Find black dressed imposters. Press K near them. Final attack: aim mouse and press G.";
int level3MessageTimer = 260;

void resetRun();
void startLevel3(int lifeCount);
void loseRun(const char* msg);
void setLevel3Message(const char* msg, int timer);

/* ================= BASIC DRAWING ================= */

void circle(float x, float y, float r, float R, float G, float B) {
    glColor3f(R, G, B);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for(int i = 0; i <= 50; i++) {
        float a = 2 * 3.1416 * i / 50;
        glVertex2f(x + cos(a) * r, y + sin(a) * r);
    }

    glEnd();
}

void rect(float x1, float y1, float x2, float y2, float R, float G, float B) {
    glColor3f(R, G, B);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void text(float x, float y, const char *s) {
    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);

    for(int i = 0; s[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s[i]);
}

void textSmall(float x, float y, const char *s) {
    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);

    for(int i = 0; s[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s[i]);
}

float clampValue(float v, float mn, float mx) {
    if(v < mn) return mn;
    if(v > mx) return mx;
    return v;
}

float distance2D(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

/* ================= TIME AND LEVEL ================= */

int getTimeLeft() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    int elapsed = (now - missionStartTime) / 1000;
    int left = 60 - elapsed;

    if(left < 0)
        left = 0;

    return left;
}

int getLevel() {
    int left = getTimeLeft();

    if(left > 40)
        return 1;
    else if(left > 20)
        return 2;
    else
        return 3;
}

/* ================= CAMERA ================= */

void updateAimWorld() {
    aimX = camX - VIEW_HALF_W + ((float)mouseScreenX / (float)winW) * (VIEW_HALF_W * 2.0f);
    aimY = camY + VIEW_HALF_H - ((float)mouseScreenY / (float)winH) * (VIEW_HALF_H * 2.0f);
}

void setupCamera() {
    winW = glutGet(GLUT_WINDOW_WIDTH);
    winH = glutGet(GLUT_WINDOW_HEIGHT);
    if(winW <= 0) winW = 1000;
    if(winH <= 0) winH = 600;
    glViewport(0, 0, winW, winH);

    camX = playerX;
    camY = playerY + 0.10f;

    camX = clampValue(camX, WORLD_MIN_X + VIEW_HALF_W, WORLD_MAX_X - VIEW_HALF_W);
    camY = clampValue(camY, WORLD_MIN_Y + VIEW_HALF_H, WORLD_MAX_Y - VIEW_HALF_H);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(camX - VIEW_HALF_W, camX + VIEW_HALF_W,
               camY - VIEW_HALF_H, camY + VIEW_HALF_H);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    updateAimWorld();
}

/* ================= LIGHT EFFECT ================= */

void lightBeam(float x, float r, float g, float b) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(r, g, b, 0.05);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, 0.88);
    glVertex2f(x - 0.16, 0.42);
    glVertex2f(x + 0.16, 0.42);
    glEnd();

    glDisable(GL_BLEND);
}

/* ================= SKY ================= */

void drawConcertCloud(float x, float y, float s) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.38f, 0.42f, 0.56f, 0.40f);
    circle(x - 0.050f * s, y, 0.042f * s, 0.38f, 0.42f, 0.56f);
    circle(x + 0.005f * s, y + 0.020f * s, 0.058f * s, 0.42f, 0.46f, 0.62f);
    circle(x + 0.070f * s, y, 0.045f * s, 0.36f, 0.40f, 0.54f);
    rect(x - 0.085f * s, y - 0.028f * s, x + 0.105f * s, y + 0.012f * s, 0.34f, 0.38f, 0.52f);

    glColor4f(0.66f, 0.70f, 0.84f, 0.22f);
    circle(x - 0.025f * s, y + 0.018f * s, 0.030f * s, 0.66f, 0.70f, 0.84f);
    circle(x + 0.040f * s, y + 0.012f * s, 0.028f * s, 0.66f, 0.70f, 0.84f);

    glDisable(GL_BLEND);
}

void drawSky() {
    if(!isNightMode) {
        // FINAL LEVEL DAY SKY
        rect(-1.35, 0.52, 1.35, 1.0, 0.56, 0.82, 1.00);
        rect(-1.35, 0.72, 1.35, 1.0, 0.72, 0.91, 1.00);

        // sun with soft glow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.90f, 0.25f, 0.22f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(-0.58f, 0.90f);
        for(int i=0;i<=60;i++){
            float a=2*3.1416f*i/60.0f;
            glVertex2f(-0.58f+cos(a)*0.105f,0.90f+sin(a)*0.105f);
        }
        glEnd();
        glDisable(GL_BLEND);
        circle(-0.58, 0.90, 0.050, 1.00, 0.90, 0.22);

        // bright moving clouds
        float base = -1.55f + fmod(cloudMove, 2.80f);
        for(int i = 0; i < 4; i++) {
            float x = base + i * 0.85f;
            if(x > 1.45f) x -= 3.10f;
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, 0.78f);
            circle(x - 0.055f, 0.83f + 0.025f*(i%2), 0.046f, 1.0f, 1.0f, 1.0f);
            circle(x + 0.000f, 0.855f + 0.025f*(i%2), 0.060f, 1.0f, 1.0f, 1.0f);
            circle(x + 0.070f, 0.830f + 0.025f*(i%2), 0.050f, 1.0f, 1.0f, 1.0f);
            rect(x - 0.095f, 0.800f + 0.025f*(i%2), x + 0.120f, 0.835f + 0.025f*(i%2), 0.92f, 0.96f, 1.0f);
            glDisable(GL_BLEND);
        }

        // small flying birds for day
        glColor3f(0.05f, 0.05f, 0.06f);
        glBegin(GL_LINES);
        glVertex2f(-0.10, 0.84); glVertex2f(-0.06, 0.87);
        glVertex2f(-0.06, 0.87); glVertex2f(-0.02, 0.84);
        glVertex2f(0.22, 0.82); glVertex2f(0.26, 0.85);
        glVertex2f(0.26, 0.85); glVertex2f(0.30, 0.82);
        glVertex2f(0.48, 0.91); glVertex2f(0.52, 0.94);
        glVertex2f(0.52, 0.94); glVertex2f(0.56, 0.91);
        glEnd();
    }
    else {
        // FINAL LEVEL NIGHT SKY
        rect(-1.35, 0.52, 1.35, 1.0, 0.005, 0.010, 0.035);
        rect(-1.35, 0.72, 1.35, 1.0, 0.010, 0.020, 0.070);

        // moon
        circle(-0.58, 0.90, 0.045, 0.92, 0.90, 0.76);
        circle(-0.56, 0.915, 0.040, 0.010, 0.020, 0.070);

        // stars
        glColor3f(0.90, 0.92, 1.0);
        glPointSize(1.5);
        glBegin(GL_POINTS);
        for(int i = 0; i < 160; i++) {
            float x = -1.25f + ((i * 37) % 250) / 100.0f;
            float y = 0.62f + ((i * 61) % 38) / 100.0f;
            glVertex2f(x, y);
        }
        glEnd();

        glColor3f(0.25, 0.35, 0.75);
        glPointSize(2.0);
        glBegin(GL_POINTS);
        for(int i = 0; i < 85; i++) {
            float x = -0.35f + ((i * 29) % 110) / 100.0f;
            float y = 0.70f + ((i * 17) % 28) / 100.0f;
            glVertex2f(x, y);
        }
        glEnd();

        // moving dark clouds behind the concert stage
        float base = -1.55f + fmod(cloudMove, 2.80f);
        for(int i = 0; i < 3; i++) {
            float x = base + i * 1.05f;
            if(x > 1.45f) x -= 3.10f;
            drawConcertCloud(x, 0.83f + 0.035f * (i % 2), 1.0f - 0.12f * (i % 2));
        }
    }
}

/* ================= REALISTIC GRASS GROUND ================= */

void drawGrassBlade(float x, float y, float h, float lean, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x + lean, y + h);
    glEnd();
}

void drawGrassPatch(float cx, float cy, float radius, float r, float g, float b) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(r, g, b, 0.18f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);

    for(int i = 0; i <= 40; i++) {
        float a = 2.0f * 3.1416f * i / 40.0f;
        glVertex2f(cx + cos(a) * radius, cy + sin(a) * radius * 0.55f);
    }

    glEnd();
    glDisable(GL_BLEND);
}

void drawGround() {
    if(!isNightMode) {
        // FINAL LEVEL DAY FIELD
        rect(-1.35, -1.25, 1.35, 0.52, 0.14, 0.48, 0.10);
        rect(-1.35, -1.10, 1.35, -0.80, 0.11, 0.42, 0.09);
        rect(-1.35, -0.80, 1.35, -0.45, 0.16, 0.54, 0.12);
        rect(-1.35, -0.45, 1.35, -0.10, 0.18, 0.60, 0.13);
        rect(-1.35, -0.10, 1.35, 0.25, 0.15, 0.52, 0.11);
        rect(-1.35, 0.25, 1.35, 0.52, 0.12, 0.44, 0.10);

        drawGrassPatch(-0.95, -0.96, 0.30, 0.08, 0.32, 0.06);
        drawGrassPatch(0.95, -0.92, 0.32, 0.08, 0.34, 0.07);
        drawGrassPatch(-0.72, -0.78, 0.28, 0.08, 0.32, 0.06);
        drawGrassPatch(0.62, -0.72, 0.30, 0.08, 0.34, 0.07);
        drawGrassPatch(-0.20, -0.32, 0.36, 0.20, 0.58, 0.12);
        drawGrassPatch(0.30, -0.05, 0.34, 0.18, 0.55, 0.12);
    } else {
        // FINAL LEVEL NIGHT FIELD
        rect(-1.35, -1.25, 1.35, 0.52, 0.025, 0.215, 0.045);
        rect(-1.35, -1.10, 1.35, -0.80, 0.020, 0.185, 0.040);
        rect(-1.35, -0.80, 1.35, -0.45, 0.030, 0.245, 0.055);
        rect(-1.35, -0.45, 1.35, -0.10, 0.035, 0.270, 0.060);
        rect(-1.35, -0.10, 1.35, 0.25, 0.025, 0.235, 0.050);
        rect(-1.35, 0.25, 1.35, 0.52, 0.020, 0.190, 0.045);

        drawGrassPatch(-0.95, -0.96, 0.30, 0.010, 0.140, 0.030);
        drawGrassPatch(0.95, -0.92, 0.32, 0.010, 0.145, 0.035);
        drawGrassPatch(-0.72, -0.78, 0.28, 0.010, 0.140, 0.030);
        drawGrassPatch(0.62, -0.72, 0.30, 0.010, 0.150, 0.035);
        drawGrassPatch(-0.20, -0.32, 0.36, 0.090, 0.360, 0.080);
        drawGrassPatch(0.30, -0.05, 0.34, 0.080, 0.330, 0.070);
        drawGrassPatch(-0.55, 0.28, 0.26, 0.012, 0.150, 0.035);
        drawGrassPatch(0.66, 0.30, 0.25, 0.012, 0.150, 0.035);
    }

    glPointSize(1.4f);
    glBegin(GL_POINTS);
    for(int i = 0; i < 2300; i++) {
        float x = -1.30f + ((i * 37) % 2600) / 1000.0f;
        float y = -1.20f + ((i * 73) % 1720) / 1000.0f;
        if(y > 0.52f) continue;
        int shade = i % 5;
        if(!isNightMode) {
            if(shade == 0) glColor3f(0.18f, 0.66f, 0.14f);
            if(shade == 1) glColor3f(0.10f, 0.46f, 0.08f);
            if(shade == 2) glColor3f(0.26f, 0.74f, 0.16f);
            if(shade == 3) glColor3f(0.13f, 0.56f, 0.10f);
            if(shade == 4) glColor3f(0.22f, 0.62f, 0.13f);
        } else {
            if(shade == 0) glColor3f(0.05f, 0.38f, 0.08f);
            if(shade == 1) glColor3f(0.02f, 0.28f, 0.06f);
            if(shade == 2) glColor3f(0.12f, 0.50f, 0.10f);
            if(shade == 3) glColor3f(0.03f, 0.32f, 0.06f);
            if(shade == 4) glColor3f(0.08f, 0.43f, 0.08f);
        }
        glVertex2f(x, y);
    }
    glEnd();

    for(int i = 0; i < 420; i++) {
        float x = -1.28f + ((i * 41) % 2560) / 1000.0f;
        float y = -1.15f + ((i * 67) % 1640) / 1000.0f;
        float h = 0.020f + (i % 5) * 0.006f;
        float lean = -0.010f + (i % 7) * 0.003f;
        if(!isNightMode) drawGrassBlade(x, y, h, lean, 0.16f, 0.70f, 0.12f);
        else drawGrassBlade(x, y, h, lean, 0.05f, 0.48f, 0.08f);
    }
}


void drawTree(float x, float y)
{
    // back-side tree for final concert field
    rect(x - 0.035f, y - 0.28f, x + 0.035f, y - 0.02f, 0.25f, 0.12f, 0.04f);

    if(!isNightMode)
    {
        circle(x - 0.075f, y + 0.035f, 0.095f, 0.05f, 0.36f, 0.09f);
        circle(x + 0.005f, y + 0.085f, 0.115f, 0.06f, 0.42f, 0.10f);
        circle(x + 0.095f, y + 0.035f, 0.095f, 0.05f, 0.34f, 0.08f);
    }
    else
    {
        circle(x - 0.075f, y + 0.035f, 0.095f, 0.01f, 0.16f, 0.04f);
        circle(x + 0.005f, y + 0.085f, 0.115f, 0.01f, 0.19f, 0.05f);
        circle(x + 0.095f, y + 0.035f, 0.095f, 0.01f, 0.14f, 0.04f);
    }
}

void drawTreesAndLights() {
    drawTree(-0.92, 0.18);
    drawTree(0.92, 0.18);
    drawTree(-0.92, -0.90);
    drawTree(0.92, -0.90);

    for(float x = -0.95; x <= -0.78; x += 0.05) {
        circle(x, 0.43, 0.008, 1.0, 0.82, 0.18);
        circle(x, -0.18, 0.008, 1.0, 0.82, 0.18);
    }

    for(float x = 0.78; x <= 0.95; x += 0.05) {
        circle(x, 0.43, 0.008, 1.0, 0.82, 0.18);
        circle(x, -0.18, 0.008, 1.0, 0.82, 0.18);
    }
}

/* ================= STALLS ================= */

void drawStalls() {
    rect(-0.99, -0.84, -0.77, -0.56, 0.28, 0.14, 0.06);
    rect(-0.98, -0.55, -0.78, -0.37, 0.95, 0.38, 0.08);
    rect(-0.94, -0.51, -0.82, -0.41, 0.78, 0.25, 0.04);
    rect(-0.96, -0.70, -0.80, -0.62, 0.40, 0.18, 0.06);
    rect(-0.945, -0.675, -0.905, -0.645, 0.95, 0.80, 0.25);
    rect(-0.890, -0.675, -0.850, -0.645, 0.85, 0.30, 0.10);

    for(float x = -0.97; x <= -0.79; x += 0.035)
        circle(x, -0.57, 0.007, 1.0, 0.82, 0.22);

    text(-0.945, -0.47, "FOOD");

    rect(0.77, -0.84, 0.99, -0.56, 0.12, 0.28, 0.10);
    rect(0.78, -0.55, 0.98, -0.37, 0.12, 0.62, 0.16);
    rect(0.82, -0.51, 0.94, -0.41, 0.07, 0.42, 0.10);
    rect(0.80, -0.70, 0.96, -0.62, 0.08, 0.22, 0.08);
    rect(0.825, -0.675, 0.855, -0.645, 0.10, 0.65, 0.90);
    rect(0.875, -0.675, 0.905, -0.645, 0.95, 0.25, 0.20);
    rect(0.925, -0.675, 0.955, -0.645, 0.95, 0.85, 0.15);

    for(float x = 0.79; x <= 0.97; x += 0.035)
        circle(x, -0.57, 0.007, 1.0, 0.82, 0.22);

    text(0.825, -0.47, "DRINKS");

    for(float x = 0.78; x <= 0.98; x += 0.04) {
        rect(x, -0.82, x + 0.02, -0.80, 0.90, 0.25, 0.10);
        rect(x, -0.84, x + 0.02, -0.82, 0.10, 0.55, 0.90);
    }
}

/* ================= BARRIER ================= */

void drawBarrier() {
    rect(-0.70, -0.05, 0.77, -0.03, 1.0, 0.8, 0.0);

    for(float x = -0.70; x <= 0.70; x += 0.10) {
        rect(x, -0.08, x + 0.010, 0.00, 1.0, 0.0, 0.0);
    }
}

/* ================= SINGER ================= */

void drawSinger() {
    float mv = 0.006 * sin(singerMove);

    // microphone stand
    rect(-0.005, 0.54, 0.005, 0.72, 0, 0, 0);
    circle(0.0, 0.735, 0.015, 0, 0, 0);

    // legs and shoes
    rect(-0.025, 0.53, -0.008, 0.61, 0.02, 0.02, 0.08);
    rect(0.008, 0.53, 0.025, 0.61, 0.02, 0.02, 0.08);
    rect(-0.032, 0.52, -0.006, 0.535, 0, 0, 0);
    rect(0.006, 0.52, 0.032, 0.535, 0, 0, 0);

    // back-facing singer outfit with Boishakh scarf
    rect(-0.045, 0.61, 0.045, 0.70, 0.85, 0.10, 0.15);
    rect(-0.010, 0.61, 0.010, 0.70, 1.0, 0.88, 0.15);
    rect(-0.045, 0.685, 0.045, 0.700, 1.0, 0.88, 0.15);

    // arms waving but no front face details
    rect(-0.065, 0.63 + mv, -0.030, 0.645 + mv, 0.85, 0.55, 0.35);
    rect(0.030, 0.63 - mv, 0.065, 0.645 - mv, 0.85, 0.55, 0.35);

    // back of head and hair/cap; no eyes/nose/mouth
    circle(0.0, 0.745 + mv, 0.035, 0.85, 0.55, 0.35);
    circle(0.0, 0.768 + mv, 0.032, 0.02, 0.018, 0.012);
    rect(-0.030, 0.735 + mv, 0.030, 0.765 + mv, 0.025, 0.018, 0.010);
    rect(-0.036, 0.790 + mv, 0.036, 0.806 + mv, 0.90, 0.05, 0.04);

    text(0.08, 0.76, "~");
    text(-0.12, 0.78, "~");
}

/* ================= STAGE ================= */

void drawStage() {
    // Day/night base light for final concert stage.
    if(!isNightMode) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.95f, 0.45f, 0.16f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.72f, 0.93f); glVertex2f(-0.35f, 0.42f); glVertex2f(0.05f, 0.42f);
        glVertex2f(0.72f, 0.93f); glVertex2f(0.35f, 0.42f); glVertex2f(-0.05f, 0.42f);
        glEnd();
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.90f, 0.18f, 0.20f);
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.78f, 0.93f); glVertex2f(-0.42f, 0.42f); glVertex2f(0.04f, 0.42f);
        glVertex2f(0.78f, 0.93f); glVertex2f(0.42f, 0.42f); glVertex2f(-0.04f, 0.42f);
        glEnd();
        glColor4f(0.20f, 0.75f, 1.0f, 0.16f);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.93f); glVertex2f(-0.18f, 0.42f); glVertex2f(0.18f, 0.42f);
        glEnd();
        glDisable(GL_BLEND);
    }

    // Clean concert stage area with clearer platform, lights and banner.
    rect(-0.92, 0.39, 0.92, 0.44, 0.025, 0.018, 0.06);
    rect(-0.86, 0.44, 0.86, 0.51, 0.08, 0.03, 0.16);
    rect(-0.78, 0.51, 0.78, 0.64, 0.40, 0.20, 0.08);

    glColor3f(0.66, 0.36, 0.13);
    glBegin(GL_LINES);
    for(float y = 0.535; y <= 0.625; y += 0.025) {
        glVertex2f(-0.76, y);
        glVertex2f(0.76, y);
    }
    for(float x = -0.74; x <= 0.74; x += 0.08) {
        glVertex2f(x, 0.51);
        glVertex2f(x, 0.64);
    }
    glEnd();

    // Main LED screen
    rect(-0.66, 0.66, 0.66, 0.88, 0.018, 0.018, 0.06);
    rect(-0.62, 0.69, 0.62, 0.85, 0.06, 0.05, 0.18);
    rect(-0.62, 0.69, -0.22, 0.85, 0.02, 0.13, 0.50);
    rect(-0.22, 0.69, 0.22, 0.85, 0.38, 0.04, 0.48);
    rect(0.22, 0.69, 0.62, 0.85, 0.04, 0.36, 0.16);

    text(-0.235, 0.795, "SHUBHO NOBOBORSHO");
    text(-0.250, 0.735, "POHELA BOISHAKH");

    // Truss frame
    rect(-0.86, 0.88, 0.86, 0.925, 0.11, 0.11, 0.15);
    rect(-0.82, 0.46, -0.75, 0.94, 0.11, 0.11, 0.15);
    rect(0.75, 0.46, 0.82, 0.94, 0.11, 0.11, 0.15);

    glColor3f(0.62, 0.62, 0.72);
    glBegin(GL_LINES);
    for(float x = -0.82; x < 0.82; x += 0.10) {
        glVertex2f(x, 0.88); glVertex2f(x + 0.07, 0.925);
        glVertex2f(x + 0.07, 0.88); glVertex2f(x, 0.925);
    }
    for(float y = 0.49; y < 0.91; y += 0.07) {
        glVertex2f(-0.82, y); glVertex2f(-0.75, y + 0.05);
        glVertex2f(-0.75, y); glVertex2f(-0.82, y + 0.05);
        glVertex2f(0.82, y); glVertex2f(0.75, y + 0.05);
        glVertex2f(0.75, y); glVertex2f(0.82, y + 0.05);
    }
    glEnd();

    // Stage lights
    for(float x = -0.66; x <= 0.66; x += 0.12) {
        int n = (int)((x + 0.70) * 100);
        if(n % 4 == 0) circle(x, 0.902, 0.013, 0.0, 0.45, 1.0);
        else if(n % 4 == 1) circle(x, 0.902, 0.013, 1.0, 0.0, 0.15);
        else if(n % 4 == 2) circle(x, 0.902, 0.013, 0.0, 0.9, 0.25);
        else circle(x, 0.902, 0.013, 1.0, 0.0, 0.85);
    }

    float lm = 0.08f * sin(lightMove);
    lightBeam(-0.48 + lm, 0.0, 0.35, 1.0);
    lightBeam(-0.24 - lm, 1.0, 0.0, 0.15);
    lightBeam(0.00 + lm, 0.0, 0.9, 0.25);
    lightBeam(0.24 - lm, 1.0, 0.0, 0.85);
    lightBeam(0.48 + lm, 0.0, 0.55, 1.0);

    // Speakers and instruments
    rect(-0.72, 0.50, -0.58, 0.76, 0.035, 0.035, 0.075);
    rect(0.58, 0.50, 0.72, 0.76, 0.035, 0.035, 0.075);
    circle(-0.65, 0.70, 0.034, 0.01, 0.01, 0.02);
    circle(-0.65, 0.58, 0.034, 0.01, 0.01, 0.02);
    circle(0.65, 0.70, 0.034, 0.01, 0.01, 0.02);
    circle(0.65, 0.58, 0.034, 0.01, 0.01, 0.02);

    rect(-0.50, 0.57, -0.32, 0.62, 0.92, 0.92, 0.86);
    rect(-0.50, 0.53, -0.32, 0.57, 0.06, 0.04, 0.08);
    circle(0.43, 0.59, 0.035, 0.65, 0.28, 0.05);
    rect(0.425, 0.59, 0.438, 0.75, 0.25, 0.10, 0.02);

    // Center singer platform
    rect(-0.24, 0.42, 0.24, 0.46, 0.09, 0.06, 0.12);
    rect(-0.19, 0.38, 0.19, 0.42, 0.11, 0.08, 0.15);
    rect(-0.14, 0.35, 0.14, 0.38, 0.13, 0.10, 0.18);

    glPushMatrix();
    glScalef(0.70, 0.70, 1);
    glTranslatef(0.0, 0.20, 0);
    drawSinger();
    glPopMatrix();
}

/* ================= PEOPLE ================= */

void drawPersonBase(float x, float y, int type, float r, float g, float b, int cheer) {
    float skinR = 0.92f;
    float skinG = 0.64f;
    float skinB = 0.44f;

    float armLift = cheer ? 0.020f * sin(waveAngle * 2.0f + x * 20.0f) : 0.0f;

    // legs and shoes
    rect(x - 0.017, y - 0.095, x - 0.006, y - 0.018, 0.02, 0.04, 0.15);
    rect(x + 0.006, y - 0.095, x + 0.017, y - 0.018, 0.02, 0.04, 0.15);
    rect(x - 0.024, y - 0.110, x - 0.002, y - 0.095, 0, 0, 0);
    rect(x + 0.002, y - 0.110, x + 0.026, y - 0.095, 0, 0, 0);

    // back-facing clothes: crowd looks toward the stage
    if(type == 4) {
        rect(x - 0.030, y - 0.075, x + 0.030, y - 0.020, r, g, b);
        glColor3f(r * 0.90f, g * 0.90f, b * 0.90f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x - 0.040, y - 0.092);
        glVertex2f(x + 0.040, y - 0.092);
        glVertex2f(x, y + 0.045);
        glEnd();
    } else {
        rect(x - 0.032, y - 0.020, x + 0.032, y + 0.050, r, g, b);
    }

    // Boishakh uttorio/scarf visible on the back
    if(type == 1 || type == 3) {
        rect(x - 0.030, y + 0.030, x + 0.030, y + 0.043, 0.95, 0.05, 0.04);
        rect(x - 0.004, y - 0.018, x + 0.004, y + 0.052, 1.0, 0.88, 0.15);
    }

    // arms
    if(cheer) {
        rect(x - 0.052, y + 0.012 + armLift, x - 0.041, y + 0.072 + armLift, skinR, skinG, skinB);
        rect(x + 0.041, y + 0.012 - armLift, x + 0.052, y + 0.072 - armLift, skinR, skinG, skinB);
    } else {
        rect(x - 0.043, y - 0.025, x - 0.033, y + 0.024, skinR, skinG, skinB);
        rect(x + 0.033, y - 0.025, x + 0.043, y + 0.024, skinR, skinG, skinB);
    }

    // neck and back of head - everyone faces the stage, so no eyes/nose/mouth.
    rect(x - 0.010, y + 0.045, x + 0.010, y + 0.062, skinR, skinG, skinB);
    circle(x, y + 0.086, 0.025, 0.055, 0.032, 0.018);

    // Large back hair/cap covers the face side. This clearly reads as piser dike.
    if(type == 4) {
        rect(x - 0.028, y + 0.060, x + 0.028, y + 0.112, 0.018, 0.012, 0.008);
        circle(x, y + 0.108, 0.021, 0.018, 0.012, 0.008);
        rect(x - 0.022, y + 0.058, x + 0.022, y + 0.074, 0.030, 0.020, 0.012);
    } else {
        circle(x, y + 0.099, 0.025, 0.018, 0.012, 0.008);
        rect(x - 0.025, y + 0.076, x + 0.025, y + 0.101, 0.018, 0.012, 0.008);
        rect(x - 0.020, y + 0.066, x + 0.020, y + 0.082, 0.030, 0.020, 0.012);
    }

    // small cap/flower on some people for Boishakh detail
    if(type == 2) {
        rect(x - 0.020, y + 0.104, x + 0.020, y + 0.115, 0.95, 0.85, 0.20);
        circle(x + 0.016, y + 0.114, 0.006, 0.95, 0.05, 0.04);
    }
}

void drawPerson(float x, float y, int type, float r, float g, float b, int cheer, float scale) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);
    drawPersonBase(0, 0, type, r, g, b, cheer);
    glPopMatrix();
}

void drawPhoneLight(float x, float y, float scale) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.7, 0.9, 1.0, 0.35);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for(int i = 0; i <= 30; i++) {
        float a = 2 * 3.1416 * i / 30;
        glVertex2f(x + cos(a) * 0.025 * scale,
                   y + sin(a) * 0.025 * scale);
    }

    glEnd();
    glDisable(GL_BLEND);

    rect(x - 0.008 * scale, y - 0.015 * scale,
         x + 0.008 * scale, y + 0.015 * scale,
         0.02, 0.02, 0.03);

    rect(x - 0.005 * scale, y - 0.011 * scale,
         x + 0.005 * scale, y + 0.011 * scale,
         0.8, 0.95, 1.0);
}

/* ================= EFFECTS ================= */

void spawnHitEffect(float x, float y) {
    int created = 0;

    for(int i = 0; i < MAX_EFFECTS && created < 24; i++) {
        if(!effects[i].active) {
            float angle = 2.0f * 3.1416f * created / 24.0f;
            float speed = 0.006f + (created % 5) * 0.002f;

            effects[i].x = x;
            effects[i].y = y;
            effects[i].dx = cos(angle) * speed;
            effects[i].dy = sin(angle) * speed;
            effects[i].life = 1.0f;
            effects[i].active = 1;

            created++;
        }
    }
}

void updateEffects() {
    for(int i = 0; i < MAX_EFFECTS; i++) {
        if(effects[i].active) {
            effects[i].x += effects[i].dx;
            effects[i].y += effects[i].dy;
            effects[i].dy -= 0.0005f;
            effects[i].life -= 0.045f;

            if(effects[i].life <= 0.0f)
                effects[i].active = 0;
        }
    }
}

void drawEffects() {
    glPointSize(4.0f);
    glBegin(GL_POINTS);

    for(int i = 0; i < MAX_EFFECTS; i++) {
        if(effects[i].active) {
            glColor3f(0.95f, 0.05f, 0.05f);
            glVertex2f(effects[i].x, effects[i].y);
        }
    }

    glEnd();
}

/* ================= CROWD SYSTEM ================= */

void initCrowd() {
    npcCount = 0;
    int k = 0;

    for(float y = -1.05f; y <= 0.22f; y += 0.13f) {
        for(float x = -0.78f; x <= 0.78f; x += 0.085f) {
            if(npcCount >= MAX_NPCS)
                return;

            float randX = ((k * 37) % 70) / 1000.0f - 0.035f;
            float randY = ((k * 53) % 60) / 1000.0f - 0.030f;

            npcs[npcCount].x = x + randX;
            npcs[npcCount].y = y + randY;

            npcs[npcCount].homeX = npcs[npcCount].x;
            npcs[npcCount].homeY = npcs[npcCount].y;

            npcs[npcCount].type = k % 5;
            npcs[npcCount].colorIndex = k % 8;
            npcs[npcCount].scale = 0.40f + (k % 4) * 0.022f;
            npcs[npcCount].imposter = 0;
            npcs[npcCount].active = 1;

            npcCount++;
            k++;
        }
    }

    int hiddenIds[IMPOSTER_TOTAL] = {28, 64, 103, 148, 187};

    for(int i = 0; i < IMPOSTER_TOTAL; i++) {
        int id = hiddenIds[i];

        if(id >= 0 && id < npcCount) {
            npcs[id].imposter = 1;
            npcs[id].colorIndex = 0;
            npcs[id].type = 0;
            npcs[id].scale = 0.46f;
        }
    }
}


void setLevel3Message(const char* msg, int timer) {
    snprintf(level3Message, sizeof(level3Message), "%s", msg);
    level3MessageTimer = timer;
}

void clearEffects() {
    for(int i = 0; i < MAX_EFFECTS; i++) {
        effects[i].active = 0;
    }
}

void resetRun() {
    playerX = 0.0f;
    playerY = -0.82f;
    camX = 0.0f;
    camY = -0.55f;
    aimX = 0.0f;
    aimY = 0.0f;
    mouseScreenX = winW / 2;
    mouseScreenY = winH / 2;

    for(int i = 0; i < 256; i++) {
        keyState[i] = 0;
    }

    impostersStopped = 0;
    bullets = 2;
    mainImpX = 0.62f;
    mainImpY = 0.08f;
    mainImpActive = 0;
    gameState = PLAYING;

    clearEffects();
    initCrowd();
    missionStartTime = glutGet(GLUT_ELAPSED_TIME);
}

void startLevel3(int lifeCount) {
    if(lifeCount < 1)
        lifeCount = 1;

    level3Lives = lifeCount;
    resetRun();

    char msg[220];
    snprintf(msg, sizeof(msg), "Final Concert started. Lives: %d. Everyone faces the stage; find black imposters and press K near them.", level3Lives);
    setLevel3Message(msg, 360);
}

void loseRun(const char* msg) {
    if(gameState == WON || gameState == LOST)
        return;

    if(level3Lives > 1) {
        level3Lives--;
        resetRun();

        char out[220];
        snprintf(out, sizeof(out), "%s Restarted Level 3. Lives left: %d.", msg, level3Lives);
        setLevel3Message(out, 360);
    } else {
        level3Lives = 0;
        gameState = LOST;

        char out[220];
        snprintf(out, sizeof(out), "%s No lives left. Press R to retry Level 3.", msg);
        setLevel3Message(out, 520);
    }
}

void updateCrowdMovement() {
    float avoidDistance = 0.105f;
    float pushStrength = 0.22f;
    float returnSpeed = 0.018f;

    int level = getLevel();

    for(int i = 0; i < npcCount; i++) {
        if(!npcs[i].active)
            continue;

        float d = distance2D(playerX, playerY, npcs[i].x, npcs[i].y);

        if(d < avoidDistance) {
            float dx;
            float dy;

            if(d < 0.001f) {
                dx = (i % 2 == 0) ? 1.0f : -1.0f;
                dy = (i % 3 == 0) ? 1.0f : -1.0f;
                d = 1.0f;
            } else {
                dx = npcs[i].x - playerX;
                dy = npcs[i].y - playerY;
                dx /= d;
                dy /= d;
            }

            float push = (avoidDistance - d) * pushStrength;

            npcs[i].x += dx * push;
            npcs[i].y += dy * push;

            if(npcs[i].x < -1.10f) npcs[i].x = -1.10f;
            if(npcs[i].x > 1.10f)  npcs[i].x = 1.10f;
            if(npcs[i].y < -1.14f) npcs[i].y = -1.14f;
            if(npcs[i].y > 0.30f)  npcs[i].y = 0.30f;
        } else {
            if(npcs[i].imposter && gameState == PLAYING) {
                float targetX = 0.0f;
                float targetY = -0.02f;

                float dx = targetX - npcs[i].x;
                float dy = targetY - npcs[i].y;
                float len = sqrt(dx * dx + dy * dy);

                if(len > 0.001f) {
                    dx /= len;
                    dy /= len;

                    float driftSpeed = 0.00035f + level * 0.00022f;

                    npcs[i].x += dx * driftSpeed;
                    npcs[i].y += dy * driftSpeed;
                }
            } else {
                npcs[i].x += (npcs[i].homeX - npcs[i].x) * returnSpeed;
                npcs[i].y += (npcs[i].homeY - npcs[i].y) * returnSpeed;
            }
        }
    }
}

/* ================= IMPOSTER HINT ================= */

void drawSuspicionHint(float x, float y, float scale, int level, float distFromPlayer) {
    int showHint = 0;

    // Easier detection range
    if(level == 1 && distFromPlayer < 0.75f)
        showHint = 1;
    else if(level == 2 && distFromPlayer < 0.55f)
        showHint = 1;
    else if(level == 3 && distFromPlayer < 0.38f)
        showHint = 1;

    if(!showHint)
        return;

    float pulse = 0.006f * sin(waveAngle * 4.0f);
    float hx = x;
    float hy = y + 0.085f * scale + 0.055f + pulse;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.75f, 0.05f, 0.22f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(hx, hy);

    for(int i = 0; i <= 24; i++) {
        float a = 2.0f * 3.1416f * i / 24.0f;
        glVertex2f(hx + cos(a) * 0.030f,
                   hy + sin(a) * 0.030f);
    }

    glEnd();
    glDisable(GL_BLEND);

    circle(hx, hy, 0.012f, 1.0f, 0.82f, 0.05f);
    circle(hx, hy, 0.004f, 0.02f, 0.02f, 0.02f);
}

void drawCrowd() {
    float colors[8][3] = {
        {0.1, 0.45, 0.9},
        {0.9, 0.25, 0.2},
        {0.2, 0.7, 0.35},
        {0.9, 0.75, 0.2},
        {0.6, 0.25, 0.8},
        {0.1, 0.7, 0.7},
        {0.95, 0.35, 0.65},
        {0.85, 0.55, 0.15}
    };

    for(int i = 0; i < npcCount; i++) {
        if(!npcs[i].active)
            continue;

        int c = npcs[i].colorIndex;

        int cheer = 0;

        if(i % 6 == 0 && sin(waveAngle + i) > 0.5f)
            cheer = 1;

        float jump = 0.0f;

        if(i % 5 == 0)
            jump = 0.006f * sin(waveAngle * 2 + i);

        if(npcs[i].imposter) {
            drawPerson(npcs[i].x, npcs[i].y + jump,
                       npcs[i].type,
                       0.005f, 0.005f, 0.005f,
                       cheer,
                       npcs[i].scale);

            float d = distance2D(playerX, playerY, npcs[i].x, npcs[i].y);
            drawSuspicionHint(npcs[i].x, npcs[i].y + jump, npcs[i].scale, getLevel(), d);
        } else {
            drawPerson(npcs[i].x, npcs[i].y + jump,
                       npcs[i].type,
                       colors[c][0], colors[c][1], colors[c][2],
                       cheer,
                       npcs[i].scale);
        }

        if(cheer == 1 && i % 3 == 0 && !npcs[i].imposter) {
            float handMove = 0.006f * sin(waveAngle);

            float handX = npcs[i].x + (0.045f * npcs[i].scale);
            float handY = npcs[i].y + jump + ((0.055f - handMove) * npcs[i].scale);

            drawPhoneLight(handX, handY, npcs[i].scale);
        }
    }
}

/* ================= PLAYER ================= */

void drawPlayer() {
    float bob = 0.004f * sin(waveAngle * 2.0f);
    float x = playerX;
    float y = playerY + bob;
    float walk = 0.005f * sin(waveAngle * 2.0f);
    float sc = 0.65f;

    // Back-facing hero: he is looking toward the concert/stage, not toward the camera.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0, 0.0, 0.0, 0.25);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y - 0.052f * sc);
    for(int i = 0; i <= 30; i++) {
        float a = 2.0f * 3.1416f * i / 30.0f;
        glVertex2f(x + cos(a) * 0.045f * sc, y - 0.052f * sc + sin(a) * 0.014f * sc);
    }
    glEnd();
    glDisable(GL_BLEND);

    // pajama and shoes
    rect(x - 0.020f * sc, y - 0.090f * sc, x - 0.006f * sc, y - 0.025f * sc + walk, 0.94f, 0.94f, 0.88f);
    rect(x + 0.006f * sc, y - 0.090f * sc, x + 0.020f * sc, y - 0.025f * sc - walk, 0.94f, 0.94f, 0.88f);
    rect(x - 0.028f * sc, y - 0.105f * sc, x - 0.002f * sc, y - 0.090f * sc, 0.02f, 0.02f, 0.025f);
    rect(x + 0.002f * sc, y - 0.105f * sc, x + 0.030f * sc, y - 0.090f * sc, 0.02f, 0.02f, 0.025f);

    // blue panjabi back
    rect(x - 0.035f * sc, y - 0.030f * sc, x + 0.035f * sc, y + 0.055f * sc, 0.05f, 0.22f, 0.72f);
    rect(x - 0.027f * sc, y - 0.020f * sc, x + 0.027f * sc, y + 0.047f * sc, 0.16f, 0.42f, 0.95f);

    // back seam + Boishakh uttorio
    rect(x - 0.003f * sc, y - 0.022f * sc, x + 0.003f * sc, y + 0.052f * sc, 0.95f, 0.82f, 0.28f);
    rect(x - 0.034f * sc, y + 0.040f * sc, x + 0.034f * sc, y + 0.052f * sc, 0.95f, 0.05f, 0.04f);
    rect(x - 0.005f * sc, y - 0.024f * sc, x + 0.005f * sc, y + 0.052f * sc, 1.0f, 0.88f, 0.15f);

    // arms
    rect(x - 0.052f * sc, y + 0.010f * sc, x - 0.034f * sc, y + 0.050f * sc, 0.05f, 0.22f, 0.72f);
    rect(x + 0.034f * sc, y + 0.010f * sc, x + 0.052f * sc, y + 0.050f * sc, 0.05f, 0.22f, 0.72f);
    circle(x - 0.045f * sc, y + 0.006f * sc, 0.010f * sc, 0.95f, 0.67f, 0.49f);
    circle(x + 0.045f * sc, y + 0.006f * sc, 0.010f * sc, 0.95f, 0.67f, 0.49f);

    // neck and back of head - no front face details
    rect(x - 0.010f * sc, y + 0.052f * sc, x + 0.010f * sc, y + 0.067f * sc, 0.95f, 0.67f, 0.49f);
    circle(x, y + 0.095f * sc, 0.029f * sc, 0.06f, 0.035f, 0.02f);

    // big back hair + blue cap/topi, covering any face side
    circle(x, y + 0.106f * sc, 0.032f * sc, 0.06f, 0.035f, 0.02f);
    rect(x - 0.028f * sc, y + 0.081f * sc, x + 0.028f * sc, y + 0.110f * sc, 0.06f, 0.035f, 0.02f);
    rect(x - 0.030f * sc, y + 0.120f * sc, x + 0.030f * sc, y + 0.137f * sc, 0.08f, 0.18f, 0.55f);
    rect(x - 0.010f * sc, y + 0.123f * sc, x + 0.010f * sc, y + 0.136f * sc, 0.95f, 0.82f, 0.28f);

    // small weapon held at side
    rect(x + 0.036f * sc, y + 0.012f * sc, x + 0.070f * sc, y + 0.022f * sc, 0.04f, 0.04f, 0.05f);
}

/* ================= MAIN IMPOSTER ================= */

void startMainAttack() {
    gameState = MAIN_ATTACK;
    mainImpActive = 1;
    mainImpX = 0.62f;
    mainImpY = 0.08f;
    bullets = 2;
}

void updateMainImposter() {
    if(gameState != MAIN_ATTACK || !mainImpActive)
        return;

    float targetX = 0.0f;
    float targetY = 0.20f;

    float dx = targetX - mainImpX;
    float dy = targetY - mainImpY;
    float d = sqrt(dx * dx + dy * dy);

    if(d > 0.001f) {
        dx /= d;
        dy /= d;

        mainImpX += dx * 0.0065f;
        mainImpY += dy * 0.0065f;
    }

    if(distance2D(mainImpX, mainImpY, targetX, targetY) < 0.045f) {
        loseRun("Final attacker reached the stage.");
    }
}

void drawMainImposter() {
    if(gameState != MAIN_ATTACK || !mainImpActive)
        return;

    float jump = fabs(sin(waveAngle * 5.0f)) * 0.045f;

    drawPerson(mainImpX, mainImpY + jump,
               0,
               0.0f, 0.0f, 0.0f,
               1,
               0.55f);

    glColor3f(1.0f, 0.05f, 0.05f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(mainImpX - 0.04f, mainImpY + jump + 0.11f);
    glVertex2f(mainImpX + 0.04f, mainImpY + jump + 0.11f);
    glEnd();
}

/* ================= ACTION LOGIC ================= */

void tryKnifeAction() {
    if(gameState != PLAYING)
        return;

    float bestD = 999.0f;
    int bestIndex = -1;

    for(int i = 0; i < npcCount; i++) {
        if(npcs[i].active && npcs[i].imposter) {
            float d = distance2D(playerX, playerY, npcs[i].x, npcs[i].y);

            if(d < bestD) {
                bestD = d;
                bestIndex = i;
            }
        }
    }

    if(bestIndex != -1 && bestD < 0.105f) {
        spawnHitEffect(npcs[bestIndex].x, npcs[bestIndex].y);
        npcs[bestIndex].active = 0;
        impostersStopped++;

        if(impostersStopped >= IMPOSTER_TOTAL) {
            startMainAttack();
        }
    }
}

void tryGunAction() {
    if(gameState != MAIN_ATTACK)
        return;

    if(bullets <= 0)
        return;

    bullets--;

    spawnHitEffect(aimX, aimY);

    float d = distance2D(aimX, aimY, mainImpX, mainImpY);

    if(d < 0.095f) {
        spawnHitEffect(mainImpX, mainImpY);
        mainImpActive = 0;
        gameState = WON;
    } else {
        if(bullets <= 0) {
            loseRun("No bullets left.");
        }
    }
}

/* ================= COLLISION ================= */

int pointInsideRect(float x, float y, float x1, float y1, float x2, float y2) {
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

int canMoveTo(float x, float y) {
    if(x < -1.18f || x > 1.18f || y < -1.15f || y > 0.34f)
        return 0;

    if(pointInsideRect(x, y, -0.76f, -0.07f, 0.76f, 0.42f))
        return 0;

    if(pointInsideRect(x, y, -1.02f, -0.87f, -0.74f, -0.34f))
        return 0;

    if(pointInsideRect(x, y, 0.74f, -0.87f, 1.02f, -0.34f))
        return 0;

    return 1;
}

void updatePlayerMovement() {
    if(gameState == WON || gameState == LOST)
        return;

    float dx = 0.0f;
    float dy = 0.0f;

    if(keyState['w']) dy += playerSpeed;
    if(keyState['s']) dy -= playerSpeed;
    if(keyState['a']) dx -= playerSpeed;
    if(keyState['d']) dx += playerSpeed;

    if(dx != 0.0f && dy != 0.0f) {
        dx *= 0.707f;
        dy *= 0.707f;
    }

    float nextX = playerX + dx;
    float nextY = playerY + dy;

    if(canMoveTo(nextX, playerY))
        playerX = nextX;

    if(canMoveTo(playerX, nextY))
        playerY = nextY;
}

/* ================= AIM POINTER ================= */

void drawAimPointer() {
    if(gameState != MAIN_ATTACK)
        return;

    glColor3f(1.0f, 0.95f, 0.2f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(aimX - 0.035f, aimY);
    glVertex2f(aimX + 0.035f, aimY);
    glVertex2f(aimX, aimY - 0.035f);
    glVertex2f(aimX, aimY + 0.035f);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.95f, 0.2f, 0.18f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(aimX, aimY);

    for(int i = 0; i <= 30; i++) {
        float a = 2.0f * 3.1416f * i / 30.0f;
        glVertex2f(aimX + cos(a) * 0.055f, aimY + sin(a) * 0.055f);
    }

    glEnd();
    glDisable(GL_BLEND);
}

/* ================= HUD ================= */

void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(-1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Level 3 navbar/message bar follows the Level 2 style: bottom black bar, white text.
    char info[220];

    if(level3MessageTimer > 0) {
        rect(-0.99, -0.98, 0.86, -0.86, 0.0, 0.0, 0.0);
        textSmall(-0.94, -0.915, level3Message);
    } else {
        rect(-0.99, -0.98, 0.86, -0.86, 0.0, 0.0, 0.0);

        if(gameState == PLAYING) {
            sprintf(info, "Level 3 | Lives: %d | Time: %02d | Imposters: %d/%d | K=Knife | R=Restart.",
                    level3Lives, getTimeLeft(), impostersStopped, IMPOSTER_TOTAL);
        } else if(gameState == MAIN_ATTACK) {
            sprintf(info, "Final attack | Lives: %d | Bullets: %d | Aim mouse + G=Shoot | R=Restart.",
                    level3Lives, bullets);
        } else if(gameState == WON) {
            sprintf(info, "Mission complete. The singer is safe. Press R to replay Level 3.");
        } else {
            sprintf(info, "Mission failed. No lives left. Press R to retry Level 3.");
        }

        textSmall(-0.94, -0.915, info);
    }

    if(gameState == WON) {
        rect(-0.45, -0.08, 0.45, 0.12, 0.02, 0.18, 0.06);
        text(-0.30, 0.04, "MISSION COMPLETE");
        textSmall(-0.31, -0.03, "All threats stopped. The singer is safe.");
    }

    if(gameState == LOST) {
        rect(-0.45, -0.08, 0.45, 0.12, 0.20, 0.02, 0.04);
        text(-0.22, 0.04, "MISSION FAILED");
        textSmall(-0.31, -0.03, "No lives left. Press R to retry.");
    }

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}


/* ================= BOISHAKH DECOR ================= */

void drawBoishakhAlpana(float x, float y, float s) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.95f, 0.82f, 0.75f);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 48; i++) {
        float a = 2.0f * 3.1416f * i / 48.0f;
        float rr = 0.035f * s + 0.010f * s * sin(6.0f * a);
        glVertex2f(x + cos(a) * rr, y + sin(a) * rr);
    }
    glEnd();
    circle(x, y, 0.010f * s, 0.95f, 0.06f, 0.04f);
    glDisable(GL_BLEND);
}

void drawBoishakhFlags() {
    glColor3f(0.90f, 0.75f, 0.20f);
    glBegin(GL_LINES);
    glVertex2f(-1.20f, 0.50f); glVertex2f(1.20f, 0.50f);
    glEnd();

    for(int i = 0; i < 22; i++) {
        float x = -1.18f + i * 0.112f;
        if(i % 3 == 0) glColor3f(0.95f, 0.05f, 0.04f);
        else if(i % 3 == 1) glColor3f(1.0f, 0.85f, 0.05f);
        else glColor3f(0.05f, 0.55f, 0.12f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, 0.50f);
        glVertex2f(x + 0.050f, 0.50f);
        glVertex2f(x + 0.025f, 0.455f);
        glEnd();
    }
}

void drawBoishakhMelaDecor() {
    // extra Boishakh atmosphere around the concert area
    drawBoishakhFlags();
    drawBoishakhAlpana(-0.55f, -0.96f, 1.25f);
    drawBoishakhAlpana(0.52f, -0.92f, 1.15f);
    drawBoishakhAlpana(0.02f, -0.70f, 0.95f);

    // colorful side entrance banners
    rect(-1.18f, -0.15f, -1.07f, 0.30f, 0.95f, 0.05f, 0.04f);
    rect(1.07f, -0.15f, 1.18f, 0.30f, 0.95f, 0.05f, 0.04f);
    rect(-1.16f, 0.22f, -1.09f, 0.28f, 1.0f, 0.90f, 0.18f);
    rect(1.09f, 0.22f, 1.16f, 0.28f, 1.0f, 0.90f, 0.18f);

    // dhol drums near field
    circle(-0.86f, -0.57f, 0.040f, 0.72f, 0.32f, 0.06f);
    circle(-0.90f, -0.57f, 0.020f, 0.95f, 0.82f, 0.50f);
    circle(-0.82f, -0.57f, 0.020f, 0.95f, 0.82f, 0.50f);
    circle(0.86f, -0.58f, 0.040f, 0.72f, 0.32f, 0.06f);
    circle(0.82f, -0.58f, 0.020f, 0.95f, 0.82f, 0.50f);
    circle(0.90f, -0.58f, 0.020f, 0.95f, 0.82f, 0.50f);

    // top note
    textSmall(-0.28f, 0.455f, "Boishakh Mela Crowd - Everyone faces the stage");
}

/* ================= DISPLAY ================= */

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    setupCamera();

    drawSky();
    drawGround();
    drawTreesAndLights();
    drawStalls();
    drawBoishakhMelaDecor();

    glPushMatrix();
    glTranslatef(0.0f, -0.26f, 0.0f);
    glScalef(0.78f, 0.78f, 1.0f);
    drawStage();
    glPopMatrix();

    drawBarrier();

    drawCrowd();
    drawMainImposter();
    drawPlayer();
    drawEffects();
    drawAimPointer();

    drawHUD();

    glFlush();
}

/* ================= INPUT ================= */

void keyboardDown(unsigned char key, int x, int y) {
    key = tolower(key);

    if(key == 27)
        exit(0);

    if(key == 'r') {
        int restartLives = level3Lives;
        if(restartLives < 1)
            restartLives = 1;
        startLevel3(restartLives);
        return;
    }

    if(key == 'k') {
        tryKnifeAction();
        return;
    }

    if(key == 'g') {
        tryGunAction();
        return;
    }

    keyState[key] = 1;
}

void keyboardUp(unsigned char key, int x, int y) {
    key = tolower(key);
    keyState[key] = 0;
}

void arrowDown(int key) {
    if(key == GLUT_KEY_LEFT)  keyState['a'] = 1;
    if(key == GLUT_KEY_RIGHT) keyState['d'] = 1;
    if(key == GLUT_KEY_UP)    keyState['w'] = 1;
    if(key == GLUT_KEY_DOWN)  keyState['s'] = 1;
}

void arrowUp(int key) {
    if(key == GLUT_KEY_LEFT)  keyState['a'] = 0;
    if(key == GLUT_KEY_RIGHT) keyState['d'] = 0;
    if(key == GLUT_KEY_UP)    keyState['w'] = 0;
    if(key == GLUT_KEY_DOWN)  keyState['s'] = 0;
}

void mouseMove(int x, int y) {
    mouseScreenX = x;
    mouseScreenY = y;
    updateAimWorld();
}

/* ================= UPDATE ================= */

void step() {
    waveAngle += 0.10f;
    singerMove += 0.12f;
    lightMove += 0.08f;
    cloudMove += 0.006f;
    if(cloudMove > 2.80f) cloudMove -= 2.80f;

    if(level3MessageTimer > 0)
        level3MessageTimer--;

    if(gameState == PLAYING || gameState == MAIN_ATTACK) {
        if(getTimeLeft() <= 0) {
            loseRun("Time finished.");
            return;
        }
    }

    updatePlayerMovement();
    updateCrowdMovement();
    updateMainImposter();
    updateEffects();
}

/* ================= WINDOW ================= */

void reshape(int w, int h) {
    winW = w;
    winH = h;

    if(winH == 0)
        winH = 1;

    glViewport(0, 0, winW, winH);
}

/* ================= INIT ================= */

void init() {
    glClearColor(0.02, 0.02, 0.04, 1);
    clearEffects();
}


} // namespace L3
