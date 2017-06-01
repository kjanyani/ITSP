  int d=500;

void setup() {
  // put your setup code here, to run once:
  pinMode( 5, OUTPUT);
  pinMode( 4, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(4, HIGH);

for( int i = 0; i< 2000 ; i++)
  {
    digitalWrite( 5 , HIGH);
    delayMicroseconds(d);
    digitalWrite( 5, LOW);
    delayMicroseconds(d) ;
  }
 delay(1000) ;
}
