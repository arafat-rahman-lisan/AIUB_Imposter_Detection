// AIUB Imposter Detection
// Module: 30_campus_environment.inc
// Shared campus vegetation and environmental decoration.

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
