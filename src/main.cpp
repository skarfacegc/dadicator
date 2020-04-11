#include <string>
#include <WiFi.h>          
#include <WebServer.h>
#include <AutoConnect.h>   // handles wifi setup

#define RED_PIN    32
#define YELLOW_PIN 33
#define GREEN_PIN  25

#define RED    1
#define YELLOW 2
#define GREEN  3


WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;


//
// Globals for tracking light state
//
byte greenLedState = LOW;
byte yellowLedState = LOW;
byte redLedState = LOW;




// prototypes
void rootPage(void);
void processLedRequest(std::string, std::string);
void setLights(void);
void updateLEDState(std::string, std::string);
std::string getHTML(void);
std::string getLEDStatusJSON();



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

  Server.on("/status", []() {
    Server.send(200, "application/json", getLEDStatusJSON().c_str());
  });

  // Handle the led requests
  Server.on("/led/{}/{}", []() {
    std::string selectedLed;
    std::string ledAction;
    
    selectedLed = Server.pathArg(0).c_str();
    ledAction = Server.pathArg(1).c_str();

    processLedRequest(selectedLed, ledAction);
  });

  Server.on("/json_led/{}/{}", []() {
    std::string selectedLed = Server.pathArg(0).c_str();
    std::string selectedLedState = Server.pathArg(1).c_str();

    updateLEDState(selectedLed, selectedLedState);
    Server.send(200, "application/json", getLEDStatusJSON().c_str());
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


// Returns JSON representing current light state
std::string getLEDStatusJSON() {
  std::string str;

  str = "{";
  if(redLedState == HIGH) {
    str += "\"red_led\": true,";
  } else {
    str += "\"red_led\": false,";
  }

  if(yellowLedState == HIGH) {
    str += "\"yellow_led\": true,";
  } else {
    str += "\"yellow_led\": false,";
  }

  if(greenLedState == HIGH) {
    str += "\"green_led\": true";
  } else {
    str += "\"green_led\": false";
  }
  str += "}";
  return str;
}

// Sets the requested led to the desired state
// selectedLed {red,yellow,green}
// selectedLedState {on,off}
void updateLEDState(std::string selectedLed, std::string selectedLedState) {
  byte newState = LOW; // default to off

  if(selectedLedState == "on") {
    newState = HIGH;
  } else {
    newState = LOW;
  }

  if(selectedLed == "red") {
    redLedState = newState;
  } else if(selectedLed == "yellow") {
    yellowLedState = newState;
  } else if(selectedLed == "green") {
    greenLedState = newState;
  }
}

// Show the HTML
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

// This reads the led state globals and sets the appropriate lights HIGH or LOW
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

// Just returns the static HTML page
std::string getHTML(void) {
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
      <script type="text/javascript">
          let lightState = {};
          window.onload = () => {
            getLightState().then(() => updateLights() );
          };

          async function getLightState() {
            let response = await fetch("/status");
            let data = await response.json()
            lightState = data;
          }

          async function toggleLight(lightID) {
            let lightEntity = document.getElementById(lightID);
            const classOnString = "lamp"+lightID+"On";
            const classOffString = "lamp"+lightID+"Off";
            let url = "/json_led/";

            // if off, toggle on.  if on, toggle off
            if(lightEntity.classList.contains(classOffString)) {
              lightEntity.classList.replace(classOffString, classOnString);
              url += lightID.toLowerCase() + "/on";
            } else {
              lightEntity.classList.replace(classOnString, classOffString);
              url += lightID.toLowerCase() + "/off";
            }

            let response = await fetch(url);
            let data = await response.json();
            lightState = data;
            return data;
          }

          function updateLights() {
            let greenLight = document.getElementById("Green");
            let redLight = document.getElementById("Red");
            let yellowLight = document.getElementById("Yellow");
            
            if(lightState.green_led === true) {
              greenLight.className = "lamp lampGreenOn";
            } else {
              greenLight.className = "lamp lampGreenOff";
            }

            if(lightState.yellow_led === true) {
              yellowLight.className = "lamp lampYellowOn";
            } else {
              yellowLight.className = "lamp lampYellowOff";
            }

            if(lightState.red_led === true) {
              redLight.className = "lamp lampRedOn";
            } else {
              redLight.className = "lamp lampRedOff";
            }
          }
      </script>

    </head>
    <body>
      <div id="trafficLight">
        <div onclick='toggleLight("Green")' class="lamp lampGreenOff" id="Green"></div>
        <div onclick='toggleLight("Yellow")' class="lamp lampYellowOff" id="Yellow"></div>
        <div onclick='toggleLight("Red")' class="lamp lampRedOff" id="Red"></div>
      </div>
    </body>
  </html>
  )";
  return str; 
}