#ifndef WIFI_H
#define WIFI_H
#define MESSAGESIZE  5
#include <SoftwareSerial.h>
#include"Arduino.h"

class wifi{
  
  public:
  wifi(int,int);
  void init();
  void update();
  void publish(String);
  char messageC[MESSAGESIZE];
  float messageI[MESSAGESIZE]; //I for instructions
  
  protected:
  void parse();
  void parse2();
  void setESP();
  void testCon();
  void echo();
  boolean isNum(char);
  boolean isLetter(char);
  
  int INDEX;
  String m ;
  SoftwareSerial esp;
};
#endif
