/*
 * ECE 5250
 * Final Project
 * Talking clock
 * 
 * Benjamin Kueffler
 * Raymond Mak
 * Rafi Meguerdijian
 * Eugene Chiu
 */

/* Dependences */

// Include software serial library to send software UART commands to MP3 player
#include <SoftwareSerial.h>

// TimerOne should be installed from Arduino Library Manager for drivers for hardware timer
#include <TimerOne.h>

// Include the EEPROM library for easy access to read/writes of AVR EEPROM
#include <EEPROM.h>

// Include open source display library
#include <TM1637Display.h>

// Include mp3 playing functionality
#include <DFRobotDFPlayerMini.h>
#include <timetomp3.h>

/* Port Assignment */
#define microphone A0 //Placeholder for microphone input port
#define mp3_rx 7      // UART RX for mp3 player
#define mp3_tx 6      // UART TX for mp3 player
#define btn_config_hr 3  // The interrupt pin for configuring hours on the display, attached to a physical button
#define btn_config_min 2 // The interrupt pin for configuring minutes on the display, attached to a physical button
#define btn_config_mode A1 // The pin change interrupt for incrementing the modes. Do not put any other circuits on D8-D13.
#define disp_clk 4 // The clock provided to the display 2 wire interface for data latching
#define disp_dio 5 // The data i/o for sending serial commands to the display 2 wire interface

/* Constants */

// Determines the trip point for the microphone to detect a clap to start the talking process
//const uint16_t kClapSense = 100; works well for 680 ohm
// 125 works okay for 1k ohm
//const uint16_t kClapSense = 119;
const uint16_t kClapSense = 146;

// Width of one clap in milliseconds
const uint8_t kClapWidth = 55;
const uint16_t kSilentPeriod = 250;
const uint16_t kDetectionInterval = 1250;

// Determines MP3 volume (0-30)
const uint8_t kVolume = 30;

// LED segment brightness from 0-7
const uint8_t kLEDBrightness = 3;

// The number of milliseconds in the LED display button configuration debounce interval
const uint8_t kDebounceDelay = 200;

// The resolution of our clock/Timer ISR. This is a number in microseconds.
// This does not neccessarily have to match the LED segment update rate
const uint32_t kTimeResolution = 500000;

// The number of bytes avilable in the EEPROM, varies by microcontroller
const uint16_t kEepromSize = EEPROM.length();

// The EEPROM addresses and their functionality
const uint8_t kTimePointerAddr = 0; // Reserved for the pointer to the start of time_disp data in the EEPROM
const uint8_t kModeAddr = 0x02; // Reserved for the storage of the previously selected mode
const uint8_t kTimeAddr = 0x03; // The start of hte address space for hte time_disp data

// The number of seconds before it's desired to change the EEPROM time starting address, this shouldn't be too low
// If this number is set too low, special address 0x00 will be written too many times, and will be destroyed
const uint16_t kStAddrUpdateTime = uint16_t(12) * 60 * 60; // After 12 hours, pointer to time_disp (time_addr) will be updated

// We want the number of reads to be a power of two, to make division easy for processor. We also want it to be about 5ms worth of samples.
// So if we take 1 MHz as our processing speed, we have 1M samples per second. So we want to divide 1M * .005s = 500 samples, so set
// to 4096 samples = 4.096 ms sample.
const uint16_t kNumReadsInAverage = 8;

//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
// One hot bytes corresponding to commonly used characters
const uint8_t kCommonSegments[] =
{
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001,    // F
  0b01010100,    // n
  0b01010000,    // r
  0b01110110     // H
};

// Messages to be displayed on the LED display in the event of a mode switch
const uint8_t kModeMsg[3][4] = 
{
  // Mode 0 EnG2
  {kCommonSegments[14], kCommonSegments[16], kCommonSegments[6], kCommonSegments[2]},
  // Mode 1 EnG1
  {kCommonSegments[14], kCommonSegments[16], kCommonSegments[6], kCommonSegments[1]},
  // Mode 2 ZH0
  {kCommonSegments[2] , kCommonSegments[18], kCommonSegments[0], 0}
};

// Error message to be displayed if the chosen mode is not supported
const uint8_t kModeErrMsg[4] = {kCommonSegments[14] , kCommonSegments[17], kCommonSegments[17], 0};

/* Function Defines */

// Sets the LED display to display different text indications, depending on what mode was selected
// Inputs : disp - Display LED object, mode - integer presenting operating time/language
void ModeToDisplay(TM1637Display* disp, uint8_t mode);