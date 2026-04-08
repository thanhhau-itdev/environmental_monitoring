#ifndef OTAWEBSERVER_H
#define OTAWEBSERVER_H

#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

class OtaServer {
  public:
    OtaServer(const char* ssid, const char* pass);
    void begin(); 
    bool startAP();    
    bool stopAP();      
    void handle();       

  private:
    bool _running;
    const char* _ssid;
    const char* _pass;
    WebServer _server;
    IPAddress _localIP;
    IPAddress _gateway;
    IPAddress _subnet;

    static const char _index_html[];
    void _handleIndex();
    void _handleUpdateFinish();
    void _handleUpload();
};

#endif
