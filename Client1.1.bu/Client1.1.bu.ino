#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

#include "config.h"



AsyncClient* client; 

static void replyToServer(void* arg ,char *msg) {
	AsyncClient* client = reinterpret_cast<AsyncClient*>(arg);
  Serial.printf("replyToServer: ");
  Serial.println(msg);
	// send reply
	if (client->space() > 32 && client->canSend()) {
		char message[32];
		sprintf(message, "this is from %s", WiFi.localIP().toString().c_str());
		client->add(msg, strlen(msg));
		client->send();
	}
}

/* event callbacks */
static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
	Serial.printf("\n data received from %s \n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t*)data, len);
  Serial.printf("\n");
	//os_timer_arm(&intervalTimer, 2000, true); // schedule for reply to server at next 2s
}

void onConnect(void* arg, AsyncClient* client) {
	Serial.printf("\n client has been connected to %s on port %d \n", SERVER_HOST_NAME, TCP_PORT);
  digitalWrite(SERVER_PIN, HIGH);
	replyToServer(client, "onConnect");
}

//--------------------WALL SENSOR'S TIMERS---------------------------------------//


int wallSensor_1_State;             // the current reading from the input pin
int lastWallSensor_1_State = LOW;   // the previous reading from the input pin

int wallSensor_2_State;             // the current reading from the input pin
int lastWallSensor_2_State = LOW;   // the previous reading from the input pin

unsigned long lastDebounceTimeSensor_1 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeSensor_2 = 0;  // the last time the output pin was toggled

//unsigned long doubleTouchTimer_1 = 0;  // Check that there is no double touch
//unsigned long doubleTouchTimer_2 = 0;  // Check that there is no double touch


unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long doubleTouchDelay = 3000;    // the debounce time; increase if the output flickers


//----------------------setup-------------------------------//

void setup() {
	Serial.begin(115200);
	delay(20);

  pinMode(WIFI_PIN, OUTPUT);
  pinMode(SERVER_PIN, OUTPUT);
  pinMode(WALL_SENSOR_1_PIN, INPUT);
 
  digitalWrite(WIFI_PIN, LOW);
  digitalWrite(SERVER_PIN, LOW);
 

	// connects to access point
	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID, PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print('.');
		delay(500);
	}
  digitalWrite(WIFI_PIN, HIGH);

 
  client = new AsyncClient;
	//AsyncClient* client = new AsyncClient;
	client->onData(&handleData, client);
	client->onConnect(&onConnect, client);
	client->connect(SERVER_HOST_NAME, TCP_PORT);

  while(!client->connected()){
    Serial.print('-');
    delay(500);
  }
  
}

char messageTouch[7];

void loop() {

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
      sprintf(messageTouch, "route 1");
      replyToServer(client , messageTouch);
      Serial.println(messageTouch);
      memset(messageTouch, 0 ,7);
      }
    }
  }
  
  lastWallSensor_1_State = wallSensor_1_Reading;

  //----------------------------

  
  if (wallSensor_2_Reading != lastWallSensor_2_State) {
     lastDebounceTimeSensor_2 = millis();
  }
  
  
  if ((millis() - lastDebounceTimeSensor_2) > debounceDelay) {
    
    if (wallSensor_2_Reading != wallSensor_2_State) {
      
      wallSensor_2_State = wallSensor_2_Reading;
      
      if (wallSensor_2_State == HIGH) {
        
      Serial.println("button2");
      sprintf(messageTouch, "route 2");
      replyToServer(client , messageTouch);
      Serial.println(messageTouch);
      memset(messageTouch, 0 ,7);
      }
    }
  }
  
  lastWallSensor_2_State = wallSensor_2_Reading;
    
}
