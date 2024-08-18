#include <WiFi.h>
#include <string.h>
#include <PubSubClient.h>


#define id1 13
#define id2 14
#define id3 27
#define id4 26
#define id5 25
#define id6 33
#define id7 32

char ssid[] =  "FAMILIABENETTI_2G";
char password[] = "Gabi120500";
char mqtt_server[] = "broker.hivemq.com";
int value = 0;
char client_id[] = "sunfounder-client-sdgvsda";
//clientId-36Tnt1gAhc



WiFiClient wifi;
PubSubClient client(wifi);

unsigned long lastMag = 0;
#define MSG_BUFFER_SIZE (64)
char msg[MSG_BUFFER_SIZE];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem chegou [");
  Serial.print(topic);
  String messageTemp;
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  if(String(topic) == "esp32/message"){
  switch((char)payload[0]){
    case '1':
      digitalWrite(id1, HIGH); 
      delay(5000);
      digitalWrite(id1, LOW);
      break;
    case '2':
      digitalWrite(id2, HIGH); 
      delay(5000);
      digitalWrite(id2, LOW);
      break;
    case '3':
      digitalWrite(id3, HIGH); 
      delay(5000);
      digitalWrite(id3, LOW);
      break;
    case '4':
      digitalWrite(id4, HIGH); 
      delay(5000);
      digitalWrite(id4, LOW);
      break;
    case '5':
      digitalWrite(id5, HIGH); 
      delay(5000);
      digitalWrite(id5, LOW);
      break;
    case '6':
      digitalWrite(id6, HIGH); 
      delay(5000);
      digitalWrite(id6, LOW);
      break;
    case '7':
      digitalWrite(id7, HIGH); 
      delay(5000);
      digitalWrite(id7, LOW);
      break;
    default:
      break;
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
      
      client.subscribe("esp32/message");
    } else {
      Serial.print("falhou, rc = ");
      Serial.print(client.state());

      Serial.println(" tente denovo em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(9600);

  pinMode(id1,OUTPUT);
  pinMode(id2,OUTPUT);
  pinMode(id3,OUTPUT);
  pinMode(id4,OUTPUT);
  pinMode(id5,OUTPUT);
  pinMode(id6,OUTPUT);
  pinMode(id7,OUTPUT);

  digitalWrite(id1,HIGH);
  digitalWrite(id2,HIGH);
  digitalWrite(id3,HIGH);
  digitalWrite(id4,HIGH);
  digitalWrite(id5,HIGH);
  digitalWrite(id6,HIGH);
  digitalWrite(id7,HIGH);


  delay(1000);

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");

  }


  digitalWrite(id1,LOW);
  digitalWrite(id2,LOW);
  digitalWrite(id3,LOW);
  digitalWrite(id4,LOW);
  digitalWrite(id5,LOW);
  digitalWrite(id6,LOW);
  digitalWrite(id7,LOW);

  delay(1000);
  setupMQTT();
}




void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMag > 2000) {
    lastMag = now;
    value++;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("esp32/leds", msg);
  }



}
