#pragma once

#include <SDL2/SDL.h>

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

struct GamepadState
{
    bool current[BUTTON_COUNT];
    bool previous[BUTTON_COUNT];
};

void                ctrlUpdate(GamepadState& state);                                 // Updates with the keyboard
void                ctrlUpdate(GamepadState& state, SDL_GameController* controller); // Updates with a controller
bool                ctrlIsDown(const GamepadState& state, Button button);
bool                ctrlIsReleased(const GamepadState& state, Button button);
bool                ctrlIsPressed(const GamepadState& state, Button button);
SDL_GameController* ctrlFindController();
