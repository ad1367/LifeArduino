const int ACCpin = A0;
const int BLUELED = 2;
const int REDLED = 4;
const int PiezoPin = 7;
float ACC;
float ACCDC;
int rest;
int MAX = 6; //acceleration allowed before considering it a fall
int ACCMAX = 0;
int ACCMIN = 0;
int dACC;
int SoundAlert1;
int SoundAlert2;
boolean RecordOn = false;

#define NOTE_C4  262
#define NOTE_G3  196

//define soundalert melody
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_C4, NOTE_G3, NOTE_C4, NOTE_G3, 0
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 4, 4, 4, 4, 4, 4
};

//include Adafruit and touchscreen libraries
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

// calibration data for touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define FRAME_X 62
#define FRAME_Y 50
#define FRAME_W 200
#define FRAME_H 150

#define REDBUTTON_X FRAME_X
#define REDBUTTON_Y FRAME_Y
#define REDBUTTON_W (FRAME_W/2)
#define REDBUTTON_H FRAME_H

#define GREENBUTTON_X (REDBUTTON_X + REDBUTTON_W)
#define GREENBUTTON_Y FRAME_Y
#define GREENBUTTON_W (FRAME_W/2)
#define GREENBUTTON_H FRAME_H

void drawFrame()
{
  tft.drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, ILI9341_BLACK);
}

void redBtn()
{
  tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, ILI9341_RED);
  tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, ILI9341_BLUE);
  drawFrame();
  tft.setCursor(GREENBUTTON_X + 6 , GREENBUTTON_Y + (GREENBUTTON_H / 2));
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("ARE YOU OKAY");
  RecordOn = false;
}

void greenBtn()
{
  tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, ILI9341_GREEN);
  tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, ILI9341_BLUE);
  drawFrame();
  tft.setCursor(REDBUTTON_X + 6 , REDBUTTON_Y + (REDBUTTON_H / 2));
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("YES");
  RecordOn = true;
}

void setup() {
  Serial.begin(9600);
  pinMode(ACCpin, INPUT);
  digitalWrite(BLUELED, LOW);
  digitalWrite(REDLED, LOW);

  // initialize touchscreen
  tft.begin();
  if (!ts.begin()) {
    Serial.println("Unable to start touchscreen.");
  }
  else {
    Serial.println("Touchscreen started.");
  }

  tft.fillScreen(ILI9341_BLUE);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(1);
  redBtn();
}

void loop() {
  //reset LEDs to off and variables to reset position
  digitalWrite(BLUELED, LOW);
  digitalWrite(REDLED, LOW);
  SoundAlert1 = 0;
  SoundAlert2 = 0;

  // See if there's any  touch data for us
  if (!ts.bufferEmpty())
  {
    // Retrieve a point
    TS_Point p = ts.getPoint();
    // Scale using the calibration #'s
    // and rotate coordinate system
    p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
    p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
    int y = tft.height() - p.x;
    int x = p.y;
  }

  if (RecordOn)  //what happens if the button is not pressed
  {
    if ((x > REDBUTTON_X) && (x < (REDBUTTON_X + REDBUTTON_W))) {
      if ((y > REDBUTTON_Y) && (y <= (REDBUTTON_Y + REDBUTTON_H))) {
        Serial.println("Red btn hit");
        redBtn();
      }
    }
  }
  else //Screen goes green when button is pressed
  {
    if ((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W))) {
      if ((y > GREENBUTTON_Y) && (y <= (GREENBUTTON_Y + GREENBUTTON_H))) {
        Serial.println("Green btn hit");
        greenBtn();
      }
    }
  }

  //read and print acceleration data
  rest = analogRead(ACCpin);
  ACC = analogRead(ACCpin);
  ACCDC = ACC - rest;
  Serial.print("ACCDC: ");
  Serial.println(ACCDC);

  // if acceleration trips a maximum magnitude, start fall timer
  if (abs(ACCDC) > MAX) {
    digitalWrite(BLUELED, HIGH);
    digitalWrite(REDLED, LOW);

    /* for 10 seconds after a potential fall has been detected,
           sense for stillness of movement that eliminates walking
           as a potential cause of  acceleration */
    for (int i = 0; i <= 300; i++) {
      // continue printing Acceleration
      rest = analogRead(ACCpin);
      ACC = analogRead(ACCpin);
      ACCDC = ACC - rest;
      Serial.print("ACCDC: ");
      Serial.println(ACCDC);

      //After a fall occurs, reset fall variables
      if (i == 10) {
        dACC = 0;
        ACCMAX = 0;
        ACCMIN = 0;
      }

      //Calculate maximum and minimum acceleration
      if (ACCDC > ACCMAX) {
        ACCMAX = ACCDC;
      }
      if (ACCDC < ACCMIN) {
        ACCMIN = ACCDC;
      }
      dACC = ACCMAX - ACCMIN;
      Serial.print("Diff ACC : ");
      Serial.println(dACC);
    }
  }
  /*if the movement after fall is too little and touchscreen
     is not touched, allow sound alert */
  if (dACC < 10) {
    SoundAlert1 = 1;
  }

  //if movement has been detected the code resets, sound disables
  else {
    digitalWrite(REDLED, LOW);
    digitalWrite(BLUELED, LOW);
    SoundAlert1 = 0;
  }

  //If the screen has been touched the code resets, sound disables
  if (RecordOn == false) {
    digitalWrite(REDLED, LOW);
    digitalWrite(BLUELED, LOW);
    SoundAlert1 = 0;
  }
  
  // If the screen hasn't been touched, enable the sound alert
  else {
    SoundAlert2 = 1;
  }
  
//if both no movement is detected and the screen isn't touched,
  if (SoundAlert1 + SoundAlert2 == 2) {
    //enact Piezo Sound Alert
    tone(PiezoPin, melody[])
  }
  
  //Turn off speaker before repeating
  noTone(PiezoPin);
  delay(10);
}
