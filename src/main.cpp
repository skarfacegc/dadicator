#include <string>
#include <WiFi.h>          // Replace with WiFi.h for ESP32
#include <WebServer.h>     // Replace with WebServer.h for ESP32
#include <AutoConnect.h>   // handles wifi setup

#define RED_PIN    32
#define YELLOW_PIN 33
#define GREEN_PIN  25

#define RED    1
#define YELLOW 2
#define GREEN  3


WebServer Server;          // Replace with WebServer for ESP32
AutoConnect Portal(Server);
AutoConnectConfig Config;


// ack globals for state tracking
byte greenLedState = LOW;
byte yellowLedState = LOW;
byte redLedState = LOW;




// prototypes
void rootPage(void);
void processLedRequest(std::string, std::string);
void setLights(void);
std::string getHTML(void);
std::string getHTMLHeader(void);
std::string getLightHTML(int);
std::string getHTMLFooter(void);



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
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  
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
    std::string selectedLed;
    std::string ledAction;
    
    selectedLed = Server.pathArg(0).c_str();
    ledAction = Server.pathArg(1).c_str();

    processLedRequest(selectedLed, ledAction);
  });

  // Connect to wifi / deal with provisioning if needed
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
}

void loop() {
    setLights();
    Portal.handleClient();
}



void rootPage(void) {
  Server.send(200, "text/html", getHTML().c_str());
}

void processLedRequest(std::string selectedLed, std::string ledAction) {
  byte newState = LOW;
  bool goodState = false;

  // make sure we have a valid state
  if(ledAction == "on") {
    newState = HIGH;
    goodState = true;
  } else if(ledAction == "off") {
    newState = LOW;
    goodState = true;
  } else {
    goodState = false;
  }

  // Make sure we have a valid LED
  // If we have a valid state and a valid led set the led state tracking variable, select the pin to set,
  if(selectedLed == "red") {
    if(goodState) redLedState = newState;
  } else if(selectedLed == "yellow") {
    if(goodState) yellowLedState = newState;
  } else if(selectedLed == "green") {
    if(goodState) greenLedState = newState;
  }

  // at this point, we know if we got a good state (on/off)
  // we know if we got a good led (red,yellow,green)
  // The global led state tracking has been udpated

  // if we have a valid state and a valid pin we can do the thing
  setLights();
  Server.send(200, "text/html", getHTML().c_str());
}


// Contains the html for the web page
std::string getHTML(void) {
  std::string html;

  // /led/{color}/{state}
  html = getHTMLHeader() +
         getLightHTML(RED) +
         getLightHTML(YELLOW) + 
         getLightHTML(GREEN) +
         getHTMLFooter();

  return html;
}



std::string getHTMLHeader(void) {
  const std::string str =
    R"(
      <!DOCTYPE html>
      <html lang=\"en\">
      <head>
        <meta charset=\"UTF-8\">
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
        <title>Dadicator</title>

        <style>
        .body {
            background-color: rgb(255,255,255);
        }
        .lamp {
            height: 250px;
            width: 250px;
            border-style: solid;
            border-width: 2px;
            border-radius: 200px;
        }
        .lampRedOn {
            background-color: rgb(255,0,0);
        }
        .lampYellowOn {
            background-color: rgb(255, 255, 0);
        }
        .lampGreenOn {
            background-color: rgb(0,255,0);
        }
        .lampGreenOff {
            background-color: rgb(0, 70, 0);
        }
        .lampYellowOff {
            background-color: rgb(70, 70, 0);
        }
        .lampRedOff {
            background-color: rgb(70, 0, 0);
        }
      </style>
    </head>
    <body>
      <div id="trafficLight">
  )";
  return str; 
}

// Bulid each lightbulb
std::string getLightHTML(int color) {
    std::string html;
    std::string divOpen;
    std::string divClose; 
    std::string onOff;
    std::string href;


    if(color == GREEN) {
      if(greenLedState == LOW) {
        html = "<a href=\"/led/green/on\">";
        onOff = " lampGreenOff\"";
      } else {
        html = "<a href=\"/led/green/off\">";
        onOff = "lampGreenOn";
      }
      html += "<div class=\"lamp " + onOff + "\" id=\"Green\"></div></a>";
    } else if(color == YELLOW) {
      if(yellowLedState == LOW) {
        html = "<a href=\"/led/yellow/on\">";
        onOff = " lampYellowOff\"";
      } else {
        html = "<a href=\"/led/yellow/off\">";
        onOff = "lampYellowOn";
      }
      html += "<div class=\"lamp " + onOff + "\" id=\"Yellow\"></div></a>";
    } else if(color == RED) {
      if(redLedState == LOW) {
        html = "<a href=\"/led/red/on\">";
        onOff = " lampRedOff\"";
      } else {
        html = "<a href=\"/led/red/off\">";
        onOff = "lampRedOn";
      }
      html += "<div class=\"lamp " + onOff + "\" id=\"Red\"></div></a>";
    } else {
      html = "Error";
    }

    return html;
}

std::string getHTMLFooter(void) {
  const std::string str = R"(</div></body></html>)";
  return str;
}


void setLights(void) {
  if(redLedState == HIGH) {
    digitalWrite(RED_PIN, HIGH);
  } else if(redLedState == LOW) {
    digitalWrite(RED_PIN, LOW);
  }

  if(yellowLedState == HIGH) {
    digitalWrite(YELLOW_PIN, HIGH);
  } else if(yellowLedState == LOW) {
    digitalWrite(YELLOW_PIN, LOW);
  }

  if(greenLedState == HIGH) {
    digitalWrite(GREEN_PIN, HIGH);
  } else if(greenLedState == LOW) {
    digitalWrite(GREEN_PIN, LOW);
  }

}