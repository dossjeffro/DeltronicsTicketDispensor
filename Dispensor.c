#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include <SoftwareSerial.h>
#include <SerialCommand.h>
#include <SPI.h>
#include <Wire.h>



#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//////////////////////////////////////////
// PINOUTS
//////////////////////////////////////////
int ledpin = 13;
int ticketdriverpin = 6;
int ticketreaderpin = 2;

//////////////////////////////////////////
/// VARIABLES
bool debug = false;
volatile unsigned int pulse = 0;
int totalpulsecount = 0;
int timer = 0;
bool complete = false;
SerialCommand sCmd;
int ticketcount;

void setup() {
  // by default, we'll generate the high voltage from the 3.3v line internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  
  // Clear the buffer.
  display.clearDisplay();
  
  // Setup the pins
  pinMode(ledpin, OUTPUT);
  pinMode(ticketdriverpin, OUTPUT);
  pinMode(ticketreaderpin, INPUT_PULLUP);
  Serial.begin(9600);
  while (!Serial);
  attachInterrupt(digitalPinToInterrupt(ticketreaderpin), count_pulse, RISING); // MAP PULSE PIN TO FUNCTION
  printStatus(0,"OK");
}

void clearCounters()
{
  pulse = 0;
  totalpulsecount = 0;
  timer = 0;
  ticketcount = 0;
}

void count_pulse() 
{ 
  pulse++; 
  if (debug)
  {
   Serial.print("RECEIVED PULSE.  Time:");
   Serial.println(millis());
  }
} 

void statusLight(bool on)
{
  if (on)
  {
    digitalWrite(ledpin, HIGH); // BUSY LIGHT ON
  }
  else
  {
    digitalWrite(ledpin, LOW); // BUSY LIGHT OFF
  }
}

void loop () 
{
  if (debug)
   Serial.println("****START*****");
   
  while (Serial.available()==0)  {
  }
  ticketcount=Serial.parseInt();
  /////////////////////////////////////////////////////////////////
  //// RECEIVED DATA
  /////////////////////////////////////////////////////////////////
  if (ticketcount > 0)
  {
  /////////////////////////////////////////////////////////
  //// GOT A REQUEST FOR TICKETS
  /////////////////////////////////////////////////////////
  if (debug)
   Serial.println(ticketcount);
   dispenseTickets(ticketcount);
  }
  
  
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
/// TICKET DISPENSOR FUNCTION
void dispenseTickets(int tktcount)
{
  printStatus(tktcount,"OK");
  pulse = 0;
  if (debug)
   Serial.println("DISPENSING TICKETS");
  
  ///// TURN ON MOTOR
  if (debug)
   Serial.println("TURN ON MOTOR");
  digitalWrite(ticketdriverpin, HIGH);

  ///// KEEP ON UNTIL TICKETS DISPENSED
  unsigned long previousMillis = millis();
  unsigned long currentMillis = millis();
  unsigned long timeout = 1200 * tktcount;
  statusLight(true);
  while (pulse < tktcount && currentMillis - previousMillis <= timeout)
  {
    //// WAIT FOR PULSES
    currentMillis = millis();  
  }
  
  ///// TURN OFF MOTOR
  if (debug)
   Serial.println("TURN OFF MOTOR");
   digitalWrite(ticketdriverpin, LOW);
   
  ///// DID ALL TICKETS GET DISPENSED?
  if (pulse == tktcount)
  {
    // YES
    printStatus(0,"OK");
  }
  else
  {
    // NO
    printStatus(tktcount - pulse,"ERROR");
  }
   
  ////CLEAR COUNT AND EXIT
  pulse = 0;
  statusLight(false);
  
}
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void printStatus(int ticketsDue, String status)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("DUE: "); display.println(ticketsDue);
  /*display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println("");*/
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("STS: "); display.println(status);
  display.display();
  //delay(1000);
  display.clearDisplay();
}
