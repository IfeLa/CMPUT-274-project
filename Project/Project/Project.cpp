/* Iyunoluwa Lapite.
LBL EA1. Project: A game
A small character moves around a drawn maze and
"eats" the objects in its path*/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "lcd_image.h"

// standard U of A library settings, assuming Atmel Mega SPI pins
#define SD_CS    5  // Chip select line for SD card
#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define JOY_VERT_ANALOG 0
#define JOY_HORIZ_ANALOG 1
#define JOY_SEL 9
#define JOY_DEADZONE 64
#define JOY_CENTRE 512
#define JOY_STEPS_PER_PIXEL 64
#define MILLIS_PER_FRAME 50

#define TFT_WIDTH 128
#define TFT_HEIGHT 160

// initialize global variables to be used
int mode = 1;
int selected_mode;
int prev_selected_mode;
uint32_t cursor_x ;
uint32_t cursor_y ;
uint32_t prev_x ;
uint32_t prev_y ;
int cursor_update;
int update;
int eaten;

// declare functions;
void character(int x0,int y0);
void drawtext(char *text, uint16_t color);
void draw_fill(int x0,int y0,int w, int h,uint16_t color);
void game_0();
void game_1();
void eating(uint32_t cursor_x,uint32_t cursor_y);
bool valid(uint32_t cursor_x,uint32_t cursor_y);


// initialize and fill global arrays
// coordinates for the blocks of maze 1
uint32_t x0[] = {0,30,116,0,70,0,100,57,70,0,30,20,110,0,34};
uint32_t y0[] = {140,110,130,100,80,55,55,68,45,20,35,22,15,0,0};
uint32_t width0[] = {100,97,16,15,57,57,27,30,57,20,97,80,17,20,93};
uint32_t height0[] = {20,20,20,40,30,45,25,5,13,35,10,5,40,20,15};

//coordinates for the blocks of maze 2
uint32_t x1[] = {0,20,0,117,20,0,20,0,20,0};
uint32_t y1[] = {0,150,130,0,100,80,60,40,20,0};
uint32_t width1[] = {10,107,105,10,107,105,107,105,107,105};
uint32_t height1[] = {160,10,10,160,20,10,10,10,10,10};

// coordinates for objecs to be eaten in maze 1
int eatX0[] = {50,90,50,25};
int eatY0[] = {133,60,30,10};

//coordinates for objects to be eaten in maze 2
int eatX1[] = {30,30,30,30};
int eatY1[] = {145,72,52,32};


// setup for main function
void setup(){
  init();

  // initialize communication with serial port
  Serial.begin(9600);
  Serial.println("Serial.begin");

  // set the pin mode for the joystick
  pinMode(JOY_SEL,INPUT);
  digitalWrite(JOY_SEL,HIGH);
  //tft.initR(INITR_GREENTAB); // initialize a ST7735R chip, green tab
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  Serial.println("filled screen");
}

// function for printing text to the screen
void drawtext(char *text, uint16_t color) {
  tft.setTextSize(2);
  tft.setCursor(30, 80);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
  delay(1000);
  tft.fillScreen(ST7735_BLACK);
}

// function to build blocks of the maze
void draw_fill(int x0,int y0,int w, int h,uint16_t color){
  tft.drawRect(x0,y0,w,h,color);
  tft.fillRect(x0,y0,w,h,color);
}

//function to draw the character
void character(int x0,int y0){
  tft.drawRect(x0,y0,4,4,ST7735_WHITE);
  tft.fillRect(x0,y0,4,4,ST7735_MAGENTA);
}

// function to update the character when the joystick moves
void updateCharacter(){
  // prevent the character from moving beyond screen dimensions
  if (cursor_x  < 0 || cursor_x > TFT_WIDTH){
    cursor_x = prev_x;
  }
  if (cursor_y  < 0 || cursor_y > TFT_HEIGHT){
    cursor_y = prev_y;
  }

  // prevent the block from moving into blocks in maze 1
  if(selected_mode == 0){
    for(int b = 0;b < 15; b++){
      if((cursor_x+4) > x0[b] && ((cursor_x) < (x0[b] + width0[b]))){
        if((cursor_y+4) > y0[b] && ((cursor_y) < (y0[b] + height0[b]))){
          cursor_x = prev_x;
          cursor_y = prev_y;
          break;
        }
      }
    }
  }
  // prevent the block from moving into blocks in maze 2
  if(selected_mode == 1){
    for(int b = 0;b < 10; b++){
      if((cursor_x+4) > x1[b] && (cursor_x < (x1[b] + width1[b]))){
        if((cursor_y+4) > y1[b] && (cursor_y < (y1[b] + height1[b]))){
          cursor_x = prev_x;
          cursor_y = prev_y;
          break;
        }
      }
    }
  }

  // redraw character and update coordinates, also check if character has "eaten"
  character(cursor_x,cursor_y);
  eating(cursor_x,cursor_y);
  tft.fillRect(prev_x,prev_y,4,4,ST7735_BLACK);
  prev_x = cursor_x;
  prev_y = cursor_y;
  cursor_update = 0;
}

// function to set up the menu
void menu_setup() {
  tft.fillScreen(0);
  tft.setCursor(0, 0); // where the characters will be displayed
  tft.setTextWrap(false);
  tft.setTextSize(2);

  selected_mode= 0; // which restaurant is selected
  for (int i = 0; i < 2; i++) {
    if (i != selected_mode) { // not highlighted
      tft.setTextColor(0xFFFF, 0x0000); // white characters on black background
    } else { // highlighted
      tft.setTextColor(0x0000, 0xFFFF); // black characters on white background
    }
    tft.print("mode");
    tft.print(i);
    tft.print("\n");
  }
  tft.print("\n");
}

// function to sca the menu with the joystick
void scanMenuJoystick() {
  int vert = analogRead(JOY_VERT_ANALOG);
  int select = digitalRead(JOY_SEL);


  tft.setTextSize(2);
  if (select == 0) {
    if (selected_mode == 0){
      game_0();
    }
    if (selected_mode == 1){
      game_1();
    }
    mode = 0;
    return;
  }

  if (abs(vert - JOY_CENTRE) > JOY_DEADZONE) {

    if (vert > JOY_CENTRE && selected_mode != 1) {

      prev_selected_mode = selected_mode;
      ++selected_mode;

      tft.setTextColor(0x0000, 0xFFFF); // black characters on white background
      tft.setCursor(0, 16*selected_mode); // where the characters will be displayed
      tft.print("mode");
      tft.print(selected_mode);
      tft.setTextColor(0xFFFF, 0x0000); // white characters on black background
      tft.setCursor(0, 16*prev_selected_mode); // where the characters will be displayed
      tft.print("mode");
      tft.print(prev_selected_mode);

    } else if (vert < JOY_CENTRE && selected_mode != 0) {

      prev_selected_mode = selected_mode;
      --selected_mode;

      tft.setTextColor(0x0000, 0xFFFF); // black characters on white background
      tft.setCursor(0, 16*selected_mode); // where the characters will be displayed
      tft.print("mode");
      tft.print(selected_mode);
      tft.setTextColor(0xFFFF, 0x0000); // white characters on black background
      tft.setCursor(0, 16*prev_selected_mode); // where the characters
      tft.print("mode");
      tft.print(prev_selected_mode);
    }
  }
}

// function for controlling the effect of joystick movements
void scanJoystick() {
  int vert = analogRead(JOY_VERT_ANALOG);
  int horiz = analogRead(JOY_HORIZ_ANALOG);
  int select = digitalRead(JOY_SEL);

  if (abs(horiz - JOY_CENTRE) > JOY_DEADZONE) {

    int delta = abs((horiz - JOY_CENTRE) / JOY_STEPS_PER_PIXEL);

    if (horiz > JOY_CENTRE ) {
      cursor_x += delta;
    } else if (horiz < JOY_CENTRE ) {
      cursor_x -= delta;
    }

    cursor_update = 1;

  }

  if (abs(vert - JOY_CENTRE) > JOY_DEADZONE) {

    int delta = abs((vert - JOY_CENTRE) / JOY_STEPS_PER_PIXEL);

    if (vert > JOY_CENTRE) {
      cursor_y += delta;
    } else if ( vert < JOY_CENTRE){
      cursor_y -= delta;
    }

    cursor_update = 1;

  }
}

// function that allows character to "eat"
void eating(uint32_t cursor_x, uint32_t cursor_y){
  if(selected_mode == 0){
    for (int b = 0; b < 4; b++) {
      if((cursor_x+2) > eatX0[b] && ((cursor_x+2) < (eatX0[b]+5))){
        if((cursor_y-2) > eatY0[b] && ((cursor_y-2) <= (eatY0[b] + 5))){
          eaten = eaten + 1;
          tft.setCursor(0,0);
          tft.print(eaten);
        }
      }
    }
  }

  if(selected_mode == 1){
    for (int d = 0; d < 4; d++) {
      if((cursor_x+2) > eatX1[d] && ((cursor_x+2) < (eatX1[d] + 5))){
        if((cursor_y -2) > eatY1[d] && ((cursor_y - 2) <= (eatY1[d] + 5))){
          eaten = eaten + 1;
          tft.setCursor(0,0);
          tft.print(eaten);
        }
      }
    }
  }
}

// function that dictates what happens when the character reaches the end
void end(uint32_t cursor_x, uint32_t cursor_y){
  if(selected_mode == 0){
    if(cursor_x > 25 && cursor_x < (25 + 6)){
      if(cursor_y > 0 && cursor_y < (0 + 6)){
        tft.fillScreen(ST7735_BLACK);
        tft.setCursor(5, 30);
        tft.print("Score:");
        tft.print(2*eaten);
        delay(1000);

        mode = 1;
      }
    }
  }
  if(selected_mode == 1){
    if(cursor_x > 108 && cursor_x < (108 + 6)){
      if(cursor_y > 0 && cursor_y < (0 + 6)){
        tft.fillScreen(ST7735_BLACK);
        tft.setCursor(5,30);
        tft.print("Score:");
        tft.print(2*eaten);
        delay(1000);
        mode = 1;
      }
    }
  }
}

// function that builds maze 1
void game_0(){
  tft.fillScreen(ST7735_BLACK);
  character(120,155);
  for (int i = 0; i < 15; i++) {
    draw_fill(x0[i],y0[i],width0[i],height0[i],ST7735_RED);
  }
  for(int j = 0; j < 4; j++){
    draw_fill(eatX0[j],eatY0[j],5,5,ST7735_WHITE);
  }
  draw_fill(25,0,6,6,ST7735_MAGENTA);
}

// function that builds maze 2
void game_1(){
  tft.fillScreen(ST7735_BLACK);
  character(13,155);
  for (int a = 0; a < 10; a++) {
    draw_fill(x1[a],y1[a],width1[a],height1[a],ST7735_YELLOW);
  }
  for(int j = 0; j < 4; j++){
    draw_fill(eatX1[j],eatY1[j],5,5,ST7735_WHITE);
  }
  draw_fill(108,0,6,6,ST7735_MAGENTA);
}

// main function to call and order functions
int main(){
  setup();
  drawtext("WELCOME",ST7735_GREEN);
  prev_x = cursor_x;
  prev_y = cursor_y;
  while(true){
    if(mode == 1){
      menu_setup();
      while(mode == 1){
        scanMenuJoystick();
        if(selected_mode == 0){
          cursor_x  = 120;
          cursor_y = 155;
        }
        if(selected_mode == 1){
          cursor_x = 15;
          cursor_y = 155;
        }
      }
    }
    if(mode == 0){
      uint16_t begin = millis();
      uint16_t end_time;
      uint16_t start_time = millis();
      uint16_t cur_time;
      while(mode == 0){
        scanJoystick();
        if(cursor_update == 1){
          updateCharacter();
        }
        end(cursor_x,cursor_y);
        cur_time = millis();
        if ((cur_time - start_time) < 150){
          delay(150 - (cur_time - start_time));
        }
        start_time = millis();
      }
      end_time = millis();
      tft.fillScreen(ST7735_BLACK);
      tft.setTextSize(1.5);
      tft.setCursor(7,40);
      tft.print("Time taken:");
      tft.print(begin - end_time);
      tft.print("ms");
      delay(1000);
    }
  }
  return 0;
}
