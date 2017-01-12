#include"wifi.h"

wifi w(10,11);//rx,tx
char a = 0;
char msg[10];
String M;
void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() 
{
  if(Serial.available())
  {
    a = Serial.read();
    if(a == 'p')
    {
      M = "(p:" + String(5) + ")";
      w.publish(M);
      M = "";
    }
    else if(a == 'b')
    {
      M = "(b:" + String(88) + ")";
      w.publish(M);
      M = "";
    }
  }
  w.update();
}
