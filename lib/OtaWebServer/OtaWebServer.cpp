#include "OtaWebServer.h"

const char OtaServer::_index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 OTA Upload</title>
</head>
<body>
  <h3>Upload firmware (.bin)</h3>
  <form method="POST" action="/update" enctype="multipart/form-data">
    <input type="file" name="firmware" accept=".bin"><br><br>
    <input type="submit" value="Upload and Install">
  </form>
</body>
</html>
)rawliteral";

OtaServer::OtaServer(const char* ssid, const char* pass)
: _ssid(ssid), _pass(pass), _server(80),
  _localIP(192,168,1,100), _gateway(192,168,1,1), _subnet(255,255,255,0),
  _running(false) {}

void OtaServer::begin() {
  _server.on("/", HTTP_GET, [this]() { _handleIndex(); });
  _server.on("/update", HTTP_POST,
             [this]() { _handleUpdateFinish(); },
             [this]() { _handleUpload(); });
}

bool OtaServer::startAP() {
  if (_running) return true;

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(_localIP, _gateway, _subnet);
  if (!WiFi.softAP(_ssid, _pass)) {
    return false;
  }

  _server.begin();
  _running = true;
  return true;
}

bool OtaServer::stopAP() {
  if (!_running) return true;

  if (Update.isRunning()) {
    return false;
  }

  _server.stop();
  WiFi.softAPdisconnect(true);
  _running = false;
  return true;
}

void OtaServer::handle() {
  if (_running) {
    _server.handleClient();
  }
}

void OtaServer::_handleIndex() {
  _server.send(200, "text/html", _index_html);
}

void OtaServer::_handleUpdateFinish() {
  if (Update.hasError()) {
    _server.send(500, "text/plain", "Update failed");
  } else {
    _server.send(200, "text/html", "<h3>Update successful. Rebooting...</h3>");
    delay(1000);
    ESP.restart();
  }
}

void OtaServer::_handleUpload() {
  HTTPUpload& upload = _server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Update.begin(UPDATE_SIZE_UNKNOWN);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.isRunning()) {
      Update.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Update.end(true);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
  }
}