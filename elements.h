/*
	This is the header file accompanying elements.c,
	it is user-defined and not part of the library.
*/

#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "guilib.h"

#define BG_DARK 			65, 65, 67, 255
#define BG_LIGHT 			226, 224, 220, 255


// color indexes to pass to set_background_color() in the list box
typedef enum {
	BG_DEFAULT,  // default light or dark mode background color
	BG_GREY,
	BG_RED,
	BG_GREEN,
	BG_TEAL,
	BG_PURPLE,
	BG_PINK,
	BG_BROWN,
	COLORS_COUNT  // last index serves as total count of items
} BG_Theme;

// color RGBA values
static SDL_Color bg_colors[COLORS_COUNT] = {
	{  65,  65,  67, 255 }, 	// BG_DEFAULT (theme default; changes on mode switch)
	{ 100, 102, 106, 255 }, 	// BG_GREY
	{ 185,  42,  42, 255 }, 	// BG_RED
	{  60, 150,  42, 255 }, 	// BG_GREEN
	{  60, 150, 175, 255 }, 	// BG_TEAL
	{ 125,  80, 155, 255 }, 	// BG_PURPLE
	{ 195,  80, 155, 255 }, 	// BG_PINK
	{ 185,  90,  42, 255 }  	// BG_BROWN
};

extern char buffer1[50];
extern char buffer2[50];
extern char buffer3[20];

extern SDL_Color bg_color;

extern GUI_Button *button1;
extern GUI_Button *button2;
extern GUI_Button *button3;
extern GUI_Button *button4;
extern GUI_Slider *slider1;
extern GUI_Slider *slider2;
extern GUI_Label *label_title;
extern GUI_Label *label1;
extern GUI_Label *label2;
extern GUI_Label *label3;
extern GUI_Label *label4;
extern GUI_Label *label5;
extern GUI_Label *label6;
extern GUI_Label *label7;
extern GUI_Label *label8;
extern GUI_Label *label9;
extern GUI_Label *label_info;
extern GUI_Input *input1;
extern GUI_Checkbox *checkbox1;
extern GUI_Checkbox *checkbox2;
extern GUI_Checkbox *checkbox3;
extern GUI_RadioButton *radiobutton1;
extern GUI_RadioButton *radiobutton2;
extern GUI_RadioButton *radiobutton3;
extern GUI_RadioButton *radiobutton4;
extern GUI_RadioButton *radiobutton5;
extern GUI_RadioButton *radiobutton6;
extern GUI_RadioGroup *radiobuttons1;
extern GUI_RadioGroup *radiobuttons2;
extern GUI_ProgressBar *progressbar;
extern GUI_ListBox *listbox;

void InitElementList(void);

extern void button1_click();
extern void SetBackgroundColor();
extern void SwitchMode();
extern void InitiateLoading();

#endif