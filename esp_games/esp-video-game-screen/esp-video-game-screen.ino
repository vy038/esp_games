#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int bigMeteorCount = 2;
const int MeteorCount = 2;
const int smallMeteorCount = 2;
#define XPOS 0
#define YPOS 1
#define DELTAY 2

// 'ship', 11x11px
const unsigned char epd_bitmap_ship [] PROGMEM = {
	0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x4e, 0x40, 0x4e, 0x40, 0xff, 0xe0, 0x7f, 0xc0, 
	0x3f, 0x80, 0x2e, 0x80, 0x04, 0x00
};

// 'meteoroid1', 23x21px
const unsigned char epd_bitmap_meteoroid1 [] PROGMEM = {
	0x01, 0x80, 0x00, 0x02, 0xfe, 0x00, 0x03, 0xff, 0xe0, 0x0f, 0xff, 0xf8, 0x3f, 0xfb, 0xf8, 0x7c, 
	0x7f, 0xfc, 0xfe, 0x3f, 0xfe, 0x7e, 0xfe, 0x7e, 0xfe, 0xbe, 0x3e, 0x9f, 0x7f, 0xf8, 0xcf, 0xf7, 
	0xf8, 0xff, 0xef, 0xde, 0xff, 0xff, 0x9e, 0xfd, 0xff, 0xbe, 0xfe, 0xff, 0xfe, 0x7f, 0xff, 0xfa, 
	0x3f, 0xfd, 0xfc, 0x1f, 0xff, 0xfc, 0x0f, 0xff, 0xe8, 0x03, 0xdf, 0xe0, 0x00, 0xff, 0x40
};
// 'meteoroid2', 16x14px
const unsigned char epd_bitmap_meteoroid2 [] PROGMEM = {
	0x01, 0xfc, 0x1f, 0xc4, 0x3f, 0xfe, 0x73, 0xff, 0x77, 0xcf, 0xff, 0xcd, 0x9e, 0xff, 0x9c, 0xfe, 
	0xff, 0xe6, 0x7f, 0xfe, 0x37, 0xfc, 0x07, 0x78, 0x0f, 0x70, 0x07, 0xf0
};
// 'meteoroid3', 10x9px
const unsigned char epd_bitmap_meteoroid3 [] PROGMEM = {
	0x07, 0xc0, 0x1e, 0xc0, 0x7d, 0xc0, 0x3f, 0xc0, 0xf3, 0xc0, 0xff, 0x40, 0xdf, 0xc0, 0x1d, 0xc0, 
	0x1c, 0x00
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 176)
const int epd_bitmap_meteoroids_LEN = 3;
const unsigned char* epd_bitmap_meteoroids[3] = {
	epd_bitmap_meteoroid1,
	epd_bitmap_meteoroid2,
	epd_bitmap_meteoroid3
};

void setup() {
  Serial.begin(9600);

  Wire.begin(22, 23);

  //OLED STUFF
  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
  display.display();
  delay(2000);
  // Clear the buffer.
  display.clearDisplay();

  //OLED ready screen display
  display.setCursor(27, 28);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.println("Ready?");
  display.display();
  delay(2000);
  display.clearDisplay();





  //character(epd_bitmap_ship, 64, 58);

  game(epd_bitmap_meteoroid1, epd_bitmap_meteoroid2, epd_bitmap_meteoroid3, 2, 4, 6);
  //display.drawBitmap(random(display.width()), 0, epd_bitmap_meteoroid1, 23, 21, SH110X_WHITE);
}


void loop() {
}

void character(const uint8_t *bitmap, int posX, int posY) {
  display.drawBitmap(posX - 6, posY - 6, bitmap, 11, 11, SH110X_WHITE);
  //display.display();
}

void game(const uint8_t *bitmap1, const uint8_t *bitmap2, const uint8_t *bitmap3, int speed1, int speed2, int speed3) {
  uint8_t bigMeteors[bigMeteorCount][3];
  uint8_t Meteors[MeteorCount][3];
  uint8_t smallMeteors[smallMeteorCount][3];

  // initialize
  for (uint8_t f = 0; f < bigMeteorCount; f++) {
    bigMeteors[f][XPOS] = random(display.width());
    bigMeteors[f][YPOS] = 0;
    bigMeteors[f][DELTAY] = speed1;
  }
  for (uint8_t f = 0; f < MeteorCount; f++) {
    Meteors[f][XPOS] = random(display.width());
    Meteors[f][YPOS] = 0;
    Meteors[f][DELTAY] = speed2;
  }
  for (uint8_t f = 0; f < smallMeteorCount; f++) {
    smallMeteors[f][XPOS] = random(display.width());
    smallMeteors[f][YPOS] = 0;
    smallMeteors[f][DELTAY] = speed3;
  }

  while (1) {
    // draw each icon
    for (uint8_t f = 0; f < bigMeteorCount; f++) {
      display.drawBitmap(bigMeteors[f][XPOS], bigMeteors[f][YPOS], bitmap1, 23, 21, SH110X_WHITE);
    }

    for (uint8_t f = 0; f < MeteorCount; f++) {
      display.drawBitmap(Meteors[f][XPOS], Meteors[f][YPOS], bitmap2, 16, 14, SH110X_WHITE);
    }

    for (uint8_t f = 0; f < smallMeteorCount; f++) {
      display.drawBitmap(smallMeteors[f][XPOS], smallMeteors[f][YPOS], bitmap3, 10, 9, SH110X_WHITE);
    }

    character(epd_bitmap_ship, 64, 58);
    display.display();
    delay(100);

    // then erase it + move it
    for (uint8_t f = 0; f < bigMeteorCount; f++) {
      display.drawBitmap(bigMeteors[f][XPOS], bigMeteors[f][YPOS], bitmap1, 23, 21, SH110X_BLACK);
      // move it
      bigMeteors[f][YPOS] += bigMeteors[f][DELTAY];
      // if its gone, reinit
      if (bigMeteors[f][YPOS] > display.height()) {
        bigMeteors[f][XPOS] = random(display.width());
        bigMeteors[f][YPOS] = 0;
        bigMeteors[f][DELTAY] = speed1;
      }
    }

    // then erase it + move it
    for (uint8_t f = 0; f < MeteorCount; f++) {
      display.drawBitmap(Meteors[f][XPOS], Meteors[f][YPOS], bitmap2, 16, 14, SH110X_BLACK);
      // move it
      Meteors[f][YPOS] += Meteors[f][DELTAY];
      // if its gone, reinit
      if (Meteors[f][YPOS] > display.height()) {
        Meteors[f][XPOS] = random(display.width());
        Meteors[f][YPOS] = 0;
        Meteors[f][DELTAY] = speed2;
      }
    }

    // then erase it + move it
    for (uint8_t f = 0; f < smallMeteorCount; f++) {
      display.drawBitmap(smallMeteors[f][XPOS], smallMeteors[f][YPOS], bitmap3, 10, 9, SH110X_BLACK);
      // move it
      smallMeteors[f][YPOS] += smallMeteors[f][DELTAY];
      // if its gone, reinit
      if (smallMeteors[f][YPOS] > display.height()) {
        smallMeteors[f][XPOS] = random(display.width());
        smallMeteors[f][YPOS] = 0;
        smallMeteors[f][DELTAY] = speed3;
      }
    }
  }
}
