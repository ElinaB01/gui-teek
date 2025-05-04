#include <SDL2/SDL.h>
#include <stdlib.h> // malloc
#include <stdio.h>  // printf
#include <math.h> 	// roundf
#include "guilib.h"
#include "defs.h"

#define TRACK_HEIGHT 		22
#define KNOB_WIDTH 			8
#define KNOB_HEIGHT 		22
#define BORDER_WIDTH 		1

// TODO:
// vertical sliders (GUI_CreateSliderH, GUI_CreateSliderV)

static void __gui_process_slider(SDL_Event *event, GUI_Slider *slider, int mx, int my);

// basic slider
GUI_Slider *GUI_CreateSlider(int x, int y, int width, float min, float max, float increment, float value) {
	if (min >= max) {
		printf("\n[!] Invalid slider range: min (%f) must be less than max (%f). Aborted.\n", min, max);
		return NULL;
	}
	if (value < min) value = min;
	if (value > max) value = max;

	GUI_Slider *s = malloc(sizeof(GUI_Slider));

	*s = (GUI_Slider){
		.tag = NULL,
		.x = x,
		.y = y,
		.width = width,
		.height = TRACK_HEIGHT,
		.knob_width = KNOB_WIDTH,
		.knob_height = KNOB_HEIGHT,
		.border_width = BORDER_WIDTH,
		.visible = VISIBLE,
		.pos_x = 0,
		.focus = 0,
		.dragging = 0,
		.smooth = SNAP,
		.min = min,
		.max = max,
		.increment = increment,
		.value = value
	};

	// snap the knob into place when the range of possible values is small
	int step_count = (max - min) / s->increment;

	// calculate knob position based on the value
	float position = (value - min) / (float)(max - min);
	s->pos_x = x + (position * (width - s->knob_width));

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_SLIDER, s, (GUI_Render)GUI_RenderSlider, (GUI_Process)__gui_process_slider);
	return s;
}

// extended slider: includes track height, knob proportions, and border width
GUI_Slider *GUI_CreateSliderEx(int x, int y, int width, int height, int knob_width, int knob_height, int border_width, float min, float max, float increment, float value, int smooth) {
	if (min >= max) {
		printf("Invalid slider range: min (%f) must be less than max (%f)\n", min, max);
		return NULL;
	}
	if (value < min) value = min;
	if (value > max) value = max;

	if (knob_width > width) knob_width = width;

	GUI_Slider *s = malloc(sizeof(GUI_Slider));

	*s = (GUI_Slider){
		.tag = NULL,
		.x = x,
		.y = y,
		.width = width,
		.height = height,
		.knob_width = knob_width,
		.knob_height = knob_height,
		.border_width = border_width,
		.visible = VISIBLE,
		.pos_x = 0,
		.focus = 0,
		.dragging = 0,
		.smooth = smooth,
		.min = min,
		.max = max,
		.increment = increment,
		.value = value
	};

	// snap the knob into place when the range of possible values is small
	int step_count = (max - min) / s->increment;

	// calculate knob position based on the value
	float position = (value - min) / (float)(max - min);
	s->pos_x = x + (position * (width - s->knob_width));

	__gui_add_element(GUI_SLIDER, s, (GUI_Render)GUI_RenderSlider, (GUI_Process)__gui_process_slider);
	return s;
}

void GUI_RenderSlider(GUI_Slider *slider) {
	if (!slider || !slider->visible) return; // in case a NULL pointer gets passed or slider is hidden

	SDL_Renderer *renderer = GUI_GetRenderer();

	// track borders
	__gui_draw_borders(slider->x, slider->y, slider->width, slider->height, slider->border_width);

	// track body
	SDL_Rect track_rect = { slider->x, slider->y, slider->width, slider->height };

	// apply track color
	SDL_Color color = slider->dragging ? current_theme->input_active : current_theme->input_inactive;
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	// render the track
	SDL_RenderFillRect(renderer, &track_rect);

	// adjust the knob's y position for if its height exceeds the track's height
	int knob_y = slider->y + (slider->height / 2) - (slider->knob_height / 2);

	// knob borders
	if (slider->border_width > 0)
		__gui_draw_borders(slider->pos_x, knob_y, slider->knob_width, slider->knob_height, slider->border_width);

	// knob body
	SDL_Rect knob_rect = { slider->pos_x, knob_y, slider->knob_width, slider->knob_height };

	// apply knob color
	if (slider->dragging)
		SDL_SetRenderDrawColor(renderer, SET_COLOR_ACTIVE);
	else if (slider->focus)
		SDL_SetRenderDrawColor(renderer, SET_COLOR_FOCUS);
	else
		SDL_SetRenderDrawColor(renderer, SET_COLOR_NORMAL);

	// render the knob
	SDL_RenderFillRect(renderer, &knob_rect);
}

static void __gui_update_slider(GUI_Slider *slider, int mx) {
	// center the knob's position on the cursor and clamp it within the track's boundaries
	mx -= (slider->knob_width / 2);
	mx = SDL_clamp(mx, slider->x, slider->x + slider->width - slider->knob_width);
	
	float new_value = 0.0f;

	// smooth slider does not snap into place at the next valid increment
	if (slider->smooth) {
		// 0.0 - 1.0; knob's relative position inside the track or distance percentage
		float ratio = (mx - slider->x) / (float)(slider->width - slider->knob_width);

		// calculate new value based on knob's relative position
		new_value = slider->min + roundf(ratio * ((slider->max - slider->min) / slider->increment)) * slider->increment;
		slider->pos_x = mx;
	} else {
		float ratio = (mx - slider->x) / (float)(slider->width - slider->knob_width);

		// calculate new value based on knob's relative position
		new_value = slider->min + roundf(ratio * ((slider->max - slider->min) / slider->increment)) * slider->increment;

		float position = (new_value - slider->min) / (slider->max - slider->min);
		slider->pos_x = slider->x + position * (slider->width - slider->knob_width);
	}
	slider->value = new_value;
}

void __gui_process_slider(SDL_Event *event, GUI_Slider *slider, int mx, int my) {
	if (!slider || !slider->visible) return; // NULL pointer or hidden element

	// check if mouse cursor is inside the slider area
	int intersect = (mx >= slider->x && mx <= slider->x + slider->width &&
					 my >= slider->y && my <= slider->y + slider->height);

	// highlight knob
	slider->focus = intersect;

	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT && intersect)
		slider->dragging = 1; // mark slider as in-use when valid area is clicked

	else if (event->type == SDL_MOUSEMOTION && slider->dragging)
		__gui_update_slider(slider, mx); // slider is in use and mouse cursor moves

	else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT)
		slider->dragging = 0; // slider no longer in use when left mouse button is released

	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT && intersect) {
		slider->pos_x = mx - (slider->knob_width / 2); 	// center knob on mouse cursor
		__gui_update_slider(slider, mx); 				// update value and knob position
	}
}