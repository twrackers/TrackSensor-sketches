# TrackSensor-sketches
These Arduino sketches form the software components for my model railroad track sensor design.  More information can be found at https://modelrrelectronics.wordpress.com/track-sensors.

##TrackSensor_Slave
This sketch is designed for the Adafruit Pro Trinket 5v and was developed and tested with the Arduino Micro.  It connects with up to 8 QRE1113 analog infrared reflectance sensors to sense the passage of a model railroad car over it.  One or more track sensor slaves (Sensor Nodes) can be daisy-chained via SPI to the track sensor master, or Sensor Controller.

This sketch requires the EnableInterrupt library, available from GitHub at https://github.com/GreyGnome/EnableInterrupt.
##TrackSensor_Master
This sketch is designed to run on an Arduino-class processor, such as the Arduino Micro or the Adafruit Pro Trinket 5v.  It was developed and tested on an Arduino Uno.  This sketch will run on the Sensor Controller described on my Wordpress blog mentioned above.  The sketch serves two roles.

1. Accept commands via I<sup>2</sup>C from a central processor and return changes of sensor states to the central processor.  This sketch acts as an I<sup>2</sup>C Slave device with a configurable address.
2. Repeatedly poll the sensor nodes connected to it by SPI (acting as the SPI Master device) to update the mask bytes in the nodes and receive the current status of all sensor channels from the nodes.

This sketch requires my StateMachine and RingBuffer libraries, available from GitHub here.

- https://github.com/twrackers/StateMachine-library
- https://github.com/twrackers/RingBuffer-library
