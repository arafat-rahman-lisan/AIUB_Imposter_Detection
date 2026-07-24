// AIUB Imposter Detection
// Module: 50_level2_gameplay.inc
// Campus mission gameplay: platforms, hazards, items, guard, collisions and Level 2 logic.

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
