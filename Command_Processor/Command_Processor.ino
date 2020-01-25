//Version 1 - change the variable pkversion too. version control started jan 20
//changed baud rate to 19200 Jan 20


#include <SPI.h>
#include <nRF24L01.h>               // shown to be not required
#include <RF24.h>

//for nrf to work, pin 10 must be high if it is not used as an nrf connecton
#define PIN10  10

RF24 radio(7, 8); // CE, CSN

// pin definitions for shutter relays

// These data pins link to  Relay board pins IN1, IN2, IN3 and IN4
#define open_shutter_pin    30      // arduino  pin 46 corresponds with same pin number on the shutter arduino
#define close_shutter_pin   34      // these 3 pins are used to ' lay off' the open close and status commands to the shutter arduino
#define shutter_status_pin  48      // to prevent the shutter status command being blocked and causing radio timeout



const byte thisaddress[6] =       "shutt";   // "shutt" - the address of this arduino board/ transmitter
const byte masterNodeaddress[6] = "mastr";

const int channel = 115;

char message[10] = "";
String receivedData;
String pkversion = "1.0";
bool shutterstatus = true;

void setup()
{

  pinMode(PIN10, OUTPUT);                     // this is an NRF24L01 requirement if pin 10 is not used
  pinMode(open_shutter_pin, OUTPUT);
  pinMode(close_shutter_pin, OUTPUT);
  pinMode(shutter_status_pin, INPUT_PULLUP);  //input on this arduino and OUTPUT on the shutter arduino

  digitalWrite(open_shutter_pin, HIGH);       //open and close pins are used as active low, so initialise to high
  digitalWrite(close_shutter_pin, HIGH);

  SPI.begin();

  Serial.begin(19200);                         //used only for debug writes to sermon

  radio.begin();
  radio.setChannel(channel);
  radio.setDataRate(RF24_250KBPS);           // set RF datarate

  // enable ack payload - slaves can reply with data using this feature if needed in future
  radio.enableAckPayload();

  radio.setPALevel(RF24_PA_LOW);            // this is one step up from MIN and provides approx 15 feet range - so fine in observatory
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);                 // 15 retries at 15ms intervals


  radio.openReadingPipe(1, thisaddress);    // the 1st parameter can be any number 1 to 5 the master routine uses 1
  radio.startListening();                   // listen for commands from the master radio which itself receives from the c# dome driver


}  // end setup

//========================================================================================================================================
//========================================================================================================================================

void loop()
{


  while (!radio.available())
  {
    //do nothing
  }


  if (radio.available())
  {
    char text[32] = "";             // used to store what the master node sent e.g AZ hash SA hash


    radio.read(&text, sizeof(text));



    if (text[0] == 'C' && text[1] == 'S' && text[2] == '#') // close shutter command
    {
      //Serial.print ("received CS");
      close_shutter();

    }


    if (text[0] == 'O' && text[1] == 'S' && text[2] == '#') // open shutter command
    {

      open_shutter();

    }

    if (text[0] == 'S' && text[1] == 'S' && text[2] == '#') //  shutter status command
    {

      shutter_status();

      radio.openWritingPipe(masterNodeaddress);
      radio.stopListening();

      delay(50);                                            // to allow the master to change from tx to rx
      bool rslt = radio.write(&message, sizeof(message));
      radio.startListening();                               // straight away after write to master, in case another message is sent

      for ( int i = 0; i < 10; i++)                        // initialise the message array back to nulls
      {
        message[i] = 0;
      }                                           //end for
    }                                             //endif SS

    text[0] = 0;   // set to null character
    text[1] = 0;
    text[2] = 0;

  } //endif radio available

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

void shutter_status()
{

  shutterstatus = digitalRead(shutter_status_pin);   // the status pin is set in shutter arduino true = closed
  if (shutterstatus == true)
  {
    message [0] = 'C';
    message [1] = 'L';
    message [2] = 'O';
    message [3] = 'S';
    message [4] = 'E';
    message [5] = 'D';
    message [6] = 0;
    digitalWrite (close_shutter_pin, HIGH);   // the status is 'closed', so set the close activation pin back to high
  }
  else
  {

    message [0] = 'O';
    message [1] = 'P';
    message [2] = 'E';
    message [3] = 'N';
    message [4] = 0;
    message [5] = 0;
    message [6] = 0;
    digitalWrite (open_shutter_pin, HIGH); // the status is 'open', so set the open activation pin back to high
  }


}
