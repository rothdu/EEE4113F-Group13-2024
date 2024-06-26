#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Pi_WiFi";
const char* password = "Thiyashan";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  server.on("/upload", HTTP_POST, handleFileUpload);
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.println("File upload started");
    // Open a file to save the uploaded data
    // Example: File file = SD.open("/path/to/uploaded/file", FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write data to the file
    // Example: file.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    // Close the file
    // Example: file.close();
    Serial.println("File upload completed");
  }
}
