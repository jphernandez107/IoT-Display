
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "config.h"
#include "webserver.h"

#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>

const char* ssid = "Vacio";
const char* pass = "";

uint8_t wiFlag = 0;
long timeFlag = 3000;
long actual = 0;
long timeOut = 10000;
long actual1 =0;

config_t conf;

DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

// Datos mqtt
String mqttBroker = "192.168.1.105";
int mqttPort = 1883;
String mqtt_user = "";
String mqtt_pass = "";

WiFiClient wclient;
PubSubClient client(wclient);

String clientID = "ESP-Juan";
String facultad = "ingenieria";
String aula = "sala_c";

String topicGen = "/" + facultad + "/" + aula + "/";
String topicTemp = topicGen + "temp";    //Topics invertidos con Living
String topicHum = topicGen + "hum";   //Topics invertidos con Comedor
String topicLdr = topicGen + "ldr";
String topicPresencia = topicGen + "presencia";

float temp = 0;
float hum = 0;
int ldr = 0;
bool presencia = false;

int led = 13;

bool updateFlag = false;

// Variables para la parte grafica

TFT_eSPI tft = TFT_eSPI();

#define TFT_GREY 0x5AEB // New colour
// *** COLORES
#define BLACK       0x000000
#define WHITE       0xFFFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001AFF
#define CYAN        0x001DFF
#define CYAN_BLACK  0x0014b4
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define GREY        0x2108
#define LIGHT_GREEN 0xFF6f03

// Esquemas de colores
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5



//Callback para el mqtt.
void callback(char* topic, byte* payload, unsigned int length) {
  String pay = "";
  String topicT = String(topic);
  for (int i = 0; i < length; i++) {
    pay += (char)payload[i];
  }

  if (topicT == topicTemp) {
    temp = pay.toFloat();
  } else if (topicT == topicHum) {
    hum = pay.toFloat();
  } else if (topicT == topicLdr) {
    ldr = pay.toInt();
  } else if (topicT == topicPresencia) {
    if (pay == "true") {
      presencia = true;
    } else if (pay == "false") {
      presencia = false;
    }
  }
}

void setup(void) {
  EEPROM.begin(sizeof(config_t));
  EEPROM.get(0, conf);

  Serial.begin(115200);
  WiFi.disconnect();
  ssid = conf.wifi_ssid;
  pass = conf.wifi_pass;
  mqttBroker = conf.mqtt_broker;
  mqttPort = conf.broker_puerto;
  iniciarWebServer();
  client.setServer(mqttBroker.c_str(), mqttPort);
  client.setCallback(callback);
  actual = millis();

  tft.init();
  tft.setRotation(3); //0 vertical
  tft.fillScreen(BLACK);
  tft.setCursor(10, 110);
  tft.setTextSize(3);
  tft.setTextColor(CYAN_BLACK, BLACK);
  tft.print("INICIALIZANDO...");
}

void loop(void) {

  updateFlag = WebServer_getActualizar();
  if((millis() - actual) >= timeOut){
    wifiSetup();
    actual = millis();
  }
  // Verificamos si hubo cambio en los datos a traves del server.
  if (updateFlag) {
    EEPROM.get(0, conf);
    ssid = conf.wifi_ssid;
    pass = conf.wifi_pass;
    mqttBroker = conf.mqtt_broker;
    mqttPort = conf.broker_puerto;
    topicTemp = topicGen + conf.topic_temp;
    topicHum = topicGen + conf.topic_hum;
    topicLdr = topicGen + conf.topic_ldr;
    topicPresencia = topicGen + conf.topic_presencia;
    mqtt_user = conf.mqtt_user;
    mqtt_pass = conf.mqtt_pass;
    clientID = conf.client_id;
    //admin_pass;
    client.disconnect();
    client.setServer(mqttBroker.c_str(), mqttPort);
    reconectarMQTT();

    WebServer_setActualizar(false);
    Serial.println("Actualizamos los datos");
  }

  if (!WebServer_isRunning()) {
    iniciarWebServer();
    Serial.println("Server iniciado");
  }
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  WebServer_loop();


  // Seccion Grafica!

  if (WiFi.status() == WL_CONNECTED) {
    tft.setCursor (67, 70);
    tft.setTextSize (1);
    tft.setTextColor (WHITE, BLACK);
    tft.print ("TEMPERATURA");
    tft.setCursor (70, 66);
    tft.setTextSize (1);
    tft.setTextColor (WHITE, BLACK);

    uint16_t xpos = 0, ypos = 5, gap = 4, radius = 52;

    // Iniciamos el anillo de temperatura
    xpos = 320 / 2 - 151, ypos = 7, gap = 100, radius = 90;
    ringMeter(temp, 0, 45, xpos, ypos, radius, "Celsius", GREEN2RED);

    tft.fillRect(0, 190, 320, 4, BLUE);
    tft.fillRect(0, 236, 320, 4, BLUE);
    tft.fillRect(195, 0, 4, 194, BLUE);
    tft.fillRect(195, 151, 320, 4, BLUE);
    tft.fillRect(0, 0, 4, 240, BLUE);
    tft.fillRect(0, 0, 320, 4, BLUE);
    tft.fillRect(316, 0, 4, 240, BLUE);

    tft.setCursor (9, 203); //208
    tft.setTextSize (3);
    tft.setTextColor (YELLOW, BLACK);
    tft.print ("HUMEDAD: ");

    tft.setCursor(170, 203);
    String humT;
    if (hum >= 10 && hum < 100) {
      humT = String(hum) + "%";
    } else if (hum < 10) {
      humT = "0" + String(hum) + "%";
    } else {
      hum = 99.99;
      humT = String(hum) + "%";
    }
    tft.print(humT);

    tft.setCursor (210, 164);
    tft.setTextSize (2);
    tft.setTextColor (WHITE, BLACK);
    tft.print ("Luz: ");
    tft.setCursor(260, 164);
    tft.print (ldr);

    tft.setCursor(205, 8);
    tft.setTextSize(1);
    tft.setTextColor(CYAN, BLACK);
    tft.print("IP: ");
    tft.print(WiFi.localIP());

    if (presencia) {
      crearPersona(255, 38, YELLOW);
    } else {
      crearPersona(255, 38, WHITE);
    }
  }
  delay(10);
}


void iniciarWebServer() {
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("IoT-JuanH", "");

  // Wait for connection

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  } else {
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }
  WebServer_init();

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);

  Serial.println("HTTP server started");
}
void pararWebServer() {
  Serial.println("Parando WebServer");
  WebServer_stop();
  WiFi.softAPdisconnect(true);
}

// Conexion de wifi y conexion con el broker MQTT
void wifiSetup() {

  if (WiFi.status() != WL_CONNECTED) {
    tft.fillScreen(BLACK);
    tft.setCursor(10, 110);
    tft.setTextSize(2);
    tft.setTextColor(CYAN_BLACK, BLACK);
    if (updateFlag) {
      EEPROM.get(0, conf);
      ssid = conf.wifi_ssid;
      pass = conf.wifi_pass;
      WebServer_setActualizar(false);
    }
    tft.print("Conectando a ");
    tft.print(ssid);
    tft.print("...");
    tft.print("\n");
    tft.print("SSID: IoT-JuanH");
    tft.print("\n");
    tft.print("IP de Conf: 192.168.4.1");
    
    Serial.print("Conectando a ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      return;
    }
    tft.fillScreen(BLACK);
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.print("Direccion IP: ");
    Serial.println(WiFi.localIP());
  }

  reconectarMQTT();
}

void reconectarMQTT() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      if (client.connect(clientID.c_str(), mqtt_user.c_str(), mqtt_pass.c_str())) {
        client.subscribe(topicTemp.c_str());
        client.subscribe(topicHum.c_str());
        client.subscribe(topicLdr.c_str());
        client.subscribe(topicPresencia.c_str());
      }
    }
    if (client.connected()) {
      client.loop();
    }
  }
}

/*
   #########################################################################
   #########################################################################

   SECCION GRAFICA.
*/




// #########################################################################
//  Creamos una persona para poder ver si hay alguien en la habitacion.
// #########################################################################
void crearPersona(uint16_t x, uint16_t y, uint16_t color) {
  //x,y = centro de cabeza
  uint16_t radioC = 15;

  //Cabeza
  //tft.drawCircle(x,y, radioC, color);
  //tft.drawCircle(x,y, radioC - 1, color);
  tft.fillCircle(x, y, radioC + 1, color);
  //tft.drawCircle(x,y, radioC + 2, color);

  //Torso
  tft.fillRect(x - 2, y + radioC, 4, radioC * 3, color);

  x = x;
  //Brazo derecho
  tft.drawLine(x, y + 2 * radioC, x + radioC * 2, y + 4 * radioC, color);
  tft.drawLine(x + 1, y + 2 * radioC, 1 + x + radioC * 2, y + 4 * radioC, color);
  tft.drawLine(x + 2, y + 2 * radioC, 2 + x + radioC * 2, y + 4 * radioC, color);
  tft.drawLine(x - 1, y + 2 * radioC, x + radioC * 2 - 1, y + 4 * radioC, color);

  //Pierna derecho
  tft.drawLine(x, y + 4 * radioC, x + radioC * 2, y + 7 * radioC, color);
  tft.drawLine(x + 1, y + 4 * radioC, 1 + x + radioC * 2, y + 7 * radioC, color);
  tft.drawLine(x + 2, y + 4 * radioC, 2 + x + radioC * 2, y + 7 * radioC, color);
  tft.drawLine(x - 1, y + 4 * radioC, x + radioC * 2 - 1, y + 7 * radioC, color);

  x = x - 1;
  //Brazo izquierdo
  tft.drawLine(x, y + 2 * radioC, x - radioC * 2, y + 4 * radioC, color);
  tft.drawLine(x - 1, y + 2 * radioC, x - radioC * 2 - 1, y + 4 * radioC, color);
  tft.drawLine(x - 2, y + 2 * radioC, x - radioC * 2 - 2, y + 4 * radioC, color);
  tft.drawLine(x + 1, y + 2 * radioC, x - radioC * 2 + 1, y + 4 * radioC, color);

  //Pierna izquierdo
  tft.drawLine(x, y + 4 * radioC, x - radioC * 2, y + 7 * radioC, color);
  tft.drawLine(x - 1, y + 4 * radioC, x - radioC * 2 - 1, y + 7 * radioC, color);
  tft.drawLine(x - 2, y + 4 * radioC, x - radioC * 2 - 2, y + 7 * radioC, color);
  tft.drawLine(x + 1, y + 4 * radioC, x - radioC * 2 + 1, y + 7 * radioC, color);



}


// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(float value, uint8_t vmin, uint8_t vmax, uint16_t x, uint16_t y, uint16_t r, char *units, byte scheme) {
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  //value = 28.8;
  x += r; y += r;   // Calculate coords of centre of ring
  int w = r / 3;    // Width of outer ring is 1/4 of radius
  int angle = 150;  // Half the sweep angle of meter (300 degrees)
  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v
  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring
  // Variable to save "value" text colour from scheme and set default
  int colour = BLUE;

  int ent = value;
  float dec = ((value - ent) * 10);
  int dec1 = dec;

  //tft.setTextColor(colour,BLACK);
  //tft.setCursor(x-74,y-10);tft.setTextSize(5);
  //tft.print("      ");



  // Draw colour blocks every inc degrees
  for (int i = -angle + inc / 2; (i < angle - inc / 2); i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      switch (scheme) {
        case 0: colour = RED; break; // Fixed colour
        case 1: colour = GREEN; break; // Fixed colour
        case 2: colour = BLUE; break; // Fixed colour
        case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
        case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break; // Green to red (high temperature etc)
        case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
        default: colour = BLUE; break; // Fixed colour
      }
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      //text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, GREY);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 4;
  dtostrf(value, len, 4, buf);
  buf[len] = ' '; buf[len] = 0; // Add blanking space and terminator, helps to centre text too!
  // Set the text colour to default
  tft.setTextSize(1);

  if (ent > 9 /*|| ent < 9*/) {
    tft.setTextColor(colour, BLACK); tft.setTextSize(5);
    tft.setCursor(x - 52, y - 10);
    tft.print(ent);
  } else {
    tft.setTextColor(colour, BLACK); tft.setTextSize(5);
    tft.setCursor(x - 52, y - 10);
    tft.print("0" + String(ent));
  }
  tft.setCursor(x - 5, y - 4); tft.setTextSize(4);
  tft.fillCircle(x + 9, y + 20, 3, colour); //Punto decimal
  tft.setCursor(x + 16, y - 10); tft.setTextSize(5);
  tft.print(dec1);
  tft.drawCircle(x + 50, y - 5, 6, colour);
  tft.drawCircle(x + 50, y - 5, 5, colour);
  tft.drawCircle(x + 50, y - 5, 4, colour);
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(x - 39, y + 70); tft.setTextSize(2);
  tft.print(units); // Units display



  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value) {
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits
  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}

