/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * modified by gave92
 *
 * WiiBrowser
 * input.h
 * Wii/GameCube controller management
 ***************************************************************************/

#ifndef _INPUT_H_
#define _INPUT_H_

#include <gccore.h>
#include <wiiuse/wpad.h>

#define PI 				3.14159265f
#define PADCAL			50

extern int rumbleRequest[4];

// Unified input abstraction (M4)
typedef enum {
    INPUT_BTN_A = 0,
    INPUT_BTN_B,
    INPUT_BTN_PLUS,
    INPUT_BTN_MINUS,
    INPUT_BTN_HOME,
    INPUT_BTN_UP,
    INPUT_BTN_DOWN,
    INPUT_BTN_LEFT,
    INPUT_BTN_RIGHT,
    INPUT_BTN_COUNT
} InputButton;

typedef struct {
    u16 held;
    u16 down;
    u16 up;
    s8 stickX;
    s8 stickY;
} InputState;

extern InputState g_inputState[4];
bool Input_IsHeld(int chan, InputButton btn);
bool Input_IsDown(int chan, InputButton btn);

void SetupPads();
void UpdatePads();
void ShutoffRumble();
void DoRumble(int i);
void MPlayerInput();

#endif
