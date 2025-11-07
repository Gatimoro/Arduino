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
  
  void mov(int dx, int dy){
    x = (12 + dx + x) % 12;
    y = (8 + dy + y) % 8;  
  }
  
  void draw(){
    setPixel(frame, x, y, true);
  }
  Pixel copy(){
    return Pixel{x,y};
  }
  bool already_seen(bool board[8][12]){
    return board[y][x];
  }
};

struct Stacc{
  static const int MAX = 140;
  int front_idx;
  int back_idx;
  int size;
  Pixel data[MAX];
  
  Stacc() : front_idx(0), back_idx(0), size(0) {}
  
  void push_back(Pixel p) {
    if (size >= MAX) {
      Serial.println("PALMASO: Deque full");
      return;
    }
    data[back_idx] = p;
    back_idx = (back_idx + 1) % MAX;
    size++;
  }
  
  void push_front(Pixel p) {
    if (size >= MAX) {
      Serial.println("PALMASO: Deque full");
      return;
    }
    front_idx = (front_idx - 1 + MAX) % MAX;
    data[front_idx] = p;
    size++;
  }
  
  Pixel pop_back() {
    if (size == 0) {
      Serial.println("PALMASO: Deque empty");
      return Pixel{-1, -1};
    }
    back_idx = (back_idx - 1 + MAX) % MAX;
    size--;
    return data[back_idx];
  }
  
  Pixel pop_front() {
    if (size == 0) {
      Serial.println("PALMASO: Deque empty");
      return Pixel{-1, -1};
    }
    Pixel p = data[front_idx];
    front_idx = (front_idx + 1) % MAX;
    size--;
    return p;
  }
  
  Pixel& operator[](int idx) {
    return data[(front_idx + idx) % MAX];
  }
  
  int length() { return size; }
  bool isEmpty() { return size == 0; }
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

int dx[] = {0, -1, 0, 1};
int dy[] = {1, 0, -1, 0};
void loop() {
  frame[0] = frame[1] = frame[2] = 0;
  if (random(0,14) != 13){
    int dir_x = random(-1, 2); 
    int dir_y = random(-1, 2);
    bro.mov(dir_x, dir_y);
    bro.draw();
  }else{
    int last_x;
    int last_y;
    bool visited[8][12] = {false};
    visited[bro.y][bro.x] = true;
    Stacc debris{};
    debris.push_front(Pixel{bro.x,bro.y});
    while (!debris.isEmpty()){
      // Serial.println("not MT");
      int t = debris.size;
      frame[0] = frame[1] = frame[2] = 0;
      for (int c = 0; c < t; c++){
        Pixel cur = debris.pop_front();
        //if (random(0, abs(bro.x - cur.x) + abs(bro.y - cur.y)) < 1){
          cur.draw();
        //}
        last_x = cur.x;
        last_y = cur.y;
        for (int dir = 0; dir < 4; dir++){
          Pixel new_pixel = cur.copy();
          new_pixel.mov(dx[dir], dy[dir]);
          // Serial.print(new_pixel.x);
          // Serial.print("and ");
          // Serial.println (new_pixel.y);
          if (!new_pixel.already_seen(visited) ){
            debris.push_back(new_pixel);
            visited[new_pixel.y][new_pixel.x] = true;
          }
        }
      }

      matrix.loadFrame(frame);  
      delay(40);
    }
    bro = Pixel(last_x,last_y);
    Serial.println("new blob");
  }
  
  
  matrix.loadFrame(frame);  
  
  delay(300);
}
