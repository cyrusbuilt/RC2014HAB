# RC2014HAB

# Introduction
The purpose of this project was to provide an interface between the [RC2014](https://rc2014.co.uk/) (and compatible) retro homebrew computer (although I personally use the [SC126](https://smallcomputercentral.wordpress.com/sc126-z180-motherboard-rc2014/)) and my home automation system.  While this is not a *NECESSARY* project because I already have total control of my home automation system via both a mobile app and a web interface (I use [OpenHAB2](https://www.openhab.org/)), I liked the idea of also having a retro terminal style interface to the system.  There are a number of ways I could achieve that using modern microcontrollers, but I like my little 8bit "modern retro" computer so much I wanted to give it a nore practical application that just playing and tinkering and just generally enjoying the nostalgia of it.  I realized that I could achieve this using a the [Digital I/O card](https://rc2014.co.uk/modules/digital-io/)(I personally use the [SC129]https://smallcomputercentral.wordpress.com/sc129-digital-i-o-rc2014/) by Stephen P Cousins who also made the SC126 kit mentioned above) and an Arduino.  I decided to use an [Arduino MEGA2560](https://store.arduino.cc/usa/mega-2560-r3) and an [Ethernet Shield](https://store.arduino.cc/usa/arduino-ethernet-shield-2). The idea here is relatively simple, although fairly specific to my own setup: The Digital I/O card can be used to drive GPIOs on the Arduino low or high, representing an 8bit command which the Arduino can then interpret and execute. The command would then translate to a specific command for one or more devices which goes out over MQTT to OpenHAB2.

For example: On the RC2014 I send b00000001 to the port address of the I/O card which drives output bit 0 high and all others low. The Arduino sees the GPIO pin that the bit 0 pin is attached to go high, and translates that to "open garage door". The Arduino then sends the following JSON payload to the cygarage/control MQTT topic:

```pre
{
	"client_id": "CYGARAGE",
	"command": 2
}
```

Ideally, then when the door state changes, the Arduino can also subscribe to the status topics and see those state changes and update those states in the client application running on the RC2014 by then driving input pins on the Digital I/O card high.  This requires 8 GPIOs on the Arduino set as inputs and 8 GPIOs set as outputs for a total of 16 pins. This effectively gives an 8bit TTL Parallel communication bus with a maximum number of 256 commands/states (0-255).  We can expand that even further by adding multiple Digital I/O cards. This is another good reason for using the Mega 2560, given the sheer number of GPIOs available.  This also effectively gives the RC2014 ethernet connectivity, albeit for a very specific purpose (that could be modified for other purposes).

# Software

The firmware for the Arduino MEGA2560 is written in C/C++ using [PlatformIO](https://platformio.org/). The source code can be found in the "src" directory.  The client application running on the RC2014 is written in BASIC and intended to run using the Microsoft BASIC-80 interpreter under CP/M (MBASIC.COM).