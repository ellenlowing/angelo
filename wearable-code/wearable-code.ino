#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

const int buttonPin = 14;     // the number of the pushbutton pin
const int ledPin = 12;
const int onPin = 33;

const char * networkName = "MIT";
const char * networkPswd = NULL;

const char * udpAddress = "18.30.7.15";
const int udpPort = 3333;
const int outPort = 4444;

boolean connected = false;
WiFiUDP udp;

int buttonState = 0;         // variable for reading the pushbutton status

unsigned long currentTime;
unsigned long currentValidationTime;

int currentBtnState = 0;

int startTimer = 0;

int count = 0;
int validCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(onPin, OUTPUT);
  connectToWiFi(networkName, networkPswd);
  currentTime = millis();
  buttonState = digitalRead(buttonPin);
  currentBtnState = buttonState;
  digitalWrite(onPin, HIGH);
}

void loop() {
  if(changedBtnState()){
    if(currentBtnState == LOW){ 
      currentTime = millis();
      if (startTimer == 0) startTimer = 1;
      while(millis() < currentTime + 1000){
        if(changedBtnState() && (currentBtnState == LOW) && (startTimer == 1)){
          count++;
        }
      }
      if(count >= 2) {
        Serial.println("WAITING FOR VALIDATION...");
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);
        digitalWrite(ledPin, HIGH); 
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);
        digitalWrite(ledPin, HIGH); 
        delay(100);
        digitalWrite(ledPin, LOW);
        currentValidationTime = millis();
        while(millis() < currentValidationTime + 4000){
          int validBtnState = digitalRead(buttonPin);
          if(validBtnState == LOW) {
            validCount++;
            //Serial.println(validCount);
          }
        }
        if(validCount >= 2000) {
          Serial.println("Sending signal...");
          digitalWrite(ledPin, HIGH);
          // buzzzzzz until signal is sent
          //if(connected) {
            OSCMessage msg("/city");
            msg.add("HELP!");
            udp.beginPacket(udpAddress,outPort);
            msg.send(udp);
            udp.endPacket();
            msg.empty();
          //}
          //listen to maxmsp patch, check if osc message is received
          digitalWrite(ledPin, LOW);
        } else {
          Serial.println("Ignoring alert signal");
        }
      }
    }
  }
  startTimer = 0;
  count = 0;
  validCount = 0;
}

// LOW: pressed, HIGH: not pressed
bool changedBtnState() {
  buttonState = digitalRead(buttonPin);
  bool changed = (currentBtnState != buttonState);
  if(changed){
    currentBtnState = buttonState;
  }
  return changed;
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
    }
}

