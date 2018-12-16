//to do pin 11 is no longer used and pin 9 is now shutter_limit_switch 
//change the arduino board connections to the terminal block to reflect this and then delete these lines
//
//change text to command_from_master to improve code undestanding
// this routine receives commands from the radio master arduino - OS# CS# and SS#
// data is only returned by SS# - if the shutter is open return char message 'open' or 'closed'
//
//

#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>               // shown to be not required
#include <RF24.h>

//for nrf to work, pin 10 must be high if it is not used as an nrf connecton
#define PIN10  10

RF24 radio(7, 8); // CE, CSN

// pin definitions for shutter relays

// These data pins link to  Relay board pins IN1, IN2, in3 and IN4
#define open_shutter_pin    53      // arduino  pin 4
#define close_shutter_pin   54      // arduino  pin 5
#define shutter_status_pin  55      // etc



const byte thisaddress[6] =       "shutt";            // 00002 the address of this arduino board/ transmitter
const byte masterNodeaddress[6] = "mastr";
char message[10] = "";
String receivedData;
bool shutterstatus = true;

void setup()
{

  pinMode(PIN10, OUTPUT);                 // this is an NRF24L01 requirement if pin 10 is not used
  pinMode(open_shutter_pin, OUTPUT); 
  pinMode(close_shutter_pin, OUTPUT); 
  pinMode(shutter_status_pin, INPUT); 

  Serial.begin(9600);

  radio.begin();
  radio.setChannel(100);
  radio.setDataRate(RF24_250KBPS);  // set RF datarate

  // enable ack payload - slaves can reply with data using this feature if needed in future
  radio.enableAckPayload();

  radio.setPALevel(RF24_PA_LOW);
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);


  radio.openReadingPipe(1, thisaddress);    // the 1st parameter can be any number 1 to 5 the master routine uses 1
  radio.startListening();


}  // end setup

void loop()
{
  //  already done in setup radio.openReadingPipe(1, thisaddress);    // 00002
 // radio.startListening();

  while (!radio.available())
  {
    //do nothing
  }


  if (radio.available())
  {
    char text[32] = "";             // used to store what the master node sent e.g AZ hash SA hash


    radio.read(&text, sizeof(text));


    Serial.println("this is the shutter command processor  ");
    Serial.print("The text received from Master was: ");
    Serial.println(text);


    if (text[0] == 'C' && text[1] == 'S' && text[2] == '#') // close shutter command
    {
      //Serial.print ("received CS");
      close_shutter();

    }


    if (text[0] == 'O' && text[1] == 'S' && text[2] == '#') // open shutter command
    {
      //Serial.print ("received OS");
      open_shutter();

    }
    if (text[0] == 'S' && text[1] == 'S' && text[2] == '#') //  shutter status command
    {

      shutter_status();

      Serial.println("the value of shutter status is ");
      Serial.print(message);
      Serial.println("--------------------------------------------------");


      radio.openWritingPipe(masterNodeaddress);
      radio.stopListening();

      delay(50);   // to allow the master to change from tx to rx
      bool rslt =       radio.write(&message, sizeof(message));
      radio.startListening();          //straight away after write to master, in case anothe message is sent

      if (rslt)
    {
      Serial.println("result of shutter Tx was true");
      }
      else
      {
        Serial.println("result of shutter Tx was error");
      }

      for ( int i = 0; i < 10; i++)                //initialise the message array back to nulls
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
  digitalWrite (open_shutter_pin, LOW);
  digitalWrite(close_shutter_pin, HIGH);
  
} // end  CS


void open_shutter()
{
digitalWrite (close_shutter_pin, LOW);
digitalWrite (open_shutter_pin, HIGH);   // activate the open shutter routine on the shutter arduino

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
    message [6] = '#';
    //Serial.println( "closed#");  // return closed to driver modded for radio
    //Serial.println("the value of shutter status is");
    //  Serial.println(message);
    //    Serial.println("--------------------------------------------------");

  }
  else
  {
    //Serial.println( "open#");   // return open to driver  modded for radio
    message [0] = 'O';
    message [1] = 'P';
    message [2] = 'E';
    message [3] = 'N';
    message [4] = '#';
    message [5] = 0;
    message [6] = 0;
    //Serial.println("the value of shutter status is");
    //  Serial.println(message);
    //    Serial.println("--------------------------------------------------");
  }


}
