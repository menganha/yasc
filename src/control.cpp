#include "control.hpp"

#include <SDL2/SDL.h>

static constexpr SDL_Scancode KEYBOARD_MAP[BUTTON_COUNT] = {
  SDL_SCANCODE_UP,
  SDL_SCANCODE_DOWN,
  SDL_SCANCODE_LEFT,
  SDL_SCANCODE_RIGHT,
  SDL_SCANCODE_Z,
  SDL_SCANCODE_X,
  SDL_SCANCODE_A,
  SDL_SCANCODE_S,
  SDL_SCANCODE_RETURN,
  SDL_SCANCODE_C};

void ctrlUpdate(Keyboard& keyboard)
{
    const Uint8* sdl_state = SDL_GetKeyboardState(nullptr);

    for ( int idx = 0; idx < BUTTON_COUNT; idx++ )
    {
        keyboard.state_prev[idx] = keyboard.state_curr[idx];
        keyboard.state_curr[idx] = sdl_state[KEYBOARD_MAP[idx]];
    }
}

bool ctrlIsDown(const Keyboard& keyboard, Button button)
{
    return keyboard.state_curr[button];
}

bool ctrlIsReleased(const Keyboard& keyboard, Button button)
{
    return not keyboard.state_curr[button] and keyboard.state_prev[button];
}

bool ctrlIsPressed(const Keyboard& keyboard, Button button)
{
    return keyboard.state_curr[button] and not keyboard.state_prev[button];
}
