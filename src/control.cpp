#include "control.hpp"

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
  SDL_SCANCODE_RSHIFT};

static constexpr SDL_GameControllerButton GAMECONTROLLER_MAP[BUTTON_COUNT] = {
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START,
  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK,
};

void ctrlUpdate(GamepadState& state)
{
    const Uint8* sdl_state = SDL_GetKeyboardState(nullptr);

    for ( int idx = 0; idx < BUTTON_COUNT; idx++ )
    {
        state.previous[idx] = state.current[idx];
        state.current[idx] = sdl_state[KEYBOARD_MAP[idx]];
    }
}

void ctrlUpdate(GamepadState& state, SDL_GameController* controller)
{
    for ( int idx = 0; idx < BUTTON_COUNT; idx++ )
    {
        state.previous[idx] = state.current[idx];
        state.current[idx] = SDL_GameControllerGetButton(controller, GAMECONTROLLER_MAP[idx]);
    }
}

bool ctrlIsDown(const GamepadState& state, Button button)
{
    return state.current[button];
}

bool ctrlIsReleased(const GamepadState& state, Button button)
{
    return not state.current[button] and state.previous[button];
}

bool ctrlIsPressed(const GamepadState& state, Button button)
{
    return state.current[button] and not state.previous[button];
}

SDL_GameController* ctrlFindController()
{
    for ( int i = 0; i < SDL_NumJoysticks(); i++ )
    {
        if ( SDL_IsGameController(i) )
        {
            return SDL_GameControllerOpen(i);
        }
    }
    return nullptr;
}

