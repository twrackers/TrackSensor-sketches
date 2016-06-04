// Include Wire (I2C) library.
#include <Wire.h>

#include "TrackSensor_Master.h"
#include "I2CSlave.h"
#include "SPIMaster.h"

I2CSlave::I2CSlave(
  const byte addr,
  CALLBACK1 receive,
  CALLBACK0 request,
  SPIMaster& spi
) : 
  StateMachine(1, false), 
  m_addr(addr), 
  m_requested(0x00),
  m_request(false),
  m_active(false),
  m_timeout(0UL),
  m_spi(spi)
{
  Wire.onReceive(receive);
  Wire.onRequest(request);
}

void I2CSlave::begin()
{
  Wire.begin(m_addr);
}

void I2CSlave::reqNumNodes()
{
  if (!m_active) {
    m_requested = m_spi.getNumNodes();
    m_request = true;
  }
}

void I2CSlave::reqNodeSensors(const byte node)
{
  if (!m_active) {
    m_requested = m_spi.getNodeSensors(node);
    m_request = true;
  }
}

void I2CSlave::setActive(bool active)
{
  m_active = active;
}

byte I2CSlave::getRequested()
{
  m_timeout = 0UL;
  byte r = m_requested;
  m_request = false;
  if (m_spi.getRingBuffer().isEmpty()) {
    digitalWrite(LOOK_AT_ME, HIGH);
  }
  return r;
}

bool I2CSlave::update()
{
  if (!StateMachine::update()) {
    return false;
  }
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
        RingBuffer& rb = m_spi.getRingBuffer();
        if (!rb.isEmpty()) {
          m_request = true;
          m_requested = rb.pop();
          digitalWrite(LOOK_AT_ME, LOW);
          m_state = eAvail;
          m_timeout = millis() + sm_timeout;
        }
      } 
      break;
    case eAwaitRequest:
      if (millis() >= m_timeout) {
        m_request = false;
        m_requested = 0;
        m_state = m_active ? eActive : eIdle;
      }
      break;
    case eAvail:
      if (millis() >= m_timeout) {
        m_request = false;
        m_requested = 0;
        m_state = eActive;
      } else {
        if (!m_request) {
          RingBuffer& rb = m_spi.getRingBuffer();
          if (!rb.isEmpty()) {
            m_requested = rb.pop();
            digitalWrite(2, LOW);
            m_timeout = millis() + sm_timeout;
          }
        }
      }
      break;
  }
  return true;
}

