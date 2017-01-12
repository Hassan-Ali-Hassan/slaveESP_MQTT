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
// Update these with values suitable for your network.

//const char* ssid = "SU27";
//const char* password = "opera2525";
//const char* mqtt_server = "192.168.1.103";

const char* ssid = "AeroStaff-4";
const char* password = "stewart2";
const char* mqtt_server = "172.28.63.46";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[MESSAGE_SIZE];
int value = 0;
boolean publishFlag = false; //this variable is defined to be used as a latch, to make sure that ESP publishes only once after detecting instructions from the master moudle. If it wasn't for it, the ESP will keep publishing an empty string.
boolean reportFlag = false; //this variable is defined to make sure that no empty packages are sent to the master arduino (uno or mega) when a topic is updated.
char outTopic[TOPIC_SIZE]=  "Topic";

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

void callback(char* topic, byte* payload, unsigned int length) {
  String outString = "";
  String PayLoad = "";
  int i = 0;
  
  /**** Forming the payload packet that will be sent to the master via serial ****/
  for(i = 0; i < length; i++)
  {
    PayLoad += char(payload[i]);
  }
  if(strcmp(topic,"Topic1") == 0) //if the incoming message belongs to this topic
  {
    outString += "$d:"+PayLoad+"#";
    reportFlag = true;
  }
  else if(strcmp(topic,"Topic2") == 0) 
  {
    outString += "$s:"+PayLoad+"#";
    reportFlag = true;
  }
  /****** Checking there are no incoming instructions from the master ********/
  if(Serial.available())
  {
    parse();
  }
  else //we have no incoming messages waiting in the buffer
  {
    if(reportFlag)
    {
      Serial.println(outString);
      reportFlag = false;
    }
  }
}

void parse()
{
  String m;
  char a = 0;
  int i = 0;
  boolean seekTopic = false; //this is a flag that tells if we have parsed the right letter corresponding to the topic or not. The topic to which we will publish to should be decided when reaching the : character
  if(Serial.available())
  {
    publishFlag = true;
    while(Serial.available())
    {
      // Here we are seeking instructions coming from the master. It takes the form of "(mode:value)", for example (p:23.5)
      a = Serial.read();
      if(a == '(') // which designates the start of a new message
      {
        m = "";
        seekTopic = true;
      }
      else if(a == ':')// we should have known which topic to publish to by now
      {
        seekTopic = false;
      }
      else if(seekTopic)
      {
        if(isLetter(a))
        {
          if(a == 'p') //corresponding to position information
          {
            outTopic[5] = '3';
          }
          else if(a == 'b') // corresponding to a baton
          {
            outTopic[5] = '4';
          }
        }
      }
      else if(!seekTopic)
      {
        if(a != ')' && isNum(a)) //which means we haven't reached the end of the instruction string yet
        {
          msg[i] = a; //please note that the position infromation for example should be sent as an integer (position in cm for instance)
          i++;
        }
      }
    }
  }
}

boolean isNum(char a)
{
  if(a >= 48 && a <= 57) {return true;}
  else {return false;}
}

boolean isLetter(char a)
{
  if(a >= 97 && a <= 122) {return true;}
  else {return false;}
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
      client.subscribe("speed1");
      client.loop();
      client.subscribe("Topic1");
      client.loop();
      client.subscribe("Topic2");
      client.loop();
//      client.subscribe("Topic3");
//      client.loop();
//      client.subscribe("Topic4");  
//      client.loop();    
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
//  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(57600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.publish("Topic3","Hello");
}
