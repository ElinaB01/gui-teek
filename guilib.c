#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h> // free
#include <stdio.h>  // printf
#include <string.h> // free
#include "guilib.h"
#include "defs.h"

#define MAX_ELEMENTS 		250

// TODO:
// add more error messages on failed element creation
// custom color support for individual elements
// cache TTF_RenderUTF8_Blended and SDL_CreateTextureFromSurface operations
// GetFont() instead of global variable
// macro 'HIDDEN' to hide library-specific symbols in GCC builds (by default, all symbols are exported in GCC)
// move library-specific symbols to a separate header file
// replace SDL_ttf: https://github.com/grimfang4/SDL_FontCache
// resizable elements
// merge the two text rendering functions and automatically truncate and clip text where needed
// replace checkmark and arrow icons (lines) with custom bitmaps (SDL_LockTexture() to draw pixels to a texture)

TTF_Font *default_font = NULL;
GUI_Theme *current_theme = NULL;

static SDL_Window *GUI_Window = NULL;
static SDL_Renderer *GUI_Renderer = NULL;

static int gui_initialized = 0;

static GUI_Element elements[MAX_ELEMENTS];
static int element_count = 0;

static GUI_Theme dark_theme = {
    {  23,  23,  23, 255 }, 	// border color
    {  23,  23,  23, 255 }, 	// base color
    {  35,  35,  37, 255 }, 	// active input and slider track color
    {  26,  26,  26, 255 }, 	// inactive input and slider track color
    {  80,  80,  85, 255 }, 	// content: 	normal
    {  96,  96, 102, 255 }, 	// 				hover
    { 110, 110, 116, 255 }, 	// 				click
    {  66,  66,  66, 255 }, 	// 				disabled
    { 240, 240, 240, 255 }, 	// text: 		enabled
    { 124, 124, 124, 255 }, 	// 				disabled
    { 135, 135, 135, 255 }, 	// 				placeholder
    {  95,  95, 100, 255 }, 	// list entry: 	normal
    { 110, 116, 178, 255 }, 	// 				selected
    {  84,  84,  90, 255 }, 	// scrollbar: 	track
    { 140, 140, 140, 255 }, 	// 				button, normal
    { 240, 240, 240, 255 }, 	// 				button, focus
    {  97, 100, 190, 255 } 		// progress bar, filled
};

static GUI_Theme light_theme = {
    {  95,  95,  97, 255 }, 	// border color
    { 229, 228, 224, 255 }, 	// base color
    { 250, 250, 250, 255 }, 	// active input, slider track color
    { 243, 243, 242, 255 }, 	// inactive input and slider track color
    { 216, 216, 213, 255 }, 	// content: 	normal
    { 206, 206, 202, 255 }, 	// 				hover
    { 200, 200, 195, 255 }, 	// 				click
    { 202, 202, 198, 255 }, 	// 				disabled
    {  32,  27,  32, 255 }, 	// text: 		enabled
    { 150, 150, 145, 255 }, 	// 				disabled
    {  90,  90,  90, 255 }, 	// 				placeholder
    { 244, 244, 240, 255 }, 	// list entry: 	normal
    { 188, 196, 227, 255 }, 	// 				selected
    { 204, 204, 204, 255 }, 	// scrollbar: 	track
    { 112, 112, 112, 255 }, 	// 			  	button, normal
    {  22,  22,  22, 255 }, 	// 				button, focus
    { 165, 175, 220, 255 } 		// progress bar, filled
};

void GUI_SetTheme(int mode) {
	current_theme = mode ? &dark_theme : &light_theme;
}

SDL_Renderer *GUI_GetRenderer() {
	return GUI_Renderer;
}

SDL_Window *GUI_GetWindow() {
	return GUI_Window;
}

/* Functions for use by end user */

// initialize library with an existing window and renderer
int GUI_Init(SDL_Window *window, SDL_Renderer *renderer) {
	if (gui_initialized) return 1; // already initialized (prevent calling twice)

	// import user-defined window and renderer
	if (window && renderer) {
		GUI_Window = window;
		GUI_Renderer = renderer;
	}
	
	TTF_Init();  // initialize SDL2_ttf
	default_font = TTF_OpenFont(LIBERATION_SANS, 12);
	GUI_SetTheme(DARK_MODE);

	gui_initialized = 1;
	return 1;
}

// clean up the library
// TODO: simplify with a per-element destroy function pointer
void GUI_Quit() {
	// free all allocated elements
	for (int i = 0; i < element_count; i++) {
		// GUI_Input contains an allocated text field which requires freeing separately
		if (elements[i].type == GUI_INPUT) {
			GUI_Input *input = (GUI_Input *)elements[i].element;
			free(input->text);
		}
		else if (elements[i].type == GUI_RADIOGROUP) {
			GUI_RadioGroup *group = (GUI_RadioGroup *)elements[i].element;
			free(group->buttons); 	// array of pointers to radio buttons
		}
		else if (elements[i].type == GUI_LISTBOX) {
			GUI_ListBox *listbox = (GUI_ListBox *)elements[i].element;
			free(listbox->entries); // text entries
		}
		free(elements[i].element);
	}
	TTF_Quit();
	SDL_DestroyRenderer(GUI_Renderer);
	SDL_DestroyWindow(GUI_Window);
	gui_initialized = 0;
}

// delete existing element
void GUI_DeleteElement(void *elem) {
	for (int i = 0; i < element_count; ++i) {
		if (elements[i].element == elem) {
			// free allocated memory for the element
			free(elements[i].element);

			// shift remaining elements back in the list
			for (int j = i; j < element_count - 1; ++j)
				elements[j] = elements[j + 1];

			element_count--;
			break;
		}
	}
}

void GUI_RenderElements(const char *tag) {
	for (int i = 0; i < element_count; i++) {
		GUI_Element *elem = &elements[i];
		// missing element or an element with no render function (e.g. groups)
		if (!elem || elem->render == NULL) continue;

		// check if tag is NULL or empty
		if (tag && tag[0] != '\0') {
			const char *elem_tag = ((GUI_ElementTag*)elem->element)->tag;
			if (!elem_tag || strcmp(elem_tag, tag) != 0) continue; // ignore element if tags don't match
		}
		// get current element's render function and pass the element to it
		elem->render(elem->element);
	}
}

void GUI_ProcessEvents(SDL_Event *event) {
	int mx, my;
	SDL_GetMouseState(&mx, &my);

	for (int i = 0; i < element_count; i++) {
		GUI_Element *elem = &elements[i];
		if (!elem->element || !elem->process) continue; // missing element, missing process function
		
		elem->process(event, elem->element, mx, my); 	// run element's process function, pass mouse cursor position
	}
}

/* Functions for in-library use only (not available to end user) */

// add newly created element to a universal list
void __gui_add_element(GUI_ElementType type, void *elem, GUI_Render render, GUI_Process process) {
	if (element_count >= MAX_ELEMENTS) return;

	GUI_Element e = {
		.type = type,
		.element = elem,
		.render = render,
		.process = process
	};
	elements[element_count++] = e;
}

void __gui_draw_borders(int x, int y, int width, int height, int border_width) {
	if (border_width < 1) return; 	// no borders

	SDL_Renderer *renderer = GUI_GetRenderer();

	// get element's location and offset it
	SDL_Rect border_rect = {
		x - border_width, 			// top and left borders
		y - border_width,
		width + border_width * 2, 	// bottom and right borders
		height + border_width * 2
	};
	SDL_SetRenderDrawColor(renderer, SET_COLOR_BORDER);
	SDL_RenderFillRect(renderer, &border_rect);
}

// helper function to render regular text
void __gui_render_text(const char *text, SDL_Rect *target_rect, SDL_Color color) {
	SDL_Renderer *renderer = GUI_GetRenderer();
	if (!text || !*text || !target_rect) return;

	SDL_Surface *surface = TTF_RenderUTF8_Blended(default_font, text, color);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

	int text_w, text_h;
	SDL_QueryTexture(texture, NULL, NULL, &text_w, &text_h);

	SDL_Rect text_rect = {
		target_rect->x,
		target_rect->y + (target_rect->h - text_h) / 2,
		text_w,
		text_h
	};
	SDL_RenderCopy(renderer, texture, NULL, &text_rect);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

// helper function to render clipped text (cut off long texts)
void __gui_render_text_clipped(const char *text, SDL_Rect *input_rect, int text_offset, SDL_Color color) {
	if (!text || !*text || !input_rect) return;

	SDL_Renderer *renderer = GUI_GetRenderer();

	SDL_Surface *surface = TTF_RenderUTF8_Blended(default_font, text, color);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

	// store text proportions
	int text_width, text_height;
	TTF_SizeUTF8(default_font, text, &text_width, &text_height);

	// clip area to prevent text from overflowing
	SDL_Rect clip_rect = {
		input_rect->x + 4, // account for padding
		input_rect->y + 2,
		input_rect->w - 8,
		input_rect->h - 4
	};
	SDL_RenderSetClipRect(renderer, &clip_rect);

	// visible part of text
	SDL_Rect text_rect = {
		input_rect->x + 4 - text_offset,
		input_rect->y + (input_rect->h - text_height) / 2,
		text_width,
		text_height
	};
	SDL_Rect *src_rect = NULL;

	SDL_RenderCopy(renderer, texture, src_rect, &text_rect);
	SDL_RenderSetClipRect(renderer, NULL); 	// reset back to default

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

// helper function for SDL2_gfx geometric functions
// uses bit-shifting to obtain specific byte
Uint32 __gui_color_to_uint32(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}