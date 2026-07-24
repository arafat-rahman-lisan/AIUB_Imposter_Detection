# Code Guide — Logic and Maths

## File Design

`src/main.cpp` includes the story modules in execution order. This keeps one translation unit, so the original global state remains compatible, while building artwork and levels stay in separate files.

| Module | Purpose |
|---|---|
| `00_core.inc` | constants, state, drawing primitives, shared sky |
| `10_annex_building.inc` | Annex/security artwork |
| `20_d_building.inc` | D Building and friend/ID scene |
| `40_c_building.inc` | round C Building and water backdrop |
| `50_level2_gameplay.inc` | hazards, inventory, guard and collision logic |
| `60_boishakh_fair.inc` | transition and Panta/Fair gameplay |
| `80_final_concert.inc` | crowd, imposters, shooting and victory |

## Movement

```cpp
vx = keyLeft ? -SPEED : keyRight ? SPEED : 0;
px += vx;

vy += GRAVITY;
py += vy;
```

Jumping gives an upward velocity:

```cpp
vy = JUMP_POWER;
```

Gravity reduces it every update, producing a parabola-like jump.

## Collision — AABB

Two rectangles overlap when their horizontal and vertical ranges both intersect:

```cpp
a.x < b.x + b.w && a.x + a.w > b.x &&
a.y < b.y + b.h && a.y + a.h > b.y
```

This is used for platforms, items, guards, water, gates and imposters.

## Camera

```cpp
camX = clamp(playerX - viewportWidth / 2, 0, WORLD_W - viewportWidth);
```

The player remains near the screen centre, while the camera is restricted to the world edges.

## Falling ID

```cpp
idVelY += GRAVITY * 0.45f;
idCard.y += idVelY;
```

The same velocity–gravity model creates the ID-card drop from D Building.

## Scene Changes

```cpp
enum AppPhase {
    PHASE_LEVEL2,
    PHASE_TRANSITION,
    PHASE_PANTA_LEVEL,
    PHASE_BOISHAKH_ENTRY,
    PHASE_LEVEL3
};
```

`display()` and `timer()` route drawing and updates according to `appPhase`. The main loop requests updates every **16 ms**; the concert logic is throttled to **40 ms** to preserve its original speed.

## Final-Level Detection

Distance checks use Euclidean distance:

```cpp
sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
```

Mouse coordinates are mapped from screen space into world space before the final shot. The concert timer is:

```cpp
60 - elapsedSeconds
```
