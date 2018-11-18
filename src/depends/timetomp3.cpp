#include <timetomp3.h>
#include <DFRobotDFPlayerMini.h>  // Library for DFPlayer (module to play MP3 files)

void TimeToVoice(DFRobotDFPlayerMini myMP3, uint8_t hour, uint8_t minute, uint8_t mode, uint8_t volume)
  {
  myMP3.volume(volume);

///////////// 24 hour time (military time)//////////////////
  if (mode == 0)                                  
  {
  /// hours///
    if (hour / 10 == 0 && hour % 10 == 0)                     // Case where hour is '00'. Want 'zero zero' to be read.
    {                                                    
      myMP3.playMp3Folder(0);
      
      if((minute / 10 != 0) && (minute % 10 != 0))        // special case where minutes is not '00', then we want to say hours as '00'. if hours = '00' and minutes = '00' we want it to read as 'zero hundred hours'.
      {
        delay(1000);
        myMP3.playMp3Folder(0);
      }

    }
    else if (hour/10 == 0 && hour%10 != 0)               // Case where hour is '0X'. Want 'zero X' to be read
    {
      myMP3.playMp3Folder(0);
      delay(1000);
      myMP3.playMp3Folder(hour%10);
    }
    else                                                  // standard case: read hour as is (10 to 23)
    {
      myMP3.playMp3Folder(hour);
    }

    // Wait for hours vocals to be read
    delay(1000);


  /// minutes///
    if (minute/10 == 0 && minute%10 == 0)       // case where minute is '00'; we want it to read 'hundred'
    {
      myMP3.playMp3Folder(103);
    }
    else if (minute/10 == 0)                    // case where minute is '0X'; we want it to read 'zero' and a number '1 through 9'
    {
      myMP3.playMp3Folder(minute/10);
      delay(1000);
      myMP3.playMp3Folder(minute%10);
    }
    else                                        // if the special cases don't apply, read minute values normally.
    {
      myMP3.playMp3Folder(minute);
    }
    delay(1000);
    myMP3.playMp3Folder(104);                   //say 'HOURS' sound clip at the end every time
    delay(1000);
  }
             
    
///////////// AM/PM //////////////////////////
  else if (mode == 1)
  {
    bool am = (hour < 12) ? true : false;
    if (hour == 0)
    {
      hour = 12;
    }
    else if (hour >= 13)
    {
      hour -= 12;      
    }

    ///////////////SAY hours///////////////
    myMP3.playMp3Folder(hour);
    delay(1000);

    ///////////////SAY minutes///////////////
    if (minute >= 10)                                 // read minutes as is
    {
      myMP3.playMp3Folder(minute);
    }
    else if (minute < 10 && minute != 0)
    {
      myMP3.playMp3Folder(102);                     //say 'oh' instead of 'zero' for minute '0X'
      delay(1000);
      myMP3.playMp3Folder(minute);
    }
    // Delay for minutes to be read
    delay(1000);

    //////////////SAY AM/PM///////////////////
    am ? myMP3.playMp3Folder(100) : myMP3.playMp3Folder(101);
    delay(1000);
  }
  
  ///////////////////MANDARIN LANGUAGE /////////////////////////
  //standard way of telling time in mandarin is to say:
  //  'time of day'   'hour #'    'hours'     'minute #'    'minutes'
 ///time of day///
  else if (mode == 2)
  {
    // Time of Day
    if(hour >= 0 && hour < 3)          myMP3.playMp3Folder(1020);
    else if (hour >= 3 && hour < 6)    myMP3.playMp3Folder(1021);
    else if (hour >= 6 && hour  < 9)   myMP3.playMp3Folder(1022);
    else if (hour >= 9 && hour  < 12)  myMP3.playMp3Folder(1023);
    else if (hour >= 12 && hour  < 15) myMP3.playMp3Folder(1024);
    else if (hour >= 15 && hour  < 18) myMP3.playMp3Folder(1025);
    else if (hour >= 18 && hour  < 21) myMP3.playMp3Folder(1026);
    else if (hour >= 21 && hour  < 24) myMP3.playMp3Folder(1027);

    // Delay for time of day
    delay(1000);
    
    // hour #
    if (hour == 0 || hour == 12)                                  //say 12 for 0 hour and 12 hour
    {
      myMP3.playMp3Folder(1010);          
      delay(700);
      myMP3.playMp3Folder(1002);
    }
    else if (hour == 2 || hour == 14) myMP3.playMp3Folder(1013); //case for 2's.  Theres two ways to say 2 in mandarin
    else if (hour == 1 || (hour >= 3 && hour <= 10)) myMP3.playMp3Folder(hour+1000); //hours betwen 1-9 (not 2) read normally         
    else if (hour == 11 || hour == 23)                            // say 11
    {
      myMP3.playMp3Folder(1010);         
      delay(700);
      myMP3.playMp3Folder(1001);         
    }
    else myMP3.playMp3Folder(hour-12+1000);                       //say hours-12 to convert to standard
    delay(700);
    myMP3.playMp3Folder(1011);                                    // say hours 'dian'     
    delay(1000);
    
    
    ///minute///
  
    if (minute/10 == 0)                     //if minute 10's digit is '0' i.e. all '0X' cases
    {
        if(minute % 10 != 0)
        //say '0X' i.e. 'ling #' 
        {
            myMP3.playMp3Folder(1000);              //say 0
            delay(700);
            myMP3.playMp3Folder(minute+1000);              //say #
            delay(700);
        }
    }

    else if (minute/10 == 1)                               //all teens cases '1X'
    {
        myMP3.playMp3Folder(10+1000);                     //say 10 always
        delay(700);
        if (minute % 10 != 0)                                // if minute = 10, say only 10 and nothing else
        {
            myMP3.playMp3Folder(minute%10+1000);         //say minute 1's digit 
            delay(700);
        }
    }
      
    else 
    {
        myMP3.playMp3Folder(minute/10+1000);              //say minute 10's digit always
        delay(700);
        myMP3.playMp3Folder(10+1000);                     //say 10 always
        delay(700);
        
        if (minute % 10 != 0)                               //say nothing if minute 1's digit ends in 0
        {
            myMP3.playMp3Folder(minute%10+1000);              // say minute 1's digit if its not 0
            delay(700);
        }
    }

    //condition for saying 'fen' 'minute'
    if (minute/10 != 0 || minute%10 != 0)
    {
        myMP3.playMp3Folder(1012);                                    // say minutes 'fen'     
        delay(1000);
    }

          
  }    



  }