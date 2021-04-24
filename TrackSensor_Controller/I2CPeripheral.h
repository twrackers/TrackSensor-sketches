#ifndef _I2C_PERIPHERAL__H_
#define _I2C_PERIPHERAL__H_

#include <Wire.h>
#include <StateMachine.h>

#include "SPICentral.h"
#include "TrackSensor_Controller.h"
#include "I2C_Commands.h"

class I2CPeripheral : public StateMachine
{
  private:
    /*
     * Command codes have Hamming distance 
     * of at least 2 from each other.
     */
    enum E_STATE {
      eIdle, eActive,
      eAwaitRequest,
      eAvail,
      eFault
    } m_state = eIdle;
    
    const byte m_addr;
    byte m_requested;
    bool m_request;
    bool m_active;
    unsigned long m_timeout;
    SPICentral& m_spi;

    static const unsigned long sm_timeout = 50UL;
    
  public:
    I2CPeripheral(
      const byte addr,
      CALLBACK1 receive,
      CALLBACK0 request,
      SPICentral& spi
    );
    void begin();
    void reqNumNodes();
    void reqNodeSensors(const byte node);
    void setActive(bool active);
    byte getRequested();
    virtual bool update();
};


#endif
