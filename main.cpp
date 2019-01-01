// ------------------------------------------------------------------
// Name: LCDMenu
// Description:
// Sample program for displaying a multi-layered menu system
// ------------------------------------------------------------------
#include "mbed.h"
#include "LCDMenu.h"
#include "LCD_DISCO_F429ZI.h"
#include "PinDetect.h" // URL: http://mbed.org/users/AjK/libraries/PinDetect

using namespace std;

uint8_t mainMenuOffset = 0;
uint8_t mainMenuPosition = 1;
uint8_t totalMenuCount = sizeof(MenuOptions) / sizeof(MenuOptions_t);
uint16_t LcdWidth = lcd.GetXSize();
uint16_t LcdHeight = lcd.GetYSize();

Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(PG_14); // red
DigitalOut led2(PG_13); // green
DigitalOut greenLed(PD_12);

// Int Handler for Encoder PB Switch
PinDetect EncoderSwitch(PF_9);

void Do_Nothing()
{
  // nothing...
}

void MenuOne_Function()
{
  lcd.Clear(LCD_COLOR_RED);
}

void MenuTwo_Function()
{
  lcd.Clear(LCD_COLOR_GREEN);
}

void MenuThree_Function()
{
  lcd.Clear(LCD_COLOR_YELLOW);
}

void MenuFour_Function()
{
  lcd.Clear(LCD_COLOR_BLUE);
}

void SwitchHandler()
{
  led2 = !led2;
  greenLed = !greenLed;
  // if button pressed, we need to determine which menu item is currently highlighted
  // then execute a callback function for that menu item
  // MenuOptions[mainMenuOffset + mainMenuPosition - 1] is the selected menu option and ndx
  MenuOptions[mainMenuOffset + mainMenuPosition - 1].callback_function(); // execute callback function for this menu item
}

// Interrupt for Encoder Rotary Out A/B
PinDetect EncoderOutA(PC_12, PullUp);
PinDetect EncoderOutB(PC_11, PullUp);
int encoderOutA_LastState = 0;
int encoderOutB_LastState = 0;
int EncoderOutA_State;
int encoderOutB_State;
int rotationValue = 0;

// Check Rotary Encoder status (switch + rotation)
// based on datasheet (CW Rotation)
// position | OutA | OutB
// 1 | 0 | 0
// 2 | 1 | 0
// 3 | 1 | 1 // starting
// 4 | 0 | 1

// Interrupt Handlers for A/B
void RotaryEncoderHandlerA_assert()
{
  EncoderOutA_State = 1;
  led1 = 1;
  if (EncoderOutA_State != encoderOutA_LastState)
  {
    // check if we moved CW, increment for each detent
    if (EncoderOutA_State != encoderOutB_State)
    {
      // CW
      rotationValue += 1;
      mainMenuPosition += 1;
    }
    else
    {
      rotationValue -= 1;
      mainMenuPosition -= 1;
    }
  }
  encoderOutA_LastState = EncoderOutA_State;
  // UpdateRotaryValue();
  UpdateDisplayMenu(mainMenuOffset, mainMenuPosition);
}
void RotaryEncoderHandlerA_deasserted()
{
  EncoderOutA_State = 0;
  led1 = 0;
  if (EncoderOutA_State != encoderOutA_LastState)
  {
    // check if we moved CW, increment for each detent
    if (EncoderOutA_State != encoderOutB_State)
    {
      // CW
      rotationValue += 1;
      mainMenuPosition += 1;
    }
  }
  encoderOutA_LastState = EncoderOutA_State;
  // UpdateRotaryValue();
}

void RotaryEncoderHandlerB_assert()
{
  encoderOutB_State = 1;
  led2 = 1;
  encoderOutB_LastState = encoderOutB_State;
}

void RotaryEncoderHandlerB_deasserted()
{
  encoderOutB_State = 0;
  led2 = 0;
  if (encoderOutB_LastState)
  {
    if (!EncoderOutA_State)
    {
      rotationValue -= 1;
      mainMenuPosition -= 1;
    }
  }
  encoderOutB_LastState = encoderOutB_State;
  // UpdateRotaryValue();
}

void UpdateRotaryValue()
{
  // display on LCD
  lcd.SetFont(&Font24);
  lcd.SetBackColor(LCD_COLOR_BLACK);
  lcd.SetTextColor(LCD_COLOR_WHITE);
  sprintf(lcdBuffer, "%03d", rotationValue);
  lcd.DisplayStringAt(1, 100, (uint8_t *)lcdBuffer, CENTER_MODE);
}

void InitRotaryEncoder()
{
  // setup Interrupts for Encoder Switch
  EncoderSwitch.attach_asserted(&SwitchHandler);
  EncoderSwitch.setSampleFrequency(10000); // Start sampling pb input using interrupts (us)
  // setup Interrupts for Encoder Output A/B
  EncoderOutA.attach_asserted(&RotaryEncoderHandlerA_assert);
  EncoderOutA.attach_deasserted(&RotaryEncoderHandlerA_deasserted);
  EncoderOutA.setSampleFrequency(500); // in (us)
  EncoderOutB.attach_asserted(&RotaryEncoderHandlerB_assert);
  EncoderOutB.attach_deasserted(&RotaryEncoderHandlerB_deasserted);
  EncoderOutB.setSampleFrequency(500); // in (us)
}

void InitLCDScreen()
{
  lcd.Clear(LCD_COLOR_BLACK);
}

void ClearStatusBar()
{
  lcd.SetTextColor(LCD_COLOR_BLACK);
  lcd.FillRect(0, LcdHeight - STATUS_BAR_HEIGHT, LcdWidth, STATUS_BAR_HEIGHT);
}

void UpdateStatusBar(char *Text)
{
  ClearStatusBar();
  lcd.SetBackColor(STATUS_BAR_COLOR);
  lcd.SetTextColor(STATUS_BAR_TEXTCOLOR);
  lcd.SetFont(&Font16);
  lcd.DisplayStringAt(5, 300, (uint8_t *)Text, LEFT_MODE);
}

void DrawMenuFrame()
{
  lcd.SetTextColor(MENU_FRAME_COLOR);
  lcd.FillRect(0, TITLE_BAR_HEIGHT, LcdWidth, LcdHeight - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
}

void DisplayMenuOptions(uint8_t menuOffset)
{
  // clear menu and redisplay
  DrawMenuFrame();
  lcd.SetBackColor(MENU_FRAME_COLOR);
  lcd.SetTextColor(MENU_FRAME_TEXTCOLOR);
  lcd.SetFont(&Font20);
  // display all visible menu items
  // but if total menu count is less than visible menu items, use totalMenuCount lower value
  for (int i = 0; i < (totalMenuCount < MENU_DISPLAY_COUNT ? totalMenuCount : MENU_DISPLAY_COUNT); i++)
  {
    lcd.DisplayStringAt(MENU_FRAME_PADDING, (TITLE_BAR_HEIGHT + MENU_FRAME_PADDING) + (MENU_FRAME_LINE_HEIGHT * i), (uint8_t *)MenuOptions[i + menuOffset].MenuText, LEFT_MODE);
  }
}

/*  This function highlights the currently selected menu option.
    This function is also aware if menus were shifted up or down   */
void HighlightMenuOption(uint8_t menuOffset, uint8_t position)
{
  // position starts at 1..MENU_DISPLAY_COUNT! ... except when total Menu count is less than MENU_DISPLAY_COUNT
  // Sanity check: position can never be > total menu count
  position = (position > totalMenuCount ? totalMenuCount : position);
  // NOTE: Text may have uneven padding, so we need to draw the highlight bkgd color manually
  lcd.SetTextColor(MENU_HIGHLIGHT_BACKCOLOR);
  lcd.FillRect(0, TITLE_BAR_HEIGHT + (MENU_FRAME_LINE_HEIGHT * (position - 1)), LcdWidth, MENU_FRAME_LINE_HEIGHT);
  // then print the text with a specific background color
  lcd.SetBackColor(MENU_HIGHLIGHT_BACKCOLOR);
  lcd.SetTextColor(MENU_HIGHLIGHT_TEXTCOLOR);
  lcd.DisplayStringAt(MENU_FRAME_PADDING, (TITLE_BAR_HEIGHT + MENU_FRAME_PADDING) + (MENU_FRAME_LINE_HEIGHT * (position - 1)), (uint8_t *)MenuOptions[menuOffset + position - 1].MenuText, LEFT_MODE);
}

/*  this function is the business logic for monitoring scroll position and 
    which menu item is currently highlighted and active       */
void UpdateDisplayMenu(uint8_t menuOffset, uint8_t position)
{
  // check if we need to shift up or down our menu options to show off-screen items
  // ex: pos=8, offset=0 ===> pos=7, offset=1

  // we need to check if totalMenuCount is less than MENU_DISPLAY_COUNT
  if ((position > MENU_DISPLAY_COUNT) && (position + menuOffset <= totalMenuCount))
  {
    position = MENU_DISPLAY_COUNT;
    menuOffset += 1;
  }
  // ex: pos=0, offset=1 ===> pos=1, offset=0
  if ((position < 1) && (menuOffset > 0))
  {
    position = 1;
    menuOffset -= 1;
  }

  // bounds checking of highlightt bar position
  if (position < 1)
  {
    position = 1;
  }
  if (position > MENU_DISPLAY_COUNT)
  {
    position = MENU_DISPLAY_COUNT;
  }
  // Sanity check: position can never be > total menu items
  if (position > totalMenuCount)
  {
    position = totalMenuCount;
  }

  // update global variables
  mainMenuPosition = position;
  mainMenuOffset = menuOffset;

  DisplayMenuOptions(menuOffset);
  HighlightMenuOption(menuOffset, position);
}

void DrawTitleBar(char *Title)
{
  lcd.SetTextColor(TITLE_BAR_COLOR);
  lcd.FillRect(0, 0, LcdWidth, TITLE_BAR_HEIGHT);
  lcd.SetTextColor(TITLE_BAR_TEXTCOLOR);
  lcd.SetBackColor(TITLE_BAR_COLOR);
  lcd.SetFont(&Font24);
  lcd.DisplayStringAt(0, 10, (uint8_t *)Title, CENTER_MODE);
}

int main()
{
  pc.baud(115200);
  pc.printf("%d menu items \n", totalMenuCount);

  InitRotaryEncoder();
  InitLCDScreen();

  // setup callbacks for different menu items
  MenuOptions[0].callback_function = &MenuOne_Function;
  MenuOptions[1].callback_function = &MenuTwo_Function;
  MenuOptions[2].callback_function = &MenuThree_Function;
  MenuOptions[3].callback_function = &MenuFour_Function;
  MenuOptions[4].callback_function = &MenuOne_Function;

  // draw title bar
  DrawTitleBar(PROGRAM_VERSION);
  DrawMenuFrame();

  // these next 2 functions control the logic of the menu display
  DisplayMenuOptions(mainMenuOffset);
  HighlightMenuOption(mainMenuOffset, mainMenuPosition);

  while (true)
  {
    // NOTE: in this main loop, there are no polling being done.
    // Rotary and switch state changes are monitored via interrupts

    // keep blinking led using blocking delay
    // so we can confirm that Interrupts are working
    led1 = !led1;
    wait(0.5);

    // update status bar with text, again using blocking delay
    UpdateStatusBar("Hello world!");
    wait(1.5);
    UpdateStatusBar("Sample text here... "); // it just gets truncated
    wait(1.5);
    UpdateStatusBar("This can be used to ");
    wait(1.4);
    UpdateStatusBar("show status messages.");
    wait(1.5);
  }
}