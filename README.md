Sketch for the dadicator project.

Simple ESP32 project with a webserver to controll 3 led lights.  Used to let kids know when dad is free when working from home.


# Setup
- Plug into usb
- Wait ~30 seconds or so for a ```dadicator``` access point to show up
- Connect to the ```dadicator``` AP
  - Password is ```12345678```
- Add a new AP from the menu (this should be your wifi credentials)
- After a few seconds it should connect to your network, the setup screen will tell you the new IP
- The provisioning AP will spin up if it can't connect to wifi after 30 seconds
  - if you change your wifi password or similar

# Use
- http
    - connect via browser to the IP (no ssl/no auth etc)
    - Click lamp to toggle
- "API"
    - control light ```http://1.1.1.1/led/[red,yellow,green]/[on,off]```
    - light status ```http://1.1.1.1/status```

# Notes
- LEDs aren't glued in, didn't know if you wanted to change order etc.
- case is openable
- Webserver is single threaded, it's easy to get missed requests if you try
- You can program the board over the USB port
- the power LED backlights the bottom LED a bit so the green light looks red until it's actually lit.


# Enjoy!
