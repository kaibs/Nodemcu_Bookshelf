#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>

// credentials
#include "credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP;
const char* user= MQTT_USER;
const char* passw= MQTT_PASSWORD;

// Wifi
WiFiClient espBookshelf;

//MQTT
PubSubClient client(espBookshelf);
String receivedString;

//FS1000A RF-Transmitter
#define rfpin 4
RCSwitch transmitter = RCSwitch();
int repetitions = 10;

// LED
const int led = 5;
int brightness = 122;

// ----------------------------------- FCNs ------------------------------------------------------


// FCNs RF-Switches
void sendSignal(char tristate[]){
  for (int i=0;i<repetitions;i++){
    transmitter.sendTriState(tristate);
    delay(10);
  }
}


// FCNs LED-Strip
void stripON(){

  analogWrite(led, brightness);
  String helpVal = (String)brightness;
  char feedback[10];
  helpVal.toCharArray(feedback, 10);
  client.publish("home/bedroom/bookshelf/feedback", feedback);
}

void stripOFF(){
  
  analogWrite(led, 0);
  char feedback[5] = "0";
  client.publish("home/bedroom/bookshelf/feedback", feedback);
}

void setBright(){
  
  analogWrite(led, brightness);
  String helpVal = (String)brightness;
  char feedback[10];
  helpVal.toCharArray(feedback, 10);
  client.publish("home/bedroom/bookshelf/feedback", feedback);
}

// Callback MQTT
void callback(char* topic, byte* payload, unsigned int length) {
 
 if (strcmp(topic,"home/bedroom/bookshelf/switch")==0){
  for (int i=0;i<length;i++) {
   receivedString += (char)payload[i];


   if (receivedString == "ON"){
     stripON();
   }

   if (receivedString == "OFF"){
     stripOFF();
   }
  }
  receivedString = "";
 }

 if (strcmp(topic,"home/bedroom/bookshelf/brightness")==0){
 
  for (int i=0;i<length;i++) {
   receivedString += (char)payload[i];
  }
  
  brightness = receivedString.toInt();
  setBright();
  
  receivedString = "";
  }

  // Christmas Tree
  if (strcmp(topic,"home/livingroom/tree")==0){

    for (int i=0;i<length;i++) {
     receivedString += (char)payload[i];

     if (receivedString == "ON"){
      sendSignal("0FFF0FFFFFFF");
     }

     if (receivedString == "OFF"){
      sendSignal("0FFF0FFF0000");
     }
    }
    receivedString = "";
  }

  // Fountain
  if (strcmp(topic,"home/livingroom/fountain")==0){

    for (int i=0;i<length;i++) {
     receivedString += (char)payload[i];

     if (receivedString == "ON"){
      sendSignal("0FFFF0FFFFFF");
     }

     if (receivedString == "OFF"){
      sendSignal("0FFFF0FF0000");
     }
    }
    receivedString = "";
  }

  // Desklamp
  if (strcmp(topic,"home/office/desklamp")==0){

    for (int i=0;i<length;i++) {
     receivedString += (char)payload[i];

     if (receivedString == "ON"){
      sendSignal("0FFFFFF0FFFF");
     }

     if (receivedString == "OFF"){
      sendSignal("0FFFFFF0FFF0");
     }
    }
    receivedString = "";
  }
 }

void reconnect() {

 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");

 // Attempt to connect
 if (client.connect("BookshelfESP", user, passw)) {
  Serial.println("connected");
  // ... and subscribe to topic
  client.subscribe("home/bedroom/bookshelf/switch");
  client.subscribe("home/bedroom/bookshelf/brightness");
  client.subscribe("home/livingroom/tree");
  client.subscribe("home/livingroom/fountain");
  client.subscribe("home/office/desklamp");
  
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

  Serial.begin(9600);
  
  // MOSFET LED
  pinMode(led, OUTPUT); 
  
  //Wifi
  WiFi.hostname("BookshelfESP");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  // MQTT 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // RF
  transmitter.enableTransmit(rfpin);
  transmitter.setProtocol(2);
  transmitter.setPulseLength(320);

}

void loop() {

 //digitalWrite(led, brightness);

 if (!client.connected()){
  reconnect();
 }
 client.loop();

}
