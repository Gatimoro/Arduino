int starting = 263; 
int buzzer = 9;

void setup() {
  pinMode(buzzer, OUTPUT);
}

struct Tuple {
  long note;
  long duration;
};
// #sshiit must be a constant :()
const int song_length = 26;
Tuple song[song_length] = {
  {5, 375}, {4, 375}, {2, 375}, {0, 375}, 
  {7, 750}, {7, 750}, 
  {5, 375}, {4, 375}, {2, 375}, {0, 375}, 
  {7, 750}, {7, 750}, 
  {5, 375}, {9, 375}, {9, 375}, {5, 375}, 
  {4, 375}, {7, 375}, {7, 375}, {4, 375}, 
  {2, 375}, {4, 375}, {5, 375}, {2, 375}, 
  {0, 750}, {0, 750}
};

// Play note for the duration time.
void play(int note, int time) {
    tone(buzzer, starting * pow(2.0, (double)note / 12.0));
    delay(time);
    noTone(buzzer);
    delay(50);
}

void loop() {
  for(int n = 0; n < song_length; n++) {
    play(song[n].note, song[n].duration);
  }delay(1000);
}
