#include <FastLED.h>
// SNAKE in arduino using an 8x8 matrix and a joystick.
#define LED_PIN     11 
#define NUM_LEDS    64
#define LED_TYPE    WS2812B
bool food[64];
int color = 221;
CRGB leds[NUM_LEDS];
float fps = 5;
double wait = 250/fps;
int frame = 0;
int death_col = 0;
int CENTER_X = 507;
int CENTER_Y = 465;

struct queue{
    int vals[64];
    int start = 0;
    int finish = 0;
    bool inside[64];
    void push(int val){
        vals[finish] = val;
        finish = (finish+1) % 64;
        inside[val] = true;
        if(color>220){
          color = 40;
        }
        leds[val] = CHSV(color, 255, 100);

    }
    int pop(){
        int ret = vals[start];
        vals[start] = 0;
        start = (start+1) % 64;
        inside[ret] = false; 
        return ret;
    }
    int tail(){
        return vals[start];
    }
    bool has_inside(int val){
        return inside[val];
    }
    void death_rattle(CRGB col){
      for(int i = (finish + 63)%64; i != start - 1; i = (i+63)%64){
        delay(275);
        leds[vals[i]] = col;
        FastLED.show();
      }
    }
};
struct delta{
    int up = 8;
    int down = 56;
    int right = 1;
    int left = 7;
};
delta directions;

struct snake{
    queue body;
    int direction;
    int head = random(0,64);
    bool alive = true;
    int score = 0;
    int invalid_direction = directions.left;
    int candidate_direction = directions.right;
    int invalid_next = directions.left;
    void move(){
        direction = candidate_direction;
        invalid_direction = invalid_next;
        int x = head/8;
        int y = head%8;
        x = (x+direction/8)%8;
        y = (y+(direction%8))%8;
        head = 8*x + y;
        if ((body.has_inside(head)) && (head != body.tail())){
            alive = false;
        }else if(food[head]){
            food[head] = false;
            score += 1;
            wait -= 1;
            body.push(head);
            spawn_food();
            
        }else{
            leds[body.pop()] = CRGB::Black;
            body.push(head);
        }
    }
    
    void spawn_food(){
      int candidate = random(0,64);
        while(body.has_inside(candidate)){
            candidate = random(0,64);
        }food[candidate] = true;
        leds[candidate] = CRGB::Red;
      
    }

    void get_direction(){
        int dx = CENTER_X - analogRead(A1);
        int dy = analogRead(A0) - CENTER_Y;
        int adx = abs(dx);
        int ady = abs(dy);
        Serial.println(dx);
        Serial.println(dy);
        
        if(adx > 200 || ady > 200){
            if(adx>ady){
                if((dx>0) && (invalid_direction != directions.right)){
                    candidate_direction = directions.right;
                    invalid_next= directions.left;
                }else if(invalid_direction != directions.left){
                    candidate_direction = directions.left;
                    invalid_next= directions.right;
                }
            }else{
                if((dy>0) && (invalid_direction != directions.up)){
                    candidate_direction = directions.up;
                    invalid_next= directions.down;
                }else if(invalid_direction != directions.down){
                    candidate_direction = directions.down;
                    invalid_next= directions.up;
                }
            }
        }
    }
    void death(int col_num){
      body.death_rattle(CHSV(col_num,255,255));
    }
    
};
snake python;



void setup() {
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  python.body.push(python.head);
  python.spawn_food();
  FastLED.show();

}

void loop(){
    if(python.alive){
        frame=(frame+1)%4;
        python.get_direction();
        if (!frame){
            python.move();
            frame = 0;
            color = ((color +2) % 256);
            FastLED.show();
        }
    }else{
      python.death(death_col);
      delay(2666);
      death_col = (death_col + 214)%256;
    }
    delay(wait);
}
