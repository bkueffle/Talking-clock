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
// TM1637 Drivers should be installed by downloading Grove 4-Digit Display from Arduino Library Manager
#include <TM1637.h>

// TimerOne should be installed from Arduino Library Manager for drivers for hardware timer
#include <TimerOne.h>

/* Port Assignment */
#define microphone 1 //Placeholder for microphone input port
#define lcd_config_hr 2  // The interrupt pin for configuring hours on the LCD, attached to a physical button
#define lcd_config_min 3 // The interrupt pin for configuring minutes on the LCD, attached to a physical button
#define lcd_clk 4 // The clock provided to the LCD 2 wire interface for data latching
#define lcd_dio 5 // The data i/o for sending serial commands to the LCD 2 wire interface

/* Constants */
// Determines the trip point for the microphone to detect a clap to start the talking process
const unsigned int clap_sensitivity = 2048;

// Swaps between military time and regular time
const bool military_time = false;

//LCD Constants
const int lcd_brightness = 0;

// The resolution of our clock/Timer ISR. This is a number in microseconds.
// This does not neccessarily have to match the LCD update rate
const unsigned long time_resolution = 500000;

/* Global Variables */

// LCD Global Variables for ISR

// Bytes to display on the LCD, 0 - Hour digit 2, 1 - Hour digit 1, 2 - Min digit 2, 3 - Min digit 1
int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};

// Update is a flag that alerts the rest of the system when the ISR has calculated a new value and may need to update the LCD
bool Update;
// Enables the colon in the LCD
unsigned char ClockPoint = 1;
// Handles the ISR memory of time, these values may be updated at any point in the program
unsigned char subsecond = 0;
unsigned char second = 0;
unsigned char minute = 0;
unsigned char hour = 12;

// Create the LCD display object
TM1637 tm1637(lcd_clk,lcd_dio);
 
void setup() {
  pinMode(microphone,INPUT);
  pinMode(lcd_config_hr,INPUT);
  pinMode(lcd_config_min,INPUT);
  pinMode(lcd_clk,OUTPUT);
  pinMode(lcd_dio,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(lcd_config_hr), changeMinISR, RISING);
  attachInterrupt(digitalPinToInterrupt(lcd_config_min), changeHourISR, RISING);
  
  // Sets initial brightness, data, address)
  tm1637.set(lcd_brightness);
  tm1637.init();
  Timer1.initialize(time_resolution); // Set the ISR frequency
  Timer1.attachInterrupt(TimingISR);//declare the interrupt serve routine:TimingISR  
 
  
}

void loop() {

  // At the top of the loop, update the LCD if the subseconds have changed
  // If a valid input is detected, the LCD will not update, but the time will continue counting, so subseconds will not be lost
  if(Update)
  {
    TimeUpdate();
    tm1637.display(TimeDisp);
  }
/*
 * Code for the input device will go here
  if (analogRead(microphone) > clap_sensitivity)
  {
    //Start talking
     
  }
*/

}

// The interrupt service routine for keeping time. This does not update the LCD by itself.
void TimingISR()
{
  // During every interrupt, increment the subsecond, this is the smallest resolution of our clock
  subsecond ++;
  Update = true;
  if(subsecond == 1000000/time_resolution){
    second ++;
    if(second == 60)
    {
      minute ++;
      if(minute == 60)
      {
        hour ++;
        if (military_time)
        {
          if(hour == 24) hour = 0;
        }
        else 
        {
          if(hour == 13) hour = 1;
        };
        minute = 0;
      }
      second = 0;
    }
    subsecond = 0;  
  }
  //Serial.println(second);
  ClockPoint = (~ClockPoint) & 0x01;
}

// Update the LCD with the time stored in the variables
void TimeUpdate(void)
{
  if(ClockPoint)tm1637.point(POINT_ON);
  else tm1637.point(POINT_OFF); 
  TimeDisp[0] = hour / 10;
  TimeDisp[1] = hour % 10;
  TimeDisp[2] = minute / 10;
  TimeDisp[3] = minute % 10;
  Update = false;
}

// Updates the minute by one every time the minute button is pushed
void changeMinISR(void)
{
  minute ++;
  if(minute == 60) minute = 0;
}

// Updates the hour by one every time the hour button is pushed
void changeHourISR(void)
{
  hour ++;
  if (military_time)
  {
    if(hour == 24) hour = 0;
  }
  else 
  {
    if(hour == 13) hour = 1;
  };
}


