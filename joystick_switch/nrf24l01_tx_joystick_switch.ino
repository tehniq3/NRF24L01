/* YourDuinoStarter Example: TRANSMIT nRF24L01 Joystick data to Pan Tilt Over Radio.
   QUESTIONS? terry@yourduino.com
 - WHAT IT DOES: Reads Joystick Analog Values on A4, A5 and transmits
   them over a nRF24L01 Radio Link, using the Radiohead library.
 - TODO! Send the Joystick push-down click to turn Laser on and off  - NO
 - SEE the comments after "//" on each line below
 - CONNECTIONS: nRF24L01 Modules See:
 http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin 8
   4 - CSN to Arduino pin 10
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED

      -
   Analog Joystick or two 10K potentiometers:
   GND to Arduino GND
   VCC to Arduino +5V
   X Pot to Arduino A5
   Y Pot to Arduino A4
   Click Button to pin 4

   -V2.00 7/12/14  by Noah King
   Based on examples at http://www.airspayce.com/mikem/arduino/RadioHead/index.html
*/

/*-----( Import needed libraries )-----*/
// SEE http://arduino-info.wikispaces.com/Arduino-Libraries  !!
// NEED the RadioHead Library installed!
// http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.23.zip
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>


/*-----( Declare Constants and Pin Numbers )-----*/
#define JoyStick_X_PIN     A5  //Pin Numbers
#define JoyStick_Y_PIN     A4
#define ClickPIN           4

#define CLIENT_ADDRESS 1      // For Radio Link
#define SERVER_ADDRESS 2


// Create an instance of the radio driver
RH_NRF24 RadioDriver;

// Create an instance of a manager object to manage message delivery and receipt, using the driver declared above
RHReliableDatagram RadioManager(RadioDriver, CLIENT_ADDRESS);// sets the driver to NRF24 and the client adress to 1

/*-----( Declare Variables )-----*/
uint8_t joystick[3];  // 3 element array of unsigned 8-bit type, holding Joystick readings

// Predefine the message buffer here: Don't put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];    // Actually: 28 bytes (32 minus 4 byte header)

void setup()  /****** SETUP: RUNS ONCE ******/
{
  // begin serial to display on Serial Monitor. Set Serial Monitor to 115200
  // See http://arduino-info.wikispaces.com/YourDuino-Serial-Monitor
  Serial.begin(9600);

  // NOTE: pinMode for Radio pins handled by RadioDriver
  if (!RadioManager.init())   // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
    Serial.println("init failed");

  pinMode(ClickPIN, INPUT);  //Not really needed: pins default to INPUT
  digitalWrite(ClickPIN,HIGH); //Pull switch high
}



void loop() /****** LOOP: RUNS CONSTANTLY ******/
{
  //Read the joystick values, scale them to 8-bit type and store them in the joystick[] array.
  Serial.println("Reading joystick values ");
  // Take the value of Joystick voltages which are 0 to 1023 (10 bit), and convert them to 0 to 255 (8 bit)
  joystick[0] = map(analogRead(JoyStick_X_PIN), 0, 1023, 0, 255);
  joystick[1] = map(analogRead(JoyStick_Y_PIN), 0, 1023, 0, 255);
if (digitalRead(ClickPIN) == LOW) joystick[2]=0;
if (digitalRead(ClickPIN) == HIGH) joystick[2]=1;
//  joystick[2] = map(analogRead(ClickPIN), 0, 1023, 0, 255);
   
  //Display the joystick values in the serial monitor.
  Serial.println("-----------");
  Serial.print("x:");
  Serial.println(joystick[0]);
  Serial.print("y:");
  Serial.println(joystick[1]);
  Serial.print("sw:");
  Serial.println(joystick[2]);


  Serial.println("Sending Joystick data to nrf24_reliable_datagram_server");
  //Send a message containing Joystick data to manager_server
  if (RadioManager.sendtoWait(joystick, sizeof(joystick), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (RadioManager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is nrf24_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");

  delay(100);  // Wait a bit before next transmission
}
