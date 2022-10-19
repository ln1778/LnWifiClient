#ifndef LnWifiClient_h
#define LnWifiClient_h

#include <Arduino.h>

class LnWifiClient
{
  public:   
  void startWifi (void); 
  void loop(void);
  void connectNewWifi(void);
//  void handleRoot(void);
//   void handleRootPost(void);
   void initSoftAP(void);
   void initWebServer(void);
   void initDNS(void);
}; 

#endif
