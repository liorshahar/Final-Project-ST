#ifndef CONFIG_H
#define CONFIG_H

/*
 * This example demonstrate how to use asynchronous client & server APIs
 * in order to establish tcp socket connections in client server manner.
 * server is running (on port 7050) on one ESP, acts as AP, and other clients running on
 * remaining ESPs acts as STAs. after connection establishment between server and clients
 * there is a simple message transfer in every 2s. clients connect to server via it's host name
 * (in this case 'esp_server') with help of DNS service running on server side.
 *
 * Note: default MSS for ESPAsyncTCP is 536 byte and defualt ACK timeout is 5s.
*/

//#define SSID "Shenkar-New"
//#define PASSWORD "Shenkarwifi"

//#define SSID "B2-WIFI"
//#define PASSWORD "basement@harder"


//#define SSID "Lior"
//#define PASSWORD "0544203322"

//#define SSID "dlink-orange"
//#define PASSWORD "shenkar123"

#define SSID "swimTouch"
#define PASSWORD "0544203322"

//192.168.0.101 slave

//192.168.0.102 master

//30.30.251.177
#define SERVER_HOST_NAME "192.168.2.100"


#define TCP_PORT 7050
#define DNS_PORT 53


// MQTT Broker  settings
#define MQTT_PORT 17022
#define MQTT_USER "oiidqtdo"
#define MQTT_PASSWORD "0TDWlrS_abQd"
#define MQTT_SERVER  "m24.cloudmqtt.com"

// MQTT Publich topics
#define MQTT_PUBLISH_START_TIME "swimTouch/startTime"
#define MQTT_PUBLISH_WALL_SENSOR_TOUCH "swimTouch/WallSensor"
#define MQTT_PUBLISH_JUMP_TIME "swimTouch/jumpTime"
#define MQTT_PUBLISH_NODE_MCU_CONNECTED "swimTouch/nodeMCUConnected"

// MQTT Subscribe
#define MQTT_RECEIVER_START "swimTouch/start"
#define MQTT_RECEIVER_CONNECTED_DEVICES_LOG "swimTouch/connectedDevicesLog"

// MQTT Connection message
#define CONNECTION_MESSAGE "NodeMCU Master Is Connected"

// MQTT Message
#define START "start"

#define ELE0_R  0x42
#define ELE1_T  0x43
#define ELE1_R  0x44
#define ELE2_T  0x45
#define ELE2_R  0x46
#define ELE3_T  0x47
#define ELE3_R  0x48
#define ELE4_T  0x49
#define ELE4_R  0x4A
#define ELE5_T  0x4B
#define ELE5_R  0x4C
#define ELE6_T  0x4D
#define ELE6_R  0x4E
#define ELE7_T  0x4F
#define ELE7_R  0x50
#define ELE8_T  0x51
#define ELE8_R  0x52
#define ELE9_T  0x53
#define ELE9_R  0x54
#define ELE10_T 0x55
#define ELE10_R 0x56
#define ELE11_T 0x57
#define ELE11_R 0x58
#define FIL_CFG 0x5D
#define ELE_CFG 0x5E

#define WIFI_PIN D0   // red
#define MQTT_PIN D3   // green
#define MPE121_PIN D4 // yellow

#define WALL_SENSOR_1_PIN D7
#define WALL_SENSOR_2_PIN D8

#define BUZZER_PIN D5

#endif // CONFIG_H
