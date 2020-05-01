#ifndef _SPI_MASTER__H_
#define _SPI_MASTER__H_

#include <SPI.h>
#include <StateMachine.h>
#include <RingBuffer.h>

// One of the following should be set to 1, the others to 0.
#define SLAVE_TEST 1
#define SLAVE_ARDUINO_UNO 0
#define SLAVE_ARDUINO_MICRO 0
#define SLAVE_PRO_TRINKET 0

class SPIMaster : public StateMachine
{
  private:
    static const unsigned long PACE = 50;  // msec
    static const int MAX_NODES = 16;

#if SLAVE_ARDUINO_UNO
    static const byte DEFAULT_MASK = 0x3F;
#elif SLAVE_ARDUINO_MICRO
    static const byte DEFAULT_MASK = 0xFF; // using first 8 of 12
#elif SLAVE_PRO_TRINKET
    static const byte DEFAULT_MASK = 0xFF;
#elif SLAVE_TEST
    static const byte DEFAULT_MASK = 0x03;
#else
#error Must specify slave node type.
#endif

    enum E_STATE {
      eEnumReady, eEnumPre, eEnumPost, eEnumUpdate,
      ePace,
      eReady, ePre, ePost, eUpdate,
      eFault
    } m_state = eEnumReady;

    RingBuffer m_rb;
    unsigned long m_delay = 0;
    unsigned long m_pacer = 0;
    byte m_nodes = 0;
    byte m_masks[MAX_NODES + 1];
    byte m_rxbuf[MAX_NODES + 1];
    byte m_sensors[MAX_NODES];
    int m_iters = 0;

  public:
    SPIMaster();
    void begin();
    RingBuffer& getRingBuffer();
    byte getNumNodes() const;
    byte getNodeSensors(const byte node) const;
    void setNodeMask(const byte node, const byte mask);
    void spiPreTransaction();
    void spiTransaction(int n);
    void spiPostTransaction();
    void spiUpdate();
    virtual bool update();
};

#endif

