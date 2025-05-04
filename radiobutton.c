#include <stdlib.h> // malloc
#include <SDL2/SDL2_gfxPrimitives.h>
#include "guilib.h"
#include "defs.h"

#define BUTTON_RADIUS 		9
#define BULLET_RADIUS 		3
#define BORDER_WIDTH 		1


static void __gui_process_radiobutton(SDL_Event *event, GUI_RadioButton *radiobutton, int mx, int my);

GUI_RadioButton *GUI_CreateRadioButton(int x, int y) {
	GUI_RadioButton *rb = malloc(sizeof(GUI_RadioButton));
    *rb = (GUI_RadioButton){
		.tag = NULL,
		.x = x,
		.y = y,
		.r = BUTTON_RADIUS,
		.bullet_r = BULLET_RADIUS,
		.border_width = BORDER_WIDTH,
		.enabled = ENABLED,
		.visible = VISIBLE,
		.focus = 0,
		.selected = OFF,
		NULL
	};

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_RADIOBUTTON, rb, (GUI_Render)GUI_RenderRadioButton, (GUI_Process)__gui_process_radiobutton);
	return rb;
}

GUI_RadioGroup *GUI_CreateRadioGroup() {
	// group radio buttons together to allow selecting only one
	GUI_RadioGroup *rg = malloc(sizeof(GUI_RadioGroup));
    *rg = (GUI_RadioGroup){ NULL, 0 };

	__gui_add_element(GUI_RADIOGROUP, rg, NULL, NULL);
	return rg;
}

void GUI_RenderRadioButton(GUI_RadioButton *radiobutton) {
	if (!radiobutton || !radiobutton->visible) return; // NULL pointer hidden element

	SDL_Renderer *renderer = GUI_GetRenderer();

	int center_x = radiobutton->x + radiobutton->r; 	// find center point for drawing
	int center_y = radiobutton->y + radiobutton->r - 2; // make button more or less aligned with checkboxes

	// render the radio button (SDL2_gfx)
	if (!radiobutton->enabled)
		filledCircleRGBA(renderer, center_x, center_y, radiobutton->r, SET_COLOR_DISABLED);
	else if (radiobutton->focus)
		filledCircleRGBA(renderer, center_x, center_y, radiobutton->r, SET_COLOR_FOCUS);
	else
		filledCircleRGBA(renderer, center_x, center_y, radiobutton->r, SET_COLOR_NORMAL);

	// due to a lack of anti-aliased filled circle, the borders must be drawn after the button
	if (radiobutton->border_width > 0)
		aacircleRGBA(renderer, center_x, center_y, radiobutton->r, SET_COLOR_BORDER); // SDL2_gfx

	// render the bullet (SDL2_gfx)
	if (radiobutton->selected) {
		filledCircleRGBA(renderer, center_x, center_y, radiobutton->bullet_r, SET_COLOR_TEXT_ENABLED);
		aacircleRGBA(renderer, center_x, center_y, radiobutton->bullet_r, SET_COLOR_TEXT_PLACEHOLDER);
	}
}

void GUI_AddToRadioGroup(GUI_RadioGroup *group, GUI_RadioButton *radiobutton) {
	group->buttons = realloc(group->buttons, sizeof(GUI_RadioButton*) * (group->button_count + 1));
	group->buttons[group->button_count++] = radiobutton;
	radiobutton->group = group;
}

static void __gui_process_radiobutton(SDL_Event *event, GUI_RadioButton *radiobutton, int mx, int my) {
	if (!radiobutton || !radiobutton->enabled || !radiobutton->visible) return; // NULL pointer, disabled or hidden element

	// check if mouse cursor is inside the button
	int intersect = (mx >= radiobutton->x && mx <= radiobutton->x + (radiobutton->r * 2) &&
					 my >= radiobutton->y && my <= radiobutton->y + (radiobutton->r * 2));
	// highlight radio button
	radiobutton->focus = intersect;

	if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT && intersect) {
		radiobutton->selected = 1;
		if (!radiobutton->group) return;

		// deselect the other button
		for (int i = 0; i < radiobutton->group->button_count; ++i) {
			GUI_RadioButton *rb = radiobutton->group->buttons[i];
			rb->selected = (rb == radiobutton) ? 1 : 0;
		}
	}
}