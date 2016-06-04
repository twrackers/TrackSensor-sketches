// Include State Machine library.
#include <StateMachine.h>

// Local includes.
#include "TrackSensor_Master.h"
#include "SPIMaster.h"
#include "I2CSlave.h"

// SPI Master object.
SPIMaster spiMaster;

// Forward reference to I2C handlers.
void i2cReceiveHandler(int);
void i2cRequestHandler();

// I2C Slave object.
I2CSlave i2cSlave(
  I2C_ADDR, 
  i2cReceiveHandler, 
  i2cRequestHandler,
  spiMaster
);

// I2C Receive Handler, called when
// this I2C slave receives bytes from
// the I2C master.  Is passed the number
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
      spiMaster.setNodeMask(node, mask);
    }
  } else {
    // ... otherwise there's only the
    // command byte.  Decode and change
    // I2C Slave state machine's state
    // variables as required.
    if (cmd == eGetNumNodes) {
      // I2C Master requested node count.
      i2cSlave.reqNumNodes();
    } else if (cmd == eGetNodeSensors) {
      // I2C Master requested node's status byte.
      i2cSlave.reqNodeSensors(node);
    } else if (cmd == eIntrEnable) {
      // Shift I2C Slave to ACTIVE mode.
      i2cSlave.setActive(true);
    } else if (cmd == eIntrDisable) {
      // Shift I2C Slave to IDLE mode.
      i2cSlave.setActive(false);
    }
  }
}

// I2C Request Handler, called when I2C Master
// performs a read operation.
void i2cRequestHandler()
{
  // Send requested byte to I2C Master.
  Wire.write(i2cSlave.getRequested());
}

// The setup() function runs after reset.
void setup()
{
  // Set up output pin which will interrupt I2C master.
  digitalWrite(LOOK_AT_ME, HIGH);
  pinMode(LOOK_AT_ME, OUTPUT);
  
  // Start SPI interface.
  spiMaster.begin();
  
  // Give slaves time to boot, assuming they powered up
  // at the same time as the master.
  delay(1000);
  
  // Start I2C interface.
  i2cSlave.begin();
}

// The loop() function runs continuously after setup().
void loop() 
{
  spiMaster.update();
  i2cSlave.update();
}

