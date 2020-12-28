#define LARGE_JSON_BUFFERS 1
#include <ESP8266WiFi.h>
#include <Thing.h>
#include <WebThingAdapter.h>
uint8_t abfrage[] = { 0xFF, 0xFF, 0x0B, 0x00, 0x06, 0x04, 0x04, 0x19 };
int i = 0;
byte bufferRX[82];
const char* ssid     = "your SSID";    (edit)
const char* password = "your PASSWORD";  (edit)

// Hostname used by mDNS
const String mDNSHostname = "weather_thing";
WebThingAdapter* adapter;
const char* sensorTypes[] = {"TemperatureSensor", nullptr};
ThingDevice sensor("Wetter", "Wetterwerte", sensorTypes);

ThingProperty sensorTempIn("Akku-Raum Temp.", "", NUMBER, nullptr);
ThingProperty sensorTempOut("Aussen Temp.", "", NUMBER, nullptr);
ThingProperty sensorWind("Wind", "", NUMBER, nullptr);
ThingProperty sensorSolar("Solar", "", NUMBER, nullptr);
ThingProperty sensorRegen("Regen heute", "", NUMBER, nullptr);
ThingProperty sensorHumOut("Aussen Feuchte", "", NUMBER, nullptr);

const char* host = "192.168.1.137"; //IP-Adress of th Weather-Station (edit)
const uint16_t port = 45000;
unsigned long timeout = 60000;
unsigned long mytimer = 0;

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  

  // Create new WebThings connection handle (default port: 80)
  adapter = new WebThingAdapter(mDNSHostname, WiFi.localIP());
  sensorTempIn.unit = "celsius";
  sensorTempOut.unit = "celsius";
  sensorHumOut.unit = "percent";
  sensorWind.unit = "km/h";
  sensorSolar.unit = "Watt / m²";
  sensorRegen.unit = "Liter / m²";

  // Associate properties with device
  sensor.addProperty(&sensorTempIn);
  sensor.addProperty(&sensorTempOut);
  sensor.addProperty(&sensorWind);
  sensor.addProperty(&sensorSolar);
  sensor.addProperty(&sensorRegen);
  sensor.addProperty(&sensorHumOut);

  // Associate device with connection
  adapter->addDevice(&sensor);

  // Start mDNS and HTTP server
  adapter->begin();
}

void loop() {
  ThingPropertyValue tpVal;

 //***** Abfrage 1x in der Minute *****
 if (millis() > timeout + mytimer) { 
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
   WiFiClient client; 
   if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }
  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.write(abfrage,8);
    delay(200);
  }
   // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    bufferRX[i] = (client.read());
    //Serial.println(" ");
    //Serial.print(bufferRX[i], HEX);
    //Serial.print(" ");
    i++;
  }
   mytimer = millis();
   i = 0;

 }

  int CR = ((uint16_t)bufferRX[5] << 8) + (uint16_t)bufferRX[5];
  int innentemperatur = ((uint16_t)bufferRX[7] << 8) + (uint16_t)bufferRX[8];
  Serial.print((innentemperatur)/10.0);
  Serial.println(" Innentemperatur");

  if (innentemperatur > 65280){
     innentemperatur = (innentemperatur - 65536);
  }
  
  int aussentemperatur = ((uint16_t)bufferRX[10] << 8) + (uint16_t)bufferRX[11];
  Serial.print((aussentemperatur)/10.0);
  Serial.println(" Aussentemperatur");

  if (aussentemperatur > 65280){
     aussentemperatur = (aussentemperatur - 65536);
  }

  int aussenfeuchte = (uint16_t)bufferRX[24];
  Serial.print((aussenfeuchte)/10.0);
  Serial.println(" Aussenfeuchte");

  int windgeschwindigkeit = ((uint16_t)bufferRX[35] << 8) + (uint16_t)bufferRX[36];
  Serial.print(((windgeschwindigkeit)/10.0)*3.6);
  Serial.println(" Windgeschwindigkeit");

  unsigned long tagesniederschlag =  ((uint32_t)bufferRX[46] << 24) + ((uint32_t)bufferRX[47] << 16) + ((uint32_t)bufferRX[48] << 8) + (uint32_t)bufferRX[49];
  Serial.print((tagesniederschlag)/10.0);
  Serial.println(" Tagesniederschlag");
  
  unsigned long jahresniederschlag =  ((uint32_t)bufferRX[61] << 24) + ((uint32_t)bufferRX[62] << 16) + ((uint32_t)bufferRX[63] << 8) + (uint32_t)bufferRX[64];
  Serial.print((jahresniederschlag)/10.0);
  Serial.println(" Jahresniederschlag");
  
  unsigned long licht = ((uint32_t)bufferRX[71] << 24) + ((uint32_t)bufferRX[72] << 16) + ((uint32_t)bufferRX[73] << 8) + (uint32_t)bufferRX[74];
  Serial.print ((licht)/10);
  Serial.println(" Lux");
  Serial.print ((licht)/1300.0);
  Serial.println(" W/m²");
  Serial.println();
  if (licht < 10){
    licht = 0.0;
  }

if (CR = 20484){
  // Update device values
  delay(200);
  tpVal.number = (innentemperatur /10.0);
  sensorTempIn.setValue(tpVal);
  adapter->update();

  delay(200);
  tpVal.number = (aussentemperatur /10.0);
  sensorTempOut.setValue(tpVal);
  adapter->update();

  delay(200);
  tpVal.number = aussenfeuchte;
  sensorHumOut.setValue(tpVal);
  adapter->update();
  
  delay(200);
  tpVal.number = ((windgeschwindigkeit /10.0)*3.6);
  sensorWind.setValue(tpVal);
  adapter->update();

  delay(200);
  tpVal.number = (licht /1300);
  sensorSolar.setValue(tpVal);
  adapter->update();

  delay(200);
  tpVal.number = (tagesniederschlag /10.0);
  sensorRegen.setValue(tpVal);
  adapter->update();
  }

}
