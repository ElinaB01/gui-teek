/*
	This is the header file for guilib.c. Every function
	defined here is in use by the library, and some may
	be called by the user. Functions defined with the
	EXPORT macro are available to the end user, the rest
	are intended for in-library use only.
*/

#ifndef GUILIB_H
#define GUILIB_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// macro for exporting and importing symbols on Windows systems (required by TCC)
#ifdef _WIN32
#ifdef BUILD_GUILIB
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

// gives the library access to the renderer and window
extern SDL_Renderer *GUI_GetRenderer();
extern SDL_Window *GUI_GetWindow();

EXPORT int GUI_Init(SDL_Window *window, SDL_Renderer *renderer);
EXPORT void GUI_Quit();

extern TTF_Font *default_font; // TODO: implement GUI_GetFont()

// for in-library use only (not available to end user)
// internal functions are all lowercase and start with a double underscore
// TODO: fix GCC importing these regardless (macro: HIDDEN)
void __gui_draw_borders(int x, int y, int width, int height, int border_width);
void __gui_render_text(const char *text, SDL_Rect *target_rect, SDL_Color color);
void __gui_render_text_clipped(const char *text, SDL_Rect *input_rect, int text_offset, SDL_Color color);
Uint32 __gui_color_to_uint32(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Theme color palette template */

typedef struct {
    SDL_Color
		border_color,
		base_color,
		input_active,
		input_inactive,
		content_normal,
		content_hover,
		content_active,
		content_disabled,
		text_enabled,
		text_disabled,
		text_placeholder,
		list_entry_normal,
		list_entry_selected,
		scrollbar_track,
		scrollbar_button_normal,
		scrollbar_button_focus,
		progress_color;
} GUI_Theme;

extern GUI_Theme *current_theme;
EXPORT void GUI_SetTheme(int mode);

/* Existing types of elements */

typedef enum {
	GUI_LABEL,
	GUI_BUTTON,
	GUI_SLIDER,
	GUI_INPUT,
	GUI_CHECKBOX,
	GUI_RADIOBUTTON,
	GUI_RADIOGROUP,
	GUI_PROGRESSBAR,
	GUI_LISTBOX
} GUI_ElementType;

// Generic container for storing pointers to any type of element
// this allows storing and processing different GUI elements in a single array/list.
// * void pointers can point to any type of data in C, in this case the element and its functions

typedef void (*GUI_Render)(void*);
typedef void (*GUI_Process)(void*, SDL_Event*, int, int);

typedef struct {
    GUI_ElementType type;
    void *element;
	GUI_Render render;
	GUI_Process process;
	// void (*destroy)(void*);  // TODO: simplify GUI_Quit()
} GUI_Element;

void __gui_add_element(GUI_ElementType type, 	// index from GUI_ElementType enum
						void *elem, 			// pointer to data type (e.g. GUI_Button)
						GUI_Render render, 		// pointer to element's render function
						GUI_Process process); 	// pointer to element's processing function
EXPORT void GUI_DeleteElement(void *elem);
EXPORT void GUI_RenderElements();
EXPORT void GUI_ProcessEvents(SDL_Event *event);

// helper struct for tag-based checks and filters
typedef struct {
	const char *tag;
} GUI_ElementTag;

/* Scrollbar */

typedef struct GUI_Scrollbar {
	int x, y, width, height,
		button_height,
		hovered_up, hovered_down,
		clicked_up, clicked_down,
		*scroll_offset, 			// pointer to external scroll offset
		max_offset, 				// how far we can scroll
		visible_entries; 			// how many entries are visible at once
	SDL_Rect up_button, down_button, track;
} GUI_Scrollbar;

void __gui_init_scrollbar(GUI_Scrollbar *sb, int x, int y, int height, int *scroll_offset, int visible_entries, int max_offset);
void __gui_render_scrollbar(GUI_Scrollbar *sb);
int __gui_process_scrollbar(GUI_Scrollbar *sb, SDL_Event *event, int mx, int my, SDL_Rect content_area);

/* Label */

typedef struct {
	const char *tag;  // used to filter or group elements together
	int x, y, visible;
	char *text;
	SDL_Color color;
	TTF_Font *font;
} GUI_Label;

EXPORT GUI_Label *GUI_CreateLabel(int x, int y, char *text);
EXPORT GUI_Label *GUI_CreateLabelEx(int x, int y, char *text, const char *font_path, int text_size);
EXPORT void GUI_RenderLabel(GUI_Label *label);
EXPORT void GUI_DestroyLabel(GUI_Label *label);

/* Button */

typedef struct GUI_Button {
	const char *tag;
    int x, y, width, height, border_width, 	// location and proportions
		enabled, visible, 					// available for processing and rendering
		hovered, pressed, 					// change color based on this state (mouse-over, on-click)
		text_size;
    const char *text;
	void (*on_click)(void*); 	// function to execute when the button is clicked
	void *args; 				// optional data to pass to on_click()
} GUI_Button;

EXPORT GUI_Button *GUI_CreateButton(int x, int y, const char *text, void (*on_click)(void*));
EXPORT void GUI_RenderButton(GUI_Button *button);

/* Slider */

typedef struct {
	const char *tag;
	int x, y, width, height, 		// track location and proportions
		knob_width, knob_height, 	// knob proportions
		border_width,
		visible,
		pos_x, 						// on-screen position of the knob
		focus,
		dragging, 					// is the slider currently in use?
		smooth; 					// knob does not snap into place when dragged
	float
		min, max, 					// range of possible values
		increment, 					// how many units to increment the value by
		value; 						// current value
} GUI_Slider;

EXPORT GUI_Slider *GUI_CreateSlider(int x, int y, int width, float min, float max, float increment, float value);
EXPORT GUI_Slider *GUI_CreateSliderEx(int x, int y, int width, int height, int knob_width, int knob_height, int border_width, float min, float max, float increment, float value, int smooth);
EXPORT void GUI_RenderSlider(GUI_Slider *slider);

/* Input field */

typedef struct {
	const char *tag;
    int x, y, width, height,
		border_width,
		visible,
		focus, 					// if in focus, highlight and draw a blinking caret
		cursor_pos,
		max_length, 			// max input length/character limit
		text_size,
		caret_visible, 			// toggles caret visibility (blinking vertical cursor)
		last_blink, 			// stores the last time it blinked for a smoother appearance
		text_offset; 			// tracks position for text scrolling on overflow
    char *text; 				// stores user input
	const char *placeholder; 	// faded placeholder text or hint text
} GUI_Input;

EXPORT GUI_Input *GUI_CreateInputField(int x, int y, int width, int max_len, char *placeholder);
EXPORT void GUI_RenderInput(GUI_Input *input);

/* Checkbox */

typedef struct {
	const char *tag;
	int x, y, width, height,
		border_width,
		enabled,
		visible,
		focus,
		selected;
} GUI_Checkbox;

EXPORT GUI_Checkbox *GUI_CreateCheckbox(int x, int y);
EXPORT void GUI_RenderCheckbox(GUI_Checkbox *checkbox);

/* Radio button */

typedef struct {
	const char *tag;
	int x, y, r, 	// position and radius
		bullet_r, 	// radius of the inner dot (bullet)
		border_width,
		enabled,
		visible,
		focus,
		selected;
	struct GUI_RadioGroup *group;
} GUI_RadioButton;

typedef struct GUI_RadioGroup {
	GUI_RadioButton **buttons; 	// array of pointers to individual radio buttons
	int button_count;
} GUI_RadioGroup;

EXPORT GUI_RadioButton *GUI_CreateRadioButton(int x, int y);
EXPORT void GUI_RenderRadioButton(GUI_RadioButton *radiobutton);
EXPORT GUI_RadioGroup *GUI_CreateRadioGroup();
EXPORT void GUI_AddToRadioGroup(GUI_RadioGroup *group, GUI_RadioButton *radiobutton);

/* Progress bar */

typedef struct {
	const char *tag;
	int x, y, width, height,
		border_width,
		visible,
		min, max,
		value; 	// actual value
	float pos; 	// in-between value to smoothe large jumps in progress
} GUI_ProgressBar;

EXPORT GUI_ProgressBar *GUI_CreateProgressBar(int x, int y, int width, int min, int max);
EXPORT void GUI_RenderProgressBar(GUI_ProgressBar *bar);

/* List box */

// general entry struct for use by collections (ListBox, ComboBox)
typedef struct {
	const char *text;
} GUI_ListEntry;

typedef struct {
	const char *tag,
				*placeholder; 	// default text if a selection has not been made yet
	int x, y, width, height,
		border_width,
		entry_height, 			// height of one entry field
		visible,
		entry_count, 			// total number of entries
		selected_id,
		max_visible, 			// how many entries are visible at once
		show_scrollbar, 		// if entry_count > max_visible, render a scrollbar
		scroll_offset, 			// offset to shift entries by
		expanded; 				// collapsed (0) by default
	GUI_ListEntry *entries, 	// all entries
				  *selected_entry, 		// (in single option lists) current selection
				  *highlighted_entry; 	// current selection or focused entry (on mouse hover)
	GUI_Scrollbar scrollbar;
	void (*on_select)(void*); 			// function to call when a new entry is selected
	void *args; 						// optional data to pass to on_select()
} GUI_ListBox;

EXPORT GUI_ListBox *GUI_CreateListBox(int x, int y, const char *placeholder, void (*on_select)(void*));
EXPORT void GUI_AddListEntry(GUI_ListBox *lb, const char *entry);
EXPORT void GUI_SetListEntries(GUI_ListBox *lb, const char **texts, int count);
EXPORT void GUI_SelectListEntry(GUI_ListBox *lb, int index);
EXPORT void GUI_RenderListBox(GUI_ListBox *listbox);


#ifdef __cplusplus
}
#endif

#endif