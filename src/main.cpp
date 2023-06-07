#include <Arduino.h>



/*
 * 6/6/2023 Works with XPLPro.h 12,178 bytes  5/31/23 and XPLPro.cpp 14,290 bytes 5/17/23 
 * XPLProBeaconDemoXp32
 *
 * 6/6/2023 JWhite Updated to display Com/Nav frequencies on an LCD 4x20
 *  
 * Created by Curiosity Workshop for XPL/Pro arduino->XPlane system.
 * 
 * This sketch was developed and tested on an Arduino Mega.
 * 
   To report problems, download updates and examples, suggest enhancements or get technical support:
  
      discord:  https://discord.gg/RacvaRFsMW
      patreon:  www.patreon.com/curiosityworkshop
      YouTube:  https://youtube.com/channel/UCISdHdJIundC-OSVAEPzQIQ
 * 
 * This sketch is specific to the ESP32 using "ESP32 Dev Module"
 * The lcd uses address 26 for the ESP32, 0x27 for the Arduino Mega
 */

#include <arduino.h>
#include <XPLPro.h>              //  include file for the X-plane direct interface 
 //#include <avr\dtostrf.h>
#include <LiquidCrystal_I2C.h>
//#include <dtostrf.h>

XPLPro XP(&Serial);      // create an instance of it
//LiquidCrystal_I2C lcd(0x27,20,4); // MEGA
LiquidCrystal_I2C lcd(0x26,20,4); // ESP32 PORT 26

long int startTime;

/*
struct inStruct // potentially 'class'
{
    int handle;
    int type;
    int element;
    long inLong;
    float inFloat;
    char* inStr;
};
*/
inStruct* inData;

int drefBeacon;         // this stores a handle to the beacon light dataref
int drefParkingBrake;
int drefComNavSel;

int drefCom1Main;
int drefCom1Stby;

int drefCom2Main;
int drefCom2Stby;

int drefNav1Main;
int drefNav1Stby;

int drefNav2Main;
int drefNav2Stby;



char  FreqValue[10];

int lastBeacon;
int lastParkingBrake;

long lastCom1Main;
long lastCom1Stby;

long lastCom2Main;
long lastCom2Stby;

long lastNav1Main = 10;
long lastNav1Stby = 10;

long lastNav2Main = 10;
long lastNav2Stby = 10;

//inStruct inData; // Declared for use in function whose address is passed to XP.begin
/*
 Procedue/function Declarations
*/
void updateComNavSel(inStruct *inData);
void updateBeacon(inStruct *inData);
void updateParkingBrake(inStruct *inData);

void updateCom1Main(inStruct *inData);
void updateCom1Stby(inStruct *inData);
void updateCom2Main(inStruct *inData);
void updateCom2Stby(inStruct *inData);

void updateNav1Main(inStruct *inData);
void updateNav1Stby(inStruct *inData);
void updateNav2Main(inStruct *inData);
void updateNav2Stby(inStruct *inData);


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     16
#define OLED_CS     5
#define OLED_RESET  17

//#define LED_BUILTIN 2   //ESP32
#define LED_BUILTIN 13  //Arduino MEGA

void xplRegister();
void xplShutdown();
void xplInboundHandler(inStruct*);

void setup() 
{
 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);

  pinMode(LED_BUILTIN, OUTPUT);     // built in LED on arduino board will turn on and off with the status of the beacon light
 
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

 /*
    needed for initialization.  Parameters are:
        a texual identifier of your device 
        your function that will be called when xplane and the plugin are ready for dataref and command registrations
        your function that will be called when xplane either shuts down or unloads an aircraft model
        your function that will be called when we have requested sends of datarefs that have changed
 * ========================================================
  The callback 'xplInboundHandler' has one argument. The declaration below is to allow the
  complete information (function pointer and it's associated argument are passed to XP.begin)
 */
 void(*funcptr)(inStruct *);
  
  XP.begin("XPLPro Beacon Light Demo!", &xplRegister, &xplShutdown, (&inData, &xplInboundHandler)); 

  //XP.begin("XPLPro Beacon Light Demo!", &xplRegister, &xplShutdown, &xplInboundHandler);               
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() 
{
 
  XP.xloop();  //  needs to run every cycle.  

/************************************************************************************************************************************
 * everything after the next line will only occur every 100ms.  You can also utilize other time values.
 * This helps maintain serial data flow and also helps with switch debounce.
 * do NOT add delays anywhere in the loop cycle, it will interfere with the reception of serial data from xplane and the plugin.
 ************************************************************************************************************************************
*/



  if (millis() - startTime > 100) startTime = millis();   else return;          

}

/*
 * This function is the callback function we specified that will be called any time our requested data is sent to us.
 * handle is the handle to the dataref.  The following variables within the plugin are how we receive the data:
 * 
 * inStruct *inData is a pointer to a structure that contains information about the incoming dataref. 
 * 
 *  int inData->handle          The handle of the incoming dataref
 *  int inData->element         If the dataref is an array style dataref, this is the element of the array
 *  long inData->inLong         long value for datarefs that are long values
 *  float inData->inFloat       float value for datarefs that are float values
 */
 
/*
 *  this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
 *  It could be called multiple times if for instance xplane is restarted or loads a different aircraft
 */
void xplRegister()         
{
/*
 * This example registers a dataref for the beacon light.  
 * In the inbound handler section of the code we will turn on/off the LED on the arduino board to represent the status of the beacon light within xplane.
 * On non-AVR boards remove the F() macro.
 */

  drefBeacon = XP.registerDataRef(("sim/cockpit2/switches/beacon_on")); 
  XP.requestUpdates(drefBeacon, 100, 0);          // Tell xplane to send us updates when the status of the beacon
                                                  // light changes.  
                                                  // 100 means don't update more often than every 100ms and 0 is
                                                  // a precision specifier for float variables which is explained
                                                  // in another example, use 0 for now.
  
  drefParkingBrake = XP.registerDataRef(("sim/cockpit2/controls/parking_brake_ratio"));    
  XP.requestUpdates(drefParkingBrake, 100, .1);   // Tell xplane to send us updates when the 
                                                  // status of the parking brake changes.  
                                                  // 100 means don't update more often than every 100ms
                                                  //  and .1 is a precision specifier for float variables
                                                  // which is explained in another example, use .1 for
                                                  // this example

  drefComNavSel = XP.registerDataRef(("sim/cockpit/g430/g430_nav_com_sel"));
  XP.requestUpdates(drefComNavSel, 100, 1,0); 
  XP.requestUpdates(drefComNavSel, 100, 1,1);

  drefCom1Main = XP.registerDataRef(("sim/cockpit2/radios/actuators/com1_frequency_hz_833"));
  XP.requestUpdates(drefCom1Main, 100, 1);

  drefCom1Stby = XP.registerDataRef(("sim/cockpit2/radios/actuators/com1_standby_frequency_hz_833"));
  XP.requestUpdates(drefCom1Stby, 100, 1);

  drefCom2Main = XP.registerDataRef(("sim/cockpit2/radios/actuators/com2_frequency_hz_833"));
  XP.requestUpdates(drefCom2Main, 100, 1);

  drefCom2Stby = XP.registerDataRef(("sim/cockpit2/radios/actuators/com2_standby_frequency_hz_833"));
  XP.requestUpdates(drefCom2Stby, 100, 1);

  drefNav1Main = XP.registerDataRef(("sim/cockpit2/radios/actuators/nav1_frequency_hz"));
  XP.requestUpdates(drefNav1Main, 100, 0);

  drefNav1Stby = XP.registerDataRef(("sim/cockpit/radios/nav1_stdby_freq_hz"));
  XP.requestUpdates(drefNav1Stby, 100, 0);

  drefNav2Main = XP.registerDataRef(("sim/cockpit2/radios/actuators/nav2_frequency_hz"));
  XP.requestUpdates(drefNav2Main, 100, 0);

  drefNav2Stby = XP.registerDataRef(("sim/cockpit/radios/nav2_stdby_freq_hz"));
  XP.requestUpdates(drefNav2Stby, 100, 0);
}

void xplInboundHandler(inStruct *inData)
{  
  // NOTE: The inData->handle is checked again inside each procedure called. Teh if/else if is used to ensure only one procedure is called within the handler
  //  Use due dilligence and confirm each updatexx is called using the correct dref.
  // CAUTION!

    if(inData->handle == drefComNavSel)
      updateComNavSel(inData);
    else if (inData->handle == drefBeacon) 
      updateBeacon(inData);
    else if (inData->handle == drefParkingBrake)
      updateParkingBrake(inData);
    else if (inData->handle == drefCom1Main)
      updateCom1Main(inData);
    else if (inData->handle == drefCom1Stby)
      updateCom1Stby(inData);
    else if (inData->handle == drefCom2Main)
      updateCom2Main(inData);
    else if (inData->handle == drefCom2Stby) 
      updateCom2Stby(inData);
    else if (inData->handle == drefNav1Main)
      updateNav1Main(inData);
    else if (inData->handle == drefNav1Stby)
      updateNav1Stby(inData);
    else if (inData->handle == drefNav2Main)
      updateNav2Main(inData);
    else if (inData->handle == drefNav2Stby)
      updateNav2Stby(inData);
}


void xplShutdown()
{
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}

void updateComNavSel(inStruct *inData)
{
  if (inData->handle == drefComNavSel)
  {
    lcd.setCursor(18,1);
    lcd.print(",");
  
    switch (inData->element )
    {
      case 0:     // Com/Nav 1
        lcd.setCursor(17,1);
        lcd.print(inData->inLong); 
        break;

      case 1:     // Com/Nav 2
        lcd.setCursor(19,1);
        lcd.print(inData->inLong);   
        break;

      default:   
        break;
  
    }
  }
}

void updateBeacon(inStruct *inData)
{
  if (inData->handle == drefBeacon)
  {
    lcd.setCursor(17,3);
    lastBeacon = inData->inLong; // save for next comparison
    lcd.setCursor(17,3);
      
    if (lastBeacon)
    {
      digitalWrite(LED_BUILTIN, HIGH);        // if beacon is on set the builtin led on
      lcd.print("BCN");
    }
    else                    
    {  
      digitalWrite(LED_BUILTIN, LOW);
      lcd.print("   ");
    }
  }
}

void updateParkingBrake(inStruct *inData)
{
    if(inData->handle == drefParkingBrake)
    {
      lastParkingBrake = inData->inFloat; // save for next comparison
      lcd.setCursor(17,0);
      if (lastParkingBrake > .2)
      {
          lcd.print("BRK");  
          digitalWrite(LED_BUILTIN, HIGH);        // if parking brake is set turn the LED on
      }    
      else
      { 
        lcd.print("   ");  
        digitalWrite(LED_BUILTIN, LOW);
      }
    }     // end parking brake
}

void updateCom1Main(inStruct *inData)
{
  if(inData->handle == drefCom1Main)
  {
      //if(lastCom1Main != inData->inLong)
      {
          lastCom1Main = (inData->inLong);  // save for next comparison
          dtostrf((double)lastCom1Main/1000.0, 7,3, FreqValue);
          lcd.setCursor(0,0);
          lcd.print(FreqValue);
      }
  }   
}

void updateCom1Stby(inStruct *inData)
{
  if(inData->handle == drefCom1Stby)
  {
      if(lastCom1Stby != inData->inLong)
      {
          lastCom1Stby = (inData->inLong);  // save for next comparison
          dtostrf((double)lastCom1Stby/1000.0, 7,3, FreqValue);
          lcd.setCursor(0,1);
          lcd.print(FreqValue);
      }
  }  
}

void updateCom2Main(inStruct *inData)
{
  if(inData->handle == drefCom2Main)
  {
    if(lastCom2Main != inData->inLong)
    {
      lastCom2Main = (inData->inLong);  // save for next comparison
      dtostrf((double)lastCom2Main/1000.0, 7,3, FreqValue);
      lcd.setCursor(9,0);
      lcd.print(FreqValue);
    }
  }   
}

void updateCom2Stby(inStruct *inData)
{
  if(inData->handle == drefCom2Stby)
  {
      if(lastCom2Stby != inData->inLong)
      {
          lastCom2Stby = (inData->inLong);  // save for next comparison
          dtostrf((double)lastCom2Stby/1000.0, 7,3, FreqValue);
          lcd.setCursor(9,1);
          lcd.print(FreqValue);
      }
  }  
}

void updateNav1Main(inStruct *inData)
{

  if(inData->handle == drefNav1Main)
  {
      if(lastNav1Main != inData->inLong)
      {
          lastNav1Main = inData->inLong; // save for next comparison
          dtostrf((double)lastNav1Main/100.0, 6,2, FreqValue);
          lcd.setCursor(0,2);
          lcd.print(FreqValue);
      }
 }   // Nav1Main
}

void updateNav1Stby(inStruct *inData)
{
  if(inData->handle == drefNav1Stby)
  {
      if(lastNav1Stby != inData->inLong)
      {
          lastNav1Stby = inData->inLong;
          dtostrf((double)lastNav1Stby/100.0, 6,2, FreqValue);
          lcd.setCursor(0,3);
          lcd.print(FreqValue);
      }
 }   // Nav1Main
}

void updateNav2Main(inStruct *inData)
{

  if(inData->handle == drefNav2Main)
  {
      if(lastNav2Main != inData->inLong)
      {
          lastNav2Main = inData->inLong;
          dtostrf((double)lastNav2Main/100.0, 6,2, FreqValue);
          lcd.setCursor(9,2);
          lcd.print(FreqValue);
      }
 }   // Nav2Main
}

void updateNav2Stby(inStruct *inData)
{

  if(inData->handle == drefNav2Stby)
  {
      if(lastNav2Stby != inData->inLong)
      {
          lastNav2Stby = inData->inLong;  // save for next comparison
          dtostrf((double)lastNav2Stby/100.0, 6,2, FreqValue);
          lcd.setCursor(9,3);
          lcd.print(FreqValue);
      }
 }   // Nav2Stby
}

