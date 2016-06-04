# TrackSensor-sketches
These Arduino sketches form the software components for my model railroad track sensor design.  More information can be found at https://modelrrelectronics.wordpress.com/track-sensors.

##TrackSensor_Slave
This sketch is designed for the Adafruit Pro Trinket 5v and was developed and tested with the Arduino Micro.  It connects with up to 8 (6 on the Micro) QRE1113 analog infrared reflectance sensors to sense the passage of a model railroad car over it.  One or more slave units (Sensor Nodes) can be daisy-chained via SPI to the track sensor master, or Sensor Controller.

This sketch requires the EnableInterrupt library, available from GitHub at https://github.com/GreyGnome/EnableInterrupt.
##TrackSensor_Master
