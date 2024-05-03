#include <WiFi.h>

const char* ssid = "sdks";             // Replace with your network credentials
const char* password = "0823992390";
// String header;                                      // Variable to store the HTTP request
// // unsigned long currentTime = millis();               // Current time
// // unsigned long previousTime = 0;                     // Previous time
// // const long timeoutTime = 2000;
// WiFiServer server(8080);                              // Set web server port number to 80

// enum class Status {REQUEST, CONTENT_LENGTH, EMPTY_LINE, BODY};
// Status status = Status::REQUEST;


// void setup() { 
//   Serial.begin(115200);                             // Start Serial Monitor
//   pinMode(Led, OUTPUT);                             // Initialize the LED as an output
//   digitalWrite(Led, LOW);                           // Set LED off
//   WiFi.begin(ssid, password);                       // Connect to Wi-Fi network with SSID and password
//   while (WiFi.status() != WL_CONNECTED)             // Display progress on Serial monitor
//   { 
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");                               // Print local IP address and start web server
//   Serial.print("WiFi connected at IP Address ");
//   Serial.println(WiFi.localIP());
//   server.begin();                                   // Start Server
// }

// void loop() { 
//   WiFiClient client = server.accept();   // Listen for incoming clients
//   if (client)
//     {   // If a new client connects,
//         currentTime = millis();
//         previousTime = currentTime;
//         Serial.println("New Client.");
//         String currentLine = "";
//         while (client.connected() && currentTime - previousTime <= timeoutTime)
//             { // loop while the client's connected
//               currentTime = millis();
//               if (client.available())
//                 {   // if there's bytes to read from the client,
//                     char c = client.read();
//                     Serial.print(c);
//                 }
//            }
//           header = "";                                                    // Clear the header variable
//           client.stop();                                                  // Close the connection
//           Serial.println("Client disconnected.");                        
//           Serial.println("");
//       }
// }

/** NEW IDEA **/

#if !( defined(ARDUINO_RASPBERRY_PI_PICO_W) )
  #error For RASPBERRY_PI_PICO_W only
#endif

#include <AsyncWebServer_RP2040W.h>

int status = WL_IDLE_STATUS;

AsyncWebServer    server(8080);

#define BUFFER_SIZE         64
char temp[BUFFER_SIZE];

void handleRoot(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", temp);
  Serial.println("Handling root");
}

void handleNotFound(AsyncWebServerRequest *request)
{  
  Serial.println("Handling not found");
  String message = "File Not Found\n\n";

  message += "URI: ";
  //message += server.uri();
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
 
  request->send(404, "text/plain", message);
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("Local IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void handleImageUpload(AsyncWebServerRequest *request)
{
  Serial.println("Handling image upload");
  if(request->hasHeader("Content-Type"))
  {
    AsyncWebHeader* h = request->getHeader("Content-Type");
    Serial.print("Content-Type: ");
    Serial.println(h->value().c_str());
  }

  Serial.println(request->params());
  // evidently not parsing any params
}

void handleFileUpload(AsyncWebServerRequest *request)
{
  Serial.println("Handling file upload");
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  delay(200);

  Serial.print("\nStart Async_HelloServer on "); Serial.print(BOARD_NAME);
  Serial.print(" with "); Serial.println(SHIELD_TYPE);
  Serial.println(ASYNCTCP_RP2040W_VERSION);
  Serial.println(ASYNC_WEBSERVER_RP2040W_VERSION);

  ///////////////////////////////////
  
    Serial.print(F("Connecting to SSID: "));
  Serial.println(ssid);

  status = WiFi.begin(ssid, password);

  delay(1000);
   
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED)
  {
    delay(500);
        
    // Connect to WPA/WPA2 network
    status = WiFi.status();
  }

  printWifiStatus();
  
  ///////////////////////////////////

  // ROOT
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    handleRoot(request);
  });

  server.on(
    "/upload_image",
    HTTP_POST,
    [](AsyncWebServerRequest * request){Serial.println("inside non-upload request");},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleFileUpload(request);
      Serial.print(len);
      Serial.print(" ");
      Serial.print(index);
      Serial.print(" ");
      Serial.println(total);
  });

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.print(F("HTTP EthernetWebServer is @ IP : "));
  Serial.println(WiFi.localIP());
}

void heartBeatPrint()
{
  static int num = 1;

  Serial.print(F("."));

  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

void check_status()
{
  static unsigned long checkstatus_timeout = 0;

#define STATUS_CHECK_INTERVAL     10000L

  // Send status report every STATUS_REPORT_INTERVAL (60) seconds: we don't need to send updates frequently if there is no status change.
  if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = millis() + STATUS_CHECK_INTERVAL;
  }
}

void loop()
{
  check_status();
}
