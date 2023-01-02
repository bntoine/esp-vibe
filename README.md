# esp-Vibe
**WIP**

Most of this code was taken from https://github.com/MonomoriumP/Buttplug.io--Lelo

I have tested this with xtoys. It should work with buttplugio but I haven't tested.

For now it emulates a lovense edge and gives you two channels (on pins 13 and 14 by default) with 20 steps.

You should be able to use https://stpihkal.docs.buttplug.io/docs/stpihkal/protocols/lovense/ to emulate a different toy from lovense.

This works as is with PIO and VS Code/Codium. (You might want to change the specific board in platformio.ini but it should work either way)
You have to uncomment the #include <analogWrite.h> and install the ESP32 analogWrite lib if you want to use the arduino IDE.