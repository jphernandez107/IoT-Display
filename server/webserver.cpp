#include "webserver.h"
#include "index.html.gz.h"
#include "config.h"
#include <EEPROM.h>

config_t newConfig;

bool isRunning = false;
bool actualizar = false;

ESP8266WebServer server(80);

void handleRoot() {
  server.sendHeader("Content-Encoding", "gzip");
  server.send_P(200, "text/html", index_html_gz, index_html_gz_len);
  //server.sendHeader("Content-Encoding", "gzip");
  //server.send_P(200, "text/html", index_html_gz, index_html_gz_len);
}

void handleConfig() {
  EEPROM.get(0, newConfig);

  String wifi_ssid(server.arg("wifissid"));
  String wifi_pass(server.arg("wifipass"));
  String mqtt_broker(server.arg("mqttbroker"));
  String mqtt_port(server.arg("mqttport"));
  String topic_temp(server.arg("topictemp"));
  String topic_hum(server.arg("topichum"));
  String topic_ldr(server.arg("topicldr"));
  String topic_presencia(server.arg("topicpresencia"));
  String mqtt_user(server.arg("mqttuser"));
  String mqtt_pass(server.arg("mqttpass"));
  String admin_pass(server.arg("adminpass"));
  String client_id(server.arg("clientid"));

  if (wifi_ssid != "") {
    wifi_ssid.toCharArray(newConfig.wifi_ssid, 50);
  }
  if (wifi_pass != "") {
    wifi_pass.toCharArray(newConfig.wifi_pass, 50);
  } else if (wifi_pass == "null")
  {
    wifi_pass.toCharArray("", 20);
  }
  if (mqtt_broker != "") {
    mqtt_broker.toCharArray(newConfig.mqtt_broker, 50);
  }
  if (mqtt_port != "") {
    newConfig.broker_puerto = mqtt_port.toInt();
  }
  if (topic_temp != "") {
    topic_temp.toCharArray(newConfig.topic_temp, 100);
  }
  if (topic_hum != "") {
    topic_hum.toCharArray(newConfig.topic_hum, 100);
  }
  if (topic_ldr != "") {
    topic_ldr.toCharArray(newConfig.topic_ldr, 100);
  }
  if (topic_presencia != "") {
    topic_presencia.toCharArray(newConfig.topic_presencia, 100);
  }
  if (mqtt_user != "") {
    mqtt_user.toCharArray(newConfig.mqtt_user, 50);
  } else if (mqtt_user == "null")
  {
    mqtt_user.toCharArray("", 20);
  }
  if (mqtt_pass != "") {
    mqtt_pass.toCharArray(newConfig.mqtt_pass, 50);
  } else if (mqtt_pass == "null")
  {
    mqtt_pass.toCharArray("", 20);
  }
  if (admin_pass != "") {
    admin_pass.toCharArray(newConfig.admin_pass, 20);
  } else if (admin_pass == "null")
  {
    admin_pass.toCharArray("", 20);
  }
  if (client_id != "") {
    client_id.toCharArray(newConfig.client_id, 10);
  }
  EEPROM.put(0, newConfig);
  EEPROM.commit();

  server.send_P(200, "text/plain", "Datos cargados correctamente");

  actualizar = true;
}

void handleData() {

}

void handleNotFound() {
  String message = "Error 404 File Not Found\n\nURI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void WebServer_init() {
  server.on("/", handleRoot);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/data", handleData); // Tomamos los datos de config, y los enviamos al arduino.
  server.onNotFound(handleNotFound);
  server.begin();
  isRunning = true;
}

void WebServer_stop() {
  server.stop();
  isRunning = false;
}

void WebServer_loop() {
  server.handleClient();
}

bool WebServer_isRunning() {
  return isRunning;
}

bool WebServer_getActualizar() {
  return actualizar;
}

void WebServer_setActualizar (bool newFlag) {
  actualizar = newFlag;
}

