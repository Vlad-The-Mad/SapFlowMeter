# SapFlowMeter

A sap flow meter measures the velocity of the sap in a tree. This is a useful indicator of the tree's health, and can provide valuable feedback for irrigation and water management.

Our meter here uses the Heat Ratio Method, where it applies a heat pulse into the sapwood, and measures the temperature of the sapwood both above and below the heater.

Currently we have designed flexible PCB probes, and we are testing our sap flow measurements in a tree against empirical water consumption measurements and a commercial sapflow probe.

This branch controls the basestation which collects and stores data from multiple devices.

## Libraries required:
- [SdFat](https://github.com/greiman/SdFat "SdFat")
- [OPEnS RTC](https://github.com/OPEnSLab-OSU/OPEnS_RTC "OPEnS_RTC")
- [Adafruit MAX31865](https://github.com/adafruit/Adafruit_MAX31865 "Adafruit MAX31865")
- [Low-Power](https://github.com/rocketscream/Low-Power "Low-Power")
- [Protothreads](https://github.com/P4SSER8Y/ProtoThreadsForArduino)
- [DSPlite](https://github.com/kamocat/DSPlite)
- [RadioHead](https://github.com/adafruit/RadioHead)
- [ArduinoJSON](https://github.com/bblanchon/ArduinoJson)
- [FeatherFault](https://github.com/OPEnSLab-OSU/FeatherFault)
- [ASF core](https://github.com/adafruit/Adafruit_ASFcore.git) commit f6ffa8b
## Basestation Branch Libraries:
- [Chart.js](https://github.com/chartjs/Chart.js)
- [CircuitPython](https://github.com/adafruit/Adafruit_Blinka)
- [Rasberry Pi GPIO](https://pypi.org/project/RPi.GPIO/)
- [Twurl](https://github.com/twitter/twurl)
