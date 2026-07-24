// AIUB Imposter Detection
// Module: 60_boishakh_fair.inc
// Transition, Panta Ilish fair and Boishakh entry scenes.

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
