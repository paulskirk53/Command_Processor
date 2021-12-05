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
3 - probably should have emergency stop, but erse

Receive functions:
From the master radio
1 - Receive OS, CS, ES and SS -shutter status, Open Shutter, Close Shutter and emergency stop

todo - set up some serial prints to PC as tests of the bluetooth receipts.

Test Plan:
NB DON'T FORGET TO CONNECT UP THE BLUETOOTH DEVICE INTO THE BLUE COMMAND BOX. tHIS CODE USES HARDWARE SERIAL1, SO CONNECT ACCORDINGLY.
NB the MCU will need to be connected to the shutter MCU, so best to test this routine by uploading the the COMMAND PROCESSOR box
1 - Upload this code to the command processor
2 - Send commands to this routine from a bluetooth MCU on the NUC
    for each function insert a serial.print to print out the command received and the status message - sermon on dev PC


    the expected output is:
    Initial state is shutter closed.
    if the shutter is closed and OS is issued:
      'the command received was OS'
      'the status message is opening'

    issue a number of SS commands whilst the shutter is opening and we should get returned 'opening', 'opening' etc followed by 'open' 'open' etc until a close command is issued
    
    if the shutter is open and CS is issued:
      'the command received was CS'
      'the status message is closing'

  at any time, SS can be issued and the appropriate one of the four available status messages should be printed


*/

#include <Arduino.h>

#define masterBluetooth Serial1     //note that hardware serial1 did not work on the test mega2560 connected to the dev machine


// pin definitions for shutter actions

// These data pins link to  Relay board pins IN1, IN2, IN3 and IN4
#define open_shutter_pin    30      // arduino  pin 46 corresponds with same pin number on the shutter arduino
#define close_shutter_pin   34      // these 3 pins are used to ' lay off' the open close and status commands to the shutter arduino
#define shutter_status_pin  48      // to prevent the shutter status command being blocked and causing radio timeout
#define closed true                 // used with the shutterstatus variable
#define open false


//function declarations
void close_shutter();
void open_shutter();
String CreateStatusMessage();
//end function declarations



String receivedData;
String pkversion     = "2.0";
String MovementState = "";
bool shutterstatus   = true;


void setup()
{

  
  pinMode(open_shutter_pin,       OUTPUT);
  pinMode(close_shutter_pin,      OUTPUT);
  pinMode(shutter_status_pin,     INPUT_PULLUP);  // input on this MCU and OUTPUT on the shutter MCU

  digitalWrite(open_shutter_pin,  HIGH);          // open and close pins are used as active low, so initialise to high
  digitalWrite(close_shutter_pin, HIGH);

  
  Serial.begin(9600);                            // used only for debug writes to a PC serial monitor
  masterBluetooth.begin(9600);                   // bluetooth comms between this module and the master radio
  
  
}  // end setup

//========================================================================================================================================
//========================================================================================================================================





void loop()
{

if (masterBluetooth.available() > 0 )
{
  String receipt = masterBluetooth.readStringUntil('#');  // string does not contain the #
  //if receipt contains SS call the ss routine
  // if receipt conatins OS open the shutter
  // if receipt conatins CS close the shutter

  // so easy....
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
    Serial.println("the command received was " + receipt);
    Serial.println("the status message is " + CreateStatusMessage() );
  }

}
  

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
