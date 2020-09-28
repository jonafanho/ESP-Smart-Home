# ESP Smart Home
A smart home system with the ESP8266.

## Introduction
The code in this repository can be used to create a sensor-controlled power strip. Each socket in the power strip, referred to as a "port", can be configured by the user to turn on under certain conditions and sensor readings.

Any number of sockets can be controlled. As an example, a power strip with six sockets and a module with four solid state relays are used. Therefore, two plugs will always be on and four of them will be controlled. The following diagram shows the different components of a sample device.

![Sample Device](/images/Example.png)

## WiFi Setup
The first thing that needs to be done when setting up the device for the first time is to input the WiFi details.

1. Make sure the power strip is off. Check to see that the it is unplugged.
1. While holding down the setup button as marked in the image above, power on the power strip by plugging it into an outlet.
1. Wait until the screen on the power strip shows the text `Connect to WiFi: Smart Home Setup`. The setup button may now be released.
1. Using a phone, laptop, or another device capable of connecting to a WiFi network, connect to the WiFi network "Smart Home Setup".
1. A webpage should pop up automatically in a few seconds on the device. If it doesn't appear, open up an internet browser on the device and nagivate to the IP address shown on the screen. For example, if the screen shows `192.168.0.123`, navigate to `http://192.168.0.123` in the internet browser.
1. Follow the instructions on the screen by entering a nearby WiFi SSID and password. Click on the `Save` button when finished.
1. Restart the power strip by unplugging it from the outlet and plugging it back in.

## Configuration
After the WiFi is set up, the logic for the ports can be configured using a web interface.

1. On the very bottom of the screen, the currently connected WiFi network and the power strip's IP address should be shown. If either of these fields is missing, the WiFi connection was unsuccessful.
1. On a phone, laptop, or another device connected to the same WiFi network as the power strip, open up an internet browser.
1. Navigate to the IP address shown on the screen of the power strip. For example, if the screen shows `192.168.0.10`, navigate to `http://192.168.0.10` in the internet browser.
1. Click on the `Edit` button.
1. For each port, a new rule can be added by clicking on `Add Rule`. Each port can have more than one rule.
1. For each rule, different conditions can be added for each sensor. The port will turn on if ALL of the conditions for AT LEAST ONE rule are met.
