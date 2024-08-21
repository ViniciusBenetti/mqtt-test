#include <WiFi.h>
#include <string.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//instale as bibliotecas acima pelo arduino IDE

char ssid[] =  ""; //nome do seu wifi
char password[] = ""; // senha do seu wifi
char device_id[] = ""; //email que aparece no dashboard
char http_server[] = ""; //servidor interno para usuarios
char mqtt_server[] = "broker.hivemq.com"; //não edite isto
char client_id[] = "sunfounder-client-sdgvsda"; //não edite isto






String full_topic_ledComumOUT = "esp32/devices/ledComumOUT/" + String(device_id);
String full_topic_displayLCDOUT = "esp32/devices/displayLCDOUT/" + String(device_id);

String connectionClient = "esp32/connection/client/"+String(device_id);

String commomLed = "esp32/devices/ledComum/"+String(device_id);
String displayLCD = "esp32/devices/displayLCD/"+String(device_id);

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

WiFiClient wifi;
PubSubClient client(wifi);
HTTPClient http;

#define MSG_BUFFER_SIZE (64)
char msg[MSG_BUFFER_SIZE];




void notify(String message){
  http.begin(http_server);

  http.addHeader("Content-Type", "application/json"); 
  String jsonPayload = "{\"notificar\":\"" + message + "\", \"dispositivo\":\"" + device_id + "\"}";
  int httpCode = http.POST(jsonPayload);
  if(httpCode == HTTP_CODE_OK){

    String response = http.getString();

    const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, response);

    if (error) {
    Serial.println("JSON FALHOU");
    return;
    }

    String mensagemResposta = doc["mensagem"];
    try{
    String id_usuario = doc["dispositivo"];
    Serial.println(id_usuario);
    }catch(String &error){
      Serial.println("erro ao mandar notificacao");
    }


    Serial.println(mensagemResposta);


}
}
void callback(char* topic, byte* payload, unsigned int length) {

  String messageTemp = "";

  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.println();

  if(String(topic) == commomLed){
      try{
        if(digitalRead(messageTemp.toInt()) == HIGH){
            digitalWrite(messageTemp.toInt(),LOW);

            client.publish(full_topic_ledComumOUT.c_str(),("desligado"+messageTemp).c_str());

        }else if(digitalRead(messageTemp.toInt()) == LOW){
          digitalWrite(messageTemp.toInt(),HIGH);

          client.publish(full_topic_ledComumOUT.c_str(),("ativo"+messageTemp).c_str());
        }       
      }catch(String &err){
        Serial.println("erro led");
        
      }
      
  }else if(String(topic) == displayLCD){
    try{
      lcd.clear();
      delay(30);
      if(length <= 16){
      lcd.setCursor(0,0);
      lcd.print(messageTemp.substring(0,16));
      }
      else if(length > 16 && length <=32){
      lcd.setCursor(0,0);
      lcd.print(messageTemp.substring(0,16));
      lcd.setCursor(0,1);
      lcd.print(messageTemp.substring(16,length));
      }

      client.publish(full_topic_displayLCDOUT.c_str(),messageTemp.c_str());
    }catch(String &err){
      Serial.println("erro display");
    }

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
    

    client.subscribe(connectionClient.c_str());
    client.subscribe(commomLed.c_str());
    client.subscribe(displayLCD.c_str());

    } else {
      Serial.print("falhou, rc = ");
      Serial.print(client.state());

      Serial.println("tente denovo em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(9600);
  try{
  lcd.init();
  lcd.backlight();
  }catch(String &err){
    Serial.println("sem display");
  }

 /* setup dos seus pinos
  pinMode(13,OUTPUT);
  pinMode(14,OUTPUT);
  pinMode(25,OUTPUT);
  pinMode(26,OUTPUT);
  pinMode(27,OUTPUT);
  pinMode(32,OUTPUT);
  pinMode(33,OUTPUT);
  */

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
  client.publish(connectionClient.c_str(),"online");
  if(digitalRead(33) == HIGH){
    notify("botão 33 aceso");
  }
  delay(300);

}


