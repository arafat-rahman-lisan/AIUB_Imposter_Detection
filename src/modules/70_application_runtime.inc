// AIUB Imposter Detection
// Module: 70_application_runtime.inc
// Display routing, timer, keyboard/mouse callbacks and main().

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
