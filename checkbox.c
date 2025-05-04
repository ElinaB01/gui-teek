#include <stdlib.h> // malloc
#include <SDL2/SDL2_gfxPrimitives.h>
#include "guilib.h"
#include "defs.h"

#define BOX_WIDTH 			16
#define BOX_HEIGHT 			16
#define BORDER_WIDTH 		1


static void __gui_process_checkbox(SDL_Event *event, GUI_Checkbox *checkbox, int mx, int my);

GUI_Checkbox *GUI_CreateCheckbox(int x, int y) {
	GUI_Checkbox *c = malloc(sizeof(GUI_Checkbox));

    *c = (GUI_Checkbox){
		.tag = NULL,
		.x = x,
		.y = y,
		.width = BOX_WIDTH,
		.height = BOX_HEIGHT,
		.border_width = BORDER_WIDTH,
		.enabled = ENABLED,
		.visible = VISIBLE,
		.focus = 0,
		.selected = OFF
	};

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_CHECKBOX, c, (GUI_Render)GUI_RenderCheckbox, (GUI_Process)__gui_process_checkbox);
	return c;
}

static void __gui_render_checkmark(GUI_Checkbox *checkbox) {
	if (!checkbox) return;

	SDL_Renderer *renderer = GUI_GetRenderer();

	int x = checkbox->x;
	int y = checkbox->y;

	// coordinates of the two lines that form the checkmark (TODO/temp: switch to center points)
	int x1 = x + 3;
	int y1 = y + 9;
	int x2 = x + 7;
	int y2 = y + 12;
	int x3 = x + 12;
	int y3 = y + 4;
	
	aalineRGBA(renderer, x1, y1, x2, y2, SET_COLOR_TEXT_ENABLED); // (SDL2_gfx)
	aalineRGBA(renderer, x2, y2, x3, y3, SET_COLOR_TEXT_ENABLED);
}

void GUI_RenderCheckbox(GUI_Checkbox *checkbox) {
	if (!checkbox || !checkbox->visible) return; // NULL pointer or hidden element

	SDL_Renderer *renderer = GUI_GetRenderer();

	// checkbox body
	SDL_Rect checkbox_rect = { checkbox->x, checkbox->y, checkbox->width, checkbox->height };

	__gui_draw_borders(checkbox->x, checkbox->y, checkbox->width, checkbox->height, checkbox->border_width);

	// apply color
	if (!checkbox->enabled)
		SDL_SetRenderDrawColor(renderer, SET_COLOR_DISABLED);
	else if (checkbox->focus)
		SDL_SetRenderDrawColor(renderer, SET_COLOR_FOCUS);
	else
		SDL_SetRenderDrawColor(renderer, SET_COLOR_NORMAL);

	// render the checkbox
	SDL_RenderFillRect(renderer, &checkbox_rect);

	// render the checkmark
	if (checkbox->selected)
		__gui_render_checkmark(checkbox);
}

static void __gui_process_checkbox(SDL_Event *event, GUI_Checkbox *checkbox, int mx, int my) {
	if (!checkbox || !checkbox->enabled || !checkbox->visible) return; // NULL pointer, disabled or hidden element

	// check if mouse cursor is inside the button
	int intersect = (mx >= checkbox->x && mx <= checkbox->x + checkbox->width &&
					 my >= checkbox->y && my <= checkbox->y + checkbox->height);
	// highlight checkbox
	checkbox->focus = intersect;

	if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT && intersect)
		checkbox->selected = !checkbox->selected;
}