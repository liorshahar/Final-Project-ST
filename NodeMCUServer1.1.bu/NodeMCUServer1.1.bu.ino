#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncTCP.h>
#include <DNSServer.h>
#include <vector>
#include "Adafruit_MPR121.h"
#include <Wire.h>

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

#include "config.h"


//--------------------------------------------------------------//

static DNSServer DNS;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//--------------------------------------------------------------//

static std::vector<AsyncClient*> clients; // a list to hold all clients
Adafruit_MPR121 cap = Adafruit_MPR121();

//--------------------------------------------------------------//

static os_timer_t intervalTimer;

uint16_t lasttouched = 0; //MPR121
uint16_t currtouched = 0; //MPR121

unsigned long speakerTimer = 0;   
const long speakerInterval = 1000; 

unsigned long currentMillis = 0;
unsigned long startTimeMillis = 0;
unsigned long slaveTouchTime = 0;
unsigned long masterTouchTime = 0;
unsigned long masterJumpTime = 0;

char touchTimeString[sizeof(long)*8+1];
char sendTouchResults[60];

char jumpTimeString[sizeof(long)*8+1];
char sendJumpResults[60];

//--------------------WALL SENSOR'S TIMERS---------------------------------------//


int wallSensor_1_State;             // the current reading from the input pin
int lastWallSensor_1_State = LOW;   // the previous reading from the input pin
boolean sensor_1_back = false;

int wallSensor_2_State;             // the current reading from the input pin
int lastWallSensor_2_State = LOW;   // the previous reading from the input pin
boolean sensor_2_back = false;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTimeSensor_1 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeSensor_2 = 0;  // the last time the output pin was toggled

unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers




//-------------------TCP clients events-------------------------------------------//

 /* clients events */
static void handleError(void* arg, AsyncClient* client, int8_t error) {
	Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
  
  slaveTouchTime = millis() - startTimeMillis; 
	Serial.printf("\n data received from client %s \n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t*)data, len);
  Serial.println(' ');
  char* dataS = static_cast<char*>(data);
  std::string slaveTouchWall(dataS, len);
  sprintf(sendTouchResults, "%s Touch-Time: %s", slaveTouchWall.c_str(), ltoa(slaveTouchTime, touchTimeString  ,10)); 
  Serial.printf("Slave: ");
  Serial.println(sendTouchResults);
  mqttClient.publish(MQTT_PUBLISH_WALL_SENSOR_TOUCH, sendTouchResults);
  memset(touchTimeString, 0, 33);
  memset(sendTouchResults, 0, 60);
  slaveTouchTime = 0;

//  reply to client
  if (client->space() > 32 && client->canSend()) {
  char reply[32];
  sprintf(reply, "this is from %s", SERVER_HOST_NAME);
	client->add(reply, strlen(reply));
	client->send();
  }
}

static void handleDisconnect(void* arg, AsyncClient* client) {
	Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time) {
	Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}


/* server events */
static void handleNewClient(void* arg, AsyncClient* client) {
	Serial.printf("\n new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());
  mqttClient.publish(MQTT_PUBLISH_NODE_MCU_CONNECTED, "slave connected");
	// add to list
	clients.push_back(client);
	
	// register events
	client->onData(&handleData, NULL);
	client->onError(&handleError, NULL);
	client->onDisconnect(&handleDisconnect, NULL);
	client->onTimeout(&handleTimeOut, NULL);
}

//--------------------setup_wifi()------------------------------------------//

void setup_wifi(){
  
  // create access point
  while (!WiFi.begin(SSID, PASSWORD, 6, false, 15)) {
    delay(500);
  }

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  
  digitalWrite(WIFI_PIN, HIGH);
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

//--------------------reconnect()------------------------------------------//

void reconnect() {
  // Loop until we're reconnected
  digitalWrite(MQTT_PIN, LOW);
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "NodeMCUClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      mqttClient.publish(MQTT_PUBLISH_NODE_MCU_CONNECTED, CONNECTION_MESSAGE);
      // ... and resubscribe   
      mqttClient.subscribe(MQTT_RECEIVER_START);
      mqttClient.subscribe(MQTT_RECEIVER_CONNECTED_DEVICES_LOG);
      digitalWrite(MQTT_PIN, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
     }
  }
}

//--------------------callback------------------------------------------//

void callback(char* topic, byte *payload, unsigned int length) {
  
    Serial.println("-------new message from broker-----");
    Serial.print("topic:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
    if(!(strcmp(topic , "swimTouch/start"))){
      analogWrite(BUZZER_PIN, 100);
      startTimeMillis = millis();
      
      os_timer_arm(&intervalTimer, 1000, true);
      
      publishDataToServer(MQTT_PUBLISH_START_TIME,START);
      Serial.println("Turn On BUZZER! ");
      sensor_1_back = false;
      sensor_2_back = false;
      
    }
}

//--------------------publishDataToServer------------------------------------------//
 
void publishDataToServer(const char *topic ,const char *message){
    if (!mqttClient.connected()) {
      reconnect();
    }
   mqttClient.publish(topic, message);
}

static void buzzerTimer(void* arg){
   analogWrite(BUZZER_PIN, 0);
}

//--------------------SETUP------------------------------------------//

void setup() {
	Serial.begin(115200);
	delay(20);

 
  pinMode(WIFI_PIN, OUTPUT);
  pinMode(MQTT_PIN, OUTPUT);
  pinMode(MPE121_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(WALL_SENSOR_1_PIN, INPUT);
  pinMode(WALL_SENSOR_2_PIN, INPUT);
 
  digitalWrite(WIFI_PIN, LOW);
  digitalWrite(MQTT_PIN, LOW);
  digitalWrite(MPE121_PIN, LOW);
  analogWrite(BUZZER_PIN, 0);
  
  setup_wifi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callback);
  reconnect();
	
	// start dns server
	if (!DNS.start(DNS_PORT, SERVER_HOST_NAME, WiFi.softAPIP()))
		Serial.printf("\n failed to start dns service \n");


  
  AsyncServer* server = new AsyncServer(TCP_PORT); // start listening on tcp port 7050
  server->onClient(&handleNewClient, server);
  server->begin();
  Serial.println("server->begin");
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  if (!cap.begin(0x5A)) {
     digitalWrite(MPE121_PIN, LOW);
    Serial.println("MPR121 not found, check wiring?");
    while (1){
      Serial.println(':');
    };
 
  }
  
  Serial.println("MPR121 found!");
  digitalWrite(MPE121_PIN, HIGH);
  
  cap.writeRegister(MPR121_ECR,0x00);
  cap.setThreshholds(30, 10);
  cap.writeRegister(0x5B, 0x77); // deboucne register
  cap.writeRegister(0x65, 0x21); // electrod 6 current
  cap.writeRegister(0x66, 0x21); // electrod 7 current
 
  
  cap.writeRegister(0x6F, 0x33);   // electrod 6 + 7 charge time
  
  cap.writeRegister(MPR121_ECR,0x8F);
    
  os_timer_disarm(&intervalTimer);
  os_timer_setfn(&intervalTimer, &buzzerTimer ,server );
}

void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}



void loop() {
	DNS.processNextRequest();
  mqttClient.loop(); 

  int wallSensor_1_Reading = digitalRead(WALL_SENSOR_1_PIN);
  int wallSensor_2_Reading = digitalRead(WALL_SENSOR_2_PIN);
  
  if (wallSensor_1_Reading != lastWallSensor_1_State) {
     lastDebounceTimeSensor_1 = millis();
  }
  
  
  if ((millis() - lastDebounceTimeSensor_1) > debounceDelay) {
    
    if (wallSensor_1_Reading != wallSensor_1_State) {
      
      wallSensor_1_State = wallSensor_1_Reading;
      
      if (wallSensor_1_State == HIGH) {
        
        Serial.println("button1");
        masterTouchTime = millis() - startTimeMillis;
        sprintf(sendTouchResults, "route 1 Touch-Time: %s", ltoa(masterTouchTime, touchTimeString  ,10)); 
        mqttClient.publish(MQTT_PUBLISH_WALL_SENSOR_TOUCH, sendTouchResults);
        Serial.println(sendTouchResults);
        memset(touchTimeString, 0, 33);
        memset(sendTouchResults, 0, 60);
        masterTouchTime = 0;
      
      }else if( sensor_1_back == false && wallSensor_1_State == LOW){
        Serial.println("back jump");
        masterJumpTime = millis() - startTimeMillis;
        sprintf(sendJumpResults, "route 1 Jump-Time: %s" , ltoa(masterJumpTime, jumpTimeString  ,10)); 
        mqttClient.publish(MQTT_PUBLISH_JUMP_TIME, sendJumpResults);
        Serial.println(sendJumpResults);
        memset(jumpTimeString, 0, 33);
        memset(sendJumpResults, 0, 60);
        masterJumpTime = 0;
        sensor_1_back = true;
      }
    }
  }
  
  lastWallSensor_1_State = wallSensor_1_Reading;
  
  
  if (wallSensor_2_Reading != lastWallSensor_2_State) {
     lastDebounceTimeSensor_2 = millis();
  }
  
  
  if ((millis() - lastDebounceTimeSensor_2) > debounceDelay) {
    
    if (wallSensor_2_Reading != wallSensor_2_State) {
      
      wallSensor_2_State = wallSensor_2_Reading;
      
      if (wallSensor_2_State == HIGH) {
        
        Serial.println("button2");
        
        masterTouchTime = millis() - startTimeMillis;
        sprintf(sendTouchResults, "route 2 Touch-Time: %s", ltoa(masterTouchTime, touchTimeString  ,10)); 
        mqttClient.publish(MQTT_PUBLISH_WALL_SENSOR_TOUCH, sendTouchResults);
        Serial.println(sendTouchResults);
        memset(touchTimeString, 0, 33);
        memset(sendTouchResults, 0, 60);
        masterTouchTime = 0;
        
      }else if( sensor_2_back == false && wallSensor_2_State == LOW){
        Serial.println("back jump");
        masterJumpTime = millis() - startTimeMillis;
        sprintf(sendJumpResults, "route 2 Jump-Time: %s", ltoa(masterJumpTime, jumpTimeString  ,10)); 
        mqttClient.publish(MQTT_PUBLISH_JUMP_TIME, sendJumpResults);
        Serial.println(sendJumpResults);
        memset(jumpTimeString, 0, 33);
        memset(sendJumpResults, 0, 60);
        masterJumpTime = 0;
        sensor_2_back = true;
      }
    }
  }
  
  lastWallSensor_2_State = wallSensor_2_Reading;
  
  
  // Get the currently touched pads
  currtouched = cap.touched();
    
  for (uint8_t i=0; i<11; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {
         
          //masterTouchTime = millis() - startTimeMillis;
          //sprintf(sendTouchResults, "route %d Touch-Time: %s", i, ltoa(masterTouchTime, touchTimeString  ,10)); 
          //mqttClient.publish(MQTT_PUBLISH_WALL_SENSOR_TOUCH, sendTouchResults);
          //Serial.println(sendTouchResults);
          //memset(touchTimeString, 0, 33);
          //memset(sendTouchResults, 0, 60);
          //masterTouchTime = 0;
    }

    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i))) {
          masterJumpTime = millis() - startTimeMillis;
          if(i == 6){
            sprintf(sendJumpResults, "route %d Jump-Time: %s", 1, ltoa(masterJumpTime, jumpTimeString  ,10)); 
            mqttClient.publish(MQTT_PUBLISH_JUMP_TIME, sendJumpResults);
            Serial.println(sendJumpResults);
            memset(jumpTimeString, 0, 33);
            memset(sendJumpResults, 0, 60);
            masterJumpTime = 0;  
          }
          
          if(i == 7){
            sprintf(sendJumpResults, "route %d Jump-Time: %s", 2, ltoa(masterJumpTime, jumpTimeString  ,10)); 
            mqttClient.publish(MQTT_PUBLISH_JUMP_TIME, sendJumpResults);
            Serial.println(sendJumpResults);
            memset(jumpTimeString, 0, 33);
            memset(sendJumpResults, 0, 60);
            masterJumpTime = 0;  
          }
          
    }
  }

  // reset our state
  lasttouched = currtouched;

  
}
