// Include State Machine library.
#include <StateMachine.h>

// Local includes.
#include "TrackSensor_Controller.h"
#include "SPICentral.h"
#include "I2CPeripheral.h"

// SPICentral object.
SPICentral spiCentral;

// Forward reference to I2C handlers.
void i2cReceiveHandler(int);
void i2cRequestHandler();

// I2CPeripheral object.
I2CPeripheral i2cPeripheral(
  I2C_ADDR, 
  i2cReceiveHandler, 
  i2cRequestHandler,
  spiCentral
);

// I2C Receive Handler, called when
// this I2C peripheral receives bytes from
// the I2C central.  Is passed the number
// of bytes in the received I2C transaction.
void i2cReceiveHandler(int n)
{
  // Get the command byte.
  byte cmd = Wire.read();
  // Some commands put the node number
  // in the low 4 bits.
  byte node = cmd & 0x0F;
  // The command is the high 4 bits only.
  cmd &= 0xF0;
  // If more than 1 byte received...
  if (n > 1) {
    // ... there should be one more byte
    // containing the mask byte for a node.
    byte mask = Wire.read();
    // If the command code is correct,
    // send the mask byte to the specified node.
    if (cmd == eSetNodeMask) {
      spiCentral.setNodeMask(node, mask);
    }
  } else {
    // ... otherwise there's only the
    // command byte.  Decode and change
    // I2C Peripheral state machine's state
    // variables as required.
    if (cmd == eGetNumNodes) {
      // I2C Central requested node count.
      i2cPeripheral.reqNumNodes();
    } else if (cmd == eGetNodeSensors) {
      // I2C Central requested node's status byte.
      i2cPeripheral.reqNodeSensors(node);
    } else if (cmd == eIntrEnable) {
      // Shift I2C Peripheral to ACTIVE mode.
      i2cPeripheral.setActive(true);
    } else if (cmd == eIntrDisable) {
      // Shift I2C Peripheral to IDLE mode.
      i2cPeripheral.setActive(false);
    }
  }
}

// I2C Request Handler, called when I2C Central
// performs a read operation.
void i2cRequestHandler()
{
  // Send requested byte to I2C Central.
  Wire.write(i2cPeripheral.getRequested());
}

// The setup() function runs after reset.
void setup()
{
  // Set up output pin which will interrupt I2C central.
  digitalWrite(LOOK_AT_ME, HIGH);
  pinMode(LOOK_AT_ME, OUTPUT);
  
  // Start SPI interface.
  spiCentral.begin();
  
  // Give peripherals time to boot, assuming they powered up
  // at the same time as the central.
  delay(1000);
  
  // Start I2C interface.
  i2cPeripheral.begin();
}

// The loop() function runs continuously after setup().
void loop() 
{
  spiCentral.update();
  i2cPeripheral.update();
}

