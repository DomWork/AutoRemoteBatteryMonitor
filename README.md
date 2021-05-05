# AutoRemoteBatteryMonitor

Arduino NodeMCU/ESP8266 battery monitor to automatically measure and report the voltage to Google Sheets, sending an alert if it falls below a threshold.
I am using this to remotely monitor my car's battery as these days weeks can pass without me using it and the battery can go flat.
However you can use it to monitor any battery, or power supply - to check on charge status for solar panel charged batteries, or
to check on battery health.

Measure battery voltage, log it to "battery monitor" google sheet, then
IF the voltage is below a set threshold, also send it as an email, then go to sleep to conserve power FOR A LONG TIME, repeat.
To enable deep sleep, RST needs to be connected to D0 / GPIO16 / Wake pin after programming (disconnect while programming!!).
ESP8266 Deep sleep mode example Rui Santos should take credit for the original code to log the data.

Numerous online sources of code to log data directly to Google Sheets created errors I could not fix, but this version worked.
Complete Project Details http://randomnerdtutorials.com
https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/

Over the versions worked on during April 2021 I have got it to diplay on a web site, but it needs to be on permanently, not good for battery
and to send it just as an email when below a threshold and now via PushingBox to load into a Google Sheets
I draw power from the battery to both power the Arduino and have a potential divider to reduce the full voltage to swing only between 0-3V
so as not to put a higher voltage into the Arduino than it can cope with.

The buck regulator, potential divider and NodeMCU microcontroller draw the following currents:
1.05mA when in deep sleep, 33mA when on, 38mA when using wifi

NOTE ON ADC TO VOLTAGE CORRECTION FACTOR
For a long time I struggled with changing readings even though the voltage source was constant.
It also changes when plugged into the computer versus being plugged into just the external power supply.
That is why I put in multiple reading and averaging and also why I turned the wifi on for a while first, so as to pull a little current and "stabilise" the battery first.
The reading on the ADC pin changes from 657 when plugged into both down to 646 or 638 when just the PSU at a stable 12.22V!!
However I discovered that once I plugged everything together with firm connectors, rather than relatively loose ones, the variability disappeared.

This version does:
wakes up
connects to wifi
turns led on
measure voltage
takes an average of 5 sequential readings
sends data reading to Google Sheets
also sends email if voltage is below a threshold
goes back to sleep

LED brief flash = all is ok (7 brief flashes to run through entire program)
LED long flash = 1 flash: voltage is below threshold, 3 flashes: failed to connect to http port 80,
              6 flashes: failed to connect to wifi,  9 flashes: timed out while sending data to pushingbox
The google sheet is in .... YOUR GOOGLE DRIVE
The google sheet script editor script for it is at .... YOUR GOOGLE DRIVE
Logged in also as .... YOUR USERNAME on Pushing Box, the programmed service and scenario are at .... YOUR SCENARIO
The youtube video I followed by Uteh Str called Arduino | Send Data to Google Sheets with LoLon NodeMCU ESP8266 V3 and Pushingbox API is at
https://www.youtube.com/watch?v=4e9hE34RMZM with his original code listed there

This program would run fine in the house, but in the car it would regularly fail and the light would be on. I tracked this down to line 157 where
the "return" command would run if it failed to get a response from pushingbox and just stop.
I replaced that with a deep-sleep which effectively reboots the device and it works fine.
There is possibly one more line (139, connecting to http port 80) with a "return" command that might need replacing if it hangs again.

NEXT ACTIVITY - I also spent a long time experimenting with how long you could put the device into deep sleep.
For me it was only about 90 minutes before its behaviour started getting erratic.
However I don't really want hourly data.
So if anyone can help by adding code to use RTC memory (that remains in place even during a deep sleep) to store how many sleeps,
and only send data every 10th I'd be grateful.

*/
