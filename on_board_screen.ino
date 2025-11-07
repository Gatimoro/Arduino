#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;
uint32_t frame[3] = {0, 0, 0};

struct Pixel;
struct Stacc;

void setPixel(uint32_t frame[3], int x, int y, bool state) {
  if (x < 0 || x >= 12 || y < 0 || y >= 8){  
    Serial.println("VERY BAD COORDINATES!");
    return;
  }
  int index = y * 12 + x;
  int arrayIndex = index / 32;
  int bitPosition = 31 - (index % 32);

  if (state) {
    frame[arrayIndex] |= (1UL << bitPosition);
  } else {
    frame[arrayIndex] &= ~(1UL << bitPosition);
  }
}

struct Pixel {
  int x;
  int y;
  Pixel() : x(0), y(0) {} 
  Pixel(int equis, int ygr) : x(equis), y(ygr) {} 
  
  void move(int dx, int dy){
    x = (12 + dx + x) % 12;
    y = (8 + dy + y) % 8;  
  }
  
  void draw(){
    setPixel(frame, x, y, true);
  }
};

struct Stacc {
  static const int MAX = 140;
  int top;
  Pixel data[MAX];
  
  
  Stacc() : top(-1) {}
  
  void push(Pixel p){
    if (top < MAX - 1){
      data[++top] = p;
    }
  }
  
  Pixel pop(){
    if (top >= 0){
      return data[top--];
    }
    Serial.println("PALMASO");
    return Pixel{-1, -1};  
  }
  
  bool isEmpty(){
    return top == -1;
  }
};

struct Blob {
  int x;
  int y;
  int body;
  
  Blob(int ex, int wa) : x(ex), y(wa), body(0) {}
};

Pixel bro{4, 4};  

void setup() {
  Serial.begin(9600);
  matrix.begin();
  randomSeed(analogRead(0));
}

void loop() {
  frame[0] = frame[1] = frame[2] = 0;
  
  int dir_x = random(-1, 2); 
  int dir_y = random(-1, 2);
  
  bro.move(dir_x, dir_y);
  bro.draw();
  
  matrix.loadFrame(frame);  
  
  delay(500);
}
