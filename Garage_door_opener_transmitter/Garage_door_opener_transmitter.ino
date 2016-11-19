#include <SPI.h>
#include <Enrf24.h>
#include <nRF24L01.h>
#include <string.h>

#define GREEN P2_3 //Green LED
#define YELLOW P2_5 //Yellow LED
#define RESULT P2_4 //Other green LED at bottom

Enrf24 radio(P2_0, P2_1, P2_2);  // P2.0=CE, P2.1=CSN, P2.2=IRQ
const uint8_t txaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };
int i, x_base, y_base, z_base, x, y, z, xtime, ytime, ztime, xflag, yflag, dimledx, dimledy, dimledz, passcode[5] = {0, 0, 0, 1, 1}, pos = 0;

const char *str_open = "OPEN";

void dump_radio_status_to_serialport(uint8_t);

void setup() {  
  pinMode (GREEN, OUTPUT);
  pinMode (YELLOW, OUTPUT);
  pinMode (RESULT, OUTPUT);

  readValues ();
  x_base = x;
  y_base = y;
  z_base = z;

  Serial.begin(9600);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  radio.begin();  // Defaults 1Mbps, channel 0, max TX power
  dump_radio_status_to_serialport(radio.radioState());
  delay(1000);

  radio.setTXaddress((void*)txaddr);
}

void loop() {
    // put your main code here, to run repeatedly: 
    readValues ();

    //Light up LEDs for directions
    if (x > 1.10 * x_base)
    {
      digitalWrite(GREEN, HIGH);
      xtime++;
    }
    else if (x < 1.05 * x_base)
    {
      digitalWrite(GREEN, LOW);
      xtime = 0;
      xflag = true;
    }
    if (y < 0.97 * y_base)
    {
      digitalWrite(YELLOW, HIGH);
      ytime++;
    }
    else if (y > 0.98 * y_base)
    {
      digitalWrite(YELLOW, LOW);
      ytime = 0;
      yflag = true;
    }
    if (z < 0.96 * z_base)
    {
      //digitalWrite(P2_5, HIGH);
      ztime++;
    }
    else if (z > 0.98 * z_base)
    {
      //digitalWrite(P2_5, LOW);
      ztime = 0;
    }

    //Compare to passcode
    if (xtime > 3)
    {
      if (passcode[pos] == 0  && xflag == true)
      {
        pos++;
        xflag = false;
        digitalWrite(RESULT, HIGH);
        Serial.print("Next character: ");
        Serial.println(passcode[pos]);
        delay(20);
      }
      else
      {
        digitalWrite(RESULT, LOW);
      }
    }
    else
    {
      digitalWrite(RESULT, LOW);
    }

    if (ytime > 3)
    {
      if (passcode[pos] == 1 && yflag == true)
      {
        pos++;
        yflag = false;
        digitalWrite(RESULT, HIGH);
        Serial.print("Next character: ");
        Serial.println(passcode[pos]);
        delay(20);
      }
      else
      {
        digitalWrite(RESULT, LOW);
      }
    }
    else
    {
      digitalWrite(RESULT, LOW);
    }

    if (ztime > 3)
    {
      if (pos >= 5)
      {
        Serial.print("Sending packet: ");
        Serial.println(str_open);
        radio.print(str_open);
        radio.flush();  // Force transmit (don't wait for any more data)
        dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
        digitalWrite(RESULT, HIGH);
        delay(1000);
        digitalWrite(RESULT, LOW);
        pos = 0;
      }
      else
      {
        pos = 0;
      }
    }
    else
    {
      digitalWrite(RESULT, LOW);
    }
    
    delay(5);
}

void readValues ()
{
  x = y = z = 0;
  for (int i = 0; i < 5; i++)
  {
    x += analogRead(A0);
    y += analogRead(A3);
    z += analogRead(A4);
  }
  x = x / 5;
  y = y / 5;
  z = z / 5;
  delay(1);
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

