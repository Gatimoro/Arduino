void setup() {
  // put your setup code here, to run once:
  pinMode(9,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int c=0; c<3; c++){
    digitalWrite(9, HIGH);
    delay(100);
    digitalWrite(9, LOW);
    delay(50);
  }
  for(int c=0; c<3; c++){
    digitalWrite(9, HIGH);
    delay(250);
    digitalWrite(9, LOW);
    delay(50);
  }
  for(int c=0; c<3; c++){
    digitalWrite(9, HIGH);
    delay(100);
    digitalWrite(9, LOW);
    delay(50);
  } 
  delay(3000);

}
