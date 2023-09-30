//good bye to NRF Radio complications and hello again to Bluetooth
// This module has the functions listed below:

/*

Send functions first:
Via Bluetooth to the master radio:
1 - response to Shutter status request OPEN, CLOSED, opening, closed


To the Shutter MCU:
1 - open the shutter
2 - close the shutter
2 - query the shutter status pin
3 - emergency stop (via reset)
4 - reset

Receive functions:
From the master radio
1 - Receive OS, CS, ES, reset and SS - Open Shutter, Close Shutter, emergency stop and shutter status.



*/

#include <Arduino.h>
#include <avr/wdt.h>

#define masterBluetooth Serial1     //note that hardware serial1 did not work on the test mega2560 connected to the dev machine


// pin definitions for shutter actions

// These data pins link to  Relay board pins IN1, IN2, IN3 and IN4
#define open_shutter_pin    30      // arduino  pin 46 corresponds with same pin number on the shutter arduino
#define close_shutter_pin   34      // these 3 pins are used to ' lay off' the open close and status commands to the shutter arduino
#define shutter_status_pin  48      // to prevent the shutter status command being blocked and causing radio timeout
#define MCU_reset           13      // used to cause a reset of the Shutter MCU
#define closed true                 // used with the shutterstatus variable
#define open false


//function declarations
void close_shutter();
void open_shutter();
String CreateStatusMessage();
//end function declarations



String receivedData;
String pkversion     = "3.0";
String MovementState = "CLOSING";     // THE CODE LOGIC is such that CLOSING here, sets the status message to closed. What if the MCUs reset when the shutter is open?
// currently the code logic wouldn't be able to close the shutter, only open it, which would destry, or even destroy, the chain drive.
bool shutterstatus   = true;


void setup()
{

  pinMode(MCU_reset,              OUTPUT);
  pinMode(open_shutter_pin,       OUTPUT);
  pinMode(close_shutter_pin,      OUTPUT);
  pinMode(shutter_status_pin,     INPUT_PULLUP);  // input on this MCU and OUTPUT on the shutter MCU
  digitalWrite(MCU_reset, HIGH);                  // the LOW state is used to reset the Shutter MCU
  digitalWrite(open_shutter_pin,  HIGH);          // open and close pins are used as active low, so initialise to high
  digitalWrite(close_shutter_pin, HIGH);

  
  Serial.begin(9600);                            // used only for debug writes to a PC serial monitor
  masterBluetooth.begin(9600);                   // bluetooth comms between this module and the master radio
  
  wdt_enable(WDTO_4S);                 //Watchdog set to 4 seconds
}  // end setup

//========================================================================================================================================
//========================================================================================================================================


void loop()
{
 
if (masterBluetooth.available() > 0 )
{
 
  String receipt = masterBluetooth.readStringUntil('#');  // string does not contain the #
  // if receipt contains SS call the shutter status routine
  // if receipt contains OS open the shutter
  // if receipt contains CS close the shutter
  // if receipt contains ES , reset the command processor and the SHUTTER MCU

  // so easy....
  if (receipt.indexOf("ES", 0) > -1)     // ASCOM 'emergency stop' resets this mcu and also the
  {                                      // shutter MCU. The resets cause the shutter to close if it is open
    
    digitalWrite(MCU_reset, LOW);   // LOW state resets the shutter MCU
    delay(1000);                    // delay to allow the Shutter mcu time to respond to the reset pin being LOW
                                    // The while(1) loop here times out the wdt and this mcu resets.
    while(1)                        // Both CP and shutter mcus now reset
    {}
  }

  if (receipt.indexOf("OS", 0) > -1)  
  {
    MovementState = "OPENING";
    //todo remove 2 LINES  below 
    Serial.println("the command received was " + receipt);
    Serial.println("the status message is " + CreateStatusMessage() );
    open_shutter();
  }

  if (receipt.indexOf("CS", 0) > -1)  
  {
    MovementState = "CLOSING";
    //todo remove 2 LINES below 
    Serial.println("the command received was " + receipt);
    Serial.println("the status message is " + CreateStatusMessage() );
    close_shutter();
  }

  if (receipt.indexOf("SS", 0) > -1)  
  {
  
    String x = CreateStatusMessage();
    masterBluetooth.print(x + '#');
    //todo remove  2 lines below 
    // Serial.println("the command received was " + receipt);
    // Serial.println("the status message is " + CreateStatusMessage() );
  }

}
  
wdt_reset();                       //execute this command within 4 seconds to keep the timer from resetting

} // end void loop

void close_shutter()
{
  // commands to close shutters
  digitalWrite (open_shutter_pin , HIGH);             // activate the open shutter routine on the shutter arduino
  digitalWrite (close_shutter_pin, LOW);             // 50 millisec delay, then high again


} // end  CS


void open_shutter()
{
  digitalWrite (close_shutter_pin, HIGH);
  digitalWrite (open_shutter_pin, LOW);               // activate the open shutter routine on the shutter arduino

}// end  OS




String CreateStatusMessage()
{

  String statusMessage;

  bool shutterstatus = digitalRead(shutter_status_pin);   // the status pin is set in shutter arduino true = closed

  if ( (MovementState=="OPENING" ) && (shutterstatus==closed) )   // true  is closed
    {
        statusMessage= "opening";
    }
  

  if ( (MovementState=="OPENING" ) && (shutterstatus==open) )    // false is open
    {

      statusMessage = "open";
      digitalWrite (open_shutter_pin, HIGH); // the status is 'open', so set the open activation pin back to high
    }


  if ( (MovementState=="CLOSING" ) && (shutterstatus==open) )  // false is open

    {
      statusMessage = "closing";
      
    }


  if ( (MovementState=="CLOSING" ) && (shutterstatus==closed) )   // true is closed
    {
      statusMessage = "closed";
      digitalWrite (close_shutter_pin, HIGH);   // the status is 'closed', so set the close activation pin back to high
    }
return statusMessage;

}
