/*
  ESP Galga
  By: Victor Yu
  Created: July 6, 2024
  Modified: July 8, 2024

  A simple galga based game for the ESP32-WROOM with an OLED display, and input from buttons and joysticks. 
  Made with the intent on making the game more customizable.
*/

// import libraries needed 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>
#include <Vector.h>
#include <Streaming.h>

// configuring OLED display

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
// #define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// meteor things for array
#define XPOS 0
#define YPOS 1
#define DELTAY 2

// pins for input on controller
const int buttonA = 14;
const int buttonB = 34;
const int joyX = 35;
const int joyY = 32;
const int speakerPin = 18;

// game things

// game variables (whether or not game is over, and score)
int gameOver = 0;
int score = 0;
const int winningScore = 1000;

// customizable 

// how many of each meteor there is (define size of the 2D array)
const int bigMeteorCount = 2;
const int MeteorCount = 2;
const int smallMeteorCount = 2;

// meteor speeds
int speed1 = 2;
int speed2 = 4;
int speed3 = 6;

// how likely (in %) per refresh the meteors are to speed up
const int rampUp = 1;

// refresh rate (or speed of game) in ms
const int refresh = 75;

// initial character position variables
int charY = 64;
int charX = 64;

// bullet speed
const int bulletSpeed = 12;

// define a vector
typedef Vector<int> Elements;

// bitmaps (art) for the game

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
// 'bullet', 3x4px
const unsigned char epd_bitmap_bullet [] PROGMEM = {
	0x40, 0xe0, 0xe0, 0xa0
};

// create vectors for positions of bullets
Elements bulletsX;
Elements bulletsY;

void setup() {
  // serial monitor for troubleshooting
  Serial.begin(9600);

  // configure vectors for bullet positions
  int storage_array[10];
  int storage_array2[10];
  bulletsX.setStorage(storage_array);
  bulletsY.setStorage(storage_array2);

  // start I2C protocol pins for OLED
  Wire.begin(22, 23);

  // configure input pin modes
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(speakerPin, OUTPUT);

  // OLED STUFF
  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  // display.setContrast (0); // dim display
  display.display();
  delay(2000);
  // Clear the buffer.
  display.clearDisplay();
  // start game function
  game(epd_bitmap_meteoroid1, epd_bitmap_meteoroid2, epd_bitmap_meteoroid3);
}


void loop() {
  if (digitalRead(buttonB) == 0) { // just for button b to reset game, can remove later
    gameOver = 0;
    score = 0;
    speed1 = 2;
    speed2 = 4;
    speed3 = 6;
    charY = 64;
    charX = 64;
    setup();
  }
}


void game(const uint8_t *bitmap1, const uint8_t *bitmap2, const uint8_t *bitmap3) {
  // intro screen with sfx
  display.setCursor(32, 28);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.println("GALGA");
  display.display();
  tone(speakerPin, 523.25, 750);
  delay(2000);
  display.clearDisplay();

  // OLED ready screen display with sfx
  display.setCursor(27, 28);
  display.println("Ready?");
  display.display();
  tone(speakerPin, 261.63, 200);
  delay(200);
  tone(speakerPin, 392, 600);
  delay(600);
  delay(1200);
  tone(speakerPin, 523.25, 200);
  display.clearDisplay();



  // initialize arrays to hold each meteor position (big, medium, and small)
  uint8_t bigMeteors[bigMeteorCount][3];
  uint8_t Meteors[MeteorCount][3];
  uint8_t smallMeteors[smallMeteorCount][3];

  // fill arrays to hold each meteor position (big, medium, and small)
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

  while (true) { // logic of the game
    if (gameOver == 0) { // if game is not over, run game

      // clear previous things on display
      display.clearDisplay();

      // configure input from buttons and joystick
      int joyYState = analogRead(joyX);
      int joyXState = analogRead(joyY);
      joyYState = map(joyYState, 0, 4095, -4, 4);
      joyXState = map(joyXState, 0, 4095, 4, -4);
      int buttonAState = digitalRead(buttonA);
      int buttonBState = digitalRead(buttonB);

      // draw each big, medium, and small meteor
      for (uint8_t f = 0; f < bigMeteorCount; f++) {
        display.drawBitmap(bigMeteors[f][XPOS], bigMeteors[f][YPOS], bitmap1, 23, 21, SH110X_WHITE);
      }
      for (uint8_t f = 0; f < MeteorCount; f++) {
        display.drawBitmap(Meteors[f][XPOS], Meteors[f][YPOS], bitmap2, 16, 14, SH110X_WHITE);
      }
      for (uint8_t f = 0; f < smallMeteorCount; f++) {
        display.drawBitmap(smallMeteors[f][XPOS], smallMeteors[f][YPOS], bitmap3, 10, 9, SH110X_WHITE);
      }

      // movement of character (proportional to how much the joystick is moved)
      charY += round(joyYState*1.5);
      charX += round(joyXState*1.5);
      
      // boundries so character cannot escape screen
      if (charY < 0) {
        charY = 0;
      } else if (charY > 64) {
        charY = 64;
      }
      if (charX < 0) {
        charX = 0;
      } else if (charX > 128) {
        charX = 128;
      }

      // create character
      display.drawBitmap(charX - 6, charY - 6, epd_bitmap_ship, 11, 11, SH110X_WHITE);

      // firing, create a bullet with coordinate x and y stored in vector
      if (buttonAState == 0) {
        bulletsX.push_back(charX - 2);
        bulletsY.push_back(charY);
        tone(speakerPin, 261.63, 50);
      }

      /* debugging
      Serial << charX<< " " << charY << endl;
      Serial << "bulletsY.max_size(): " << bulletsY.max_size() << endl;
      Serial << bulletsX << " " << bulletsY << endl;
      */

      // draw every bullet
      for (int i = 0; i < bulletsY.size(); i++) {
        display.drawBitmap(bulletsX[i], bulletsY[i], epd_bitmap_bullet, 3, 4, SH110X_WHITE);
      }

      // display and refresh
      display.display();
      delay(refresh);

      // move each bullet after
      for (int i = 0; i < bulletsY.size(); i++) {
        if (bulletsY[i] < 0) {
          bulletsY.remove(i);
          bulletsX.remove(i);
        }
        bulletsY[i] -= bulletSpeed;
      }

      // moving meteors
      for (uint8_t f = 0; f < bigMeteorCount; f++) {
        // moving meteor
        bigMeteors[f][YPOS] += bigMeteors[f][DELTAY];
        // if its gone,make another
        if (bigMeteors[f][YPOS] > display.height()) {
          bigMeteors[f][XPOS] = random(display.width());
          bigMeteors[f][YPOS] = 0;
          bigMeteors[f][DELTAY] = speed1;
          score += 1;
        }
        
        // check to see if player is touching meteor, game over if so
        if ((bigMeteors[f][XPOS] + 23 >= charX - 5 && bigMeteors[f][XPOS] <= charX + 5) && (bigMeteors[f][YPOS] + 21 >= charY - 5 && bigMeteors[f][YPOS] <= charY + 5)) {
          gameOver = 1;
        }

        // checks to see if any bullet is touching paticular meteor
        for (int i = 0; i < bulletsY.size(); i++) {
          if ((bigMeteors[f][XPOS] + 23 >= bulletsX[i] && bigMeteors[f][XPOS] <= bulletsX[i] + 3) && (bigMeteors[f][YPOS] + 21 >= bulletsY[i] && bigMeteors[f][YPOS] <= bulletsY[i] + 4)) {
            bigMeteors[f][XPOS] = random(display.width());
            bigMeteors[f][YPOS] = 0;
            bigMeteors[f][DELTAY] = speed1;
            bulletsY.remove(i);
            bulletsX.remove(i);
            score += 3;
          }
        }
      }


      for (uint8_t f = 0; f < MeteorCount; f++) { // looks through all meteors of paticular type
        // move it
        Meteors[f][YPOS] += Meteors[f][DELTAY];

        // if its gone, make another
        if (Meteors[f][YPOS] > display.height()) {
          Meteors[f][XPOS] = random(display.width());
          Meteors[f][YPOS] = 0;
          Meteors[f][DELTAY] = speed2;
          score += 1;
        }

        // game over once meteor touches player
        if ((Meteors[f][XPOS] + 16 >= charX - 5 && Meteors[f][XPOS] <= charX + 5) && (Meteors[f][YPOS] + 14 >= charY - 5 && Meteors[f][YPOS] <= charY + 5)) {
          gameOver = 1;
        }

        // check if any bullets are touching paticular meteor
        for (int i = 0; i < bulletsY.size(); i++) {
          if ((Meteors[f][XPOS] + 16 >= bulletsX[i] && Meteors[f][XPOS] <= bulletsX[i] + 3) && (Meteors[f][YPOS] + 14  + 21 >= bulletsY[i] && Meteors[f][YPOS] <= bulletsY[i] + 4)) {
            Meteors[f][XPOS] = random(display.width());
            Meteors[f][YPOS] = 0;
            Meteors[f][DELTAY] = speed2;
            bulletsY.remove(i);
            bulletsX.remove(i);
            score += 3;
          }
        }
      }

      for (uint8_t f = 0; f < smallMeteorCount; f++) { // looks through all meteors of paticular type
        // moving meteors
        smallMeteors[f][YPOS] += smallMeteors[f][DELTAY];
        // if its gone, make another
        if (smallMeteors[f][YPOS] > display.height()) {
          smallMeteors[f][XPOS] = random(display.width());
          smallMeteors[f][YPOS] = 0;
          smallMeteors[f][DELTAY] = speed3;
          score += 1;
        }

        // game over once meteor touches player
        if ((smallMeteors[f][XPOS] + 10 >= charX - 5 && smallMeteors[f][XPOS] <= charX + 5) && (smallMeteors[f][YPOS] + 9 >= charY - 5 && smallMeteors[f][YPOS] <= charY + 5)) {
          gameOver = 1;
        }

        // check if any bullets are touching paticular meteor
        for (int i = 0; i < bulletsY.size(); i++) {
          if ((smallMeteors[f][XPOS] + 10 >= bulletsX[i] && smallMeteors[f][XPOS] <= bulletsX[i] + 3) && (smallMeteors[f][YPOS] + 9 >= bulletsY[i] && smallMeteors[f][YPOS] <= bulletsY[i] + 4)) {
            smallMeteors[f][XPOS] = random(display.width());
            smallMeteors[f][YPOS] = 0;
            smallMeteors[f][DELTAY] = speed3;
            bulletsY.remove(i);
            bulletsX.remove(i);
            score += 3;
          }
        }
      }

      // rampUp% chance with each refresh to make meteors move faster
      if (rand() % 100 > (100 - rampUp)) {
        speed1 += 1;
        speed2 += 1;
        speed3 += 1;
      }

      if (score >= winningScore) {
        gameOver = 2; // game win condition (over 1000 score)
      }

      /* debugging
      Serial.print(joyYState);
      Serial.print(" ");
      Serial.print(joyXState);
      Serial.print(" ");
      Serial.print(buttonAState);
      Serial.print(" ");
      Serial.println(buttonBState);
      */

    } else { // break from game, game ends
      break;
    }
  }
  if (gameOver == 1) {
    // lose screen
    display.clearDisplay();
    display.setCursor(12, 26);
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.println("Game Over");
    display.display();
    tone(speakerPin, 523.25, 200);
    delay(200);
    tone(speakerPin, 392, 200);
    delay(200);
    tone(speakerPin, 261.63, 200);
    delay(200);
  } else {
    // win screen
    display.clearDisplay();
    display.setCursor(12, 26);
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.println("YOU WIN!");
    display.display();
    tone(speakerPin, 293.66, 200);
    delay(200);
    tone(speakerPin, 440, 200);
    delay(200);
    tone(speakerPin, 554.37, 200);
    delay(200);
  }

  // display score
  delay(2400);
  display.clearDisplay();
  display.setCursor(3, 26);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  if (score > 1000) {
    display.print("Score:");
  } else {
    display.print("Score: ");
  }
  display.print(score);
  display.display();
  tone(speakerPin, 261.63, 200);
  delay(200);
}
