//The code for a simple padlock that locks the kitchen backdoor so that the students can't reach the candy.
const byte ROWS = 4;
const byte COLS = 4;

const byte rowPins[ROWS] = {2, 3, 4, 5};
const byte colPins[COLS] = {6, 7, 8, 9};
const byte unlockPin = 13;

const byte pass_leng = 5;
int password[pass_leng] = {3, 7, 11, 15, 14};
int lastPresses[pass_leng] = {-1, -1, -1, -1, -1};
int pressIndex = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize rows as outputs
  for (byte i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH); // Set to HIGH (inactive)
  }

  // Initialize columns as inputs with pullups
  for (byte j = 0; j < COLS; j++) {
    pinMode(colPins[j], INPUT_PULLUP);
  }
  
  //the pin that opens the lock.
  pinMode(unlockPin, OUTPUT);
}

void loop() {
  for (byte i = 0; i < ROWS; i++) {
    // Activate one row at a time
    digitalWrite(rowPins[i], LOW);
    
    for (byte j = 0; j < COLS; j++) {
      if (digitalRead(colPins[j]) == LOW) { // Button pressed
        int btnID = i * COLS + j;

        // Debounce basic: wait until released
        while (digitalRead(colPins[j]) == LOW);

        // Store in circular buffer
        lastPresses[pressIndex] = btnID;
        pressIndex = (pressIndex + 1) % 5;

        Serial.print("Button ");
        Serial.print(btnID);
        Serial.print(" pass correct? ");
        Serial.print(correctPassword());
        Serial.println(" pressed");

        printLastPresses();
        if (correctPassword()){
          digitalWrite(unlockPin, HIGH);
          delay(5000);
          digitalWrite(unlockPin, LOW);
        }
        delay(200); // Simple debounce delay
      }
    }

    digitalWrite(rowPins[i], HIGH); // Deactivate row
  }
}

void printLastPresses() {
  Serial.print("Last 5: ");
  for (int k = 0; k < 5; k++) {
    Serial.print(lastPresses[k]);
    Serial.print(" ");
  }
  Serial.println();
}


bool correctPassword() {
  for(int dx = 0; dx < pass_leng; dx++){
    if (password[dx] != lastPresses[(pressIndex + dx) % pass_leng]){
      return false;
    }
  }
  return true;
}
