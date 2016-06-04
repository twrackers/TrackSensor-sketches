# TrackSensor-sketches
These Arduino sketches form the software components for my model railroad track sensor design.  More information can be found at https://modelrrelectronics.wordpress.com/track-sensors.

##TrackSensor_Slave
This sketch is designed for the Adafruit Pro Trinket 5v and was developed and tested with the Arduino Micro.  It connects with up to 8 (6 on the Micro) QRE1113 analog infrared reflectance sensors to sense the passage of a model railroad car over it.  One or more slave units (Sensor Nodes) can be daisy-chained via SPI to the track sensor master, or Sensor Controller.

This sketch requires the EnableInterrupt library, available from GitHub at https://github.com/GreyGnome/EnableInterrupt.
##TrackSensor_Master
This sketch is designed to run on an Arduino-class processor, such as the Arduino Micro or the Adafruit Pro Trinket 5v.  It was developed and tested on an Arduino Uno.  This sketch will run on the Sensor Controller described on my Wordpress blog mentioned above.  The sketch serves two roles.

1. Accept commands via I2C from a central processor and return changes of sensor states to the central processor.  This sketch acts as an I2C Slave devie with a configurable address.
2. Repeatedly poll the sensor nodes connected to it by SPI (acting as the SPI Master device) to update the mask bytes in the nodes and receive the current status of all sensor channels.

This sketch requires my StateMachine library, available from GitHub at https://github.com/twrackers/StateMachine-library.
