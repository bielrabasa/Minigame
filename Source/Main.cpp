// -------------------------------------------------------------------------
// Awesome simple game with SDL
// Lesson 2 - Input Events
//
// SDL API: http://wiki.libsdl.org/APIByCategory
// -------------------------------------------------------------------------

#include <stdio.h>			// Required for: printf()
#include <stdlib.h>			// Required for: EXIT_SUCCESS
#include <string.h>			// Required for: memset()
#include <math.h>			// Required for: sinf(), cosf()

// Include SDL libraries
#include "SDL/include/SDL.h"				// Required for SDL base systems functionality
//#include "SDL_image/include/SDL_image.h"	// Required for image loading functionality

// Define libraries required by linker
// WARNING: Not all compilers support this option and it couples 
// source code with build system, it's recommended to keep both 
// separated, in case of multiple build configurations
//#pragma comment(lib, "SDL/lib/x86/SDL2.lib")
//#pragma comment(lib, "SDL/lib/x86/SDL2main.lib")
//#pragma comment(lib, "SDL_image/lib/x86/SDL2_image.lib")

// -------------------------------------------------------------------------
// Defines, Types and Globals
// -------------------------------------------------------------------------
#define SCREEN_WIDTH		1280
#define SCREEN_HEIGHT		 720

#define MAX_KEYBOARD_KEYS	 256
#define MAX_MOUSE_BUTTONS	   5
#define JOYSTICK_DEAD_ZONE  8000

#define SHIP_SPEED			   3
#define MAX_SHIP_SHOTS		  32
#define SHOT_SPEED			   5

enum WindowEvent
{
	WE_QUIT = 0,
	WE_HIDE,
	WE_SHOW,
	WE_COUNT
};

enum KeyState
{
	KEY_IDLE = 0,		// DEFAULT
	KEY_DOWN,			// PRESSED (DEFAULT->DOWN)
	KEY_REPEAT,			// KEEP DOWN (sustained)
	KEY_UP				// RELEASED (DOWN->DEFAULT)
};

struct Projectile
{
	int x, y;
	bool alive;
};

// Global context to store our game state data
struct GlobalState
{
	// Window and renderer
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;

	// Input events
	KeyState* keyboard;
	KeyState mouse_buttons[MAX_MOUSE_BUTTONS];
	int mouse_x;
	int mouse_y;
	SDL_Joystick* gamepad;
	int gamepad_axis_x_dir;
	int gamepad_axis_y_dir;
	bool window_events[WE_COUNT];

	// Game elements
	int ship_x;
	int ship_y;
	Projectile shots[MAX_SHIP_SHOTS];
	int last_shot;
};

// Global game state variable
GlobalState state;

// Functions Declarations
// Some helpful functions to draw basic shapes
// -------------------------------------------------------------------------
static void DrawRectangle(int x, int y, int width, int height, SDL_Color color);
static void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color);
static void DrawCircle(int x, int y, int radius, SDL_Color color);

// Functions Declarations and Definition
// -------------------------------------------------------------------------
void Start()
{
	// Initialize SDL internal global state
	SDL_Init(SDL_INIT_EVERYTHING);

	// Init input events system
	//if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) printf("SDL_EVENTS could not be initialized! SDL_Error: %s\n", SDL_GetError());

	// Init window
	state.window = SDL_CreateWindow("Super Awesome Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	state.surface = SDL_GetWindowSurface(state.window);

	// Init renderer
	state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawColor(state.renderer, 100, 149, 237, 255);		// Default clear color: Cornflower blue

	// L2: TODO 1: Init input variables (keyboard, mouse_buttons)


	// L2: TODO 2: Init input gamepad 
	// Check SDL_NumJoysticks() and SDL_JoystickOpen()


	// Init game variables
	state.ship_x = 100;
	state.ship_y = SCREEN_HEIGHT / 2;
	state.last_shot = 0;
}

// ----------------------------------------------------------------
void Finish()
{
	// L2: TODO 3: Close game controller


	// Deinitialize input events system
	//SDL_QuitSubSystem(SDL_INIT_EVENTS);

	// Deinitialize renderer and window
	// WARNING: Renderer should be deinitialized before window
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);

	// Deinitialize SDL internal global state
	SDL_Quit();

	// Free any game allocated memory
	free(state.keyboard);
}

// ----------------------------------------------------------------
bool CheckInput()
{
	// Update current mouse buttons state 
	// considering previous mouse buttons state
	for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i)
	{
		if (state.mouse_buttons[i] == KEY_DOWN) state.mouse_buttons[i] = KEY_REPEAT;
		if (state.mouse_buttons[i] == KEY_UP) state.mouse_buttons[i] = KEY_IDLE;
	}

	// Gather the state of all input devices
	// WARNING: It modifies global keyboard and mouse state but 
	// its precision may be not enough
	//SDL_PumpEvents();

	// Poll any currently pending events on the queue,
	// including 'special' events like window events, joysticks and 
	// even hotplug events for audio devices and joysticks,
	// you can't get those without inspecting event queue
	// SDL_PollEvent() is the favored way of receiving system events
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
			case SDL_QUIT: state.window_events[WE_QUIT] = true; break;
			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
					//case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_FOCUS_LOST: state.window_events[WE_HIDE] = true; break;
					//case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED: state.window_events[WE_SHOW] = true; break;
					case SDL_WINDOWEVENT_CLOSE: state.window_events[WE_QUIT] = true; break;
					default: break;
				}
			} break;
			// L2: TODO 4: Check mouse events for button state
			case SDL_MOUSEBUTTONDOWN: /*...*/ break;
			case SDL_MOUSEBUTTONUP:	/*...*/ break;
			case SDL_MOUSEMOTION:
			{
				state.mouse_x = event.motion.x;
				state.mouse_y = event.motion.y;
			} break;
			case SDL_JOYAXISMOTION:
			{
				// Motion on controller 0
				if (event.jaxis.which == 0)
				{
					// X axis motion
					if (event.jaxis.axis == 0)
					{
						if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) state.gamepad_axis_x_dir = -1;
						else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) state.gamepad_axis_x_dir = 1;
						else state.gamepad_axis_x_dir = 0;
					}
					// Y axis motion
					else if (event.jaxis.axis == 1)
					{
						if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) state.gamepad_axis_y_dir = -1;
						else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) state.gamepad_axis_y_dir = 1;
						else state.gamepad_axis_y_dir = 0;
					}
				}
			} break;
			default: break;
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	// L2: TODO 5: Update keyboard keys state
	// Consider previous keys states for KEY_DOWN and KEY_UP
	for (int i = 0; i < MAX_KEYBOARD_KEYS; ++i)
	{
		// A value of 1 means that the key is pressed
		// and a value of 0 means that it is not
	}

	// L2: TODO 6: Check ESCAPE key pressed to finish the game


	// Check QUIT window event to finish the game
	if (state.window_events[WE_QUIT] == true) return false;

	return true;
}

// ----------------------------------------------------------------
void MoveStuff()
{
	// L2: TODO 7: Move the ship with arrow keys


	// L2: TODO 8: Initialize a new shot when SPACE key is pressed


	// Update active shots
	for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
	{
		if (state.shots[i].alive)
		{
			if (state.shots[i].x < SCREEN_WIDTH) state.shots[i].x += SHOT_SPEED;
			else state.shots[i].alive = false;
		}
	}
}

// ----------------------------------------------------------------
void Draw()
{
	// Clear screen to Cornflower blue
	SDL_SetRenderDrawColor(state.renderer, 100, 149, 237, 255);
	SDL_RenderClear(state.renderer);

	// Draw ship rectangle
	DrawRectangle(state.ship_x, state.ship_y, 250, 100, { 255, 0, 0, 255 });

	// L2: TODO 9: Draw active shots


	// Finally present framebuffer
	SDL_RenderPresent(state.renderer);
}


// Main Entry point
// -------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	Start();

	while (CheckInput())
	{
		MoveStuff();

		Draw();
	}

	Finish();

	return(EXIT_SUCCESS);
}

// Functions Definition
// -------------------------------------------------------------------------
void DrawRectangle(int x, int y, int width, int height, SDL_Color color)
{
	SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, color.a);

	SDL_Rect rec = { x, y, width, height };

	int result = SDL_RenderFillRect(state.renderer, &rec);

	if (result != 0) printf("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
}

// ----------------------------------------------------------------
void DrawLine(int x1, int y1, int x2, int y2, SDL_Color color)
{
	SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, color.a);

	int result = SDL_RenderDrawLine(state.renderer, x1, y1, x2, y2);

	if (result != 0) printf("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
}

// ----------------------------------------------------------------
void DrawCircle(int x, int y, int radius, SDL_Color color)
{
	SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(state.renderer, color.r, color.g, color.b, color.a);

	SDL_Point points[360];
	float factor = (float)M_PI / 180.0f;

	for (int i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x + radius * cosf(factor * i));
		points[i].y = (int)(y + radius * sinf(factor * i));
	}

	int result = SDL_RenderDrawPoints(state.renderer, points, 360);

	if (result != 0) printf("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
}