//Function for mp3
//Uses TimeDisp to play sound byte


#include <SoftwareSerial.h>       // Library to support DFPlayer
#include <DFRobotDFPlayerMini.h>  // Library for DFPlayer (module to play MP3 files)
#include <TM1637Display.h>        // Library for TM1637 Display (4 digitis x 7 segments)
#include <Time.h>                 // Time Library
#include <TimeLib.h>
#include <EEPROM.h>               // Library to read/write on EEPROM


SoftwareSerial mySoftwareSerial(3, 4);    // RX, TX
DFRobotDFPlayerMini myMP3;

void setup()
{
  Serial.begin(9600);
  Serial.println("--- Start Serial Monitor SEND_RCVE ---");
  Serial.println(" Type in Box above, . ");
  Serial.println("(Decimal)(Hex)(Character)");
  Serial.println();

  mySoftwareSerial.begin(9600);       // Set serial for DFPlayer-Minutei mp3 module
  myMP3.begin(mySoftwareSerial);      // Begin MP3

  myMP3.volume (20);                  // Set Volume (0-30max)

  myMP3.EQ(DFPLAYER_EQ_ROCK);         // Set EQ
  // myMP3.EQ(DFPLAYER_EQ_NORMAL);
  // myMP3.EQ(DFPLAYER_EQ_POP);
  // myMP3.EQ(DFPLAYER_EQ_JAZZ);
  // myMP3.EQ(DFPLAYER_EQ_CLASSIC);
  // myMP3.EQ(DFPLAYER_EQ_BASS);
}



<<<<<<< HEAD
int8_t TimeDisp[] = {0x00, 0x02, 0x00, 0x00};             ///// delete later when TimeDisp is function input
int mode = 0;                                             ///// delete later when mode is function input
=======
int8_t TimeDisp[] = {0x00, 0x00, 0x00, 0x00};             ///// delete later when TimeDisp is function input
int mode = 2;                                             ///// delete later when mode is function input
>>>>>>> 5dc3111ca4d15fcc95d2a98a1e3a410d4717f89c
int Hour = (TimeDisp[0] * 10 + TimeDisp[1]);              // Calculate hour value
int Minute = (TimeDisp[2] * 10 + TimeDisp[3]);         // Calculate Minute value

///////////////////////////////////////
/////function for speaking out the time////////////////
//////////////////////////////////////
void loop() //Uses Hour(0 to 24), Minute(0 to 59), and mode(0 to 2) as inputs. 
//mode 0 = military time, mode 1 = standard 12-hour time (english), mode 2 = mandarin time
{
////////////////////
//// used to cycle through mode/hour/minutes in testing
//  Mode = (Mode + 1) % 3;                    // cycle through modes (3 modes so far)
// Hour = (Hour + 1) % 24;                   // cycle through hours (24 hours )
<<<<<<< HEAD
  Minute = (Minute + 5) % 60;             // cycle through minutes for testing in 10s intervals
=======
//  Minute = (Minute + 5) % 60;             // cycle through minutes for testing in 10s intervals
>>>>>>> 5dc3111ca4d15fcc95d2a98a1e3a410d4717f89c
////////////////////

////////////////////////////////////////////


///////////// 24 Hour time (military time)//////////////////
  if (mode == 0)                                  
  {
  /// Hours///
    if (Hour/10 == 0 && Hour%10 == 0)                     // Case where Hour is '00'. Want 'zero zero' to be read.
    {                                                     /////////////////////////
      myMP3.playMp3Folder(0);
      delay(1000);
          if(Minute/10 == 0 && Minute%10 == 0)            // special case where minutes is not '00', then we want to say hours as '00'. if hours = '00' and minutes = '00' we want it to read as 'zero hundred hours'.
          {
          }
          else
          {
              myMP3.playMp3Folder(0);
              delay(1000);
          }
    }
    else if (Hour/10 == 0 && Hour%10 != 0)               // Case where Hour is '0X'. Want 'zero X' to be read
    {
      myMP3.playMp3Folder(0);
      delay(1000);
      myMP3.playMp3Folder(Hour%10);
      delay(1000);
    }
    else                                                  // standard case: read hour as is (10 to 23)
    {
      myMP3.playMp3Folder(Hour);
      delay(1000);
    }
    

  /// Minutes///
    if (Minute/10 == 0 && Minute%10 == 0)       // case where Minute is '00'; we want it to read 'hundred'
    {
      myMP3.playMp3Folder(103);           
      delay(1000);
    }
    else if (Minute/10 == 0)                    // case where Minute is '0X'; we want it to read 'zero' and a number '1 through 9'
    {
      myMP3.playMp3Folder(Minute/10);
      delay(1000);
      myMP3.playMp3Folder(Minute%10);
      delay(1000);
    }
    else                                        // if the special cases don't apply, read Minute values normally.
    {
      myMP3.playMp3Folder(Minute);
      delay(1000);
    }
    myMP3.playMp3Folder(104);                   //say 'HOURS' sound clip at the end every time
    delay(1000);
  }
             
    
///////////// AM/PM //////////////////////////
  else if (mode == 1)
  {
    if (Hour < 12)                                   ///////////////// AM HOURS
    {
      if (Hour == 0)                                 // Case where Hour = 0, time should be read as 'twelve'
      {
        myMP3.playMp3Folder(Hour+12);
        delay(1000);
      }     
      else 
      {
        myMP3.playMp3Folder(Hour);                  // all other AM hours are read normally.
        delay(1000);
      }
    }        
    else                                            ////////PM HOURS
    {
        if (Hour == 12)
        {
          myMP3.playMp3Folder(Hour);
          delay(1000);                                // case when hour > 12, we just subtract 12 and read out the new hour value to get the PM converted time
        }
        else
        {
          myMP3.playMp3Folder(Hour-12);               // PM time: subtract 12 and read.
          delay(1000);
        }
    }

      ///////////////SAY Minutes///////////////
    if (Minute >= 10)                                 // read minutes as is
    {
      myMP3.playMp3Folder(Minute);
      delay(1000);
    }
    
    else if (Minute < 10 && Minute != 0)
    {
      myMP3.playMp3Folder(102);                     //say 'oh' instead of 'zero' for Minute '0X'
      delay(1000);
      myMP3.playMp3Folder(Minute);
      delay(1000);
    }
    
    else                                             //if minute is '00'  say nothing or say 'o clock'
    {
    }

  ////////////////SAY AM/PM////////////////////////
    if (Hour < 12 )
    {
      myMP3.playMp3Folder(100);
      delay(1000);
    }
    else
    {
      myMP3.playMp3Folder(101);
      delay(1000);
    }
  }
  
  
  
  
  ///////////////////MANDARIN LANGUAGE /////////////////////////
  //standard way of telling time in mandarin is to say:
  //  'time of day'   'Hour #'    'Hours'     'Minute #'    'Minutes'
 ///time of day///
  else if (mode == 2)
  {
    if(Hour >= 0 && Hour < 3)
    {
      myMP3.playMp3Folder(1020);
      delay(1000);
    }
    else if (Hour >= 3 && Hour < 6)
    {
      myMP3.playMp3Folder(1021);
      delay(1000);
    }
    else if (Hour >= 6 && Hour  < 9)
    {
      myMP3.playMp3Folder(1022);
      delay(1000);
    }
    else if (Hour >= 9 && Hour  < 12)
    {
      myMP3.playMp3Folder(1023);
      delay(1000);
    }
    else if (Hour >= 12 && Hour  < 15)
    {
      myMP3.playMp3Folder(1024);
      delay(1000);
    }
    else if (Hour >= 15 && Hour  < 18)
    {
      myMP3.playMp3Folder(1025);
      delay(1000);
    }
    else if (Hour >= 18 && Hour  < 21)
    {
      myMP3.playMp3Folder(1026);
      delay(1000);
    }
    else if (Hour >= 21 && Hour  < 24)
    {
      myMP3.playMp3Folder(1027);
      delay(1000);
    }
    
////hour///
    if (Hour == 0 || Hour == 12)                                  //say 12 for 0 hour and 12 hour
    {
      myMP3.playMp3Folder(1010);          
      delay(700);
      myMP3.playMp3Folder(1002);          
      delay(700);
    }
    else if (Hour == 2 || Hour == 14) //say liang                 //case for 2's.  Theres two ways to say 2 in mandarin
    {
      myMP3.playMp3Folder(1013);          
      delay(700);
    }
    else if (Hour == 1 || (Hour >= 3 && Hour <= 10))              //hours betwen 1-9 (not 2) read normally
    {
      myMP3.playMp3Folder(Hour+1000);         
      delay(700);
    }
    else if (Hour == 11 || Hour == 23)                            // say 11
    {
      myMP3.playMp3Folder(1010);         
      delay(700);
      myMP3.playMp3Folder(1001);         
      delay(700);
    }
    else
    {
      myMP3.playMp3Folder(Hour-12+1000);                          //say hours-12 to convert to standard
      delay(700);
    }
    myMP3.playMp3Folder(1011);                                    // say hours 'dian'     
    delay(1000);
    
    
///Minute///
  
    if (Minute/10 == 0)                     //if minute 10's digit is '0' i.e. all '0X' cases
    {
        if(Minute%10 == 0)                  // say nothing if '00'
        {
        }
        else                                  //say '0X' i.e. 'ling #' 
        {
            myMP3.playMp3Folder(1000);              //say 0
            delay(700);
            myMP3.playMp3Folder(Minute+1000);              //say #
            delay(700);
        }
    }

    else if (Minute/10 == 1)                               //all teens cases '1X'
    {
        myMP3.playMp3Folder(10+1000);                     //say 10 always
        delay(700);
        if (Minute%10 == 0)                                // if minute = 10, say only 10 and nothing else
        {
        }
        else 
        {
            myMP3.playMp3Folder(Minute%10+1000);         //say minute 1's digit 
            delay(700);
        }
    }
      
    else 
    {
        myMP3.playMp3Folder(Minute/10+1000);              //say minute 10's digit always
        delay(700);
        myMP3.playMp3Folder(10+1000);                     //say 10 always
        delay(700);
        
        if (Minute%10 == 0)                               //say nothing if minute 1's digit ends in 0
        {
        }
        else
        {
            myMP3.playMp3Folder(Minute%10+1000);              // say minute 1's digit if its not 0
            delay(700);
        }
    }

    //condition for saying 'fen' 'minute'
    if (Minute/10 != 0 || Minute%10 != 0)
    {
        myMP3.playMp3Folder(1012);                                    // say minutes 'fen'     
        delay(1000);
    }

          
  }

}
      
      
      
      
