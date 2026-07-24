// AIUB Imposter Detection
// Module: 80_final_concert.inc
// Final concert crowd and imposter-detection level.

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
