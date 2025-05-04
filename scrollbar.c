#include <SDL2/SDL2_gfxPrimitives.h>
#include "guilib.h"
#include "defs.h"

#define BUTTON_SIZE 	15

// TODO: draggable scrollbar knob
// horizontal scrollbars

void __gui_init_scrollbar(GUI_Scrollbar *sb, int x, int y, int height, int *scroll_offset, int visible_entries, int max_offset) {
	sb->x = x - BUTTON_SIZE;
	sb->y = y;
	sb->width = BUTTON_SIZE; 	// same width as up and down buttons (square buttons)
	sb->height = height; 		// height of the element the scrollbar is drawn on
	sb->button_height = BUTTON_SIZE;

	sb->up_button = (SDL_Rect){
		x - BUTTON_SIZE, y, BUTTON_SIZE, BUTTON_SIZE
	};
	sb->down_button = (SDL_Rect){
		x - BUTTON_SIZE, y + height - BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE
	};
	sb->track = (SDL_Rect){
		x - BUTTON_SIZE, y + BUTTON_SIZE, BUTTON_SIZE, height - (2 * BUTTON_SIZE)
	};

	sb->scroll_offset = scroll_offset;
	sb->visible_entries = visible_entries;
	sb->max_offset = max_offset;
}

void __gui_render_scrollbar_arrows(GUI_Scrollbar *sb) {
	SDL_Renderer *renderer = GUI_GetRenderer();

	Uint32 color_up, color_down; 	// arrow colors
	int cx, cy; 					// center point coordinates

	// up arrow
	if (sb->hovered_up)
		color_up = __gui_color_to_uint32(SET_COLOR_SCROLLBAR_BUTTON_FOCUS);
	else
		color_up = __gui_color_to_uint32(SET_COLOR_SCROLLBAR_BUTTON_NORMAL);

	cx = sb->up_button.x + sb->up_button.w / 2;
	cy = sb->up_button.y + sb->up_button.h / 2;
	aalineColor(renderer, cx - 4, cy + 2, cx, cy - 3, color_up); // SDL2_gfx
	aalineColor(renderer, cx + 4, cy + 2, cx, cy - 3, color_up);

	// down arrow
	if (sb->hovered_down)
		color_down = __gui_color_to_uint32(SET_COLOR_SCROLLBAR_BUTTON_FOCUS);
	else
		color_down = __gui_color_to_uint32(SET_COLOR_SCROLLBAR_BUTTON_NORMAL);

	cx = sb->down_button.x + sb->down_button.w / 2;
	cy = sb->down_button.y + sb->down_button.h / 2;

	aalineColor(renderer, cx - 4, cy - 2, cx, cy + 3, color_down);
	aalineColor(renderer, cx + 4, cy - 2, cx, cy + 3, color_down);
}

void __gui_render_scrollbar(GUI_Scrollbar *sb) {
	SDL_Renderer *renderer = GUI_GetRenderer();

	// track
	SDL_SetRenderDrawColor(renderer, SET_COLOR_SCROLLBAR_TRACK);
	SDL_RenderFillRect(renderer, &sb->track);

	// buttons' background rects
	SDL_RenderFillRect(renderer, &sb->up_button);
	SDL_RenderFillRect(renderer, &sb->down_button);

	// arrows on buttons
	__gui_render_scrollbar_arrows(sb);
}

int __gui_process_scrollbar(GUI_Scrollbar *sb, SDL_Event *event, int mx, int my, SDL_Rect content_area) {
	if (!sb || !sb->scroll_offset) return 0; // NULL pointer or missing offset

	SDL_Point mouse = { mx, my };

	sb->hovered_up = 0; // reset highlight
	sb->hovered_down = 0;

	// handle scrollbar button clicks
	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
		if (SDL_PointInRect(&mouse, &sb->up_button)) {
			if (*sb->scroll_offset > 0) {
				(*sb->scroll_offset)--;
				sb->hovered_up = 1;
			}
			return 1;
		}
		else if (SDL_PointInRect(&mouse, &sb->down_button)) {
			if (*sb->scroll_offset < sb->max_offset) {
				(*sb->scroll_offset)++;
				sb->hovered_down = 1;
			}
			return 1;
		}
		else if (SDL_PointInRect(&mouse, &sb->track)) {
			// TODO: track clicked; put dragging code here
			return 1;
		}
	}

	// mouse wheel support
	if (event->type == SDL_MOUSEWHEEL) {
		if (SDL_PointInRect(&mouse, &content_area)) {
			if (event->wheel.y > 0) { // scroll up
				if (*sb->scroll_offset > 0) (*sb->scroll_offset)--;
				return 1;
			} else if (event->wheel.y < 0) { // scroll down
				if (*sb->scroll_offset < sb->max_offset) (*sb->scroll_offset)++;
				return 1;
			}
		}
	}
	return 0;
}