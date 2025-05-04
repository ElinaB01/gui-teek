#include <stdlib.h> // malloc
#include <stdio.h>  // printf
#include <string.h> // strncpy, strlen
#include "guilib.h"
#include "defs.h"

#define BUTTON_WIDTH 		100
#define BUTTON_HEIGHT 		22
#define BORDER_WIDTH 		1
#define TEXT_SIZE 			12

// TODO:
// custom font support
// rounded corners - https://sdl-draw.sourceforge.net/

static void __gui_process_button(SDL_Event *event, GUI_Button *button, int mx, int my);

GUI_Button *GUI_CreateButton(int x, int y, const char *text, void (*on_click)(void*)) {
	GUI_Button *b = malloc(sizeof(GUI_Button));

	*b = (GUI_Button){
		.tag = NULL,
		.x = x,
		.y = y,
		.width = BUTTON_WIDTH,
		.height = BUTTON_HEIGHT,
		.border_width = BORDER_WIDTH,
		.enabled = ENABLED,
		.visible = VISIBLE,
		.hovered = 0,
		.pressed = 0,
		.text_size = TEXT_SIZE,
		.text = text,
		.on_click = on_click,
		.args = NULL
	};

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_BUTTON, b, (GUI_Render)GUI_RenderButton, (GUI_Process)__gui_process_button);
	return b;
}

void GUI_RenderButton(GUI_Button *button) {
	if (!button || !button->visible) return; // in case a NULL pointer gets passed or buttin is hidden

	SDL_Renderer *renderer = GUI_GetRenderer();

	// button body
	SDL_Rect button_rect = { button->x, button->y, button->width, button->height };

	__gui_draw_borders(button->x, button->y, button->width, button->height, button->border_width);

	// set button color
	if (!button->enabled)
		SDL_SetRenderDrawColor(renderer, SET_COLOR_DISABLED);
	else if (button->pressed)
		SDL_SetRenderDrawColor(renderer, SET_COLOR_ACTIVE);
	else if (button->hovered)
		SDL_SetRenderDrawColor(renderer, SET_COLOR_FOCUS);
	else
		SDL_SetRenderDrawColor(renderer, SET_COLOR_NORMAL);

	// render the button
	SDL_RenderFillRect(renderer, &button_rect);

	// render button text
	if (button->text) {
		// set text color
		SDL_Color text_color = button->enabled ? current_theme->text_enabled : current_theme->text_disabled;

		// if text is too long, truncate it
		char truncated_text[32];
		strncpy(truncated_text, button->text, sizeof(truncated_text) - 1);
		truncated_text[sizeof(truncated_text) - 1] = '\0'; // include null-terminator

		int text_width, text_height;
		TTF_SizeText(default_font, truncated_text, &text_width, &text_height);

		// if the text is too tall, don't render it at all
		if (text_height > button->height) return;

		// keep removing one character from the end untl the text fits inside the button
		while (text_width > button->width && truncated_text[0]) {
			truncated_text[strlen(truncated_text) - 1] = '\0';
			TTF_SizeText(default_font, truncated_text, &text_width, &text_height);
		}
	
		// render text
		if (text_width > 0) {
			SDL_Rect text_rect = {
				button->x + (button->width - text_width) / 2,  // center text within the button
				button->y + (button->height - text_height) / 2,
				text_width, text_height
			};
			__gui_render_text(truncated_text, &text_rect, text_color);
		}
	}
}

static void __gui_process_button(SDL_Event *event, GUI_Button *button, int mx, int my) {
	if (!button || !button->enabled || !button->visible) return; // NULL pointer, disabled or hidden element

	// check if mouse cursor is inside the button
	int intersect = (mx >= button->x && mx <= button->x + button->width &&
					 my >= button->y && my <= button->y + button->height);

	// highlight button
	button->hovered = intersect;

	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT && intersect)
		button->pressed = 1; // button appears pressed visually, but does not execute yet

	if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT && button->pressed) {
		if (intersect) {
			// execute the on_click callback function, if one is assigned
			if (button->on_click) button->on_click(button->args);
		}
		button->pressed = 0;
	}
}