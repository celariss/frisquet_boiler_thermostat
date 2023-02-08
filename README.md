# Frisquet Boiler Thermostat
#### A virtual thermostat to connect a standard boiler to the magic world of IoT

This project intends to control an old Frisquet boiler (Eco Radio System) by connecting it to the network, using [ESPHome] on an ESP8266 (or any other ESPHome compatible) microcontroller.
The boiler is then seen as a connected thermostat via MQTT or directly on a [Home Assistant] server instance.

## References
It is highly inspired from a fantastic "Frisquet hacking" thread and related projects (mostly in french) :
- The thread that made all this possible :
  - [Régulation d'une chaudière Frisquet ECO radio System](https://www.easydomoticz.com/forum/viewtopic.php?t=1486) - 
- A step-by-step reverse engineering of the communication protocol used by the boiler :
  - [Décodage du signal Frisquet Eco Radio System](https://antoinegrall.wordpress.com/decodage-frisquet-ers/) - 
- An implementation of the communication protocol used by the Frisquet boiler (Arduino IDE), very useful to get your boiler "ID" (needed to drive the boiler) :
  - https://github.com/etimou/frisquet-arduino
- A RFLink variant :
  - https://github.com/ChristopheHD/frisquet-arduino
- A well-documented ESPhome project :
  - [philippemezzadri]

> THANK YOU, GUYS, FOR YOUR WORK !

## Features
This program is able to :
- Expose a virtual thermostat that represents the boiler on the wifi network
- Regulate the boiler power using a PID controller
- Send setpoints to the boiler using a direct wired connection
- Handle network and client failures to keep the boiler in a secure state

> NOTE: This program needs to get the current temperature from a sensor, so a pre-requisite is a temperature sensor installed on your Home Assistant instance.

## Tech and dependencies
This app uses a number of tech and projects to work properly:
- [ESPHome] - Control your ESP8266/ESP32 by simple yet powerful configuration files and control them remotely through Home Automation systems
- [MQTT] - The Standard for IoT Messaging
- [Home Assistant] - Open source home automation that puts local control and privacy first

## Wiring
> This section is taken from [philippemezzadri]'s project readme.

The ESPHome replaces the original Eco Radio System HF receiver and is conneted to the boiler main board through a micro-fit 4 socket.

| ESP32                 | Boiler Side         | Pin number |
| --------------------- | ------------------- |:----------:|
| GND                   | black wire          | 1          |
| Pin 21 (configurable) | yellow wire         | 2          |
| 5V                    | red wire (optional) | 3          |

**Micro-fit 4 pin out:**

<img src="doc/img/connector_4pin1.png" alt="Micro-fit 4 pinout drawing" width="80"/>

Defined viewing direction for the connector pin out:

- Receptable - _rear view_
- Header - _front view_

## Installation
TBD : coming soon ...


[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

  [ESPHome]: <https://esphome.io/>
  [mqtt]: <https://mqtt.org/>
  [Home Assistant]: <https://www.home-assistant.io/>
  [philippemezzadri]: <https://github.com/philippemezzadri/frisquet-esphome>