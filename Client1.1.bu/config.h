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

//192.168.0.101
#define SERVER_HOST_NAME "192.168.2.102"
//#define SERVER_HOST_NAME "30.30.222.120"


#define TCP_PORT 7050
#define DNS_PORT 53

#define WIFI_PIN D0   // red
#define SERVER_PIN D3   // green

#define WALL_SENSOR_1_PIN D0
#define WALL_SENSOR_2_PIN D2

#endif // CONFIG_H
