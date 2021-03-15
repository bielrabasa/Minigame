// -------------------------------------------------------------------------
// Awesome simple game with SDL
// Lesson 2 - Input Events
//
// SDL API: http://wiki.libsdl.org/APIByCategory
// -------------------------------------------------------------------------

#include <stdio.h>			// Required for: printf()
#include <stdlib.h>			// Required for: EXIT_SUCCESS
#include <math.h>			// Required for: sinf(), cosf()
#include <time.h>

// Include SDL libraries
#include "SDL/include/SDL.h"				// Required for SDL base systems functionality
#include "SDL_image/include/SDL_image.h"	// Required for image loading functionality
#include "SDL_mixer/include/SDL_mixer.h"	// Required for audio loading and playing functionality

// Define libraries required by linker
// WARNING: Not all compilers support this option and it couples 
// source code with build system, it's recommended to keep both 
// separated, in case of multiple build configurations
//#pragma comment(lib, "SDL/lib/x86/SDL2.lib")
//#pragma comment(lib, "SDL/lib/x86/SDL2main.lib")
//#pragma comment(lib, "SDL_image/lib/x86/SDL2_image.lib")
//#pragma comment( lib, "SDL_mixer/libx86/SDL2_mixer.lib" )

// -------------------------------------------------------------------------
// Defines, Types and Globals
// -------------------------------------------------------------------------
#define SCREEN_WIDTH		 720
#define SCREEN_HEIGHT		 1024 //vertical

#define MAX_KEYBOARD_KEYS	 256
#define MAX_MOUSE_BUTTONS	   5
#define JOYSTICK_DEAD_ZONE  8000

#define SHIP_SPEED			   10
#define MAX_SHIP_SHOTS		   32
#define SHOT_SPEED			   8
#define SCROLL_SPEED		   8

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

enum GameScreen
{
	LOGO = 0,
	TITLE,
	GAMEPLAY,
	ENDING
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

	// Texture variables
	SDL_Texture* background;
	SDL_Texture* logo;
	SDL_Texture* title;
	SDL_Texture* ending;
	SDL_Texture* ship;
	SDL_Texture* shot;//shot -> box
	SDL_Texture* pacage;
	SDL_Texture* num0;
	int background_height;

	// Audio variables
	Mix_Music* music;
	Mix_Chunk* fx_shoot;

	// Game elements
	int ship_x;
	int ship_y;
	Projectile shots[MAX_SHIP_SHOTS];
	int last_shot;
	Projectile box[MAX_SHIP_SHOTS];
	int last_box;
	int scroll;

	GameScreen currentScreen;

	int cont = 0;//Contador
	int num = 0;
	int num2 = 0;
	int randomx;
	int randpmS;
	int randoma;
	int randome;
	int timeNum2 = 50;//V respawn caixes
	int timeNum = 50; //mes menys v de aparició
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
	state.window = SDL_CreateWindow("Delivery Minigame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	state.surface = SDL_GetWindowSurface(state.window);

	// Init renderer
	state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawColor(state.renderer, 100, 149, 237, 255);		// Default clear color: Cornflower blue

	// L2: DONE 1: Init input variables (keyboard, mouse_buttons)
	state.keyboard = (KeyState*)calloc(sizeof(KeyState) * MAX_KEYBOARD_KEYS, 1);
	for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) state.mouse_buttons[i] = KEY_IDLE;

	// L2: DONE 2: Init input gamepad 
	// Check SDL_NumJoysticks() and SDL_JoystickOpen()
	if (SDL_NumJoysticks() < 1) printf("WARNING: No joysticks connected!\n");
	else
	{
		state.gamepad = SDL_JoystickOpen(0);
		if (state.gamepad == NULL) printf("WARNING: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
	}

	// Init image system and load textures
	//INICIALITZACIÓ
	IMG_Init(IMG_INIT_PNG);
	state.background = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/fondofinal.png"));//fons
	state.logo = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/pantalla1.png")); //pantalla logo
	state.title = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/pantalla2.png")); //pantalla titol
	state.ending = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/Pantalla_Muerte.png")); //pantallafinal
	state.ship = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/amazon_trackv5.png"));//furgo
	state.shot = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/Sprite_Agujerov2.png"));//escotilla
	state.pacage = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/Sprite_Caixa2.png"));//Caja
	{
		state.num0 = SDL_CreateTextureFromSurface(state.renderer, IMG_Load("Assets/num0.png"));//Caja






	}
	SDL_QueryTexture(state.background, NULL, NULL, NULL, &state.background_height);//CANVI W H

	// L4: TODO 1: Init audio system and load music/fx
	// EXTRA: Handle the case the sound can not be loaded!
	int Mix_Init(MIX_INIT_OGG); //inicialitzar musica
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024); //obrir audio
	state.music = Mix_LoadMUS("Assets/halo.ogg"); //carregar la musica, a la variable //MUSICA
	state.fx_shoot = Mix_LoadWAV("Assets/salchichon.wav"); //carregar l'audio, a la variable

	// L4: TODO 2: Start playing loaded music
	Mix_PlayMusic(state.music, -1); //utilitzar la musica, de la variable state.music

	// Init game variables //POSICIONS INICIALS
	state.ship_x = SCREEN_WIDTH / 2 - 128;
	state.ship_y = 750; //posicio furgo
	//state.last_shot = 0;
	state.scroll = 0;
	state.currentScreen = LOGO;
}

// ----------------------------------------------------------------
void Finish()
{
	// L4: TODO 3: Unload music/fx and deinitialize audio system
	Mix_FreeMusic(state.music); //tancar la musica, la variable //TANCAR MUSICA
	Mix_FreeChunk(state.fx_shoot); //tancar els sons, variable
	Mix_CloseAudio(); //tancar audio general
	Mix_Quit(); //tancar la llibreria

	// Unload textures and deinitialize image system
	SDL_DestroyTexture(state.background);
	SDL_DestroyTexture(state.ship);
	SDL_DestroyTexture(state.shot);
	SDL_DestroyTexture(state.logo);
	SDL_DestroyTexture(state.title);
	SDL_DestroyTexture(state.ending);
	SDL_DestroyTexture(state.pacage);
	IMG_Quit();

	// L2: DONE 3: Close game controller
	SDL_JoystickClose(state.gamepad);
	state.gamepad = NULL;

	// Deinitialize input events system
	//SDL_QuitSubSystem(SDL_INIT_EVENTS);

	// Deinitialize renderer and window
	// WARNING: Renderer should be deinitialized before window
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);

	// Deinitialize SDL internal global state
	SDL_Quit();

	// Free any game allocated memory
	free(state.keyboard); //free el malloc
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
			// L2: DONE 4: Check mouse events for button state
			case SDL_MOUSEBUTTONDOWN: state.mouse_buttons[event.button.button - 1] = KEY_DOWN; break;
			case SDL_MOUSEBUTTONUP:	state.mouse_buttons[event.button.button - 1] = KEY_UP; break;
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

	// L2: DONE 5: Update keyboard keys state
    // Consider previous keys states for KEY_DOWN and KEY_UP
	for (int i = 0; i < MAX_KEYBOARD_KEYS; ++i)
	{
		// A value of 1 means that the key is pressed and a value of 0 means that it is not
		if (keys[i] == 1)
		{
			if (state.keyboard[i] == KEY_IDLE) state.keyboard[i] = KEY_DOWN;
			else state.keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (state.keyboard[i] == KEY_REPEAT || state.keyboard[i] == KEY_DOWN) state.keyboard[i] = KEY_UP;
			else state.keyboard[i] = KEY_IDLE;
		}
	}

	// L2: DONE 6: Check ESCAPE key pressed to finish the game
	if (state.keyboard[SDL_SCANCODE_ESCAPE] == KEY_DOWN) return false;

	// Check QUIT window event to finish the game
	if (state.window_events[WE_QUIT] == true) return false;

	return true;
}

// ----------------------------------------------------------------
void MoveStuff()
{
	//separador
	switch (state.currentScreen)
	{
	case LOGO:
	{
		if (state.keyboard[SDL_SCANCODE_RETURN] == KEY_DOWN) state.currentScreen = TITLE;
	} break;
	case TITLE:
	{
		if (state.keyboard[SDL_SCANCODE_RETURN] == KEY_DOWN) state.currentScreen = GAMEPLAY;
	} break;
	case GAMEPLAY: //er juego
	{
		if ((state.keyboard[SDL_SCANCODE_LEFT] == KEY_REPEAT) && (state.ship_x > -5)) state.ship_x -= SHIP_SPEED; //Límits laterals
		else if ((state.keyboard[SDL_SCANCODE_RIGHT] == KEY_REPEAT) && (state.ship_x < 470)) state.ship_x += SHIP_SPEED;

		if (state.num == 0)
		{
			state.randomx = (rand() % 525 + 65);//65 minima 590 maxima, 590-65 = 525
			state.randpmS = (rand() % 51 + state.timeNum);

			if (state.last_shot == MAX_SHIP_SHOTS) state.last_shot = 0;

			state.shots[state.last_shot].alive = true;
			state.shots[state.last_shot].x = state.randomx;
			state.shots[state.last_shot].y = -20;
			state.last_shot++;

			state.num = state.randpmS;

			if (state.timeNum > 20) state.timeNum--;
		}

		if (state.num > 0)
		{
			state.num--;
		}
		// Update active shots
		for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
		{
			if (state.shots[i].alive)
			{
				if (state.shots[i].y < SCREEN_HEIGHT) state.shots[i].y += SHOT_SPEED;
				else { state.shots[i].alive = false; }
			}
		}

		//colisions
		for (int i = 0; i < MAX_SHIP_SHOTS; ++i) {
			if (((state.shots[i].x > state.ship_x + 20) && (state.shots[i].x < state.ship_x + 160)) && ((state.shots[i].y > state.ship_y - 15) && (state.shots[i].y < state.ship_y + 180))) {
				//PERFECTES = 20 160 -15 180
				SDL_Delay(500);
				state.currentScreen = ENDING;
			}
		}


		//Test paquetes + puntuaciones

		if (state.num2 == 0)
		{
			state.randoma = (rand() % 525 + 65);//65 minima 590 maxima, 590-65 = 525
			state.randome = (rand() % 51 + state.timeNum2);

			if (state.last_box == MAX_SHIP_SHOTS) state.last_box = 0;

			state.box[state.last_box].alive = true;
			state.box[state.last_box].x = state.randomx;
			state.box[state.last_box].y = -20;
			state.last_box++;

			state.num2 = state.randome;

			if (state.timeNum2 > 20) state.timeNum2--;
		}

		if (state.num2 > 0)
		{
			state.num2--;
		}
		// Update active shots
		for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
		{
			if (state.box[i].alive)
			{
				if (state.box[i].y < SCREEN_HEIGHT) state.box[i].y += SHOT_SPEED;
				else { state.box[i].alive = false; }
			}
		}

		//colisions
		for (int i = 0; i < MAX_SHIP_SHOTS; ++i) {
			if (((state.box[i].x > state.ship_x + 20) && (state.box[i].x < state.ship_x + 160)) && ((state.box[i].y > state.ship_y - 15) && (state.box[i].y < state.ship_y + 180))) {
				//PERFECTES = 20 160 -15 180
				//SDL_Delay(500);
				state.cont++;
			}
		}

	} break;
	case ENDING:
	{

		if (state.keyboard[SDL_SCANCODE_RETURN] == KEY_DOWN) state.window_events[WE_QUIT] = true;
	} break;
	default: break;
	}
}

// ----------------------------------------------------------------
void Draw()
{
	// Clear screen to Cornflower blue
	SDL_SetRenderDrawColor(state.renderer, 100, 149, 237, 255);
	SDL_RenderClear(state.renderer);

	switch (state.currentScreen)
	{
	case LOGO:
	{
		SDL_Rect rec = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderCopy(state.renderer, state.logo, NULL, &rec);
	} break;
	case TITLE:
	{
		SDL_Rect rec = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderCopy(state.renderer, state.title, NULL, &rec);
	} break;
	case GAMEPLAY:
	{
		// Draw background and scroll
		state.scroll += SCROLL_SPEED;
		if (state.scroll >= 0)	state.scroll = -SCREEN_HEIGHT; //SÓC MOLT FELIÇ

		// Draw background texture (two times for scrolling effect)
		// NOTE: rec rectangle is being reused for next draws
		SDL_Rect rec = { 0, state.scroll, SCREEN_WIDTH, state.background_height }; //state.background_width
		SDL_RenderCopy(state.renderer, state.background, NULL, &rec);
		rec.y += state.background_height;
		SDL_RenderCopy(state.renderer, state.background, NULL, &rec);

		// Draw ship rectangle
		//DrawRectangle(state.ship_x, state.ship_y, 250, 100, { 255, 0, 0, 255 });

		// L2: DONE 9: Draw active shots
		rec.w = 64; rec.h = 64;
		for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
		{
			if (state.shots[i].alive)
			{
				//DrawRectangle(state.shots[i].x, state.shots[i].y, 50, 20, { 0, 250, 0, 255 });
				rec.x = state.shots[i].x; rec.y = state.shots[i].y;
				SDL_RenderCopy(state.renderer, state.shot, NULL, &rec);
			}
		}

		//Draw box

		rec.w = 64; rec.h = 64;
		for (int i = 0; i < MAX_SHIP_SHOTS; ++i)
		{
			if (state.box[i].alive)
			{
				//DrawRectangle(state.shots[i].x, state.shots[i].y, 50, 20, { 0, 250, 0, 255 });
				rec.x = state.box[i].x; rec.y = state.box[i].y;
				SDL_RenderCopy(state.renderer, state.pacage, NULL, &rec);
			}

			// Draw ship texture
			rec.x = state.ship_x; rec.y = state.ship_y; rec.w = 256; rec.h = 256; //mides sprite
			SDL_RenderCopy(state.renderer, state.ship, NULL, &rec);

			//Puntuacion
			if (state.box[i].alive) {
				if (state.cont == 1) {
					rec.w = 64; rec.h = 64;
					SDL_RenderCopy(state.renderer, state.num0, NULL, &rec);
				}
			}
		}
		// Clear screen to Cornflower blue
	} break;
	case ENDING:
	{
		SDL_Rect rec = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderCopy(state.renderer, state.ending, NULL, &rec);
		state.cont = 0;

	} break;
	default: break;
	}

	// Finally present framebuffer
	SDL_RenderPresent(state.renderer);
}


// Main Entry point
// -------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	srand(time(NULL));
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