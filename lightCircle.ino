float theta;
int circle[8] = {2,3,4,5, 6,7,8,9};
setup(){
for(int pin=2; pin<10; pin++){
  pinMode(pin, OUTPUT);
}
}
loop(){
  for(int i=0; i<8; i++){
    digitalWrite(circle[i], HIGH);
    delay(200);
    digitalWrite(circle[i], LOW);
  }
}
