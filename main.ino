//
// Project link: https://www.bhoite.com/sculptures/satellite-2202/
//
#include "Adafruit_SHT31.h"
// #include <TinyWireM.h>
// #include <USI_TWI_Master.h>
 #include <avr/io.h>
#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/wdt.h> //Needed to enable/disable watch dog timer
//Following is the header file for the LCD module bought on ebay
//Seller link https://www.ebay.com/itm/284843842249
//Information on the LCD http://www.hexpertsystems.com/LCD/
#include "cdm4101.h"

int ctr=1000;
CDM4101 LCD;
char data[25];
int temperature = 0;

bool displayToggle = 0;
uint8_t loopCnt = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() 
{
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);




  ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
  // put your setup code here, to run once:
  //TinyWireM.begin();
  Wire.begin();

  sht31.begin(0x44);

  LCD.Init();

  LCD.DispStr("sat6");
  
  delay(1000);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();


}

//This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect) {
  //watchdog_counter++;
  displayToggle=!displayToggle;
}

void loop() 
{
  setup_watchdog(9); //Setup watchdog to go off after 8sec
  sleep_mode(); //Go to sleep! Wake up 8 sec later
  
  // int temperature = (sht31.readTemperature()*1.8)+32;
  int temperature = sht31.readTemperature();
  int humidity = sht31.readHumidity();
  if (displayToggle)
  {
  //insure that the temperature is in a valid range
    if ((temperature <110) && (temperature > -20))
    {
      sprintf(data,"%d*C",temperature);
      LCD.DispStr(data);
    }
  }

  else
  {
  //insure that the humidity is in a valid range
    if ((humidity <101) && (humidity > 0))
    {
      sprintf(data,"%drH",humidity);
      LCD.DispStr(data);
    }
  }
}

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}