
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include <Keypad.h>
// ---------------------------------------------------------------
// Hardware configuration
// ---------------------------------------------------------------

#define TFT_DC 18
#define TFT_CS 10

#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 17
#define TFT_MISO 12

#define TFT_LED 23

#define PHOTO_A 22
#define PHOTO_B 21
#define PHOTO_C 20

#define TOUCH_IRQ 15
#define TOUCH_CS 14
#define SD_CS 16

#define BUTTON_ROW_1 0
#define BUTTON_ROW_2 1
#define BUTTON_ROW_3 2
#define BUTTON_ROW_4 3
#define BUTTON_COL_1 5
#define BUTTON_COL_2 6
#define BUTTON_COL_3 7
#define BUTTON_COL_4 8

char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[4] = {BUTTON_ROW_1, BUTTON_ROW_2, BUTTON_ROW_3, BUTTON_ROW_4}; //connect to the row pinouts of the keypad
byte colPins[4] = {BUTTON_COL_1, BUTTON_COL_2, BUTTON_COL_3, BUTTON_COL_4}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, 4, 4 );

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);


// ---------------------------------------------------------------
// Color definitions configuration
// ---------------------------------------------------------------

#define COLOR_BLACK 0x0000       ///<   0,   0,   0
#define COLOR_NAVY 0x000F        ///<   0,   0, 123
#define COLOR_DARKGREEN 0x03E0   ///<   0, 125,   0
#define COLOR_DARKCYAN 0x03EF    ///<   0, 125, 123
#define COLOR_MAROON 0x7800      ///< 123,   0,   0
#define COLOR_PURPLE 0x780F      ///< 123,   0, 123
#define COLOR_OLIVE 0x7BE0       ///< 123, 125,   0
#define COLOR_LIGHTGREY 0xC618   ///< 198, 195, 198
#define COLOR_DARKGREY 0x7BEF    ///< 123, 125, 123
#define COLOR_BLUE 0x001F        ///<   0,   0, 255
#define COLOR_GREEN 0x07E0       ///<   0, 255,   0
#define COLOR_CYAN 0x07FF        ///<   0, 255, 255
#define COLOR_RED 0xF800         ///< 255,   0,   0
#define COLOR_MAGENTA 0xF81F     ///< 255,   0, 255
#define COLOR_YELLOW 0xFFE0      ///< 255, 255,   0
#define COLOR_WHITE 0xFFFF       ///< 255, 255, 255
#define COLOR_ORANGE 0xFD20      ///< 255, 165,   0
#define COLOR_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define COLOR_PINK 0xFC18        ///< 255, 130, 198



// ---------------------------------------------------------------
// Application configuration
// ---------------------------------------------------------------

int account_starting_value = 10000000;

typedef struct {
  int card_id;
  String card_color_name;
  uint16_t background_color;
  int account_balance;
} player_info;


player_info players[4] = {
  {1, "red", COLOR_RED, account_starting_value},
  {2, "green", COLOR_GREEN, account_starting_value},
  {3, "blue", COLOR_BLUE, account_starting_value},
  {4, "orange", COLOR_ORANGE, account_starting_value}  
};


typedef enum {
  None,
  Add,
  Subtract,
  Transfer,
} Mode;

Mode mode = Mode::None;

String input_string = "";

int inserted_card = 0; // 0 will mean there is no card inserted

boolean wastouched = true;

// ---------------------------------------------------------------
// Setup
// ---------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.println("ILI9341 Test!"); 

  pinMode( TFT_LED, OUTPUT);
  
  pinMode( SD_CS, OUTPUT);
//  pinMode( TOUCH_IRQ, OUTPUT);
//  pinMode( TOUCH_CS, OUTPUT);

  digitalWrite(SD_CS, HIGH);
//  digitalWrite(TOUCH_CS, HIGH);
//  digitalWrite(TOUCH_IRQ, LOW);

  pinMode( PHOTO_A, INPUT );
  pinMode( PHOTO_B, INPUT );
  pinMode( PHOTO_C, INPUT );

  attachInterrupt(digitalPinToInterrupt(PHOTO_A), card_inserted, CHANGE);
  
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
  
  analogWrite( TFT_LED, 100 );
  
  tft.begin(40000000 );

  tft.setRotation(1);
  draw_background();
  draw_accounts();
  draw_input_value();


}


void loop(void) {
  // update keypad.  actions are handled by the event listener
  char key = keypad.getKey();

  delay(100);
}


void draw_background(void) {
  tft.fillScreen(COLOR_BLACK);
  yield();
  tft.drawLine(0, 80, 320, 80, COLOR_WHITE);
}

void draw_accounts(void) {
  draw_account(  0,  0, COLOR_WHITE, players[0].background_color, players[0].account_balance);
  draw_account(160,  0, COLOR_WHITE, players[1].background_color, players[1].account_balance);
  draw_account(  0, 40, COLOR_WHITE, players[2].background_color, players[2].account_balance);
  draw_account(160, 40, COLOR_WHITE, players[3].background_color, players[3].account_balance);
}

void draw_account(int16_t x0, int16_t y0, uint16_t text_color, uint16_t background_color, float account_value ) {
  tft.fillRect(x0, y0, 160, 40, background_color);
  yield();
  tft.setCursor(x0+4, y0+12);
  tft.setTextColor(text_color);
  if (account_value < 99999999999) {
    tft.setTextSize(2);  
  } else {
    tft.setTextSize(1);
  }
  
  tft.print(account_value, 0);
  yield();
}

void draw_card_id() {
  tft.fillRect(0, 90, 320, 40, COLOR_BLACK);
  yield();
  tft.setCursor(4, 90+12);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(4);  

  tft.print(players[inserted_card-1].card_color_name);
  yield();
}


void clear_card_id() {
  Serial.println("clear screen");
  tft.fillRect(0, 90, 320, 40, COLOR_BLACK);
  yield();
}


void card_inserted() {
//  state = !state;
  int val_a = digitalRead(PHOTO_A);
  int val_b = digitalRead(PHOTO_B);
  int val_c = digitalRead(PHOTO_C);

  inserted_card = val_b*2+val_c+1;

  if (val_a == 1) {
    clear_card_id();  
  } else {
    draw_card_id();
  }
}


void draw_mode() {
  tft.fillRect(0, 130, 320, 40, COLOR_BLACK);
  yield();
  tft.setCursor(4, 130+12);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(4);  
  
  switch (mode) {
    case Mode::Add: 
      tft.print("Add");
      break;
    case Mode::Subtract: 
      tft.print("Subtract");
      break;
    case Mode::Transfer: 
      tft.print("Transfer");
      break;
    case Mode::None: 
      break;
    default:
       break;
  }
  yield();
}

void draw_input_value() {
  tft.fillRect(0, 170, 320, 40, COLOR_BLACK);
  yield();
  tft.setCursor(4, 170+12);
  tft.setTextColor(COLOR_WHITE);
  
  if (input_string.length() <= 12) {
    tft.setTextSize(4);
  } else if (input_string.length() <= 17) {
    tft.setTextSize(3);
  } else if (input_string.length() <= 26) {
    tft.setTextSize(2);
  } else {
    tft.setTextSize(1);
  }
  
  tft.print(input_string);
  yield();
}

void apply_change() {

  switch (mode) {
    case Mode::Add:
      players[inserted_card-1].account_balance += input_string.toInt();
      break;
      
    case Mode::Subtract:
      players[inserted_card-1].account_balance -= input_string.toInt();
      break;
      
    case Mode::Transfer: 
      // TO DO
      break;
      
    default:
      break;
  }
  
  draw_accounts();
  
  set_mode(Mode::None);

  clear_input_value();
}



// Taking care of some special events.
void keypadEvent(KeypadEvent key){
  
    switch (keypad.getState()){
    case PRESSED:
        if (key == 'A') {
          set_mode(Mode::Add);
        } else if (key == 'B') {
          set_mode(Mode::Subtract);
        } else if (key == 'C') {
          set_mode(Mode::Transfer);
        } else if (key == 'D') {
          set_mode(Mode::None);
        } else if (key == '*') {
          clear_input_value();
        } else if (key == '#') {
          apply_change();
        } else {
          input_string += key;
          draw_input_value();
        }
        break;

    case RELEASED:
      break;
      
    case HOLD:
      break;
      
    case IDLE:
      break;
    }
    
}


void set_mode(Mode m) {
  mode = m;
  draw_mode();
}


void clear_input_value() {
  input_string = "";
  draw_input_value();
}
