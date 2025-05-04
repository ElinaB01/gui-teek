#include <stdlib.h> // malloc
#include <SDL2/SDL2_gfxPrimitives.h>
#include "guilib.h"
#include "defs.h"

#define BOX_WIDTH 			185
#define ENTRY_HEIGHT 		22
#define BORDER_WIDTH 		1
#define MAX_VISIBLE 		4
#define COLLAPSED 			0
#define EXPANDED 			1

// TODO:
// multi-select support
// toggle scrollbar visibility

static void __gui_process_listbox(SDL_Event *event, GUI_ListBox *listbox, int mx, int my);

GUI_ListBox *GUI_CreateListBox(int x, int y, const char *placeholder, void (*on_select)(void*)) {
	GUI_ListBox *lb = malloc(sizeof(GUI_ListBox));

	*lb = (GUI_ListBox){
		.tag = NULL,
		.placeholder = placeholder,
		.x = x,
		.y = y,
		.width = BOX_WIDTH,
		.height = ENTRY_HEIGHT,
		.border_width = BORDER_WIDTH,
		.entry_height = ENTRY_HEIGHT,
		.visible = VISIBLE,
		.selected_id = -1,
		.max_visible = MAX_VISIBLE,
		.expanded = COLLAPSED,
		.entries = NULL,
		.selected_entry = NULL,
		.highlighted_entry = NULL,
		.scrollbar = {0},
		.on_select = on_select,
		.args = NULL
	};

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_LISTBOX, lb, (GUI_Render)GUI_RenderListBox, (GUI_Process)__gui_process_listbox);

	int max_offset = lb->entry_count - lb->max_visible;
	if (max_offset < 0) max_offset = 0;

	__gui_init_scrollbar(
		&lb->scrollbar,
		lb->x + lb->width,
		lb->y + lb->entry_height, // position at the first entry rect when expanded
		lb->entry_height * lb->max_visible,
		&lb->scroll_offset,
		lb->max_visible,
		max_offset
	);
	return lb;
}

// add one entry at a time; allows to add new entries dynamically
void GUI_AddListEntry(GUI_ListBox *lb, const char *text) {
	if (!lb) return;

	lb->entries = realloc(lb->entries, sizeof(GUI_ListEntry) * (lb->entry_count + 1));
	lb->entries[lb->entry_count++] = (GUI_ListEntry){ text };
}

// define all entries at once; overwrites old data
void GUI_SetListEntries(GUI_ListBox *lb, const char **texts, int count) {
	if (!lb) return; // NULL pointer

	free(lb->entries);
	lb->entries = malloc(sizeof(GUI_ListEntry) * count);
	for (int i = 0; i < count; i++)
		lb->entries[i] = (GUI_ListEntry){ texts[i] };

	lb->entry_count = count;

	int max_offset = lb->entry_count - lb->max_visible;
	if (max_offset < 0) max_offset = 0;
	lb->scrollbar.max_offset = max_offset;
}

// runtime entry selection (allows automatic selection with no user input, or default assignment)
void GUI_SelectListEntry(GUI_ListBox *lb, int index) {
	if (!lb || index < 0) return;
	if (index > lb->entry_count) index = lb->entry_count;

	lb->selected_entry = &lb->entries[index];
	lb->selected_id = index;
}

static void __gui_render_display_arrow(GUI_ListBox *lb) {
	SDL_Renderer *renderer = GUI_GetRenderer();

	Uint32 arrow_color = __gui_color_to_uint32(SET_COLOR_SCROLLBAR_BUTTON_NORMAL); // convert SDL_Color to Uint32
	int cx, cy; // center point coordinates

	cx = lb->x + lb->width - 10;
	cy = lb->y + lb->entry_height / 2;

	if (!lb->expanded) { 	// collapsed list: arrow points right
		aalineColor(renderer, cx - 4, cy - 2, cx, cy + 3, arrow_color); // SDL2_gfx
		aalineColor(renderer, cx + 4, cy - 2, cx, cy + 3, arrow_color);
	} else { 				// expanded list: arrow points down
		aalineColor(renderer, cx - 1, cy - 4, cx + 4, cy, arrow_color);
		aalineColor(renderer, cx - 1, cy + 4, cx + 4, cy, arrow_color);
	}
}

void GUI_RenderListBox(GUI_ListBox *listbox) {
	if (!listbox || !listbox->visible) return; // NULL pointer, disabled or hidden element

	SDL_Renderer *renderer = GUI_GetRenderer();
	
	int rect_x = listbox->x;
	int rect_y = listbox->y;
	int rect_w = listbox->width;
	int rect_h = listbox->entry_height;

	const char *display_text = listbox->selected_entry ? listbox->selected_entry->text : listbox->placeholder;

	SDL_Color text_color = current_theme->text_enabled;

	// how many entries are visible (all if the list is short, or cap to max_visible)
	int visible_entries = (listbox->entry_count < listbox->max_visible)
						 ? listbox->entry_count : listbox->max_visible;
	// calculate height of border rect based on whether or not the list has been expanded
	int border_height = listbox->expanded ? rect_h * (visible_entries + 1) : rect_h;

	__gui_draw_borders(rect_x, rect_y, rect_w, border_height, listbox->border_width);

	// main rect; always visible and displays current selection or placeholder text
	SDL_Rect display_rect = { rect_x, rect_y, rect_w, rect_h };
	SDL_SetRenderDrawColor(renderer, SET_COLOR_NORMAL);
	SDL_RenderFillRect(renderer, &display_rect);
	
	// selected entry or placeholder text
	SDL_Rect text_rect = { rect_x + 4, rect_y, rect_w - 4, rect_h };
	__gui_render_text(display_text, &text_rect, text_color);
	
	// expand/collapse arrow
	__gui_render_display_arrow(listbox);

	if (!listbox->expanded) return;

	// render expanded entries
	int start = listbox->scroll_offset;
	int end = start + listbox->max_visible;
	if (end > listbox->entry_count) end = listbox->entry_count;

	for (int i = start; i < end; i++) {
		GUI_ListEntry *entry = &listbox->entries[i];

		int entry_y = rect_y + (i - start + 1) * rect_h;
		SDL_Rect entry_rect = { rect_x, entry_y, rect_w, rect_h };

		// set color
		if (entry == listbox->highlighted_entry)
			SDL_SetRenderDrawColor(renderer, SET_COLOR_ENTRY_SELECTED);
		else
			SDL_SetRenderDrawColor(renderer, SET_COLOR_ENTRY_NORMAL);

		SDL_RenderFillRect(renderer, &entry_rect);

		// entry text
		if (entry->text && *entry->text) {
			SDL_Rect text_rect = { entry_rect.x + 4, entry_rect.y, rect_w - 4, rect_h };
			__gui_render_text(entry->text, &text_rect, text_color);
		}
	}
	// render scrollbar
	if (listbox->expanded && listbox->entry_count > listbox->max_visible)
		__gui_render_scrollbar(&listbox->scrollbar);
}

static void __gui_process_listbox(SDL_Event *event, GUI_ListBox *listbox, int mx, int my) {
	if (!listbox || !listbox->visible) return; // NULL pointer, disabled or hidden element
	
	int rect_x = listbox->x;
	int rect_y = listbox->y;
	int rect_w = listbox->width;
	int rect_h = listbox->entry_height;

	SDL_Point mouse = { mx, my };

	int intersect_list = (mx >= rect_x && mx <= rect_x + rect_w &&
						  my >= rect_y && my <= rect_y + rect_h);

	// if mouse is clicked, expand or collapse the list based on mouse position
	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
		if (intersect_list) {
			listbox->expanded = EXPANDED;
			listbox->highlighted_entry = listbox->selected_entry; // highlight previously selected entry
			return;
		}
	}

	// skip the rest if list is collapsed
	if (!listbox->expanded) return;

	// entire expanded list area
	SDL_Rect content_area = {
		listbox->x,
		listbox->y + listbox->entry_height, // skip the display box
		listbox->width,
		listbox->entry_height * listbox->max_visible
	};

	// process scrollbar and skip entry processing if the scrollbar has been clicked
	if (__gui_process_scrollbar(&listbox->scrollbar, event, mx, my, content_area)) return;

	int start = listbox->scroll_offset;
	int end = start + listbox->max_visible;

	if (end > listbox->entry_count)
		end = listbox->entry_count;
	
	int in_scrollbar =  SDL_PointInRect(&mouse, &listbox->scrollbar.up_button)  ||
						SDL_PointInRect(&mouse, &listbox->scrollbar.down_button);

	// loop through entries
	for (int i = start; i < end; i++) {
		GUI_ListEntry *entry = &listbox->entries[i];
		int entry_y = rect_y + (i - start + 1) * rect_h;
		SDL_Rect entry_rect = { rect_x, entry_y, rect_w, rect_h };

		int intersect = SDL_PointInRect(&mouse, &entry_rect);

		// highlight hovered entry
		if (intersect && !in_scrollbar)
			listbox->highlighted_entry = entry;

		// single entry select: on click, overwrite the selected entry value
		if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT && intersect) {
			if (listbox->selected_id == i) {
				listbox->expanded = COLLAPSED;
				break;
			}
			listbox->selected_id = i;
			listbox->selected_entry = entry;
			listbox->highlighted_entry = entry;
			listbox->expanded = COLLAPSED;

			if (listbox->on_select)
				listbox->on_select(listbox->args); // execute optional callback function

			break;
		}
		// when clicked outside, collapse the list
		else if (event->type == SDL_MOUSEBUTTONDOWN && !intersect)
			listbox->expanded = COLLAPSED;
	}
}