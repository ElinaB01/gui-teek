/* Macros used across the library to keep code cleaner */

#ifndef DEFS_H
#define DEFS_H

#define LIBERATION_SANS 		"LiberationSans-Regular.ttf"

/* Global true/false definitions for use by the end user */

// element is greyed out and not interactive
#define DISABLED 				0
#define ENABLED 				1

// hide individual element
#define HIDDEN 					0
#define VISIBLE 				1

// element can be resized by dragging (unused)
#define FIXED_SIZE 				0
#define RESIZABLE 				1

// library-wide theme
#define LIGHT_MODE 				0
#define DARK_MODE 				1

// slider: knob does not snap into place
#define SNAP 					0
#define SMOOTH 					1

// slider: direction of the knob's movement (unused)
#define HORIZONTAL 				0
#define VERTICAL 				1

// checkbox and radio button state
#define OFF 					0
#define ON 						1

/* Macros for setting colors in SDL_SetRenderDrawColor() */

#define DEFAULT_TEXT_COLOR 					current_theme->text_enabled

// interactive element color (button, slider knob, etc.)
#define SET_COLOR_NORMAL 					current_theme->content_normal.r, current_theme->content_normal.g, current_theme->content_normal.b, current_theme->content_normal.a

// mouse is hovering above the element
#define SET_COLOR_FOCUS 					current_theme->content_hover.r, current_theme->content_hover.g, current_theme->content_hover.b, current_theme->content_hover.a

// element is clicked
#define SET_COLOR_ACTIVE 					current_theme->content_active.r, current_theme->content_active.g, current_theme->content_active.b, current_theme->content_active.a

#define SET_COLOR_DISABLED 					current_theme->content_disabled.r, current_theme->content_disabled.g, current_theme->content_disabled.b, current_theme->content_disabled.a

#define SET_COLOR_BORDER 					current_theme->border_color.r, current_theme->border_color.g, current_theme->border_color.b, current_theme->border_color.a

// background color of elements (input field, slider background, etc.)
#define SET_COLOR_INPUT_ACTIVE 				current_theme->input_active.r, current_theme->input_active.g, current_theme->input_active.b, current_theme->input_active.a

#define SET_COLOR_INPUT_INACTIVE 			current_theme->input_inactive.r, current_theme->input_inactive.g, current_theme->input_inactive.b, current_theme->input_inactive.a

#define SET_COLOR_TEXT_ENABLED 				current_theme->text_enabled.r, current_theme->text_enabled.g, current_theme->text_enabled.b, current_theme->text_enabled.a

#define SET_COLOR_TEXT_PLACEHOLDER 			current_theme->text_placeholder.r, current_theme->text_placeholder.g, current_theme->text_placeholder.b, current_theme->text_placeholder.a

// list and combo boxes: entries
#define SET_COLOR_ENTRY_NORMAL 				current_theme->list_entry_normal.r, current_theme->list_entry_normal.g, current_theme->list_entry_normal.b, current_theme->list_entry_normal.a

#define SET_COLOR_ENTRY_SELECTED 			current_theme->list_entry_selected.r, current_theme->list_entry_selected.g, current_theme->list_entry_selected.b, current_theme->list_entry_selected.a

// scrollbar colors
#define SET_COLOR_SCROLLBAR_TRACK 			current_theme->scrollbar_track.r, current_theme->scrollbar_track.g, current_theme->scrollbar_track.b, current_theme->scrollbar_track.a

#define SET_COLOR_SCROLLBAR_BUTTON_NORMAL 	current_theme->scrollbar_button_normal.r, current_theme->scrollbar_button_normal.g, current_theme->scrollbar_button_normal.b, current_theme->scrollbar_button_normal.a

#define SET_COLOR_SCROLLBAR_BUTTON_FOCUS 	current_theme->scrollbar_button_focus.r, current_theme->scrollbar_button_focus.g, current_theme->scrollbar_button_focus.b, current_theme->scrollbar_button_focus.a

// progress bar colors
#define SET_COLOR_PROGRESS 					current_theme->progress_color.r, current_theme->progress_color.g, current_theme->progress_color.b, current_theme->progress_color.a

#endif // DEFS_H