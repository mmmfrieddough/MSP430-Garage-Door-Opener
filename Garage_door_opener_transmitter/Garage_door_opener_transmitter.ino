#include <SPI.h>
#include <Enrf24.h>
#include <nRF24L01.h>
#include <string.h>

//Pin definitions for LED's
#define GREEN P2_3 //Green LED
#define YELLOW P2_5 //Yellow LED
#define CORRECT P2_4 //Other green LED at bottom
#define INCORRECT P1_1 //Red LED

//Variables used by RF transmitter
Enrf24 radio(P2_0, P2_1, P2_2);  // P2.0=CE, P2.1=CSN, P2.2=IRQ
const uint8_t txaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x02 };
const char *str_open = "OPEN";

//Variables used by accelerometer and passcode logic
int i, x_base, y_base, z_base, x, y, z, xtime, ytime, ztime, xflag, yflag, zflag, passlen = 5, passcode[5] = {0, 0, 1, 0, 1}, pos = 0;

//Functions
void dump_radio_status_to_serialport(uint8_t);
void readValues ();

void setup() {
  //Set LED's to output
  pinMode (GREEN, OUTPUT);
  pinMode (YELLOW, OUTPUT);
  pinMode (CORRECT, OUTPUT);
  pinMode (INCORRECT, OUTPUT);
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(CORRECT, LOW);
  digitalWrite(INCORRECT, LOW);

  //Read initial accelerometer variables to use as a base
  readValues ();
  x_base = x;
  y_base = y;
  z_base = z;

//  Serial.begin(9600);

  //Set up rf transmitter
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  radio.begin();  // Defaults 1Mbps, channel 0, max TX power
//  dump_radio_status_to_serialport(radio.radioState());
  delay(1000);
  radio.setTXaddress((void*)txaddr);
}

void loop() {
    //Get new values at the begininng of loop
    readValues ();

    //Light up LEDs for directions

    //Forward - Green
    if (x > 1.12 * x_base)
    {
      xtime++;
    }
    else if (x < 1.08 * x_base)
    {
      xtime = 0;
      xflag = true;
    }

    //Left - Yellow
    if (y < 0.94 * y_base)
    {
      ytime++;
    }
    else if (y > 0.96 * y_base)
    {
      ytime = 0;
      yflag = true;
    }

    //Down
    if (z < 0.88 * z_base)
    {
      ztime++;
    }
    else if (z > 0.90 * z_base)
    {
      ztime = 0;
      zflag = true;
    }

    //Compare to passcode
    if (xtime > 3 && xflag == true)
    {
      digitalWrite(GREEN, HIGH);
      delay(200);
      digitalWrite(GREEN, LOW);
      xflag = false;
      
      if (passcode[pos] == 0)
      {
        pos++;
//        Serial.println("Correct!");
      }
      else
      {
        pos = 0;
//        Serial.println("Incorrect!");
      }

//      Serial.print("Next character: ");
//      Serial.println(passcode[pos]);
      delay(20);
    }

    if (ytime > 3 && yflag == true)
    {
      digitalWrite(YELLOW, HIGH);
      delay(200);
      digitalWrite(YELLOW, LOW);
      yflag = false;
      
      if (passcode[pos] == 1)
      {
        pos++;
//        Serial.println("Correct!");
      }
      else
      {
        pos = 0;
//        Serial.println("Incorrect!");
      }

//      Serial.print("Next character: ");
//      Serial.println(passcode[pos]);
      delay(20);
    }

    if (ztime > 3 && zflag == true)
    {
      zflag = false;
      
      if (pos >= passlen)
      {
//        Serial.print("Sending packet: ");
//        Serial.println(str_open);
        radio.print(str_open);
        radio.flush();  // Force transmit (don't wait for any more data)
//        dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
        digitalWrite(CORRECT, HIGH);
        delay(1000);
        digitalWrite(CORRECT, LOW);
      }
      else
      {
//        Serial.println("Password Incorrect");
        digitalWrite(INCORRECT, HIGH);
        delay(1000);
        digitalWrite(INCORRECT, LOW);
      }
      
      pos = 0;
      ztime = 0;
    }
    
    delay(5);
}

void readValues ()
{
  x = y = z = 0;
  for (int i = 0; i < 10; i++)
  {
    x += analogRead(A0);
    y += analogRead(A3);
    z += analogRead(A4);
  }
  x = x / 10;
  y = y / 10;
  z = z / 10;
}

void dump_radio_status_to_serialport(uint8_t status)
{
  Serial.print("Enrf24 radio transceiver status: ");
  switch (status) {
    case ENRF24_STATE_NOTPRESENT:
      Serial.println("NO TRANSCEIVER PRESENT");
      break;

    case ENRF24_STATE_DEEPSLEEP:
      Serial.println("DEEP SLEEP <1uA power consumption");
      break;

    case ENRF24_STATE_IDLE:
      Serial.println("IDLE module powered up w/ oscillators running");
      break;

    case ENRF24_STATE_PTX:
      Serial.println("Actively Transmitting");
      break;

    case ENRF24_STATE_PRX:
      Serial.println("Receive Mode");
      break;

    default:
      Serial.println("UNKNOWN STATUS CODE");
  }
}

