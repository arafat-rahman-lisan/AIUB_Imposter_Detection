// AIUB Imposter Detection
// Module: 10_annex_building.inc
// Annex / security-complex visual module.

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
