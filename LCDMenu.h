// LCD Menu
#include "LCD_DISCO_F429ZI.h"

// forward declare functions
void DrawTitleBar(char *Title);
void ClearStatusBar();
void UpdateStatusBar(char *Text);
void DrawMenuFrame();
void UpdateDisplayMenu(uint8_t menuOffset, uint8_t position);
void DisplayMenuOptions(uint8_t menuOffset);
void HighlightMenuOption(uint8_t menuOffset, uint8_t position);

// callbacks for different menu items
void Do_Nothing();
void MenuOne_Function();
void MenuTwo_Function();
void MenuThree_Function();
void MenuFour_Function();

LCD_DISCO_F429ZI lcd;
char lcdBuffer[20]; // lcd display buffer

#define PROGRAM_VERSION "LCDMenu v0.6"

#define TITLE_BAR_HEIGHT 40
#define TITLE_BAR_COLOR 0xff1253d6
#define TITLE_BAR_TEXTCOLOR 0xffffffff

#define STATUS_BAR_HEIGHT 30
#define STATUS_BAR_COLOR 0xff000000
#define STATUS_BAR_TEXTCOLOR 0xffaaaaaa

#define MENU_FRAME_COLOR 0xff1f537f
#define MENU_FRAME_TEXTCOLOR 0xffffffff
#define MENU_FRAME_LINE_HEIGHT 35
#define MENU_FRAME_PADDING 10

#define MENU_DISPLAY_COUNT 7
#define MENU_HIGHLIGHT_TEXTCOLOR 0xffffffff
#define MENU_HIGHLIGHT_BACKCOLOR 0xffe95431

typedef struct MenuOptions_t
{
    char MenuText[20];
    void (*callback_function)(); // declare function pointer
};

static MenuOptions_t MenuOptions[] = {
    {"1 Menu AAAA", &Do_Nothing},
    {"2 Menu BBBB", &Do_Nothing},
    {"3 Menu CCCC", &Do_Nothing},
    {"4 Menu DDDDD", &Do_Nothing},
    {"5 Menu EEEE", &Do_Nothing}};