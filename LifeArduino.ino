const int ACCpin = A0;
const int BLUELED = 2;
const int REDLED = 4;
float ACC;
float ACCDC;
int rest;
int MAX = 6;
int ACCMAX = 0;
int ACCMIN = 0;
int dACC;

void setup() {
  Serial.begin(9600);
  pinMode(ACCpin, INPUT);
  digitalWrite(BLUELED, LOW);
  digitalWrite(REDLED, LOW);
}

void loop() {
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
    /* if the person is at rest for 10 seconds after jolt,
       second LED will turn on, signaling an alert */
    if (dACC < 10) {
      digitalWrite(REDLED, HIGH);
    }
    else {
      digitalWrite(REDLED, LOW);
      digitalWrite(BLUELED, LOW);
    }
  }
  delay(10);
}
