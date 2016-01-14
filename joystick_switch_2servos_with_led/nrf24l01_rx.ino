/* YourDuinoStarter Example:RECEIVE nRF24L01 Joystick data to control Pan Tilt Servos Over Radio.
   QUESTIONS? terry@yourduino.com
 -WHAT IT DOES:
  -Receives Joystick Analog Values over a nRF24L01 Radio Link, using the Radiohead library.
  - Sends Joystick position to 2 servos, usually X,Y to pan-tilt arrangement
  - TODO! Send the Joystick push-down click to turn Laser on and off
------------------------  
  OTHER QUESTION? niq_ro@yahoo.com - http://www.tehnic.go.ro
  WHAT IT DOES: 
  - send Joystick position to 2 servos and if push-switch is pushed, a LED is ON
  - see on Serial monitor position and if momentary-button is pushed or not
-------------------------  
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


   -V2.00 7/12/14 by Noah King
   Based on examples at http://www.airspayce.com/mikem/arduino/RadioHead/index.html
*/

/*-----( Import needed libraries )-----*/
// SEE http://arduino-info.wikispaces.com/Arduino-Libraries  !!
// NEED the SoftwareServo library installed
// http://playground.arduino.cc/uploads/ComponentLib/SoftwareServo.zip
#include <SoftwareServo.h>  // Regular Servo library creates timer conflict!

// NEED the RadioHead Library installed!
// http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.23.zip
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>

#include <SPI.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define CLIENT_ADDRESS     1
#define SERVER_ADDRESS     2

#define ServoHorizontalPIN 3   //Pin Numbers
#define ServoVerticalPIN   5
#define LaserPIN           6

#define ServoMIN_H  20  // Don't go to very end of servo travel
#define ServoMAX_H  160 // which may not be all the way from 0 to 180. 
#define ServoMIN_V  40  // Don't go to very end of servo travel
#define ServoMAX_V  130 // which may not be all the way from 0 to 180. 


/*-----( Declare objects )-----*/
SoftwareServo HorizontalServo;
SoftwareServo VerticalServo;  // create servo objects to control servos

// Create an instance of the radio driver
RH_NRF24 RadioDriver;

// Create an instance of a manager object to manage message delivery and receipt, using the driver declared above
RHReliableDatagram RadioManager(RadioDriver, SERVER_ADDRESS);

/*-----( Declare Variables )-----*/
int HorizontalJoystickReceived; // Variable to store received Joystick values
int HorizontalServoPosition;    // variable to store the servo position
int HorizontalServoPosition1;    // variable to store old the servo position

int VerticalJoystickReceived;   // Variable to store received Joystick values
int VerticalServoPosition;      // variable to store the servo position
int VerticalServoPosition1;      // variable to store old the servo position


uint8_t ReturnMessage[] = "JoyStick Data Received";  // 28 MAX
// Predefine the message buffer here: Don't put this on the stack:
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

//--------------------------------( SETUP Runs ONCE )-----------------------------------------------------
void setup()
{
  pinMode(LaserPIN, OUTPUT);
  digitalWrite(LaserPIN, LOW); // turn off Laser

  /*-----( Set up servos )-----*/
  HorizontalServo.attach(ServoHorizontalPIN);  // attaches the servo to the servo object
  VerticalServo.attach(ServoVerticalPIN);      // attaches the servo to the servo object


  // begin serial to display on Serial Monitor. Set Serial Monitor to 115200
  // See http://arduino-info.wikispaces.com/YourDuino-Serial-Monitor
  Serial.begin(9600);

  if (!RadioManager.init()) // Initialize radio. If NOT "1" received, it failed.
    Serial.println("init failed");
    else Serial.println("   init ok");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
} // END Setup



//--------------------------------( LOOP runs continuously )-----------------------------------------------------
void loop()
{
  if (RadioManager.available())
  {
 // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (RadioManager.recvfromAck(buf, &len, &from))
 //Serial Print the values of joystick
    {
      Serial.println("----------------------");
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": X = ");
      Serial.print(buf[0]);
      Serial.print(" Y = ");
      Serial.println(buf[1]);
      Serial.print(" SW = ");
      Serial.println(buf[2]);
      Serial.println("--------------");
      Serial.println("relative value");
      Serial.print("X = ");
      Serial.print(buf[0]-125);
      if ((buf[0]-125)<-2) Serial.println(" < LEFT");
      if ((buf[0]-125)>2) Serial.println(" > RIGHT");
if (((buf[0]-126)>=-2) and ((buf[0]-126)<=2)) Serial.println(" STOP");
      Serial.print("Y = ");
      Serial.print(buf[1]-126);
      if ((buf[1]-126)<-2) Serial.println("  v DOWN");
      if ((buf[1]-126)>2) Serial.println("  ^ UP");
if (((buf[1]-125)>=-2) and ((buf[1]-126)<=2)) Serial.println(" STOP");
      Serial.print("SW= ");
      if (buf[2]==0)  Serial.print("PUSH");
      if (buf[2]==1) Serial.print("OPEN (FREE)");      
      Serial.println("--------------");
      Serial.println("--------------");


      // Send a reply back to the originator client, check for error
      if (!RadioManager.sendtoWait(ReturnMessage, sizeof(ReturnMessage), from))
        Serial.println("sendtoWait failed");
    }// end 'IF Received data Available
  }// end 'IF RadioManager Available
  
 //   {
    SoftwareServo::refresh();//refreshes servo to keep them updating
    HorizontalJoystickReceived  = buf[1];  // Get the values received
    VerticalJoystickReceived    = buf[0]; 

    // scale it to use it with the servo (value between MIN and MAX)
    HorizontalServoPosition  = map(HorizontalJoystickReceived, 0, 255, ServoMIN_H , ServoMAX_H);
    VerticalServoPosition    = map(VerticalJoystickReceived,   0, 255, ServoMIN_V , ServoMAX_V);
    
 if (HorizontalServoPosition != HorizontalServoPosition1)
{
    Serial.print("HorizontalServoPosition : ");
    Serial.println(HorizontalServoPosition);
  HorizontalServo.write(HorizontalServoPosition);
}
 if (VerticalServoPosition != VerticalServoPosition1)
{
    Serial.print("  VerticalServoPosition : ");
    Serial.println(VerticalServoPosition);  
 VerticalServo.write(VerticalServoPosition);
}
 /*   
    // tell servos to go to position
    HorizontalServo.write(HorizontalServoPosition);
    VerticalServo.write(VerticalServoPosition);
 */
 
 HorizontalServoPosition1 = HorizontalServoPosition;
 VerticalServoPosition1 = VerticalServoPosition;

     if (buf[2]==0)   digitalWrite(LaserPIN, HIGH); // turn on Laser ;
     if (buf[2]==1)   digitalWrite(LaserPIN, LOW); // turn off Laser;      

 
 //   delay(25);                      // wait for the servo to reach the position
//  }
 
}// END Main LOOP
