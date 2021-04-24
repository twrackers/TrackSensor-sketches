/* 
 * Track Sensor Node
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
 * During each acquisition cycle by the controller,
 * multiple bytes will circulate through the nodes,
 * one byte per node.  When SS* is deasserted by the controller,
 * each node will hold its new mask value, and all nodes'
 * status bytes will have been received by the controller.
 */
 
// Include SPI (Serial Peripheral Interface) library. 
// Does not directly support SPI "slave" mode,
// used for symbol definitions.
#include <SPI.h>

// Include interrupt library from github.
// https://github.com/GreyGnome/EnableInterrupt
#include <EnableInterrupt.h>

// Number of analog inputs available
const int NUM_AIN = 8;          // 6 for Arduino Uno,
                                // 8 for Pro Trinket
// Sense threshold
const float THR = 0.9;
// Sampling interval
const unsigned long INTERVAL = 50;  // milliseconds, 20 Hz

// Mask byte received from track sensor controller
volatile byte g_mask = 0x00;    // from controller, 1 bit enables
                                // analog channel
// Sense outputs, sent to track sensor controller
volatile byte g_analogs = 0xFF; // to controller, 0 bit indicates sense

// channel calibrations
unsigned int g_thr[NUM_AIN];    // sensor thresholds from calibration
// next scheduled sample time (milliseconds)
unsigned long g_next = 0;

// Interrupt service routine
// This function runs every time the SS pin changes state.
// SS goes low when the SPI central is beginning a data transfer,
// so the ISR copies the current status byte to the SPI data
// register.  The actual SPI transfer occurs in hardware.
// SS goes high when the SPI central has completed a data transfer.
// This is the cue for the ISR to copy the updated mask byte from
// the SPI data register.
void isr()
{
  // Interrupt triggers on rising or falling edge of SS.
  if (digitalRead(SS) == LOW) {
    // If SS just went low, transaction is starting.
    // Write analog status to SPI data register.
    SPDR = g_analogs;
  } else {
    // If SS just went high, transaction is complete.
    // Read new mask from SPI data register.
    g_mask = SPDR;
  }
}

// Calibrate sensor channels by reading and averaging
// no-sense outputs.
void calibrateSensors()
{
  // Will average ITERS samples.
  const int ITERS = 60;
  // Accumulators for averaging.
  unsigned long sums[NUM_AIN];
  // Zero accumulators.
  for (int i = 0; i < NUM_AIN; ++i) {
    sums[i] = 0;
  }
  // For ITERS samples...
  for (int j = 0; j < ITERS; ++j) {
    // For NUM_AIN channels...
    for (int i = 0; i < NUM_AIN; ++i) {
      // Read and accumulate.
      sums[i] += analogRead(i);
    }
    // Delay 1 msec before next iteration.
    delay(1);
  }
  // For each channel, sense threshold is THR times
  // average of no-sense samples.
  for (int i = 0; i < NUM_AIN; ++i) {
    g_thr[i] = (unsigned int) (THR * sums[i] / ITERS);
  }
}

// The setup() function runs after reset.
void setup()
{
  // Calibrate sensors on analog inputs.
  calibrateSensors();
  
  // Make sure MISO is output.
  // SCLK, MOSI, and SS default to inputs.
  pinMode(MISO, OUTPUT);
  
  // Enable SPI as "slave" (peripheral).
  SPCR |= (1 << SPE);
  
  // Enable SS as pin-change-interrupt source
  // with pull-up.
  pinMode(SS, INPUT_PULLUP);

  // Enable pin-change interrupt on SS.
  enableInterrupt(SS, isr, CHANGE);

  // Schedule first data sample.
  g_next = millis() + INTERVAL;
}

// The read function runs at a high rate on the peripheral,
// but will be interrupted when an SPI transaction is
// started by the SPI central.
void readAnalogs()
{
  // Copy analog status (atomic read).
  byte ain = g_analogs;

  // Read all analog channels, set to 0 any bits for
  // sensors which detect object.
  for (int i = 0; i < NUM_AIN; ++i) {
    if (analogRead(i) > g_thr[i]) {
      // No sense, set bit to 1.
      ain |= (1 << i);
    } else {
      // Sense, set bit to 0.
      ain &= ~(1 << i);
    }
  }
  
  // Update analog status, masking off unwanted bits
  // (atomic write).  Bits masked out will be forced
  // to 1, i.e. no-sense.
  g_analogs = ain | ~g_mask;
}

// The loop function runs continuously after setup().
void loop()
{
  // Time to read the analog inputs again?
  if ((long) (millis() - g_next) >= 0) {
    // Update analogs.
    readAnalogs();
    // Pace the acquisition loop.
    g_next = millis() + INTERVAL;
  }
}

