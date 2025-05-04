#define SDL_MAIN_HANDLED // required by MinGW on Windows to prevent SDL2 from redefining main()
#include <SDL2/SDL.h>
#include <stdlib.h> // rand
#include <stdio.h>  // printf
#include "guilib.h"
#include "defs.h"
#include "elements.h"

#define WINDOW_WIDTH 		640
#define WINDOW_HEIGHT 		480


void InitElementList(void);
void load_progress_bar(GUI_ProgressBar *pb);

static Uint8 loading = 0;
static Uint32 time_now, last_update;

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow(
		"GUI Library Demo",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	GUI_Init(window, renderer); // initialize our library
	GUI_SetTheme(DARK_MODE); 	// all our library functions use the prefix 'GUI_'

	// function in elements.c where all elements are defined to minimize clutter (provided by the user, not library)
	InitElementList();

	int quit = 0;
	SDL_Event event;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) quit = 1;
			GUI_ProcessEvents(&event);
		}
		snprintf(buffer1, sizeof(buffer1), "Value: %.0f", slider1->value); 		  // display values of sliders
		snprintf(buffer2, sizeof(buffer2), "Value: %.1f", slider2->value);
		snprintf(buffer3, sizeof(buffer3), "Progress: %d%%", progressbar->value); // progress bar percentage

		// clear the screen
		SDL_SetRenderDrawColor(SDL_GetRenderer(window), bg_color.r, bg_color.g, bg_color.b, bg_color.a);
		SDL_RenderClear(SDL_GetRenderer(window));

		/* Render existing GUI elements, filter by tag */

		// GUI_RenderButton(button1); 		// render only button1
		// GUI_RenderElements("button"); 	// render all elements tagged "button"
		GUI_RenderElements(NULL); 			// render all elements, with and without a tag

		load_progress_bar(progressbar); 	// update progress bar's value (demo-specific function)

		SDL_RenderPresent(SDL_GetRenderer(window));

		SDL_Delay(16); // cap the framerate at approximately 60 FPS (alternative: toggle vsync)
	}
	GUI_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

void button1_click(void *args) {
	if (!args) return;
	const char *message = (const char *)args;
	printf("Message: %s\n", message);

	/* Optional demonstrations */
	
	// uncomment to delete the Disabled button
	// GUI_DeleteElement(button2);

	// uncomment to toggle the Disabled button's visibility
	// button2->visible = button2->visible ? HIDDEN : VISIBLE;
}

void SwitchMode() {
	static int mode = DARK_MODE; 			// initialize with initial/default mode

	mode = mode ? LIGHT_MODE : DARK_MODE; 	// toggle between dark and light mode
	button3->text = mode ? "Light mode" : "Dark mode";

	if (mode == LIGHT_MODE)
		bg_colors[0] = (SDL_Color){ BG_LIGHT };
	else
		bg_colors[0] = (SDL_Color){ BG_DARK };

	if (listbox->selected_id == 0)
		SetBackgroundColor(); 	// update the background color if bg_colors[0] is selected
	GUI_SetTheme(mode); 		// set color theme of elements
}

void SetBackgroundColor() {
	if (listbox->selected_id >= 0 && listbox->selected_id < COLORS_COUNT) {
		bg_color = bg_colors[listbox->selected_id];
	}
}

void InitiateLoading(void *args) {
	if (args) {
		GUI_ProgressBar *pb = (GUI_ProgressBar*)args;
		if (!loading) {
			pb->value = pb->min;
			pb->pos = pb->min;
			last_update = time_now;
			loading = 1;
			button4->text = "Loading...";
			button4->enabled = 0;
		}
	}
}

void load_progress_bar(GUI_ProgressBar *pb) {
	if (!loading) return;
	time_now = SDL_GetTicks(); 			// current time, for timed events

	if (time_now - last_update > 250) { // run every 250 milliseconds
		int inc = rand() % 10; 			// get a random number between 0-9
		if ((pb->value += inc) >= pb->max) {
			pb->value = pb->max;
			button4->text = "Start over!";
			button4->enabled = 1;
			loading = 0;
		}
		last_update = time_now;
	}
}