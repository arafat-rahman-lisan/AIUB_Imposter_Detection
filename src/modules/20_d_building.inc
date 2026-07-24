// AIUB Imposter Detection
// Module: 20_d_building.inc
// D Building visual module and ID-drop friend scene.

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
