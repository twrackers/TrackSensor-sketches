// Include Wire (I2C) library.
#include <Wire.h>

#include "TrackSensor_Controller.h"
#include "I2CPeripheral.h"
#include "SPICentral.h"

// I2CPeripheral class - Handles peripheral end of I2C link to
// central processor.
I2CPeripheral::I2CPeripheral(
  const byte addr,      // I2C address of this processor
  CALLBACK1 receive,    // address of onReceive callback
  CALLBACK0 request,    // address of onRequest callback
  SPICentral& spi       // SPICentral object
) : 
  StateMachine(1, false), 
  m_addr(addr), 
  m_requested(0x00),
  m_request(false),
  m_active(false),
  m_timeout(0UL),
  m_spi(spi)
{
  // Set up callbacks for I2C interface.
  Wire.onReceive(receive);
  Wire.onRequest(request);
}

void I2CPeripheral::begin()
{
  // Begin I2C interface.
  Wire.begin(m_addr);
}

void I2CPeripheral::reqNumNodes()
{
  // If not in active mode...
  if (!m_active) {
    // Number of connected SPI sensor nodes requested.
    m_requested = m_spi.getNumNodes();
    m_request = true;
  }
}

void I2CPeripheral::reqNodeSensors(const byte node)
{
  // If not in active mode...
  if (!m_active) {
    // Status of node's sensors is requested.
    m_requested = m_spi.getNodeSensors(node);
    m_request = true;
  }
}

void I2CPeripheral::setActive(bool active)
{
  // Switch to active mode.
  m_active = active;
}

byte I2CPeripheral::getRequested()
{
  // Get byte of data requested by I2C central.
  m_timeout = 0UL;
  byte r = m_requested;
  m_request = false;
  // ???
  if (m_spi.getFifo().isEmpty()) {
    digitalWrite(LOOK_AT_ME, HIGH);
  }
  return r;
}

bool I2CPeripheral::update()
{
  // Return immediately if not time to update.
  if (!StateMachine::update()) {
    return false;
  }
  // Otherwise...
  switch (m_state) {
    case eIdle:
      if (m_active) {
        m_state = eActive;
      } else if (m_request) {
        m_state = eAwaitRequest;
        m_timeout = millis() + sm_timeout;
      }
      break;
    case eActive:
      if (!m_active) {
        m_state = eIdle;
      } else {
        FIFO& fifo = m_spi.getFifo();
        if (!fifo.isEmpty()) {
          m_request = true;
          m_requested = fifo.pop();
          digitalWrite(LOOK_AT_ME, LOW);
          m_state = eAvail;
          m_timeout = millis() + sm_timeout;
        }
      } 
      break;
    case eAwaitRequest:
      if ((long) (millis() - m_timeout) >= 0) {
        m_request = false;
        m_requested = 0;
        m_state = m_active ? eActive : eIdle;
      }
      break;
    case eAvail:
      if ((long) (millis() - m_timeout) >= 0) {
        m_request = false;
        m_requested = 0;
        m_state = eActive;
      } else {
        if (!m_request) {
          FIFO& fifo = m_spi.getFifo();
          if (!fifo.isEmpty()) {
            m_requested = fifo.pop();
            digitalWrite(LOOK_AT_ME, LOW);
            m_timeout = millis() + sm_timeout;
          }
        }
      }
      break;
  }
  return true;
}

