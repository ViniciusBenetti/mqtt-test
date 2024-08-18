#include <WiFi.h>
#include <string.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <cstring> 

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

char ssid[] =  "FAMILIABENETTI_2G";
char password[] = "Gabi120500";

char mqtt_server[] = "broker.hivemq.com";
char client_id[] = "sunfounder-client-sdgvsda";

WiFiClient wifi;
PubSubClient client(wifi);


#define MSG_BUFFER_SIZE (32)
char msg[MSG_BUFFER_SIZE];

void callback(char* topic, byte* payload, unsigned int length) {
   
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  
  }
  strcpy(msg,messageTemp.c_str());
  Serial.println();

  if(String(topic) == "esp32/display"){
      lcd.clear();
      delay(100);
      if(length <= 16){
      lcd.setCursor(0,0);
      lcd.print(messageTemp.substring(0,length));
      }
      if(length > 16 && length <=32){
      lcd.setCursor(0,1);
      lcd.print(messageTemp.substring(16,length));
      }
      
    client.publish("esp32/displaycallback",msg);
  }

}
void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void reconnect() {

  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT");
    if (client.connect(client_id)) {
      Serial.println("connected");
      
      client.subscribe("esp32/display");
    } else {
      Serial.print("falhou, rc = ");
      Serial.print(client.state());

      Serial.println(" tente denovo em 5 segundos");
      delay(5000);
    }
  }
}
void setup() {
  lcd.init();
  lcd.backlight();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");

  }
  setupMQTT();

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
