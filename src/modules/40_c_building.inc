// AIUB Imposter Detection
// Module: 40_c_building.inc
// C Building visual module and water-zone backdrop.

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
