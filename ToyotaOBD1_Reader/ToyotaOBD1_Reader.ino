// ToyotaOBD1_Reader by talofer99
// In order to read the data from the OBD connector, short E1 + TE2. then to read the data connect to VF1.
// Note the data line output is 12V - connecting it directly to one of the arduino pins might damage (proabably) the board
// Ignore top comment - works just fine without optoisolator, see last note...
// This is made for diaply with an OLED display using the U8glib - which allow wide range of display types with minor adjusments.
// Many thanks to GadgetFreak for the greate base code for the reasding of the data.
// If you want to use invert line - note the comments on the MY_HIGH and the INPUT_PULLUP in the SETUP void.
// As of 5-9-2023, this does NOT work on the 1993 Camry while the engine is running. Not sure why???
// 5-12-2023 UPDATE: Electromagnetic interference from spark plugs, alternator, etc. may most likely be the culprit. This is why we cannot
// get constant data streams from the ECU while the alternator is running, the spark plugs are sparking, etc. Have you ever wondered why your
// ECU is in a metal box?????
// Still trying to figure out an isolation circuit, but for now, I believe the problem is solved...
// 5-15: Optocoupler beta circuit is complete, utilizes 4N25, 1 LED, and 2 1K resistors. Beta testing will commence soon...


//Very crucial to include the U8g library or else you won't have any output!!! Wire.h is included within U8g
#include "U8glib.h"

// for debug option - swith output to Serial
#define DEBUG_OUTPUT true
#define ENGINE_DATA_PIN 2 // pin 2 to VF1 - see fritzing/schematic file
#define ENGINE_DATA_INT 0  // for attachInterrupt
#define LED_PIN          13

// I have inverted the Eng line using an Opto-Coupler, if yours isn't then reverse these low & high defines.
#define  MY_HIGH  HIGH //LOW
#define  MY_LOW   LOW //HIGH

#define  TOYOTA_MAX_BYTES  24
volatile uint8_t ToyotaNumBytes, ToyotaID;
volatile uint8_t ToyotaData[TOYOTA_MAX_BYTES];
volatile uint16_t ToyotaFailBit = 0;


// "names" for the OND data to make life easier
#define OBD_INJ 1 //Injector pulse width (INJ)
#define OBD_IGN 2 //Ignition timing angle (IGN)
#define OBD_IAC 3 //Idle Air Control (IAC)
#define OBD_RPM 4 //Engine speed (RPM)
#define OBD_MAP 5 //Manifold Absolute Pressure (MAP)
#define OBD_ECT 6 //Engine Coolant Temperature (ECT)
#define OBD_TPS 7 //Throttle Position Sensor (TPS)
#define OBD_SPD 8 //Speed (SPD)

// dfeine connection flag and last success packet - for lost connection function.
boolean OBDConnected;
unsigned long OBDLastSuccessPacket;

#define TOGGLE_BTN_PIN 3
#define TOGGLE_BTN_INT 1
int CurrentDisplayIDX;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

// VOID SETUP
void setup() {
  Serial.begin(9600);
  if (DEBUG_OUTPUT) {
    Serial.println("system Started");}
  
  // Display no connection
  displayNoConnection();
  
  // setup input and output pins
  pinMode(ENGINE_DATA_PIN, INPUT); // _PULLUP
  pinMode(LED_PIN, OUTPUT);
  //setup Interrupt for data line
  attachInterrupt(ENGINE_DATA_INT, ChangeState, CHANGE);
  
  //setup button
  pinMode(TOGGLE_BTN_PIN, INPUT);
  attachInterrupt(TOGGLE_BTN_INT, ButtonChangeState, CHANGE);
  
  // Set OBD to not connected
  OBDConnected=false;
  CurrentDisplayIDX = 2; // set to display 2
} // END VOID SETUP


// VOID LOOP
void loop() {
  // if found bytes
  if (ToyotaNumBytes > 0)  {
    if (DEBUG_OUTPUT) { debugdataoutput();  }
     // draw screen
     drawScreenSelector();
     
     // set last success 
     OBDLastSuccessPacket = millis();
     // set connected to true
     OBDConnected = true;
     // reset the counter.
     ToyotaNumBytes = 0;
  } // end if (ToyotaNumBytes > 0)
  
  // if found FAILBIT and dbug
  if (ToyotaFailBit > 0 && DEBUG_OUTPUT )  { debugfaildataoutput(); } 
  
  //check for lost connection
  if (OBDLastSuccessPacket + 3500 < millis() && OBDConnected) {
    // show no connection
     displayNoConnection();
     // set OBDConnected to false.
     OBDConnected = false;
  } // end if loas conntcion
  
} // end void loop 


// VOID drawScreenSelector()
void drawScreenSelector() {
  if (CurrentDisplayIDX == 1){
     drawSpeedRpm();
   } else if (CurrentDisplayIDX == 2){
     drawAllData();
   } else if (CurrentDisplayIDX == 3){
    drawFlagsBinnary();  
   }
} // end drawScreenSelector() 



void drawFlagsBinnary() {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_osr18);

  // picture loop
  u8g.firstPage();  
   do {
      u8g.setPrintPos(0, 17) ;
      u8g.print( int(getOBDdata(11)),BIN);
      u8g.setPrintPos(0, 50) ;
      u8g.print( int(getOBDdata(12)),BIN);
  } while( u8g.nextPage() ); // end picture loop
} // end void


void drawAllData() {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
 
  // picture loop
  u8g.firstPage();  
   do {
           u8g.drawStr( 0, 17, "INJ" );
          u8g.setPrintPos(25, 17) ;
          u8g.print( getOBDdata(OBD_INJ));
          
          u8g.drawStr( 0, 32, "IGN");
          u8g.setPrintPos(25, 32) ;
          u8g.print( int(getOBDdata(OBD_IGN)));
          
          u8g.drawStr( 0, 47, "IAC");
          u8g.setPrintPos(25, 47) ;
          u8g.print( int(getOBDdata(OBD_IAC)));
          
          u8g.drawStr( 0, 62, "RPM");
          u8g.setPrintPos(25, 62) ;
          u8g.print( int(getOBDdata(OBD_RPM)));
          
          u8g.drawStr( 65, 17, "MAP" );
          u8g.setPrintPos(92, 17) ;
          u8g.print( int(getOBDdata(OBD_MAP)));
          
          u8g.drawStr( 65, 32, "ECT");
          u8g.setPrintPos(92, 32) ;
          u8g.print( int(getOBDdata(OBD_ECT)));
          
          u8g.drawStr( 65, 47, "TPS");
          u8g.setPrintPos(92, 47) ;
          u8g.print( int(getOBDdata(OBD_TPS)));
          
          u8g.drawStr( 65, 62, "SPD");
          u8g.setPrintPos(92, 62) ;
          u8g.print( int(getOBDdata(OBD_SPD)));
          
          u8g.drawVLine(63, 0, 64);
    } while( u8g.nextPage() ); // end picture loop
} // end void drawalldata




// VOID DRAWSPEEDRPM 
void drawSpeedRpm() {
 
    // define setytings for screen
   u8g.setFont(u8g_font_osb35n);
   u8g.setFontRefHeightExtendedText();
   u8g.setDefaultForegroundColor();
   u8g.setFontPosTop();                          
  
  // convert into to char so we can get the width
  char buf[4];
  itoa (int(getOBDdata(OBD_SPD)), buf, 10);

  // "calc" rpm bars to diaply - MAX 14 
  int rpmToDisplay = int (float(getOBDdata(OBD_RPM))/500);// calc the rpm bars    then check to add +1 if pased the +250
  if (double(double(getOBDdata(OBD_RPM))/500.0) - int(getOBDdata(OBD_RPM)/500) > 0.5) {
      rpmToDisplay += 1;
  } // end if */
   
   // picture loop
   u8g.firstPage();  
   do {
          // draw box 
           u8g.drawRFrame(20,4,90, 48,7);
           // print speed
           u8g.drawStr(int(108-u8g.getStrWidth(buf)),10, buf);
          
             // draw RPM
             for (int i=0; i<rpmToDisplay; i++) {
                    u8g.drawBox(20+((4+1)*i) + i, 55,4,8);
             } // end for 
     } while( u8g.nextPage() ); // end picture loop
       
} //end void drawSpeedRpm()

// NO CONNECTION VOID
void displayNoConnection() {
   u8g.setFont(u8g_font_courB12r);
   u8g.setFontRefHeightExtendedText();
   u8g.setDefaultForegroundColor();
   u8g.setFontPosTop();
   // picture loop
     u8g.firstPage();  
      do {
           // draw box 
           u8g.drawRFrame(0,0,127, 63,7);
           u8g.drawStr(15,25, "NO SIGNAL");
      } while( u8g.nextPage() );
      
} // end void



// GET DATA FROM OBD
float getOBDdata(int OBDdataIDX) {
 // define return value
  float returnValue;
  switch (OBDdataIDX) {
    case 0:// UNKNOWN
        returnValue = ToyotaData[0]; 
        break;
    case OBD_INJ: //  Injector pulse width (INJ) - in milisec
        returnValue = ToyotaData[OBD_INJ]/10;
        break;
    case OBD_IGN: // Ignition timing angle (IGN) - degree- BTDC
        returnValue = ToyotaData[OBD_IGN]-90;
        break;
    case OBD_IAC: //Idle Air Control (IAC) - Step # X = 125 = open 100%
        returnValue = ToyotaData[OBD_IAC]/125*100;
         break;
     case OBD_RPM: //Engine speed (RPM)
         returnValue = ToyotaData[OBD_RPM]*25;
         break; 
     case OBD_MAP: //Manifold Absolute Pressure (MAP) - kPa Abs
         returnValue = ((float)(ToyotaData[OBD_MAP])/6.895); //Remove /6.895 and (float) if it doesn't work, should convert to psi
          break;  
    case OBD_ECT: // Engine Coolant Temperature (ECT). In C, voltage conversion already factored in. Converted to F, hopefully...
          if (ToyotaData[OBD_ECT] >= 244)
            returnValue = ((float)(((ToyotaData[OBD_ECT] - 244) * 10.0) + 132.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 238)
            returnValue = ((float)(((ToyotaData[OBD_ECT] - 238) * 4.0) + 103.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 228)
            returnValue = ((float)(((ToyotaData[OBD_ECT] - 228) * 2.1) + 80.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 210)
            returnValue = ((float)(((ToyotaData[OBD_ECT] - 210) * 1.11) + 60.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 180)
            returnValue = ((float)(((ToyotaData[OBD_ECT] - 180) * 0.666) + 40.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 135)
            returnValue = ((float)(((ToyotaData[OBD_ECT] - 135) * 0.444) + 20.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 82)
            returnValue = ((float)(((ToyotaData[OBD_ECT] - 82) * 0.377) + 0.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 39)
            returnValue = ((float)((ToyotaData[OBD_ECT] - 39) * 0.465 - 20.0) * 1.8) + 32;
          else if (ToyotaData[OBD_ECT] >= 15)
            returnValue = (((float)ToyotaData[OBD_ECT] - 15) * 0.833 - 40.0) * 1.8 + 32;
          else
            returnValue = ((float)(((ToyotaData[OBD_ECT] * 2.0) - 70.0) * 1.8) + 32);
       
         break;  
    case OBD_TPS: // Throttle Position Sensor (TPS) - DEGREE
         returnValue = ToyotaData[OBD_TPS]/2;
         break;       
    case OBD_SPD: // Speed (SPD) - km/h
         returnValue = (ToyotaData[OBD_SPD]/1.609); //change back to just ToyotaData[OBD_SPD] if it doesn't work, should covert kmh to mph...
         break; 
    case 9:// UNKNOWN
        returnValue = ToyotaData[9]; 
        break;
    case 10:// UNKNOWN
        returnValue = ToyotaData[10]; 
        break;   
    case 11:// FLAG #1
        returnValue = ToyotaData[11]; 
        break;
    case 12:// FLAG # 2
        returnValue = ToyotaData[12]; 
        break;   
    default: // DEFAULT CASE (in no match to number)
      // send "error" value
      returnValue =  9999.99;
   } // end switch
  // send value back
  return returnValue;
} // end void getOBDdata



// VOID ButtonChangeState
void ButtonChangeState() {
  int buttonState = digitalRead(TOGGLE_BTN_PIN);
  // only on HIGH ((press) and OBDConnected = true
  if (buttonState &&OBDConnected ) {
    CurrentDisplayIDX +=1;
    if (CurrentDisplayIDX > 3) {
      CurrentDisplayIDX = 1;} 
      // all screen chnage
     drawScreenSelector();
  } // end if 
  
} // end void  ButtonChangeState() 



// VOID CHANGE 
void ChangeState()
{
  //Serial.print(digitalRead(ENGINE_DATA_PIN));
  static uint8_t ID, EData[TOYOTA_MAX_BYTES];
  static boolean InPacket = false;
  static unsigned long StartMS;
  static uint16_t BitCount;

  int state = digitalRead(ENGINE_DATA_PIN);
  digitalWrite(LED_PIN, state);
  
  if (InPacket == false)  {
    if (state == MY_HIGH)   {
      StartMS = millis();
     }   else   { // else  if (state == MY_HIGH)  
       if ((millis() - StartMS) > (15 * 8))   {
          StartMS = millis();
          InPacket = true;
          BitCount = 0;
      } // end if  ((millis() - StartMS) > (15 * 8))  
    } // end if  (state == MY_HIGH) 
  }  else   { // else  if (InPacket == false)  
    uint16_t bits = ((millis() - StartMS)+1 ) / 8;  // The +1 is to cope with slight time errors
    StartMS = millis();
    // process bits
    while (bits > 0)  {
      if (BitCount < 4)  {
        if (BitCount == 0)
          ID = 0;
        ID >>= 1;
        if (state == MY_LOW)  // inverse state as we are detecting the change!
          ID |= 0x08;
      }   else    { // else    if (BitCount < 4) 
        uint16_t bitpos = (BitCount - 4) % 11;
        uint16_t bytepos = (BitCount - 4) / 11;
        if (bitpos == 0)      {
 
          // Start bit, should be LOW
           if ((BitCount > 4) && (state != MY_HIGH))  { // inverse state as we are detecting the change!
               ToyotaFailBit = BitCount;
               InPacket = false;
               break;
            } // end if ((BitCount > 4) && (state != MY_HIGH)) 
 
        }  else if (bitpos < 9)  { //else TO  if (bitpos == 0)  
      
           EData[bytepos] >>= 1;
           if (state == MY_LOW)  // inverse state as we are detecting the change!
               EData[bytepos] |= 0x80;
        
       } else { // else if (bitpos == 0) 
          
          // Stop bits, should be HIGH
          if (state != MY_LOW)  { // inverse state as we are detecting the change!
            ToyotaFailBit = BitCount;
            InPacket = false;
            break;
          } // end if (state != MY_LOW) 
          
          if ( (bitpos == 10) && ((bits > 1) || (bytepos == (TOYOTA_MAX_BYTES - 1))) ) {
            ToyotaNumBytes = 0;
            ToyotaID = ID;
            for (int i=0; i<=bytepos; i++)
              ToyotaData[i] = EData[i];
            ToyotaNumBytes = bytepos + 1;
            if (bits >= 16)  // Stop bits of last byte were 1's so detect preamble for next packet
              BitCount = 0;
            else  {
              ToyotaFailBit = BitCount;
              InPacket = false;
            }
            break;
          }
        }
      }
      ++BitCount;
      --bits;
    } // end while
  } // end (InPacket == false)  
} // end void change




// DEBUG OUTPUT VOIDS 


void debugdataoutput() {
   // output to Serial.
    Serial.print("ID=");
    Serial.print(ToyotaID);
    for (int i=0; i<ToyotaNumBytes; i++)
    {
      Serial.print(", ");
      Serial.print(ToyotaData[i]);
    }
     Serial.println(".");
} // end void

void debugfaildataoutput() {
        Serial.print("FAIL = ");
        Serial.print(ToyotaFailBit);
        if (((ToyotaFailBit - 4) % 11) == 0)
        Serial.print(" (StartBit)");
        else if (((ToyotaFailBit - 4) % 11) > 8)
        Serial.print(" (StopBit)");
        Serial.println(".");
        ToyotaFailBit = 0;
} // end void
