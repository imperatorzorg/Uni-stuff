/*
To run that code all you need is properly wired ESP.
 
1. Connect your phone to ESP Access Point, credentials are in the code,
2. On your phone browse to:
    x.x.x.x/wifiSetup?ssid=napier5gee&pass=fourwordsalluppercase (where ssid and pass are your home access point credentials)
    hit Go, wait 5s.
 3. Browse to 192.168.4.1 on your phone to get Local IP, access to server using provided IP.
 4. To reset settings browse either AP IP or local IP and append following to IP address: /cleareeprom , hit Go or enter.
 
*/
  
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <time.h>

#define DEBUG Serial

ESP8266WebServer server(80);

DHT dht1(5, DHT22); 

const char* ssid = "smartpot01";
const char* passphrase = "napier01";
static bool hasSD = false;

File conf;
File uploadFile;   // rather not needed ...from example ...
File sensors;

String st;
String content;
int statusCode;
float data;                                          // data to be read from sensor
String data2;                                        // string data of the sensor
String text;                                         // any text
int discharge = 100;                                          // Required delay to read valid sensor data
int pinA0 = A0;                                               // Analog Sensors ADC pin read
int ldr_value;                                                // LDR light sensors value  
char ldrValue[4];                                          
int mst_value;                                                // MST sensors value
int a,b,c;  // y0, y1, y2
int r[] = { 0, 0, 0 }; 
int selPin[] = { 2, 4, 16 };
int count = 0;    // count Mux'Y' pin selection
char full[6] ="Full";
char half[6] ="Half";
char low[5]  ="Low";
char empty[7]="Empty";
int mstval;
char filename[16];
char *s = "log";

int set[3],i=0;

unsigned long intervalALL = 10000;                            // Time to wait for 
unsigned long previousMillisALL = 0;                 

void setup() {
  DEBUG.begin(115200);
  
  if (SD.begin(SS)){
  DEBUG.println("SD Card initialized.");
  hasSD = true;
  } else {
  DEBUG.println("SD not exists");
  }
  
  for(int pin = 0; pin < 3; pin++){      // setup of Mux select pins
    pinMode(selPin[pin], OUTPUT);
  } 

  EEPROM.begin(512);
  delay(10);
  DEBUG.println();
  DEBUG.println();
  DEBUG.println("Startup");
  // read eeprom for ssid and pass
  DEBUG.println("Reading EEPROM SSID");
  String esid;
  for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  DEBUG.print("SSID: ");
  DEBUG.println(esid);
  DEBUG.println("Reading EEPROM password");
  String epass = "";
  for (int i = 32; i < 96; ++i)
    {
      epass += char(EEPROM.read(i));
    }
  DEBUG.print("PASS: ");
  DEBUG.println(epass);  
  if ( esid.length() > 1 ) {
      WiFi.begin(esid.c_str(), epass.c_str());
      if (testWifi()) {
        launchWeb(0);
        return;
      } 
  }
  setupAP();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  DEBUG.println("\nWaiting for time");
  while (!time(nullptr)) {
    DEBUG.print(".");
    delay(1000);
  }
}

bool testWifi(void) {
  int c = 0;
  DEBUG.println("Wait...");  
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; } 
    delay(500);
    DEBUG.print(WiFi.status());    
    c++;
  }
  DEBUG.println("");
  DEBUG.println("Connection timed out, AP mode");
  return false;
} 

void launchWeb(int webtype) {
  DEBUG.println("");
  DEBUG.println("WiFi connected");
  DEBUG.print("Local IP: ");
  DEBUG.println(WiFi.localIP());
  DEBUG.print("SoftAP IP: ");
  DEBUG.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  DEBUG.println("Server started"); 
}

void setupAP(void) {
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  DEBUG.println("scan done");
  if (n == 0)
    DEBUG.println("no networks found");
  else
  {
    DEBUG.print(n);
    DEBUG.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      DEBUG.print(i + 1);
      DEBUG.print(": ");
      DEBUG.print(WiFi.SSID(i));
      DEBUG.print(" (");
      DEBUG.print(WiFi.RSSI(i));
      DEBUG.print(")");
      DEBUG.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     }
  }
  DEBUG.println(""); 
  st = "<ol>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<li>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</li>";
    }
  st += "</ol>";
  delay(100);
  WiFi.softAP(ssid, passphrase, 6);

  DEBUG.println("softap");
  launchWeb(1);
  DEBUG.println("over");
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {

    /* Main webpage of the server, displays details about device when device is in AP mode */
    
    server.on("/", []() {
    IPAddress ip = WiFi.softAPIP();
    String page = "{";
    page += ("\"Soft AP IP\":\[\"");
    page += WiFi.softAPIP().toString();
    page += ("\"\]\,\n\n");
    page += ("\"Chip ID\":\[\"");
    page += ESP.getChipId();
    page += ("\"\]\,\n\n");
    page += ("\"Flash Chip ID\":\[\"");
    page += ESP.getFlashChipId();
    page += ("\"\]\,\n\n");
    page += ("\"IDE Flash Size\":\[\"");
    page += ESP.getFlashChipSize();
    page += ("\"\]\,\n\n");
    page += ("\"Real Flash Size\":\[\"");
    page += ESP.getFlashChipRealSize();
    page += ("\"\]\,\n\n");
    page += ("\"Soft AP MAC\":\[\"");
    page += WiFi.softAPmacAddress();
    page += ("\"\]\,\n\n");
    page += ("\"Station MAC\":\[\"");
    page += WiFi.macAddress();
    page += ("\"]}");
    server.send(200, "application/json", page);
    });

    /* This URL is used to take two specific arguments: SSID and password and store them in EEPROM of the device, 
       usage e.g.: x.x.x.x/wifiSetup?ssid=SSID&pass=PASSWORD */
    
    server.on("/wifiSetup", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        if (qsid.length() > 0 && qpass.length() > 0) {
          DEBUG.println("clearing eeprom");
          for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
          DEBUG.println(qsid);
          DEBUG.println("");
          DEBUG.println(qpass);
          DEBUG.println("");
            
          DEBUG.println("Writing EEPROM SSID:");
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(i, qsid[i]);
              DEBUG.print("Wrote: ");
              DEBUG.println(qsid[i]); 
            }
          DEBUG.println("Writing EEPROM password:"); 
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(32+i, qpass[i]);
              DEBUG.print("Wrote: ");
              DEBUG.println(qpass[i]); 
            }    
          EEPROM.commit();
          content = "{\"Success\":\"saved to EEPROM... reset\"}";
          statusCode = 200;
          delay(500);
          ESP.restart();
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          DEBUG.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    
    /* Main webpage of the server, displays server Local IP */
    
    server.on("/", []() {
      String page = "{";
      page += ("\"Server IP\":\[\"");
      page += WiFi.localIP().toString();
      page += ("\"]}");
      server.send(200, "application/json", page);
    });
    
    /* This URL is used to display details about the hardware, can be used to distinguish from 2 or more different pots */
    
    server.on("/details", []() {
      String page = "{";
      page += ("\"Soft AP IP\":\[\"");
      page += WiFi.softAPIP().toString();
      page += ("\"\]\,\n\n");
      page += ("\"Server IP\":\[\"");
      page += WiFi.localIP().toString();
      page += ("\"\]\,\n\n");
      page += ("\"Chip ID\":\[\"");
      page += ESP.getChipId();
      page += ("\"\]\,\n\n");
      page += ("\"Flash Chip ID\":\[\"");
      page += ESP.getFlashChipId();
      page += ("\"\]\,\n\n");
      page += ("\"IDE Flash Size\":\[\"");
      page += ESP.getFlashChipSize();
      page += ("\"\]\,\n\n");
      page += ("\"Real Flash Size\":\[\"");
      page += ESP.getFlashChipRealSize();
      page += ("\"\]\,\n\n");
      page += ("\"Soft AP MAC\":\[\"");
      page += WiFi.softAPmacAddress();
      page += ("\"\]\,\n\n");
      page += ("\"Station MAC\":\[\"");
      page += WiFi.macAddress();
      page += ("\"]}");
      server.send(200, "application/json", page);
    });

    server.on("/sensors", []() {
      String page = "{";
      page += ("\"airhumid\":\[\"");
      page += humidDHT22();
      page += ("\"\]\,\n\n");
      page += ("\"airtempc\":\[\"");
      page += tempcDHT22();
      page += ("\"\]\,\n\n");
      page += ("\"airtempf\":\[\"");
      page += tempfDHT22();
      page += ("\"\]\,\n\n");
      page += ("\"soilhumid\":\[\"");
      page += loggingMst();
      page += ("\"\]\,\n\n");
      page += ("\"light\":\[\"");
      page += loggingLDR1();
      page += ("\"\]\,\n\n");
      page += ("\"water\":\[\"");
      page += readH2O();
      page += ("\"]}");
      server.send(200, "application/json", page);
    });
    
    /* This URL is used to take any number of arguments and save them to the file, 
    usage e.g.: x.x.x.x/uploadSettings?arg1=1000&arg2=2000 */
    
    server.on("/uploadSettings", []() {
      String message = "";
      for (int i = 0; i < server.args(); i++) {
        message += server.arg(i) + "\n";          //Get the value of the parameter
      }
      char* buf = "config.ini";
      SD.remove(buf);
      conf = SD.open("config.ini", FILE_WRITE);
      if (conf) 
      {
        conf.print(message);
        conf.close();
        server.send(200, "text/plain", message + "Data saved to file.");
      } 
      else 
      {
        DEBUG.println("No file");
        server.send(200, "text/plain", message + "File corrupt or not exists, data not delivered.");
      }
    });
    
    /* This URL is used to clear SSID and password from EEPROM memory of the device */
    
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      DEBUG.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
      WiFi.disconnect();
      delay(500);
      ESP.restart();
    });
   
    server.on("/log", []() {
    //String message = "{";
    String message = "{";
    String logData = "";
    if (server.argName(0) == "data") {
      String filePath = "";
      filePath += server.arg(0);
      //filePath += ".txt";
      logData += readFile(filePath);
      if (logData == "-") {
        server.send(404, "text/plain", "Not Found");
      }
      else {
        message += logData;
        message += "}";
        server.send(200, "application/json", message);
      }
    }
    else {
      server.send(404, "text/plain", "Not Found!");
    }
  });
    
  }
}

String readFile(String path) {
  File myFile;
  String fileBuffer = "";
  myFile = SD.open(path);
  if (myFile)
  {
    while (myFile.available()) 
      {
        //DEBUG.println("blablabla");
        fileBuffer += char(myFile.read()); 
      }
    myFile.close();
  }
  else 
    { 
      DEBUG.println("Missing file");
    }
  return fileBuffer;
}

/*
void startSD(){  
  if (SD.begin(SS)){                                      // pin SS:15
     DEBUG.println("SD Card initialized.");
     hasSD = true;
  }
}
*/

float LoggingToSD(String text){         /////////////// Logs float data to SD card
  int n = 0;
  snprintf(filename, sizeof(filename), "%s%02d", s, n);
  while(SD.exists(filename)) {
    n++;
    snprintf(filename, sizeof(filename), "%s%02d", s, n);
  }
  File sensors = SD.open(filename, FILE_WRITE);
  if (sensors) {
    sensors.print(text);
    sensors.close();
  } 
  else 
  {
    DEBUG.println("No file");
  }
}
//////////////////////////////////////////////////////////////// Log file size to SD card /////
float filesize(){                                          
  if (sensors = SD.open("logfile", FILE_WRITE)){
    sensors.print("Log file size:  ");
    sensors.println(sensors.size());                     
    sensors.close();
    } 
  else 
  {
    DEBUG.println("No file");
  }
}

/////////////////////////////////////////////////////////////// Log Time Stamp to SD card //////
char logTimeStamp(){                                      
  sensors = SD.open("logfile", FILE_WRITE);
   if (sensors){
    time_t now = time(nullptr);
    sensors.print("Reading at:     ");
    sensors.println(ctime(&now));                     
    sensors.close();
    } 
  else 
  {
    DEBUG.println("No file");
  }
}

//H20+//////////////////////// WATER LVL SENSOR BLOCK////////////////////////////////
String readH2O()
  {
  // select the read sequence( bit) for 3 channels of water level sensor, 1st read all 0 ...
  for (count=0; count<=2; count++) {
    r[0] = bitRead(count,0);         
    r[1] = bitRead(count,1);         
    r[2] = bitRead(count,2);         
  // truth table / channel selection 
   digitalWrite(selPin[0], r[0]);
   digitalWrite(selPin[1], r[1]);
   digitalWrite(selPin[2], r[2]);
  // store Mux channels y0-2 value (returned sum of 16 readings) No water = no ground & wire = High(16300)
 switch(count){ 
  case 0:
    a = suma();
    if(a < 16300){  
     return full;
    }
  case 1:
    b = suma();
    if(b < 16300){
    return half;
    }
  case 2:
    c = suma();
    if(c < 16300){
     return low;
    }else{
    if(count != 0 && count != 1){
      return empty;
      break;                          // No water Error handling
    }
    }
   }
  }
  DEBUG.println("");
}
//H20-/////////////////////////////////////////////////////////////////////////////////
int suma(){                          // pre read mark & sum for water lvl sensor accuracy, and used for average 
    analogRead(A0);  
    delay(200);
    int sum = 0;
    for (int i = 0; i < 16; i++) {
    sum += analogRead(A0);
    delay(1);
    }
    return sum;
}
//DHT+////////////////////////////////////////////////////////////////////////////////
float tempcDHT22()
{  
  float tempC; 
  tempC = dht1.readTemperature();
  //DEBUG.print("Temperature: "); 
  //DEBUG.print(tempC,1);
  //DEBUG.println("�C");
  //DEBUG.println("");
  return tempC;
}
float tempfDHT22()
{  
  float tempF; 
  tempF = dht1.readTemperature(true);
  //DEBUG.print("Temperature: "); 
  //DEBUG.print(tempF,1);
  //DEBUG.println("�F");
  //DEBUG.println("");
  return tempF;
}
float humidDHT22()
{
  float humid;
  humid = dht1.readHumidity();

  //DEBUG.print("Humidity:    "); 
  //DEBUG.print(humid); 
  //DEBUG.println("%RH");
  //DEBUG.println("");

  return humid;
}
//DHT-/////////////////////////////////////////////////////////////////////////////LDR+// 
int loggingLDR1()                            // Analog Light sensor logging: LDR: Phototransistor #1
{   
  //set the selectPins to read channel y4
  digitalWrite(selPin[0],LOW);  // a
  digitalWrite(selPin[1],LOW);  // b
  digitalWrite(selPin[2],HIGH); // c
  analogRead(pinA0);
  delay(100);
  ldr_value = suma()/16;                      // Take average from 16 readings
  ldr_value = map(ldr_value, 0, 1024, 0, 100);// Map analog reading to precentage value 0-100

  //DEBUG.print("Light Meter: ");             // Measuring light level in precentage of LUX
  //DEBUG.print(ldr_value);
  //DEBUG.println("% LUX");
  //DEBUG.println("");

  return ldr_value;
}
//LDR-//////////////////////////////////////////////////////////////////////////////MST+//
int loggingMst()
  {
  digitalWrite(selPin[0],HIGH);  // a
  digitalWrite(selPin[1],HIGH);  // b
  digitalWrite(selPin[2],LOW);   // c
  analogRead(pinA0);
  delay(100);
  mst_value = suma()/16;         // Take average from 16 readings
  mst_value = map(mst_value, 300, 900, 0, 100);
  mst_value = constrain(mst_value,0,100);

  //DEBUG.print("Mositure: ");
  //DEBUG.print(mst_value);
  //DEBUG.println("%");
  //DEBUG.println("");
  return mst_value;
  }
//MST-////////////////////////////////////////////////////////////////////////////////////
void pump(){      // Pump ON - OFF connected to Mux channel  or this case directelly to pin3 due issues with reading

  pinMode(3,OUTPUT);
  int dly = set[0];

  File sttngs = SD.open("config.ini");
  if (sttngs)
  {
    while (sttngs.available())
    { 
      //write array from file   
      for(i=0;i<3;i++) 
      {
        set[i]=sttngs.parseInt();
      }
      sttngs.close();
    }
  } else {
    Serial.println("Cannot open file!");
  }
  if (dly == 1)
  {
    digitalWrite(3, HIGH);
    delay(1000);
    digitalWrite(3, LOW);
    delay(1000);
  }
  else {
    digitalWrite(3, LOW);
  }
}

void loop() {
  server.handleClient();
  int dly = set[0];
  unsigned long currentMillis = millis();

  if ((unsigned long)(currentMillis - previousMillisALL) >= intervalALL) 
  {
    mstval = loggingMst();
    //data2 = loggingMst();
    if(mstval < 25){
      pump();               // Testing powering device ( LED on pin3)
    }
    String data = "";
    data += ("\"airhumid\":\[\"");
    data += humidDHT22();
    data += ("\"\]\,\n\n");
    data += ("\"airtempc\":\[\"");
    data += tempcDHT22();
    data += ("\"\]\,\n\n");
    data += ("\"airtempf\":\[\"");
    data += tempfDHT22();
    data += ("\"\]\,\n\n");
    data += ("\"soilhumid\":\[\"");
    data += loggingMst();
    data += ("\"\]\,\n\n");
    data += ("\"light\":\[\"");
    data += loggingLDR1();
    data += ("\"\]\,\n\n");
    data += ("\"water\":\[\"");
    data += readH2O();
    data += ("\"]");
    LoggingToSD(data);
    DEBUG.println(data);
    previousMillisALL = currentMillis;      // Save current time dht pin previousMillis
  }
}




