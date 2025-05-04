#include <SDL2/SDL.h>
#include <stdlib.h> // malloc
#include <stdio.h>  // printf
#include "guilib.h"
#include "defs.h"

#define BAR_HEIGHT 			22
#define BORDER_WIDTH 		1
#define SMOOTHING_SPEED 	0.1f // linear interpolation speed for drawing the filled portion


GUI_ProgressBar *GUI_CreateProgressBar(int x, int y, int width, int min, int max) {
	if (min >= max) {
		printf("\n[!] Invalid progress bar range: min (%d) must be less than max (%d). Aborted.\n", min, max);
		return NULL;
	}
	GUI_ProgressBar *pb = malloc(sizeof(GUI_ProgressBar));

	*pb = (GUI_ProgressBar){
		.tag = NULL,
		.x = x,
		.y = y,
		.width = width,
		.height = BAR_HEIGHT,
		.border_width = BORDER_WIDTH,
		.visible = VISIBLE,
		.min = min,
		.max = max,
		.value = 0,
		.pos = 0
	};

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_PROGRESSBAR, pb, (GUI_Render)GUI_RenderProgressBar, NULL);
	return pb;
}

void GUI_RenderProgressBar(GUI_ProgressBar *bar) {
	if (!bar || !bar->visible) return; // NULL pointer or hidden element

	SDL_Renderer *renderer = GUI_GetRenderer();

	__gui_draw_borders(bar->x, bar->y, bar->width, bar->height, bar->border_width);

	// render empty base bar
	SDL_SetRenderDrawColor(renderer, SET_COLOR_INPUT_ACTIVE);
	SDL_Rect bar_rect = { bar->x, bar->y, bar->width, bar->height };
	SDL_RenderFillRect(renderer, &bar_rect);

	// interpolate lerped value towards real value
	bar->pos += (bar->value - bar->pos) * SMOOTHING_SPEED;

	float ratio = (float)(bar->pos - bar->min) / (bar->max - bar->min);

	int filled_width = (int)(ratio * bar->width);
	if (bar->pos > (bar->max - 1)) filled_width = bar->width; // fix rounding errors (99.999... = 100)
	// if (bar->value == bar->max) filled_width = bar->width; // instantly snap to end when finished

	// render bar's filled portion
	SDL_SetRenderDrawColor(renderer, SET_COLOR_PROGRESS);
	SDL_Rect filled_rect = { bar->x, bar->y, filled_width, bar->height };
	SDL_RenderFillRect(renderer, &filled_rect);
}