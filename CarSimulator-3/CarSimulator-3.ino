#include <Nextion.h>
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>


int EGDoor_Status = 0;
int FLDoor_Status = 0;
int FRDoor_Status = 0;
int BLDoor_Status = 0;
int BRDoor_Status = 0;
int TDoor_Status = 0;
int Lock_Status = 0;
int AC_Status = 0;
int Speedkph = 0;
int EGRPM = 0;
int Oil = 0;
int EGTemp = 0;
int InTemp = 0;
int OutTemp = 0;
char carid[] = "";
String car = "";
File fsUploadFile;

bool wifistatus = false;
bool serverstatus = false;


void startWiFi();
void startSPIFFS();
void startWebSocket();
void startServer();



ESP8266WiFiMulti wifiMulti;         // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server(80);       // create a web server on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81
SoftwareSerial Serial2(13, 15);
Nextion hmi(Serial2, 9600);
WiFiClientSecure client;

const char* host = "testapi.dennysora.com";
const int httpsPort = 8081;

const char fingerprint[] PROGMEM = "38 1C C2 79 A5 68 13 DB 21 C2 83 54 2B 12 65 82 E5 7D 91 74";

const char *ssid = "ESP8266 Access Point"; // The name of the Wi-Fi network that will be created
const char *password = "thereisnospoon";   // The password required to connect to it, leave blank for an open network

const char *OTAName = "ESP8266";           // A name and a password for the OTA service
const char *OTAPassword = "esp8266";


void handledisplay() {
  String message = hmi.listen();
  if (message != "") { // if a message is received...
    Serial.println(message); //...print it out
  }

  if (message == "65 2 40 0 ff ff ff") {
    String hmissid = hmi.listen().substring(3);
    Serial.print("ssid:");
    Serial.println(hmissid);
    String hmips = hmi.listen().substring(3);
    Serial.print("ps:");
    Serial.println(hmips);
  }
  if (message == "65 1 2 1 ff ff ff")
    if (wifistatus == true) {
      hmi.setComponentText("t6", "ON");
      hmi.setComponentType("t6", "bco", "2024");
    }
    else {
      hmi.setComponentText("t6", "OFF");
      hmi.setComponentType("t6", "bco", "63488");
    }
  if (message == "65 3 40 0 ff ff ff") {
    String hmicarid = hmi.listen().substring(3);
    Serial.print("carid:");
    Serial.println(hmicarid);
    String hmitoken = hmi.listen().substring(3);
    Serial.print("token:");
    Serial.println(hmitoken);
  }
  if (message == "65 1 1 1 ff ff ff")
    if (wifistatus == true) {
      hmi.setComponentText("t6", "ON");
      hmi.setComponentType("t6", "bco", "2024");
    }
    else {
      hmi.setComponentText("t6", "OFF");
      hmi.setComponentType("t6", "bco", "63488");
    }
  if (message == "65 1 3 0 ff ff ff") {
    if (EGDoor_Status == 1)
      hmi.setComponentType("b0", "bco", "2024");
    else
      hmi.setComponentType("b0", "bco", "63488");
    if (FLDoor_Status == 1)
      hmi.setComponentType("b1", "bco", "2024");
    else
      hmi.setComponentType("b1", "bco", "63488");
    if (FRDoor_Status == 1)
      hmi.setComponentType("b2", "bco", "2024");
    else
      hmi.setComponentType("b2", "bco", "63488");
    if (BLDoor_Status == 1)
      hmi.setComponentType("b3", "bco", "2024");
    else
      hmi.setComponentType("b3", "bco", "63488");
    if (BRDoor_Status == 1)
      hmi.setComponentType("b4", "bco", "2024");
    else
      hmi.setComponentType("b4", "bco", "63488");
    if (TDoor_Status == 1)
      hmi.setComponentType("b5", "bco", "2024");
    else
      hmi.setComponentType("b5", "bco", "63488");
  }
  if (message == "65 4 1 0 ff ff ff") {
    if (EGDoor_Status == 0) {
      EGDoor_Status = 1;
      hmi.setComponentType("b0", "bco", "2024");
    }
    else {
      EGDoor_Status = 0;
      hmi.setComponentType("b0", "bco", "63488");
    }
    Serial.print("Hood:");
    Serial.println(EGDoor_Status);
  }
  if (message == "65 4 2 0 ff ff ff") {
    if (FLDoor_Status == 0) {
      FLDoor_Status = 1;
      hmi.setComponentType("b1", "bco", "2024");
    }
    else {
      FLDoor_Status = 0;
      hmi.setComponentType("b1", "bco", "63488");
    }
    Serial.print("Front Left Door:");
    Serial.println(FLDoor_Status);
  }
  if (message == "65 4 3 0 ff ff ff") {
    if (FRDoor_Status == 0) {
      FRDoor_Status = 1;
      hmi.setComponentType("b2", "bco", "2024");
    }
    else {
      FRDoor_Status = 0;
      hmi.setComponentType("b2", "bco", "63488");
    }
    Serial.print("Front Right Door:");
    Serial.println(FRDoor_Status);
  }
  if (message == "65 4 4 0 ff ff ff") {
    if (BLDoor_Status == 0) {
      BLDoor_Status = 1;
      hmi.setComponentType("b3", "bco", "2024");
    }
    else {
      BLDoor_Status = 0;
      hmi.setComponentType("b3", "bco", "63488");
    }
    Serial.print("Back Left Door:");
    Serial.println(BLDoor_Status);
  }
  if (message == "65 4 5 0 ff ff ff") {
    if (BRDoor_Status == 0) {
      BRDoor_Status = 1;
      hmi.setComponentType("b4", "bco", "2024");
    }
    else {
      BRDoor_Status = 0;
      hmi.setComponentType("b4", "bco", "63488");
    }
    Serial.print("Back Right Door:");
    Serial.println(BRDoor_Status);
  }
  if (message == "65 4 6 0 ff ff ff") {
    if (TDoor_Status == 0) {
      TDoor_Status = 1;
      hmi.setComponentType("b5", "bco", "2024");
    }
    else {
      TDoor_Status = 0;
      hmi.setComponentType("b5", "bco", "63488");
    }
    Serial.print("Trunk Door:");
    Serial.println(TDoor_Status);
  }
  if (message == "65 1 4 1 ff ff ff") {
    hmi.setComponentText("t2", String(Speedkph));
    hmi.setComponentType("h0", "val", String(Speedkph));
    hmi.setComponentType("h1", "val", String(Oil));
    hmi.setComponentText("t3", String(Oil));
  }
  if (message == "65 5 1 0 ff ff ff") {
    String speedtemp = hmi.listen().substring(3);
    Speedkph = speedtemp.toInt();
    Serial.print("Speed:");
    Serial.println(Speedkph);
  }
  if (message == "65 5 2 0 ff ff ff") {
    String oiltemp = hmi.listen().substring(3);
    Oil = oiltemp.toInt();
    Serial.print("Oil:");
    Serial.println(Oil);
  }
  if (message == "65 1 5 1 ff ff ff") {
    hmi.setComponentText("t3", String(EGTemp));
    hmi.setComponentText("t4", String(InTemp));
    hmi.setComponentText("t5", String(OutTemp));
    hmi.setComponentType("h0", "val", String(EGTemp));
    hmi.setComponentType("h1", "val", String(InTemp));
    hmi.setComponentType("h2", "val", String(OutTemp));
  }
  if (message == "65 6 1 0 ff ff ff") {
    String EGTemptemp = hmi.listen().substring(3);
    EGTemp = EGTemptemp.toInt();
    Serial.print("EGTemp:");
    Serial.println(EGTemp);
  }
  if (message == "65 6 2 0 ff ff ff") {
    String InTemptemp = hmi.listen().substring(3);
    InTemp = InTemptemp.toInt();
    Serial.print("InTemp:");
    Serial.println(InTemp);
  }
  if (message == "65 6 7 0 ff ff ff") {
    String OutTemptemp = hmi.listen().substring(3);
    OutTemp = OutTemptemp.toInt();
    Serial.print("OutTemp:");
    Serial.println(OutTemp);
  }
}
String togetcaruuid(String AccountID,String CarName,String TempCode) {
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);


  if (client.connect(host, httpsPort)) {

    Serial.println("API Server Connect!");
    DynamicJsonDocument serverrequest(1024);
    
    String request = "{\"operationName\":\"AddCarID\",\"variables\":{},\"query\":\"mutation AddCarID {\\n AddCarID(AccountID: \\\"abcde@gmail.com\\\", CarName: \\\"123456\\\", TemporarilyToken: \\\"567878\\\") {\\n Status {\\n StatusCode\\n Description\\n }\\n AccountID\\n CarToken\\n }\\n}\\n\"}";
    deserializeJson(serverrequest, request);
    JsonObject serverrequestobj = serverrequest.as<JsonObject>();JsonObject obj = doc.as<JsonObject>();
    
    client.println("POST /query HTTP/1.1");
    client.println("Host: testapi.dennysora.com:8081");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(request.length());
    client.println();
    client.println(request);
    Serial.println("request sent");
    while (client.connected()) {
      while (client.available()) {
        Serial.println( client.readStringUntil('\r'));
      }
    }
    Serial.println("closing connection");
    client.stop();
  }

  else
    Serial.println("API Server Fail!");




}




/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {
  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  Serial.println("\r\n");

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server

  startServer();               // Start a HTTP server with a file read handler and an upload handle

  hmi.init();

  togetcaruuid();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(100);
  }
  delay(10);

}







/*__________________________________________________________LOOP__________________________________________________________*/
// The rainbow effect is turned off on startup


void loop() {
  webSocket.loop();
  server.handleClient();
  handledisplay();
}



/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/


void startWiFi() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");

  wifiMulti.addAP("DJMP", "andy0970");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("Mountain.P", "00008888");
  wifiMulti.addAP("Mountain.P'sMacBookPro", "00008888");
  wifiMulti.addAP("Unini", "123456500");
  wifiMulti.addAP("EE_2.4G", "123456500");

  Serial.println("Connecting");
  while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {  // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");
  if (WiFi.softAPgetStationNum() == 0) {     // If the ESP is connected to an AP
    wifistatus = true;
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println("\r\n");
}

void startSPIFFS() { // Start the SPIFFS and list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started.");
}

void startServer() { // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);                       // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists

  server.begin();                             // start the HTTP server
  Serial.println("HTTP server started.");
}
/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound() { // if the requested file or page doesn't exist, return a 404 not found error
  if (!handleFileRead(server.uri())) {        // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {                         // The file server always prefers a compressed version of a file
      String pathWithGz = path + ".gz";                  // So if an uploaded file is not compressed, the existing compressed
      if (SPIFFS.exists(pathWithGz))                     // version of that file must be deleted (if it exists)
        SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/success.html");     // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      //Serial.printf("%s\n", payload);
      char recivestring;
      char * root = (char *)payload;
      char *delim = ":";
      int type = *strtok(root , delim) - '0';
      if (type == 1) {
        car = strtok(NULL, delim);
        EGDoor_Status = atoi(strtok(NULL, delim));
        FLDoor_Status = atoi(strtok(NULL, delim));
        FRDoor_Status = atoi(strtok(NULL, delim));
        BLDoor_Status = atoi(strtok(NULL, delim));
        BRDoor_Status = atoi(strtok(NULL, delim));
        TDoor_Status = atoi(strtok(NULL, delim));
        Lock_Status = atoi(strtok(NULL, delim));
        AC_Status = atoi(strtok(NULL, delim));
      } else if (type == 0) {
        car = strtok(NULL, delim);
        Oil = atoi(strtok(NULL, delim));
        EGTemp = atoi(strtok(NULL, delim));
        InTemp = atoi(strtok(NULL, delim));
        OutTemp = atoi(strtok(NULL, delim));
      } else if (type == 2) {
        car = strtok(NULL, delim);
        Serial.println(car);
      }
      else if (type == 3) {
        car = strtok(NULL, delim);
        Speedkph = atoi(strtok(NULL, delim));
        EGRPM = atoi(strtok(NULL, delim));
      }

      //   String senddata = String(EGDoor_Status) + ":" + String(FLDoor_Status) + ":" + String( FRDoor_Status) + ":" + String(BLDoor_Status) + ":" + String(BRDoor_Status) + ":" + String(TDoor_Status) + ":" + String(Lock_Status) + ":" + String(AC_Status) + ":" + String(Speedkph) + ":" + String( Oil) + ":" + String(EGRPM) + ":" + String(EGTemp) + ":" + String(InTemp) + ":" + String(OutTemp);
      StaticJsonDocument<200> jsonBuffer;
      // char json[] ="{\"type\":\"gquge\","\"km\":110,\"krpm\":1,\"radiator\":\"55\",\"tank\":50,\"carId\":\"a123\",\"f\":\"Close\",\"lf\":\"Close\",\"rf\":\"Close\",\"lb\":\"Close\",\"rb\":\"Close\",\"b\":\"Close\"}";

      // deserializeJson(jsonBuffer, json);
      //JsonObject& cardata = jsonBuffer.parseObject(json);
      jsonBuffer["type"] = "gauge";
      jsonBuffer["km"] = Speedkph;
      jsonBuffer["krpm"] = EGRPM;
      jsonBuffer["radiator"] = EGTemp;
      jsonBuffer["tank"] = Oil;
      jsonBuffer["carId"] = car;
      if (EGDoor_Status == 1)
        jsonBuffer["f"] = "Open";
      else
        jsonBuffer["f"] = "Close";
      if (FLDoor_Status == 1)
        jsonBuffer["lf"] = "Open";
      else
        jsonBuffer["lf"] = "Close";
      if (FRDoor_Status == 1)
        jsonBuffer["rf"] = "Open";
      else
        jsonBuffer["rf"] = "Close";
      if (BLDoor_Status == 1)
        jsonBuffer["lb"] = "Open";
      else
        jsonBuffer["lb"] = "Close";
      if (BRDoor_Status == 1)
        jsonBuffer["rb"] = "Open";
      else
        jsonBuffer["rb"] = "Close";
      if (TDoor_Status == 1)
        jsonBuffer["b"] = "Open";
      else
        jsonBuffer["b"] = "Close";
      serializeJson(jsonBuffer, Serial);
      String output;
      serializeJson(jsonBuffer, output);
      webSocket.sendTXT(num, output);
      serializeJsonPretty(jsonBuffer, Serial);
      Serial.println();
      delay(10);
      break;
  }
}





/*__________________________________________________________api post__________________________________________________________*/


/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text / html";
  else if (filename.endsWith(".css")) return "text / css";
  else if (filename.endsWith(".js")) return "application / javascript";
  else if (filename.endsWith(".ico")) return "image / x - icon";
  else if (filename.endsWith(".gz")) return "application / x - gzip";
  return "text / plain";
}
