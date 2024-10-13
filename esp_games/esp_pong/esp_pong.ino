/*
  ESP Pong
  By: Victor Yu
  Created: July 20, 2024
  Modified: August 25, 2024

  A simple pong based game for the ESP32-WROOM with an OLED display, and input from buttons and joysticks. 
  Made with the intent on making the game more customizable.
*/

// import libraries needed 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Streaming.h>
#include <Arduino.h>

// configuring OLED display

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
// #define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// pins for input on controller
const int buttonA = 14;
const int buttonB = 34;
const int joyX = 15;
const int joyY = 4;
const int speakerPin = 18;

// game things

// game variables (whether or not game is over, and score)
int gameOver = 0;
int score = 0;
const int winningScore = 50;

// how likely (in %) per refresh the meteors are to speed up
const int rampUp = 1;

// refresh rate (or speed of game) in ms
const int refresh = 25;

// initial character position variables
int charY = 58;
int charX = 64;

int ballX = 64;
int ballY = 10;

int speed = 4;

int speedX;
int speedY;

// bitmaps (art) for the game

// 'pong board', 14x3px
const unsigned char epd_bitmap_pong_board [] PROGMEM = {
	0xff, 0xfc, 0x7f, 0xf8, 0x1f, 0xe0
};

// 'ball', 4x4px
const unsigned char epd_bitmap_ball [] PROGMEM = {
	0x60, 0xf0, 0xf0, 0x60
};

void setup() {
  // serial monitor for troubleshooting
  Serial.begin(9600);

  // configure input pin modes
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(speakerPin, OUTPUT);

  // start I2C protocol pins for OLED
  Wire.begin(22, 23);

  // OLED STUFF
  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  // display.setContrast (0); // dim display
  display.display();
  delay(2000);
  // Clear the buffer.
  display.clearDisplay();

  // intro screen with sfx
  display.setCursor(36, 28);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.println("PONG");
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

  // start game function
  game();
}


void loop() {} // empty


void game() {

  speedX = rand() % speed + 1;
  speedY = rand() % speed + 1;

  while (true) { // logic of the game
    if (gameOver == 0) { // if game is not over, run game


      // clear previous things on display
      display.clearDisplay();

      // configure input from buttons and joystick
      int joyYState = analogRead(joyX);
      int joyXState = analogRead(joyY);


      Serial.print(speedX);
      Serial.print(" ");
      Serial.println(speedY);


      Serial.println(joyXState);
      joyYState = map(joyYState, 0, 4095, -4, 4);
      joyXState = map(joyXState, 0, 4095, 4, -4);

      ballX += speedX;
      ballY += speedY;

      display.drawBitmap(ballX, ballY, epd_bitmap_ball, 4, 4, SH110X_WHITE);

      // movement of character (proportional to how much the joystick is moved)
      charX += round(joyXState*1.5);
      
      // boundries so character cannot escape screen
      if (charX < 0) {
        charX = 0;
      } else if (charX > 128) {
        charX = 128;
      }

      // create character
      display.drawBitmap(charX - 7, charY - 1, epd_bitmap_pong_board, 14, 3, SH110X_WHITE);

      // display and refresh
      display.display();
      delay(refresh);

      if (ballX + 4 >= 128 || ballX <= 0) {
        ballX = ballX - speedX;
        speedX = -speedX;
      }

      if (ballY <= 0) {
        ballY = ballY - speedY;
        speedY = -speedY;
      }

      if (ballY + 4 >= 64) {
        gameOver = 1;
      }

      if (score >= winningScore) {
        gameOver = 2; // game win condition (over 50 score)
      }



      // rampUp% chance with each refresh to make meteors move faster
      if (rand() % 100 > (100 - rampUp)) {
        speed += 1;
      }

      if ((ballX + 4 >= charX - 7 && ballX <= charX + 7) && (ballY + 4 >= charY - 1 && ballY <= charY + 1)) {
        ballY = ballY - speedY;
        speedY = -speedY;
        score += 1;
      }

      if (score >= winningScore) {
        gameOver = 2; // game win condition (over 1000 score)
      }

      
      Serial.println(joyYState);
      

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
