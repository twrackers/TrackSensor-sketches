#ifndef _I2C_MASTER__H_
#define _I2C_MASTER__H_

// NOTE: A copy of this file also exists under
// the I2C_Master sketch.  Both must
// contain the same content.

// Each command code has a Hamming distance
// of at least 2 bits from all other codes.
// Additional codes which would meet this
// criterion are (using only high 4 bits):
//   0xC0  0xE0  0xF0
enum E_CMD {
  eGetNumNodes    = 0x10,
  eSetNodeMask    = 0x20,
  eGetNodeSensors = 0x40,
  eIntrEnable     = 0x70,
  eIntrDisable    = 0x80
};

const byte I2C_ADDR = 8;    // I2C address

#endif

