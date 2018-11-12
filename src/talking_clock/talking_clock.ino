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
#include <TM1637Display.h>

// TimerOne should be installed from Arduino Library Manager for drivers for hardware timer
#include <TimerOne.h>

// Include mp3 playing functionality
#include <DFRobotDFPlayerMini.h>
#include <timetomp3.h>

// Include software serial library to send software UART commands to MP3 player
#include <SoftwareSerial.h>

/* Port Assignment */
#define microphone A0 //Placeholder for microphone input port
#define mp3_rx 7      // UART RX for mp3 player
#define mp3_tx 6      // UART TX for mp3 player
#define lcd_config_hr 3  // The interrupt pin for configuring hours on the LCD, attached to a physical button
#define lcd_config_min 2 // The interrupt pin for configuring minutes on the LCD, attached to a physical button
#define lcd_config_mode A1 // The pin change interrupt for incrementing the modes. Do not put any other circuits on D8-D13.
#define lcd_clk 4 // The clock provided to the LCD 2 wire interface for data latching
#define lcd_dio 5 // The data i/o for sending serial commands to the LCD 2 wire interface

//#define TEST_MODE 0 // A pin used for testing

/* Constants */
// Determines the trip point for the microphone to detect a clap to start the talking process
const unsigned int clap_sensitivity = 400;

// Determines MP3 volume (0-30)
const short int volume = 15;

//LCD Constants
const int lcd_brightness = 2;

// The number of milliseconds in the LCD button configuration debounce interval
const unsigned short debounce_delay = 200;

// The resolution of our clock/Timer ISR. This is a number in microseconds.
// This does not neccessarily have to match the LCD update rate
const unsigned long time_resolution = 500000;

/* Global Variables */

// LCD Global Variables for ISR

// Bytes to display on the LCD, 0 - Hour digit 2, 1 - Hour digit 1, 2 - Min digit 2, 3 - Min digit 1
int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};

// Update is a flag that alerts the rest of the system when the ISR has calculated a new value and may need to update the LCD
bool Update = false;
// Enables the colon in the LCD
unsigned char ClockPoint = 1;
// Handles the ISR memory of time, these values may be updated at any point in the program
unsigned char subsecond = 0;
unsigned char second = 0;
unsigned char minute = 0;
unsigned char hour = 12;

// Handles the memory of morning or afternoon, only used when english standard mode is selected
bool am = false;

// Swaps between military time and regular time
// 0 - English Military Time
// 1 - English Regular Time
// 2 - Mandarin
int mode = 1;
int8_t modeDisp[] = {0x0,0x1,0x2,0x3};

unsigned long int noise = 0;
unsigned int loudest = 0;
unsigned long int count = 0;
unsigned long int maxnoise = 0;

// Create the LED display object
TM1637Display tm1637(lcd_clk,lcd_dio);

// Create the MP3 playing object
DFRobotDFPlayerMini myMP3;

// Declare a software UART over the MP3 device pins
SoftwareSerial mp3_serial(mp3_rx, mp3_tx);
//AltSoftSerial mp3_serial;

void setup() {
  pinMode(microphone,INPUT);
  pinMode(lcd_config_hr,INPUT);
  pinMode(lcd_config_min,INPUT);
  pinMode(lcd_config_mode,INPUT);
  pinMode(lcd_clk,OUTPUT);
  pinMode(lcd_dio,OUTPUT);

  // Attach external interrupts to change Minute/Hour
  attachInterrupt(digitalPinToInterrupt(lcd_config_hr), changeHourISR, RISING);
  attachInterrupt(digitalPinToInterrupt(lcd_config_min), changeMinISR, RISING);
  
  // Sets initial brightness, data, address)
  tm1637.setBrightness(lcd_brightness);
  //tm1637.init();
  //tm1637.point(POINT_ON);
  Timer1.initialize(time_resolution); // Set the ISR frequency
  Timer1.attachInterrupt(TimingISR);//declare the interrupt serve routine:TimingISR  
  count = 0;
  Serial.begin(9600);
  mp3_serial.begin(9600);
  myMP3.begin(mp3_serial);
  myMP3.volume(volume);
  myMP3.EQ(DFPLAYER_EQ_NORMAL);
  // For testing
  //#if defined(TEST_MODE)
    //Serial.begin(9600);
  //#endif
  
  
}

void loop() {

  // At the top of the loop, update the LCD if the subseconds have changed
  // If a valid input is detected, the LCD will not update, but the time will continue counting, so subseconds will not be lost
  if(Update)
  {
    TimeUpdate();
    //tm1637.display(TimeDisp);
    tm1637.showNumberDec(TimeDisp[0],false,1,4);
    tm1637.showNumberDec(TimeDisp[3],false,1,3);
    tm1637.showNumberDec(TimeDisp[2],false,1,2);
    tm1637.showNumberDec(TimeDisp[1],false,1,1);
    #if defined(TEST_MODE)
      Serial.print(String(TimeDisp[0]));
      Serial.print(String(TimeDisp[1]));
      Serial.print(':');
      Serial.print(String(TimeDisp[2]));
      Serial.println(String(TimeDisp[3]));
    #endif
  }
  
  // We don't have enough interrupts to include the Mode button as an interrupt, but changeMode can act as a fake interrupt
  if (digitalRead(lcd_config_mode) == 1) changeMode();
  
  //Code for the input device
  noise = analogRead(microphone);
  if (noise > maxnoise)
  {
    maxnoise = noise;
    Serial.println(maxnoise);
  }
  //Serial.println(analogRead(microphone));
  if (noise > clap_sensitivity)
  {
    Serial.println("Noise > 1000");
    delay(13);
    noise = analogRead(microphone);
    while ((noise < clap_sensitivity) && (count < 10000))
    {
      count++;
      delayMicroseconds(50);
      noise = analogRead(microphone);
      if (noise >= clap_sensitivity) 
      {
        Serial.println("Clap Trig");
        voicetime(myMP3, TimeDisp, am, mode, volume);
        delay(150);
      }
    }
  }
    //Serial.println(noise/count);
    noise = 0;
    count = 0;
  //if (noise > loudest) loudest=noise;
  //Serial.println(loudest);

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
        if (mode == 0)
        {
          if(hour == 24) hour = 0;
        }
        else 
        {
          if(hour == 12)
          {
            am = !am;
          }
          else if (hour == 13) hour = 1;
        };
        minute = 0;
      }
      second = 0;
    }
    subsecond = 0;  
  }
  
}

// Update the LCD with the time stored in the variables
void TimeUpdate(void)
{

  TimeDisp[0] = hour / 10;
  TimeDisp[1] = hour % 10;
  TimeDisp[2] = minute / 10;
  TimeDisp[3] = minute % 10;
  Update = false;
}

// Updates the minute by one every time the minute button is pushed
void changeMinISR()
{
  static unsigned long last_interrupt_time = 0;
  if (millis() - last_interrupt_time > debounce_delay){
    minute ++;
    if(minute == 60) minute = 0;
    // Wait for the button to be depressed
    while(digitalRead(lcd_config_min) == 1);
  }

  last_interrupt_time = millis();
}

// Updates the hour by one every time the hour button is pushed
void changeHourISR()
{
  static unsigned long last_interrupt_time = 0;
  if (millis() - last_interrupt_time > debounce_delay)
  {
    //while(digitalRead(lcd_config_min) == 1)
    //{
    hour ++;
    if (mode == 0)
    {
      if(hour == 24) hour = 0;
    }
    else 
    {
      if(hour == 12)
      {
        am = !am;
      }
      else if (hour == 13) hour = 1;
    };
    //delay(500);
    //};
    // Wait for the button to be depressed
    while(digitalRead(lcd_config_hr) == 1);
  }

  last_interrupt_time = millis();
}

// Changes the mode by one every time the button is pushed
void changeMode()
{
  static unsigned long last_interrupt_time = 0;
  if (millis() - last_interrupt_time > debounce_delay)
  {
    if (mode == 0) mode = 2;
    else mode--;
    
    //if (mode == 0) tm1637.display(modeDisp);
    //else if (mode == 1) tm1637.display(modeDisp);
    //else tm1637.display(modeDisp);
    
    // Wait for the button to be depressed
    while(digitalRead(lcd_config_mode) == 1);
  }

  last_interrupt_time = millis();
}


