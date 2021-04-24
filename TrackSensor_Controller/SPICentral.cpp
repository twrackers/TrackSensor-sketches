/* 
 * Track Sensor SPI Central
 *
 * NOTE: The terminology for the SPI signals are under review by
 * the electronics industry.  For now, I am using the label "Main"
 * to refer to an SPI Central interface, and "Secondary" for an SPI
 * Peripheral interface.
 *
 * SPI pin numbers on Arduino Uno and Pro Trinket:
 * SCK   13  // Serial Clock.
 * MISO  12  // Main In Secondary Out.
 * MOSI  11  // Main Out Secondary In.
 * SS    10  // Secondary Select
 * 
 * SCK and SS from central are bussed to the same pins on
 * all peripheral nodes.
 * MOSI on the central goes to MOSI on the last peripheral.
 * MISO on the last peripheral goes to MOSI on the 2nd-last peripheral.
 * ...
 * MISO on the 3rd peripheral goes to MOSI on the 2nd peripheral.
 * MISO on the 2nd peripheral goes to MOSI on the 1st peripheral.
 * MISO on the 1st peripheral goes to MISO on the central.
 * 
 * Note that the nodes receive on MOSI and send on MISO,
 * while the controller sends on MOSI and receives on MISO.
 * 
 * The first byte shifted out will go through all the nodes
 * starting with the last node and ending up in the first node.
 * The last byte shifted out will go to the last node.
 * In other words, bytes circulate from the controller through
 * the last node on an SPI loop towards first node and back to
 * the controller.
 * 
 * In the enumeration phase, the number of connected nodes
 * on the SPI loop will be determined.  This is done by the controller
 * sending a series of 0x00 mask bytes, first one, then two, then
 * three, and so on.  The nodes boot up with their masks set to 
 * 0x00 as well.  A mask bit of 0 forces its status bit to 1.  
 * As long as the number of 0x00 mask bytes sent doesn't exceed 
 * the number of nodes on the loop, the received status bytes will 
 * all be 0xFF.
 * 
 * When the number of mask bytes sent exceeds the number of
 * nodes by one, the first 0x00 sent will circulate through all 
 * the nodes and end up in the last position of the controller's 
 * receive buffer.  Once this occurs, the number of nodes is 
 * known to be one fewer than the number of mask bytes sent
 * on that cycle.
 * 
 * On each acquisition cycle, the controller will send m_nodes
 * mask bytes, starting with the mask for the first node.
 * 1-bits in the mask indicates that channel should be sampled.
 * While the masks are being sent to the nodes, the controller will
 * receive in turn the sensor outputs of the nodes, starting 
 * with the first node.  A 0-bit indicates a detection by the
 * corresponding sensor channel in that node.
 */
 
#include "SPICentral.h"

// Specify size of FIFO (ring buffer) which will collect
// sensor state-change events.
#define BUFFER_SIZE 64

// Constructor
//
// Sets mask bytes to 0x00 for node enumeration procedure.
SPICentral::SPICentral() : 
  StateMachine(1, false),
  m_fifo(FIFO(BUFFER_SIZE))
{
  // Initialize masks for node enumeration.
  // Test for up to one more than max.
  register byte* p = m_masks;
  for (byte i = 0; i <= MAX_NODES; ++i) {
    *p++ = 0x00;
  }
  // Initially look for one node.
  m_nodes = 1;
}

// Starts SPI interface.
void SPICentral::begin()
{
  // Initialize SPI.
  // Puts SCK, MOSI, SS pins into output mode,
  // and MISO in input state.
  // Sets SCK and MOSI into LOW state, SS into HIGH state.
  // Then puts SPI hardware into Central mode 
  // and enables SPI.
  SPI.begin();
}

// Returns reference to this object's ring buffer.
FIFO& SPICentral::getFifo()
{
  return m_fifo;
}

// Returns number of nodes found by enumeration
// procedure once it's completed.
byte SPICentral::getNumNodes() const
{
  return m_nodes;
}

// Returns sensor status byte from specified node.
byte SPICentral::getNodeSensors(const byte node) const
{
  return m_sensors[node];
}

// Sets mask byte to be sent to specified node.
void SPICentral::setNodeMask(const byte node, const byte mask)
{
  m_masks[node] = mask;
}

// Asserts Node Select (active low) to signal nodes
// of impending transfer cycle.
void SPICentral::spiPreTransaction()
{
  // Assert SS* (active low) to all nodes.
  digitalWrite(SS, LOW);
  // Allow time for nodes to prepare.
  m_delay = millis() + 10;
}

// Performs transfer of mask bytes to nodes
// while receiving sensor status bytes from nodes.
void SPICentral::spiTransaction(int n)
{
  // Shift mask bytes out to nodes, 
  // and at the same time receive status bytes.
  byte* txp = m_masks;
  byte* rxp = m_rxbuf;
  for (byte i = 0; i < n; ++i) {
    *rxp++ = SPI.transfer(*txp++);
  }
  // Allow time for nodes to complete.
  m_delay = millis() + 20;
}

// Deasserts Node Select to allow nodes to complete
// data transfer.
void SPICentral::spiPostTransaction()
{
  // Deassert SS*.
  digitalWrite(SS, HIGH);
  m_delay = millis() + 20;
}

// Performs state-change detection on sensor status
// bits for all nodes.  For each bit which has changed
// state, queue up into the FIFO a byte containing:
//   SNNNNCCC
// where 
//   S = 0 for 1->0 or 1 for 0->1 transitions
//   NNNN = node (0 to (m_nodes - 1))
//   CCC = sensor channel (0 to 7)
void SPICentral::spiUpdate()
{
  register byte* sp = m_rxbuf;
  register byte* dp = m_sensors;
  for (byte i = 0; i < m_nodes; ++i) {
    // 1 bits in delta mark bits that changed
    // since the previous data transfer.
    byte delta = *dp ^ *sp;
    // Start with LSB (sensor channel 0).
    byte c = 0;
    // Status byte being examined.
    byte s = *sp;
    // While there are 1-bits remaining in delta...
    while (delta != 0) {
      // If LSB of delta is 1...
      if ((delta & 0x01) != 0) {
        // Create and queue up event byte.
        byte q = 
          ((s & 0x01) << 7) | (i << 3) | c;
        m_fifo.push(q);
      }
      // Shift delta and sensor state right 1.
      delta >>= 1;
      s >>= 1;
      // Increment sensor channel number.
      ++c;
    }
    // Save received byte as new node state,
    // and increment pointers.
    *dp++ = *sp++;
  }
}

// State Machine update method.
bool SPICentral::update()
{
  // Return immediately if not time for update.
  if (!StateMachine::update()) {
    return false;
  }
  // If a programmed delay has elapsed...
  if ((long) (millis() - m_delay) >= 0) {
    // Switch according to current state.
    switch(m_state) {
      case eEnumReady:
        // Prepare for node enumeration step.
        spiPreTransaction();
        m_state = eEnumPre;
        break;
      case eEnumPre:
        // Test for m_nodes nodes.
        spiTransaction(m_nodes);
        m_state = eEnumPost;
        break;
      case eEnumPost:
        // Complete node enumeration step.
        spiPostTransaction();
        m_state = eEnumUpdate;
        break;
      case eEnumUpdate:
        // Check results of enumeration step.
        if (m_rxbuf[m_nodes - 1] == 0x00) {
          // m_nodes is one too high now.
          // Adjust it down to correct value.
          --m_nodes;
          // Initialize masks to default values.
          for (byte i = 0; i < m_nodes; ++i) {
            m_masks[i] = DEFAULT_MASK;
          }
          // Initialize status-change detection.
          register byte* sp = m_rxbuf;
          register byte* dp = m_sensors;
          for (byte i = 0; i < m_nodes; ++i) {
            *dp++ = *sp++;
          }
          // Enter normal state loop.
          m_pacer = millis() + PACE;
          m_state = ePace;
        } else if (m_nodes <= MAX_NODES) {
          // May not have found all nodes yet,
          // try raising number of nodes
          // and repeat.
          ++m_nodes;
          m_state = eEnumReady;
        } else {
          // Too many nodes connected, 
          // enter fault state.
          m_state = eFault;
        }
        break;
      case ePace:
        // If time for acquisition cycle,
        // go to ready state.
        if ((long) (millis() - m_pacer) >= 0) {
          m_pacer += PACE;
          m_state = eReady;
        }
        break;
      case eReady:
        // Prepare for data transfer step.
        spiPreTransaction();
        m_state = ePre;
        break;
      case ePre:
        // Perform data transfer.
        spiTransaction(m_nodes);
        m_state = ePost;
        break;
      case ePost:
        // Complete data transfer.
        spiPostTransaction();
        m_state = eUpdate;
        break;
      case eUpdate:
        // Process acquired sensor status data.
        spiUpdate();
        m_state = ePace;
        break;
      default:
        // Come here if in fault state, or if
        // state has invalid value.
        m_delay = millis() + 1000;
        m_state = eFault;
    }
  }
  // State machine has updated.
  return true;
}

