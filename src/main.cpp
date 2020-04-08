#include <WiFi.h>          // Replace with WiFi.h for ESP32
#include <WebServer.h>     // Replace with WebServer.h for ESP32
#include <AutoConnect.h>   // handles wifi setup

#define RED_PIN    13
#define YELLOW_PIN 12
#define GREEN_PIN  14


WebServer Server;          // Replace with WebServer for ESP32
AutoConnect Portal(Server);
AutoConnectConfig Config;


// ack globals for state tracking
byte redLedState = LOW;
byte yellowLedState = LOW;
byte greenLedState = LOW;

void rootPage(void);
void processLedRequest(String, String);
String getHTML(byte, byte, byte, String);



void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  // init the pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  // make sure the leds are off on boot
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);


  // setup provisioning AP name
  Config.apid = "dadicator";
  Portal.config(Config);

  //
  // Init the endpoints
  //

  // main page
  Server.on("/", rootPage);

  // Handle the led requests
  Server.on("/led/{}/{}", []() {
    String selectedLed = Server.pathArg(0);
    String ledAction = Server.pathArg(1);
    processLedRequest(selectedLed, ledAction);
  });

  // Connect to wifi / deal with provisioning if needed
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
}

void loop() {
    Portal.handleClient();
}



void rootPage(void) {
  Server.send(200, "text/html", getHTML(redLedState,yellowLedState,greenLedState, "welcome"));
}

void processLedRequest(String selectedLed, String ledAction) {

  byte ledPin = 0;
  byte newState = LOW;
  bool goodState = false;
  bool goodLed = false;
  String message;

  // make sure we have a valid state
  if(ledAction.compareTo("on") == 0) {
    newState = HIGH;
    goodState = true;
    message = "Turning on ";
  } else if(ledAction.compareTo("off") == 0) {
    newState = LOW;
    goodState = true;
    message = "Turning off ";
  } else {
    goodState = false;
    message = "Invalid state ";
    message += ledAction;
  }

  // Make sure we have a valid LED
  // If we have a valid state and a valid led set the led state tracking variable, select the pin to set,
  // and update the message
  if(selectedLed.compareTo("red") == 0) {
    goodLed = true;
    if(goodState) redLedState = newState;
    ledPin = RED_PIN;
    message += "red led";
  } else if(selectedLed.compareTo("yellow") == 0) {
    goodLed = true;
    if(goodState) yellowLedState = newState;
    ledPin = YELLOW_PIN;
    message += "yellow led";
  } else if(selectedLed.compareTo("green") == 0) {
    goodLed = true;
    if(goodState) greenLedState = newState;
    ledPin = GREEN_PIN;
    message += "green led";
  } else {
    goodLed = false;
    message = "Invalid led chosen: ";
    message += selectedLed;
  }

  // at this point, we know if we got a good state (on/off)
  // we know if we got a good led (red,yellow,green)
  // The global led state tracking has been udpated

  // if we have a valid state and a valid pin we can do the thing
  if(goodLed && goodState) digitalWrite(ledPin,newState);

  Server.send(200, "text/html", getHTML(redLedState,yellowLedState,greenLedState,message));

}


// Contains the html for the web page
String getHTML(byte redLed, byte yellowLed, byte greenLed, String message) {
  String redLedStatusString;
  String yellowLedStatusString;
  String greenLedStatusString;

  // Set the light Status strings
  if(redLed == HIGH) {
    redLedStatusString = "on";
  } else {
    redLedStatusString = "off";
  }

  if(yellowLed == HIGH) {
    yellowLedStatusString = "on";
  } else {
    yellowLedStatusString = "off";
  }

if(greenLed == HIGH) {
    greenLedStatusString = "on";
  } else {
    greenLedStatusString = "off";
  }


  
  String html = "<!DOCTYPE html>\n";
        html += "<html>\n";
        html += "<head>\n";
        html += "<title>Dadicator</title>\n";
        html += "</head>\n";
        html += "<h1>dadicator</h1>\n";

        if(message.compareTo("") != 0){
          html += message;
        }

        // red led control
        html += "<p>Red Led : ";
        html += redLedStatusString;
        html += "</p>\n";

        // green led control
        html += "<p>Green Led : ";
        html += greenLedStatusString;
        html += "</p>\n";

        // yellow led control
        html += "<p>Yellow Led : ";
        html += yellowLedStatusString;
        html += "</p>\n";

        html += "</html>\n";
  return html; 
}