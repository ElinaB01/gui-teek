#include <stdlib.h> // malloc
#include <stdio.h>  // printf
#include <string.h> // strncpy, strlen, strcat
#include "guilib.h"
#include "defs.h"

#define FIELD_HEIGHT 		22
#define CARET_HEIGHT 		13
#define TEXT_SIZE 			12
#define BORDER_WIDTH 		1
#define PADDING 			4
#define CARET_BLINK_MS 		500

// TODO:
// clipboard support (SDL_GetClipboardText())
// 		example: https://lazyfoo.net/tutorials/SDL/32_text_input_and_clipboard_handling/index.php
// callback function on send (Enter key, send button click)
// FIX: 2-byte unicode characters (e.g. Ä) should increment character count by 1 (separate char limit from array size)
// switch to SDL_StartTextInput() on focus?

static void __gui_process_input_field(SDL_Event *event, GUI_Input *input, int mx, int my);

GUI_Input *GUI_CreateInputField(int x, int y, int width, int max_length, char *placeholder) {
	if (!max_length) {
		printf("\n[!] Input length of 0. Aborting input field creation.");
		return NULL;
	}

	// allocate text buffer to initialize text field with the max length
	char *buffer = calloc(max_length + 1, sizeof(char));
	char *placeholder_valid = NULL;
	
	if (placeholder) {
		// if placeholder text exceeds the character limit (max_length), trim it
		// this makes a copy of the variable and ensures its value is valid
		if (strlen(placeholder) > max_length) {
			placeholder_valid = malloc(max_length + 1);
			strncpy(placeholder_valid, placeholder, max_length);
			placeholder_valid[max_length] = '\0'; // null-terminate the string
		} else {
			placeholder_valid = strdup(placeholder);
		}
	}

	GUI_Input *i = malloc(sizeof(GUI_Input));

	*i = (GUI_Input){
		.tag = NULL,
		.x = x,
		.y = y,
		.width = width,
		.height = FIELD_HEIGHT,
		.border_width = BORDER_WIDTH,
		.visible = VISIBLE,
		.focus = 0,
		.cursor_pos = 0,
		.max_length = max_length,
		.text_size = TEXT_SIZE,
		.caret_visible = 0,
		.last_blink = 0,
		.text_offset = 0,
		.text = NULL,
		.placeholder = placeholder_valid
	};

	i->text = buffer;

	// add to general list of elements for simplified processing
	__gui_add_element(GUI_INPUT, i, (GUI_Render)GUI_RenderInput, (GUI_Process)__gui_process_input_field);
	return i;
}

/* Helper functions */

/* cursor (caret) reading and positioning */

int __gui_get_cursor_position(const char *text, int cursor_pos) {
	if (!text || !*text) return 0; 		// NULL pointer or empty string

	char buffer[256];
	strncpy(buffer, text, cursor_pos); 	// copy text into buffer up until the cursor position
	buffer[cursor_pos] = '\0'; 			// null-terminate the string

	int offset = 0; 					// store pixel width of the substring
	TTF_SizeUTF8(default_font, buffer, &offset, NULL);
	return offset; 						// return cursor offset within the text
}

void __gui_update_cursor_position(GUI_Input *input) {
	int caret_x = __gui_get_cursor_position(input->text, input->cursor_pos);
	int max_width = input->width - 2 * PADDING; // determine how much text can fit within the field

	// scroll text if the caret goes out of bounds
	if (caret_x - input->text_offset > max_width)  // scroll right
		input->text_offset = caret_x - max_width;
	else if (caret_x - input->text_offset < 0) 	   // scroll left
		input->text_offset = caret_x;
}

void __gui_place_caret(GUI_Input *input, int mx) {
	if (!input || !input->text || !*input->text) return;  // NULL pointers or empty string

	int cursor_pos = mx - input->x + input->text_offset;  // relative position within the field
	int pos = 0, text_width = 0;

	// process text one character at a time
	while (input->text[pos]) {
		int char_width = 0;

		// measure character width (UTF-8)
		char c[2] = { input->text[pos], '\0' };
		TTF_SizeUTF8(default_font, c, &char_width, NULL);

		// stop iterating if cursor is on the character
		if (text_width + char_width / 2 >= cursor_pos) break;

		text_width += char_width;
		pos++;
	}
	input->cursor_pos = pos;
}

void __gui_draw_caret(SDL_Renderer *renderer, GUI_Input *input) {
	Uint32 time_now = SDL_GetTicks();

	// blink every 500 ms
	if (time_now - input->last_blink >= CARET_BLINK_MS) {
		input->caret_visible = !input->caret_visible;
		input->last_blink = time_now;
	}

	if (!input->caret_visible) return;

	// get caret's position within the input field and clamp it
	int caret_x = input->x + PADDING + __gui_get_cursor_position(input->text, input->cursor_pos) - input->text_offset;
	caret_x = SDL_clamp(caret_x, input->x, input->x + input->width - PADDING);

	// draw the caret
	SDL_SetRenderDrawColor(renderer, SET_COLOR_TEXT_ENABLED);
	SDL_RenderDrawLine(renderer, caret_x, input->y + PADDING, caret_x, input->y + CARET_HEIGHT + PADDING);
}

/* Word scanning for Ctrl key modifier */

static int __gui_prev_word_pos(const char *text, int cursor_pos) {
	if (cursor_pos == 0) return 0;
	cursor_pos--;

	// skip spaces
	while (cursor_pos > 0 && text[cursor_pos] == ' ')
		cursor_pos--;

	// if prev char is not a space, move cursor to the beginning of the previous word
	while (cursor_pos > 0 && text[cursor_pos - 1] != ' ')
		cursor_pos--;

	return cursor_pos;
}

static int __gui_next_word_pos(const char *text, int cursor_pos) {
	int length = strlen(text);

	// skip spaces
	while (cursor_pos < length && text[cursor_pos] == ' ')
		cursor_pos++;
	
	// if next char is not a space, move cursor to the end of the next word
	while (cursor_pos < length && text[cursor_pos] != ' ')
		cursor_pos++;

	return cursor_pos;
}

/* Main logic */

void GUI_RenderInput(GUI_Input *input) {
	if (!input || !input->visible) return;

	SDL_Renderer *renderer = GUI_GetRenderer();

	// input field body
	SDL_Rect input_rect = { input->x, input->y, input->width, input->height };

	__gui_draw_borders(input->x, input->y, input->width, input->height, input->border_width);

	// light up the box when in focus
	SDL_Color color = input->focus ? current_theme->input_active : current_theme->input_inactive;
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &input_rect);

	// set text colors
	SDL_Color text_color = { SET_COLOR_TEXT_ENABLED };
	SDL_Color placeholder_color = { SET_COLOR_TEXT_PLACEHOLDER };

	// render input or placeholder text
	if (input->text && *input->text)
		__gui_render_text_clipped(input->text, &input_rect, input->text_offset, text_color);
	else if (input->placeholder && *input->placeholder)
		__gui_render_text_clipped(input->placeholder, &input_rect, input->text_offset, placeholder_color);

	if (input->focus)
		__gui_draw_caret(renderer, input);
}

static void __gui_process_input_field(SDL_Event *event, GUI_Input *input, int mx, int my) {
	if (!input) return; // NULL pointer

	int intersect = (mx >= input->x && mx <= input->x + input->width &&
					 my >= input->y && my <= input->y + input->height);

	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT)
		input->focus = intersect;

	if (!input->focus) return;

	// handle text input
	if (event->type == SDL_TEXTINPUT) {
		int input_len = strlen(input->text); 		// current input length
		int added_len = strlen(event->text.text); 	// newly added characters

		if (input_len + added_len <= input->max_length) {
			// shift text right to make room for new input
			memmove(input->text + input->cursor_pos + added_len,
					input->text + input->cursor_pos,
					input_len - input->cursor_pos + 1);
			// insert new characters at cursor (caret) position
			memcpy(input->text + input->cursor_pos, event->text.text, added_len);
			input->cursor_pos += added_len;
		}
	}
	// handle special key presses
	else if (event->type == SDL_KEYDOWN) {
		// state of Ctrl modifier key for processing whole words
		SDL_Keymod mod = SDL_GetModState();
		int ctrl = (mod & KMOD_CTRL);

		switch (event->key.keysym.sym) {
			// delete previous character or word
			case SDLK_BACKSPACE:
				if (input->cursor_pos > 0) {
					// shift input array to the left by one character
					// if ctrl is held, shift by whole word
					int new_pos = ctrl ? __gui_prev_word_pos(input->text, input->cursor_pos) : input->cursor_pos - 1;

					// shift memory to the left, overwriting the deleted character(s)
					memmove(input->text + new_pos,
							input->text + input->cursor_pos,
							strlen(input->text + input->cursor_pos) + 1);

					input->cursor_pos = new_pos;
				}
				break;

			// delete next character or word
			case SDLK_DELETE: {
				int input_len = strlen(input->text);

				if (input->cursor_pos < input_len) {
					int del_end = ctrl ? __gui_next_word_pos(input->text, input->cursor_pos) : input->cursor_pos + 1;

					// shift memory to the left
					memmove(input->text + input->cursor_pos,
							input->text + del_end,
							input_len - del_end + 1);
				}
				break;
			}

			// move cursor left
			case SDLK_LEFT:
				if (ctrl)
					input->cursor_pos = __gui_prev_word_pos(input->text, input->cursor_pos);
				else if (input->cursor_pos > 0)
					input->cursor_pos--;
				break;

			// move cursor right
			case SDLK_RIGHT:
				if (ctrl)
					input->cursor_pos = __gui_next_word_pos(input->text, input->cursor_pos);
				else if (input->cursor_pos < strlen(input->text))
					input->cursor_pos++;
				break;

			// bring cursor to the start of input
			case SDLK_HOME:
			case SDLK_UP:
				input->cursor_pos = 0;
				break;

			// bring cursor to the end of input
			case SDLK_END:
			case SDLK_DOWN:
				input->cursor_pos = strlen(input->text);
				break;

			// send input for processing
			case SDLK_RETURN:
				// DEMO: print input to console (replace with processing function)
				printf("Input: %s\n", input->text);
				// reset input
				input->text[0] = '\0';
				input->cursor_pos = 0;
				input->focus = 0;
				break;
		}
		__gui_update_cursor_position(input);
		input->caret_visible = 1;
		input->last_blink = SDL_GetTicks(); // keep caret from blinking while typing
	}
	else if (event->type == SDL_MOUSEBUTTONDOWN)
		__gui_place_caret(input, mx); 		// place caret inside text on click
}