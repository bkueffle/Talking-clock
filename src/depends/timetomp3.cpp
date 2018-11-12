#include <timetomp3.h>
#include <DFRobotDFPlayerMini.h>  // Library for DFPlayer (module to play MP3 files)

void voicetime(DFRobotDFPlayerMini myMP3, int8_t time[], bool am, int mode, int volume)
  {
  unsigned int Hour = (time[0] * 10 + time[1]);    // Calculate hour value
  unsigned int Minute = (time[2] * 10 + time[3]);  // Calculate Minute value
  myMP3.volume(volume);

///////////// 24 Hour time (military time)//////////////////
  if (mode == 0)                                  
  {
  /// Hours///
    if (Hour/10 == 0 && Hour%10 == 0)                     // Case where Hour is '00'. Want 'zero zero' to be read.
    {                                                    
      myMP3.playMp3Folder(0);
      
      if((Minute / 10 != 0) && (Minute % 10 != 0))        // special case where minutes is not '00', then we want to say hours as '00'. if hours = '00' and minutes = '00' we want it to read as 'zero hundred hours'.
      {
        delay(1000);
        myMP3.playMp3Folder(0);
      }

    }
    else if (Hour/10 == 0 && Hour%10 != 0)               // Case where Hour is '0X'. Want 'zero X' to be read
    {
      myMP3.playMp3Folder(0);
      delay(1000);
      myMP3.playMp3Folder(Hour%10);
    }
    else                                                  // standard case: read hour as is (10 to 23)
    {
      myMP3.playMp3Folder(Hour);
    }

    // Wait for hours vocals to be read
    delay(1000);


  /// Minutes///
    if (Minute/10 == 0 && Minute%10 == 0)       // case where Minute is '00'; we want it to read 'hundred'
    {
      myMP3.playMp3Folder(103);
    }
    else if (Minute/10 == 0)                    // case where Minute is '0X'; we want it to read 'zero' and a number '1 through 9'
    {
      myMP3.playMp3Folder(Minute/10);
      delay(1000);
      myMP3.playMp3Folder(Minute%10);
    }
    else                                        // if the special cases don't apply, read Minute values normally.
    {
      myMP3.playMp3Folder(Minute);
    }
    delay(1000);
    myMP3.playMp3Folder(104);                   //say 'HOURS' sound clip at the end every time
    delay(1000);
  }
             
    
///////////// AM/PM //////////////////////////
  else if (mode == 1)
  {

    ///////////////SAY Hours///////////////
    myMP3.playMp3Folder(Hour);
    delay(1000);

    ///////////////SAY Minutes///////////////
    if (Minute >= 10)                                 // read minutes as is
    {
      myMP3.playMp3Folder(Minute);
    }
    else if (Minute < 10 && Minute != 0)
    {
      myMP3.playMp3Folder(102);                     //say 'oh' instead of 'zero' for Minute '0X'
      delay(1000);
      myMP3.playMp3Folder(Minute);
    }
    // Delay for minutes to be read
    delay(1000);

    //////////////SAY AM/PM///////////////////
    am ? myMP3.playMp3Folder(100) : myMP3.playMp3Folder(101);
    delay(1000);
  }
  
  ///////////////////MANDARIN LANGUAGE /////////////////////////
  //standard way of telling time in mandarin is to say:
  //  'time of day'   'Hour #'    'Hours'     'Minute #'    'Minutes'
 ///time of day///
  else if (mode == 2)
  {
    // Time of Day
    if(Hour >= 0 && Hour < 3)          myMP3.playMp3Folder(1020);
    else if (Hour >= 3 && Hour < 6)    myMP3.playMp3Folder(1021);
    else if (Hour >= 6 && Hour  < 9)   myMP3.playMp3Folder(1022);
    else if (Hour >= 9 && Hour  < 12)  myMP3.playMp3Folder(1023);
    else if (Hour >= 12 && Hour  < 15) myMP3.playMp3Folder(1024);
    else if (Hour >= 15 && Hour  < 18) myMP3.playMp3Folder(1025);
    else if (Hour >= 18 && Hour  < 21) myMP3.playMp3Folder(1026);
    else if (Hour >= 21 && Hour  < 24) myMP3.playMp3Folder(1027);

    // Delay for time of day
    delay(1000);
    
    // Hour #
    if (Hour == 0 || Hour == 12)                                  //say 12 for 0 hour and 12 hour
    {
      myMP3.playMp3Folder(1010);          
      delay(700);
      myMP3.playMp3Folder(1002);
    }
    else if (Hour == 2 || Hour == 14) myMP3.playMp3Folder(1013); //case for 2's.  Theres two ways to say 2 in mandarin
    else if (Hour == 1 || (Hour >= 3 && Hour <= 10)) myMP3.playMp3Folder(Hour+1000); //hours betwen 1-9 (not 2) read normally         
    else if (Hour == 11 || Hour == 23)                            // say 11
    {
      myMP3.playMp3Folder(1010);         
      delay(700);
      myMP3.playMp3Folder(1001);         
    }
    else myMP3.playMp3Folder(Hour-12+1000);                       //say hours-12 to convert to standard
    delay(700);
    myMP3.playMp3Folder(1011);                                    // say hours 'dian'     
    delay(1000);
    
    
    ///Minute///
  
    if (Minute/10 == 0)                     //if minute 10's digit is '0' i.e. all '0X' cases
    {
        if(Minute % 10 != 0)
        //say '0X' i.e. 'ling #' 
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
        if (Minute % 10 != 0)                                // if minute = 10, say only 10 and nothing else
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
        
        if (Minute % 10 != 0)                               //say nothing if minute 1's digit ends in 0
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