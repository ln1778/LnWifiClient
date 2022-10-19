/*
    This sketch sends a string to a TCP server, and prints a one-line response.
    You must run a TCP server in your local network.
    For example, on Linux you can use this command: nc -v -l 3000
*/


#include "LnWifiClient.h"

LnWifiClient lnWifiClient;


void setup(void) {
  Serial.print("setup");
  Serial.begin(115200);
  lnWifiClient.startWifi();
}


void loop(void) {
//  
lnWifiClient.loop();

}
