#include "wifi.h"

wifi::wifi(int rx, int tx):esp(rx,tx)
{
  int i = 0;
  for(i = 0; i < MESSAGESIZE; i++)
  {
    messageI[i] = 0;
    messageC[i] = 0;
  }
  esp.begin(57600);
}

void wifi::init()
{
  setESP();
}

void wifi::setESP()
{
  Serial.println("setting the esp");
  String networkConfiguration = "wifi.sta.config(\"SU27\",\"opera2525\")\r\n";
  String brokerSettings = "m:connect(\"192.168.1.101\", 1883, 0, function(client) print(\"connected\")end)\r\n";
//  String instructions = "m:on(\"message\", function(conn, topic, data)if data ~= nil then T = topic.gsub(topic,\"topic\",'$') print(T .. \":\".. data..\"#\") end end)\r\n";
//  String topics = "m:subscribe({[\"topic0\"]=0,[\"topic1\"]=0,[\"topic2\"]=0}, function(conn) print(\"subscribe success\") end)\r\n";
  String instructions = "m:on(\"message\", function(conn, topic, data)if data ~= nil then if string.find(topic,\"direction\") ~= nil then T = topic.gsub(topic,\"direction\",'$d') print(T .. \":\".. data..\"#\") elseif string.find(topic,\"speed\") ~= nil then T = topic.gsub(topic,\"speed\",'$s') print(T .. \":\".. data..\"#\") end end end) \r\n";
  String topics = "m:subscribe({[\"direction1\"]=0,[\"speed1\"]=0}, function(conn) print(\"subscribe success\") end)\r\n";
  String payload[] = {
                      "wifi.setmode(wifi.STATION)\r\n",                    
                      networkConfiguration,
                      "print(wifi.sta.status())\r\n",
                      "m = mqtt.Client(\"client_id\", 120, \"\",\"\")\r\n",
                      "m:on(\"connect\", function(con) print (\"connected\") end)\r\n",
                      "m:on(\"offline\", function(con) print (\"offline\") end)\r\n",
                      instructions,
                      topics,
                      brokerSettings                                            
                     };
  for (int i = 0; i < 9; i++)
  {
    esp.print(payload[i]);
//    Serial.println(payload[i]);
    delay(200);
    echo();
    delay(200);
  }
}

void wifi::testCon()
{
  char a = 0;
  esp.print("print(wifi.sta.status())\r\n");
  while (esp.available())
  {
    a = esp.read();
    Serial.write(a);
    if(a == '5')
    {
      Serial.println("ESP is connected to wifi");
    }
  }
}

void wifi::echo()
{
  while (esp.available())
  {
    Serial.write(esp.read());
  }
}

void wifi::parse()
{
  char a = 0;
  int i = 0;
  int j = 0;
  bool seekIndex = false;
  int topicIndex = 0;
  j = 0;
  while (esp.available())
  {
    a = esp.read();
    if ( (a >= 97 && a <= 122)/*if a is a letter*/ || (a >= 48 && a <= 57)/*...or a is a number*/ || a == '#' /*...or a is an end of phrase*/|| a == '$' /*... or a is the begining of a phrase*/|| a == ':'/*...or a is a separator between topic name and its value*/)
    {
      if (a == '$')
      {
        seekIndex = true;
        j = 0;
        m = "";
      }
      else if (a == ':')
      {
        seekIndex = false;
      }
      else if (seekIndex)
      {
        topicIndex = (int)a - 48;
      }
      else if (!seekIndex)
      {
        if(a != '#') 
        {
          m += a;
        }
        else
        {
          messageI[topicIndex] = m.toInt();
          analogWrite(13,messageI[topicIndex]);
        }
      }
      delay(2);
    }
  }
}

void wifi::parse2() //this is parsing instructions from the CUSBOT_CONTROL flow on node-red
{ 
  // The output of this function is to save the direction instruction in the first element of messageI and the speed 
  // instruction in the second element.
  char a = 0;
  int i = 0;
  int j = 0;
  bool seekIndex = false;

  j = 0;
  while(esp.available())
  {
    a = esp.read();
    if ( isLetter(a)/*if a is a letter*/ || isNum(a)/*...or a is a number*/ || a == '#' /*...or a is an end of phrase*/|| a == '$' /*... or a is the begining of a phrase*/|| a == ':'/*...or a is a separator between topic name and its value*/|| a == '-' /*...or a is a minus sign*/ || a == '.'/*a decimal point*/)
    {
      if (a == '$') //which marks the begining of a new data string. Here we set our variables to their default states
      {
        seekIndex = true;
        j = 0;
        m = "";
      }
      else if (a == ':') //here we've reached the separator between the topic name and its value
      {
        seekIndex = false;
      }
      else if (seekIndex) //if the characheter is not a special charachter, and we haven't reached the address/value separator, this is the topic's name 
      {
        if(isLetter(a)) //in the case of this parsing function specifically,the topic address is merely a number and a letter. IF the letter is a d, then it is a direction instruction, while s is for speed instruction. We save the direction instruction as the first element in the message array and we save the speed instruction in the second entry.
        {
          if(a == 'd') // hence it is a direction instruction
          {
            INDEX = 0;
          }
          else if(a == 's')
          {
            INDEX = 1;
          }
        }
        else if(isNum(a))
        {
        }
      }
      else if (!seekIndex)
      {
        if(a != '#') 
        {
          m += a;
        }
        else
        {
          messageI[INDEX] = (float)m.toInt()/10.0;
//          Serial.println(messageI[INDEX]);
          Serial.println(m);
//          Serial.println(messageI[INDEX]);
//          analogWrite(13,messageI[INDEX]);
        }
      }
      delay(2); //add this delay or else no readings will be obtained and the device may hang
    }
  }
}

void wifi::publish(String a) //sending the message containing the necessary information about the topic to be published to and the value of the message
{
  esp.println(a);
}
boolean wifi::isNum(char a)
{
  if(a >= 48 && a <= 57) {return true;}
  else {return false;}
}

boolean wifi::isLetter(char a)
{
  if(a >= 97 && a <= 122) {return true;}
  else {return false;}
}


void wifi::update()
{
  parse2();
}
