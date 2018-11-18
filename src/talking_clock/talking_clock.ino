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
// The MP3 player drivers should be installed by downloading DFRobotDFPlayerMini.h
// The depends folder should be added to the libraries path

// Includes pin definitions, libraries, and display functions
#include <talking_clock.h>

//#define TEST_MODE 0 // A pin used for testing

/* Global Variables */

// LCD Global Variables for ISR

// Bytes to display on the LCD, 0 - Hour digit 2, 1 - Hour digit 1, 2 - Min digit 2, 3 - Min digit 1
uint8_t time_disp[] = {0x00,0x00,0x00,0x00};

// The pointer to the time_disp data within the EEPROM
uint16_t time_addr = kTimeAddr;

// The number of updates that have occured. 
// After this rolls over, a new EEPROM address should be chosen so the EEPROM doesn't wear out
uint32_t update_count = 0;

// Update is a flag that alerts the rest of the system when the ISR has calculated a new value and may need to update the LCD
bool Update = false;

// Handles the ISR memory of time, these values may be updated at any point in the program
unsigned char subsecond = 0;
unsigned char second = 0;
unsigned char minute = 0;
unsigned char hour = 12;

// Swaps between military time and regular time
// 0 - English Military Time
// 1 - English Regular Time
// 2 - Mandarin
uint8_t mode = 1;
uint8_t modeDisp[] = {0x0,0x1,0x2,0x3};

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

void setup() {
  pinMode(microphone,INPUT);
  pinMode(lcd_config_hr,INPUT);
  pinMode(lcd_config_min,INPUT);
  pinMode(lcd_config_mode,INPUT);
  pinMode(lcd_clk,OUTPUT);
  pinMode(lcd_dio,OUTPUT);
  Serial.begin(9600);
  // Mode and time_disp Initialization Code
  // The EEPROM will be read in order to grab the last displayed time/mode before shutdown

  // Obtain the starting address of the last recorded time from address 0x00
  EEPROM.get(kTimePointerAddr, time_addr);

  // Obtain the mode from the mode address in the EEPROM
  mode = EEPROM.read(kModeAddr);

  // If the mode is out of bounds of the mode options, set to English Regular Time by default.
  if (mode > 2) mode = 1;

  // Go to that starting address and grab the last recorded time
  EEPROM.get(time_addr, hour);
  EEPROM.get(time_addr + sizeof(hour), minute);

  // If the hours or minutes that were read from the EEPROM are out of range, reassign them to some default values
  if (hour >= 24) hour = 12;
  if (minute >= 60) minute = 0;

  // Every time we reboot the device, a new address will be chosen to host the
  // time_disp data, this prevents the EEPROM from being destroyed too quickly

  // Protect Special Address 0, and mode address 1, if we're about to roll over, start
  // at address 0x02 instead of 0x00. Otherwise, increment address by one
  time_addr = ((time_addr >= kEepromSize - 1) || (time_addr < kTimeAddr)) ? kTimeAddr : time_addr + 1;

  // Burn the time into the new time location
  EEPROM.update(time_addr, hour);
  EEPROM.update(time_addr + sizeof(hour), minute);

  // Burn the time location into special pointer address 0x00.
  EEPROM.put(kTimePointerAddr, time_addr);
  
  // Sets initial brightness
  tm1637.setBrightness(kLCDBrightness);
  
  Timer1.initialize(kTimeResolution); // Set the ISR frequency
  Timer1.attachInterrupt(TimingISR);//declare the interrupt serve routine:TimingISR  
  count = 0;
  mp3_serial.begin(9600);
  myMP3.begin(mp3_serial);
  myMP3.volume(kVolume);
  myMP3.EQ(DFPLAYER_EQ_NORMAL);
  
  // Attach external interrupts to change Minute/Hour
  attachInterrupt(digitalPinToInterrupt(lcd_config_hr), changeHourISR, RISING);
  attachInterrupt(digitalPinToInterrupt(lcd_config_min), changeMinISR, RISING);

  #if defined(TEST_MODE)
    Serial.print("The EEPROM time address is: ");
    Serial.println(time_addr);
    Serial.print("The EEPROM mode is: ");
    Serial.println(mode);
    Serial.print("The EEPROM hour is: ");
    Serial.println(hour);
    Serial.print("The EEPROM minute is: ");
    Serial.println(minute);
  #endif
}

void loop() {

  // At the top of the loop, update the LED segments if the subseconds have changed
  // If a valid input is detected, the LED will not update, but the time will continue counting, so subseconds will not be lost
  if(Update)
  {
    // If Hours/Minutes have changed, erase and write to the EEPROM
    updateEepromTime();

    // Update display with current hour and minute
    TimeUpdate();
    
    #if defined(TEST_MODE)
      Serial.print(String(time_disp[0]));
      Serial.print(String(time_disp[1]));
      Serial.print(':');
      Serial.print(String(time_disp[2]));
      Serial.println(String(time_disp[3]));
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
  if (noise > kClapSense)
  {
    Serial.println("Noise > 1000");
    delay(13);
    noise = analogRead(microphone);
    while ((noise < kClapSense) && (count < 10000))
    {
      count++;
      delayMicroseconds(50);
      noise = analogRead(microphone);
      if (noise >= kClapSense) 
      {
        Serial.println("Clap Trig");
        TimeToVoice(myMP3, hour, minute, mode, kVolume);
        delay(150);
      }
    }
  }
  noise = 0;
  count = 0;

}

// The interrupt service routine for keeping time. This does not update the LCD by itself.
void TimingISR()
{
  // During every interrupt, increment the subsecond, this is the smallest resolution of our clock
  subsecond ++;
  Update = true;
  if(subsecond == 1000000/kTimeResolution){
    second ++;
    if(second == 60)
    {
      minute ++;
      if(minute == 60)
      {
        hour ++;
        if(hour == 24) hour = 0;
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
  if (mode == 1)
  {
    if (hour == 0)
    {
      time_disp[0] = 1;
      time_disp[1] = 2;
    }
    else if (hour >= 13)
    {
      time_disp[0] = (hour - 12) / 10;
      time_disp[1] = (hour - 12) % 10;         
    }
    else
    {
      time_disp[0] = hour / 10;
      time_disp[1] = hour % 10;        
    }
  }
  else
  {

    time_disp[0] = hour / 10;
    time_disp[1] = hour % 10;    
  }
  time_disp[2] = minute / 10;
  time_disp[3] = minute % 10;

  // Encode the time display digits in the proper format for LED display
  for (uint8_t i=0; i<4; i++)
  {
    time_disp[i] = tm1637.encodeDigit(time_disp[i]);
  }
  //Pack on a colon
  time_disp[1] |= 0x80;
  tm1637.setSegments(time_disp,4,0);
  Update = false;
}

// Updates the minute by one every time the minute button is pushed
void changeMinISR()
{
  static unsigned long last_interrupt_time = 0;
  if (millis() - last_interrupt_time > kDebounceDelay){
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
  if (millis() - last_interrupt_time > kDebounceDelay)
  {
    hour ++;
    if (hour >= 24) hour = 0;
    // Wait for the button to be depressed
    while(digitalRead(lcd_config_hr) == 1);
  }

  last_interrupt_time = millis();
}

// Changes the mode by one every time the button is pushed
void changeMode()
{
  static unsigned long last_interrupt_time = 0;
  if (millis() - last_interrupt_time > kDebounceDelay)
  {
    // Switches the mode every time the button is pushed
    mode = (mode == 0) ? 2 : mode - 1;

    // Displays the name of the mode that was entered to the 7 Segment Display
    ModeToDisplay(&tm1637, mode);
    
    // Write the new mode to the EEPROM mode address
    EEPROM.write(kModeAddr, mode);

    // If hours have changed, erase and write to the EEPROM
    updateEepromTime();

    // Wait for the button to be depressed
    while(digitalRead(lcd_config_mode) == 1);

    // Update display with current hour now that we've changed modes
    TimeUpdate();
  }

  last_interrupt_time = millis();
}

// Time (hour, minute) are written to the EEPROM as permanent memory when this function is called
// Update Count is an incrementing counter that counts to kStAddrUpdateTime seconds
// After that time, the pointer to the Time (hour, minute) will increment by one to prevent EEPROM degredation
void updateEepromTime()
{
  // Enter this statement every update_count seconds
  if ((kTimeResolution * update_count++) / 1000 == kStAddrUpdateTime)
  {
    // The starting address must be incremented every so often so that no particular byte of the EEPROM
    // degrades to the point of value
    // Protect Special Address 0, if we're about to roll over, start at address 0x01
    // instead of 0x00. Otherwise, increment address by one
    time_addr = (time_addr >= kEepromSize - 1) ? kTimeAddr : time_addr + 1;

    // Write the pointer to the time_disp data into special address 0x00
    EEPROM.put(kTimePointerAddr, time_addr);
    update_count = 0;
  }
  // If the hours or minutes in the EEPROM memory do not match our current
  // hours/minutes, burn the new hours/minutes, otherwise no write/erase will occur
  EEPROM.update(time_addr, hour);
  EEPROM.update(time_addr + sizeof(hour), minute);
}


