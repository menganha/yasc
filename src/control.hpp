#pragma once

enum Button
{
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_A,
    BUTTON_B,
    BUTTON_Y,
    BUTTON_X,
    BUTTON_START,
    BUTTON_SELECT,
    BUTTON_COUNT
};

struct Keyboard
{
    bool state_curr[BUTTON_COUNT];
    bool state_prev[BUTTON_COUNT];
};

void ctrlUpdate(Keyboard& keyboard);
bool ctrlIsDown(const Keyboard& keyboard, Button button);
bool ctrlIsReleased(const Keyboard& keyboard);
bool ctrlIsPresed(const Keyboard& keyboard);
