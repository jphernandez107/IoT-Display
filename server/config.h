typedef struct {
  // Accesspoint 
  char wifi_ssid[50];
  char wifi_pass[50];

  // Broker
  char mqtt_broker[50];
  int broker_puerto;
  
  char topic_temp[100];
  char topic_hum[100];
  char topic_ldr[100];
  char topic_presencia[100];
  
  char mqtt_user[50];
  char mqtt_pass[50];

  //extras
  char admin_pass[20];

  char client_id[10];

} config_t;
