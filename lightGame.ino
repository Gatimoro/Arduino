//This game consists of an array of leds with a light going up and down the array. Your goal is to stop it in the middle by pressing the button. 
int leds[13];
int delta = 1;
int curr=7;
void setup() {
  // put your setup code here, to run once:
  leds[0] = A1;
  pinMode(A1, OUTPUT);

  for(int pin = 1; pin<13; pin++){
    pinMode(pin, OUTPUT);
    leds[pin] = pin+1;
  }
  pinMode(A0, OUTPUT);
  leds[12] = A0;


  pinMode(13, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(leds[curr],HIGH);
  delay(20);
  if(digitalRead(13)==0){
    while(digitalRead(13)==0){}

    while(digitalRead(13)==1){}

    while(digitalRead(13)==0){}

    digitalWrite(leds[curr],LOW);
    curr=random(1,12);
  }
  digitalWrite(leds[curr],LOW);
  if(curr==12 || curr == 0){
    delta = -delta;
  }delay(3);
  curr+=delta;


}
