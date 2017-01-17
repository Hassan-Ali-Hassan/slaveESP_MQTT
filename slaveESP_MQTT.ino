/*
 This is a demonestration code that is to be used in inter-agent communication in formation control. 
 the purpose of this code is to make the ESP modules subscribe in various topics: Topic1, Topic2 and 
 Topic3. These topics emulate the position information that is to be garnered by each agent for itself.
 Now the call back function will detect any change in the value of each of the topics. 

 The purpose now is to let one agent publish a new value in one of the topics -supposedly the topic
 corresponding to its position- then the other agents subscribing to the same topic will be notified
 which emulates the situation when one agent updates its position and its neighbours are informed with
 its new position. 

 The code below sifts the incoming data and extracts the data related to the topics relevent to the agent
 -the agent's neighbours- and then send the received data via serial to the master device (can be an 
 arduino uno or mega). The code also allows the ESP to be able to receive instructions from the master
 module which may contain data to be published in other topics or it can be other sorts of instructions.
 The parse() function defined below is the function responsible for decoding the incoming instructions from
 the master device and makes the ESP act accordingly. 

N.B: please note that if agent#1 for instance subscribes in topic1, any update or publish by agent#1 in topic1
will be notified to agent#1 (which is trivial),however, the call back function in the ESP of agent#1 will
sift out any data from any topics other than that of agent#1's neighbours.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define TOPIC_SIZE 10
#define MESSAGE_SIZE 50
#define AGENT 3

// Update these with values suitable for your network.

//const char* ssid = "SU27";
//const char* password = "opera2525";
//const char* mqtt_server = "192.168.1.101:1883";

const char* ssid = "AeroStaff-4";
const char* password = "stewart2";
const char* mqtt_server = "172.28.63.46";
//IPAddress mqtt_server(172,28,63,46);

void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espClient;
PubSubClient client(espClient);
//PubSubClient client(mqtt_server, 1883, callback, espClient);

char msg[MESSAGE_SIZE];
String outString;
int myValue[2] = {0,0};
int value[6] = {23,54,23,66};
float oldTime = 0;
float oldTime2 = 0;
float t;
int batonValue = -1;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  #if AGENT == 1
  if(strcmp(topic,"Topic2") == 0)
  {
    parseValue(payload,length,1);
//    Serial.println(payload[0]);
  }
  #elif AGENT == 2
  if(strcmp(topic,"Topic1") == 0)
  {
    parseValue(payload,length,1);
//    Serial.println(payload[0]);
  }
  else if(strcmp(topic,"Topic3") == 0)
  {
    parseValue(payload,length,2);
//    Serial.println(payload[0]);
  }
  #elif AGENT == 3
  if(strcmp(topic,"Topic2") == 0)
  {
    parseValue(payload,length,1);
//    Serial.println(payload[0]);
  }
  #endif
  else if(strcmp(topic,"Baton") == 0) //the baton 
  {
    parseValue(payload,length,9);
//    Serial.println(payload[0]);
  }
}

// this function takes the payload received by the callback function, its length and 
// the index that indicates where it will be saved in the storage array
void parseValue(byte* p, unsigned int L, int index)
{
  String xVal;
  String yVal;
  int i = 0;
  boolean fillX = true;
  boolean fillY = false; 

  if(index == 9) //which means we have to update the baton value. The position values should be updated otherwise
  {
    for(i=0; i<L; i++)
    {
      xVal += char(p[i]);
    }
    batonValue = xVal.toInt();
//    Serial.println(batonValue);
  }
  else
  {
    for(i=0; i<L; i++)
    {
      if(char(p[i])==',') //this is the delimiter between x and y values
      {
        fillX = false;
        fillY = true;
      }
      else
      {
        if(fillX)xVal += char(p[i]);
        else if(fillY)yVal += char(p[i]);
      }
      
//      Serial.println(char(p[i]));
    }
    // saving values in respective places in array
      value[2*(index-1)] = xVal.toInt();
      value[2*(index-1)+1] = yVal.toInt();
//      Serial.print(value[2]);
//      Serial.print("\t");
//      Serial.println(value[3]);
  } 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("Baton");
      client.loop();  
      #if AGENT == 1
      client.subscribe("Topic2");
      client.loop();      
      #elif AGENT == 2
      client.subscribe("Topic1");
      client.loop();
      client.subscribe("Topic3");
      client.loop();
      #elif AGENT == 3
      client.subscribe("Topic2");
      client.loop();
      #endif  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void checkSerial()
{
  String xVal;
  String yVal;
  String outStr;
  boolean fillX = true;
  boolean fillY = false; 
  char a = 0;
  #if AGENT == 1
  int numOfEntries = 1;
  #elif AGENT == 2
  int numOfEntries = 2;
  #elif AGENT == 3
  int numOfEntries = 1;
  #endif
  
  if(Serial.available()) //note that every agent supplies its position only
  {
    while(Serial.available())
    {
      a = Serial.read();
      if(a == ',')
      {
        fillX = false;
        fillY = true;
      }
      else
      {
        if(fillX)xVal += a;
        else if(fillY) yVal += a;
      }      
    }
    myValue[0] = xVal.toInt();
    myValue[1] = yVal.toInt();

    outStr = "$";
    for(int i = 1; i <= numOfEntries; i++)
    {
      outStr = outStr + String(value[2*(i-1)]) + "," + String(value[2*(i-1)+1]);
      if(numOfEntries > 1)
      {
        outStr = outStr + ",";
      }
    }
    outStr = outStr +"#";
    Serial.print(outStr);//sending to the master
  }
}
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  t = (float)(millis()/1000.0);
  if(t-oldTime > 0.01)//if there is no delay, the loop will be way too fast and the parse() function will be invokes so quickly that the serial buffer will not be appropriately filled with he incoming message, and the while(available) will read each byte one by one (which is not desirable)
  {
    checkSerial();
    oldTime = t;
  }  
  
  if(t-oldTime2 > 0.05)
  {
    #if AGENT == 1 //this is intended to mean: 2 will send after 1 has sent, 3 will send after 2 has sent and 1 will send after 3 has sent
    if(batonValue == 3)
    #elif AGENT == 2
    if(batonValue == 1)
    #elif AGENT == 3
    if(batonValue == 2)
    #endif
    {
      #if AGENT == 1
      outString = String(myValue[0])+","+String(myValue[1]);
      outString.toCharArray(msg,MESSAGE_SIZE);
      client.publish("Topic1",msg);
      delay(10);
      client.publish("Baton","1");//sending a mark to the baton topic for the consequent device to send data
      outString="";
      batonValue = -1;
      #elif AGENT == 2
      outString = String(myValue[0])+","+String(myValue[1]);
      outString.toCharArray(msg,MESSAGE_SIZE);
      client.publish("Topic2",msg);
      delay(10);
      client.publish("Baton","2");
      outString="";
      batonValue = -1;
      #elif AGENT == 3
      outString = String(myValue[0])+","+String(myValue[1]);
      outString.toCharArray(msg,MESSAGE_SIZE);
      client.publish("Topic3",msg);
      delay(10);
      client.publish("Baton","3");
      outString="";
      batonValue = -1;
      #endif
    }
    oldTime2 = t;
  }
}
