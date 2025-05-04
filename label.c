#include <stdlib.h>  // malloc
#include <stdio.h>   // printf
#include <string.h>  // strdup, strtok
#include <SDL2/SDL_ttf.h>
#include "guilib.h"
#include "defs.h"

#define TEXT_SIZE 			12
#define COLOR_IS_SET 		label->color.r || label->color.g || label->color.b || label->color.a

// TODO:
// allow anti-aliasing toggle: use TTF_RenderText_Solid() for pixelated text
// bold, italics, underlined text

// basic label
GUI_Label *GUI_CreateLabel(int x, int y, char *text) {
	GUI_Label *l = malloc(sizeof(GUI_Label));
    *l = (GUI_Label){ NULL, x, y, VISIBLE, text, {0}, NULL };

	l->font = TTF_OpenFont(LIBERATION_SANS, TEXT_SIZE);
	if (!l->font)
		printf("\n[!] Failed to load label font: %s\n", TTF_GetError());

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_LABEL, l, (void (*)(void*))GUI_RenderLabel, NULL);
	return l;
}

// extended label: includes color, font, text size
GUI_Label *GUI_CreateLabelEx(int x, int y, char *text, const char *font_path, int text_size) {
	GUI_Label *l = malloc(sizeof(GUI_Label));
    *l = (GUI_Label){ NULL, x, y, VISIBLE, text, {0}, NULL };

	l->font = TTF_OpenFont(font_path, text_size);
	if (!l->font)
		printf("\n[!] Failed to load label font: %s\n", TTF_GetError());

	__gui_add_element(GUI_LABEL, l, (void (*)(void*))GUI_RenderLabel, NULL);
	return l;
}

void GUI_RenderLabel(GUI_Label *label) {
	if (!label || !label->font || !label->visible) return; // NULL pointer, missing font, hidden element

	SDL_Renderer *renderer = GUI_GetRenderer();
	
	SDL_Surface *surface;
	SDL_Texture *texture;

	char *text_cpy = strdup(label->text); 	// duplicate of the label text

	int line_y = label->y; 					// position to start rendering new lines from
	char *line = strtok(text_cpy, "\n"); 	// split text into tokens (substrings) by newline '\n' characters

	SDL_Color text_color;

	// if no custom color is set, use the theme default
	if (COLOR_IS_SET)
		text_color = label->color;
	else
		text_color = current_theme->text_enabled;

	while (line) {
		// create text surface and texture
		surface = TTF_RenderUTF8_Blended(label->font, line, text_color);
		texture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_Rect label_rect = { label->x, line_y, surface->w, surface->h }; // text bounding box
		SDL_RenderCopy(renderer, texture, NULL, &label_rect); 				// render text

		line_y += surface->h; 		// move to next line
		line = strtok(NULL, "\n"); 	// pointer to next token (next line of text)
		
		SDL_FreeSurface(surface); 	// clean up
		SDL_DestroyTexture(texture);
	}
	free(text_cpy);
}

void GUI_DestroyLabel(GUI_Label *label) {
	if (label->font) {
		TTF_CloseFont(label->font);
		label->font = NULL;
	}
}