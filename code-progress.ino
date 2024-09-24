#include <WiFi.h>
#include <string.h>
#include <PubSubClient.h> 
#include <LiquidCrystal_I2C.h> 
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>


//---------------------------------------OUTPUTS-------------------------------------------------------------
char ssid[] =  "";
char password[] = "";
char device_id[] = "";

int redRGBPin = 18;
int blueRGBPin = redRGBPin+1;
int greenRGBPin = redRGBPin-3;
int buzzerPin = 27;
int ledPin = 26;
int relayPin = 2;
int servoPin = 2;

//edit the pin number to match with the website 
//if you add more devices will need to include in void setup() on pinMode



//---------------------------------------IF YOU EDIT THIS CONFIG, YOU MAY NEED TO EDIT SETUP AND LOOP---------------------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//---------------------------------------DO NOT EDIT BELOW THIS LINE-------------------------------------------------------------

char http_server[] = "https://vinixwebhook1488.glitch.me/postNotificar";
String full_topic_ledComumOUT = "esp32/devices/ledComumOUT/" + String(device_id);
String full_topic_displayLCDOUT = "esp32/devices/displayLCDOUT/" + String(device_id);
String full_topic_displayOLEDOUT = "esp32/devices/displayOLEDOUT/"+String(device_id);
String full_topic_buzzerAtivoOUT = "esp32/devices/buzzerAtivoOUT/"+String(device_id);
String full_topic_servoOUT = "esp32/devices/servoOUT/"+String(device_id);
String full_topic_releSRDOUT = "esp32/devices/releSRDOUT/"+ String(device_id);
String full_topic_ledRGBOUT = "esp32/devices/ledRGBOUT/"+String(device_id);
String connectionClient = "esp32/connection/client/"+String(device_id);
String commomLed = "esp32/devices/ledComum/"+String(device_id);
String displayLCD = "esp32/devices/displayLCD/"+String(device_id);
String displayOLED = "esp32/devices/displayOLED/"+String(device_id);
String activeBuzzer = "esp32/devices/buzzerAtivo/"+String(device_id);
String servoSG90 = "esp32/devices/servoSG90/"+String(device_id);
String releSRD = "esp32/devices/releSRD/"+String(device_id);
String ledRGB = "esp32/devices/ledRGB/"+String(device_id);
char mqtt_server[] = "broker.hivemq.com";
char client_id[] = "sunfounder-client-sdgvsda";


Servo servo;  
WiFiClient wifi;
PubSubClient client(wifi);
HTTPClient http;
int countRGB = 1;
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
  String topicStr = String(topic);
  if(topicStr == commomLed){
      try{
         int pin = messageTemp.toInt();
         int state = digitalRead(pin);
         digitalWrite(pin, state == HIGH ? LOW : HIGH);
         client.publish(full_topic_ledComumOUT.c_str(), ((state == HIGH ? "desligado" : "ativo") + messageTemp).c_str());
        
      }catch(String &err){
        Serial.println("erro led");
        
      }

      }else if(topicStr == displayLCD){
        try{
        lcd.clear();
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

  }else if(topicStr == displayOLED){
    try {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      for (int i = 0; i < length; i += 21) {
        display.setCursor(0, i / 21 * 8);
        display.println(messageTemp.substring(i, min(i + 21, (int)length)));
      }
      client.publish(full_topic_displayOLEDOUT.c_str(), messageTemp.c_str());
    } catch (String &err) {
      Serial.println("erro display");
    }

  }else if(topicStr == activeBuzzer){
        try{
         int pin = messageTemp.toInt();
         int state = digitalRead(pin);
         digitalWrite(pin, state == HIGH ? LOW : HIGH);
         client.publish(full_topic_buzzerAtivoOUT.c_str(), ((state == HIGH ? "mudo" : "som") + messageTemp).c_str());
        
      }catch(String &err){
        Serial.println("erro buzzer");
        
      }
  }else if(topicStr == servoSG90){
    servo.detach();
    String angleStr = messageTemp.substring(0,messageTemp.indexOf(':'));
    String pinStr = messageTemp.substring(messageTemp.indexOf(':')+1,messageTemp.length());
    int pin = pinStr.toInt();
    int angle = angleStr.toInt();
    
    try{
      servo.attach(pin);
      servo.write(angle);
  
      client.publish(full_topic_servoOUT.c_str(), ("angulo atual: "+ angleStr).c_str());
      
    }catch(String &err){
      Serial.println("erro servo");
    }
  }else if(topicStr == releSRD){
      try{
         int pin = messageTemp.toInt();
         int state = digitalRead(pin);
         digitalWrite(pin, state == HIGH ? LOW : HIGH);
         client.publish(full_topic_ledComumOUT.c_str(), ((state == HIGH ? "releOFF" : "releON") + messageTemp).c_str());
        
      }catch(String &err){
        Serial.println("erro rele");
        
      }

  }else if(topicStr == ledRGB){
    if(countRGB >4){
      countRGB = 1;
    }
    int pin = messageTemp.toInt();
    try{
    if(countRGB == 1){

    analogWrite(pin, 255); 
    client.publish(full_topic_ledRGBOUT.c_str(),(("cor atual: vermelha")+messageTemp).c_str());
    }
    else if(countRGB == 2){
      analogWrite(pin-3,255);
      analogWrite(pin,0);
      client.publish(full_topic_ledRGBOUT.c_str(),(("cor atual: verde")+messageTemp).c_str());

    }
    else if(countRGB == 3){
      analogWrite(pin+1,255);
      analogWrite(pin-3,0);
      client.publish(full_topic_ledRGBOUT.c_str(),(("cor atual: azul")+messageTemp).c_str());

    }
    else if(countRGB == 4){

      analogWrite(pin+1,0);
      client.publish(full_topic_ledRGBOUT.c_str(),(("cor atual: nenhuma")+messageTemp).c_str());
    }
  countRGB++;
  }catch(String &err){
    Serial.println("erro rgb");
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

    client.subscribe(connectionClient.c_str());
    client.subscribe(commomLed.c_str());
    client.subscribe(displayLCD.c_str());
    client.subscribe(displayOLED.c_str());
    client.subscribe(activeBuzzer.c_str());
    client.subscribe(servoSG90.c_str());
    client.subscribe(releSRD.c_str());
    client.subscribe(ledRGB.c_str());

    } else {
      Serial.print("falhou, rc = ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  bool displayInitialized = false;

  Serial.begin(9600);
  try{
  lcd.init();
  lcd.backlight();
  }catch(String &err){
    Serial.println("erro LCD");
  }
  try{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  displayInitialized = true;
  Serial.println("inicializado oled");
  display.display();
  }catch(String &err){

    Serial.println("erro OLED");
  }
  delay(2000); 
  pinMode(buzzerPin,OUTPUT);
  pinMode(ledPin,OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(redRGBPin,OUTPUT);
  pinMode(blueRGBPin,OUTPUT);
  pinMode(greenRGBPin,OUTPUT);
  digitalWrite(relayRGBPin, LOW);
  servo.attach(servoPin);


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);

  }

  setupMQTT();
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 
  client.publish(connectionClient.c_str(),"online");

  delay(1000);

}



