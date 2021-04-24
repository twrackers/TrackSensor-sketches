# TrackSensor-sketches #

These Arduino sketches form the software components for my model railroad track sensor design.  More information can be found at [modelrrelectronics.wordpress.com/track-sensors](https://modelrrelectronics.wordpress.com/track-sensors).

## TrackSensor_Node ##

This sketch is designed for the Adafruit Pro Trinket 5v and was developed and tested with the Arduino Micro.  This sketch will run on the Sensor Node described in my Wordpress blog mentioned above.  It connects with up to 8 QRE1113 analog infrared reflectance sensors to sense the passage of a model railroad car over it.  Up to 16 track sensor nodes can be daisy-chained via SPI to the track sensor controller.

This sketch requires the EnableInterrupt library, available from GitHub at [github.com/GreyGnome/EnableInterrupt](https://github.com/GreyGnome/EnableInterrupt).

## TrackSensor_Controller ##

This sketch is designed to run on an Arduino-class processor, such as the Arduino Micro or the Adafruit Pro Trinket 5v.  It was developed and tested on an Arduino Uno.  This sketch will run on the Sensor Controller described on my Wordpress blog mentioned above.  The sketch serves two roles.

1. Accept commands via I<sup>2</sup>C from a central processor and return changes of sensor states to the central processor.  This sketch acts as an I<sup>2</sup>C peripheral device with a configurable address.
2. Repeatedly poll the sensor nodes connected to it by SPI to update the mask bytes in the nodes and receive the current status of all sensor channels from the nodes.

This sketch requires my *StateMachine* and *FIFO* libraries, available from GitHub here.

- [github.com/twrackers/StateMachine-library](https://github.com/twrackers/StateMachine-library)
- [github.com/twrackers/FIFO-library](https://github.com/twrackers/FIFO-library)
