// script qui a pour but de maintenir esp32 et raspi uniquement quand le voltage detecté par ina3221 reste au
// dessus d'une certaine valeur (par exemple 14V) et qu'il fait jour
// (par exemple courant positif arrivant du panneau ou négatif)
// le script gère aussi le PIR de makeblock sans la librairie. allume un NPN de puissance et rentre en deep sleep
// si le voltage passe en dessous d'un certain votlage ce qui coupe le raspi. tout est donc dans la fonction setup.A0

#include <Arduino.h>
#include <Wire.h>
#include <SDL_Arduino_INA3221.h>

#define I2C_SDA 33
#define I2C_SCL 32

SDL_Arduino_INA3221 ina3221;

// the three channels of the INA3221 named for SunAirPlus Solar Power Controller channels (www.switchdoc.com)
#define RASPI 1
//#define ESP32 2
#define SOLAR 3

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 1 * 60  /* Time ESP32 will go to sleep (in seconds) */

//#define TIME_TO_SLEEP 1 * 10  /* testing purpose */

RTC_DATA_ATTR int bootCount = 0;

const int GPIOPIN = 27; // for NPN (to switch on the base)
//const int delay_Switch_ON=1000*20; //sec
const int delay_Switch_ON = 0; //milli fois sec

const float Minimal_Voltage_To_Switch_On_Raspi = 14.5;    // volt
const float Minimal_Voltage_To_Switch_Off_Raspi = 13.5; //volt

void setup(void)
{


  pinMode(23, OUTPUT);      //Use as VCC for ina3221
  digitalWrite(23, HIGH);// switch on Ina3221
  delay(10);// needed to get correct first value of INA3221

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(9600);
  ina3221.begin();

  
  pinMode(26, OUTPUT);      //Use as VCC for PIR (not use anymore)
  pinMode(25, OUTPUT);      //PIR mode
  pinMode(34, INPUT);       // PIR reading
  digitalWrite(25, 1);      // mode PIR =1 (0 may be interesting as well)
  digitalWrite(26, 1);      //Use as VCC for PIR
  pinMode(GPIOPIN, OUTPUT); // for NPN
  digitalWrite(GPIOPIN, LOW);
  
  //delay(2000); 

  float shuntvoltage1 = 0;
  float busvoltage1 = 0;
  float current_mA1 = 0;
  float loadvoltage1 = 0;

  float shuntvoltage3 = 0; // to enter while loop
  float busvoltage3 = 0;
  float current_mA3 = 0;
  float loadvoltage3 = 0;

  //delay(100); 
  busvoltage3 = ina3221.getBusVoltage_V(SOLAR);
  current_mA3 = ina3221.getCurrent_mA(SOLAR);

  Serial.println(busvoltage3);
  Serial.println(current_mA3);
  if ((busvoltage3 > Minimal_Voltage_To_Switch_On_Raspi)  && (current_mA3 < 2.40)) // last condition to check daylight
  {
    
    digitalWrite(GPIOPIN, HIGH);
    while ((busvoltage3 > Minimal_Voltage_To_Switch_Off_Raspi) && (current_mA3 < 2.40))
    {
      Serial.flush();
      //pinMode(GPIOPIN, OUTPUT);
      // switch on the raspberry py (base of NPN transistor)

      busvoltage1 = ina3221.getBusVoltage_V(RASPI);
      shuntvoltage1 = ina3221.getShuntVoltage_mV(RASPI);
      current_mA1 = ina3221.getCurrent_mA(RASPI); // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
      loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);

      busvoltage3 = ina3221.getBusVoltage_V(SOLAR);
      shuntvoltage3 = ina3221.getShuntVoltage_mV(SOLAR);
      current_mA3 = ina3221.getCurrent_mA(SOLAR);
      loadvoltage3 = busvoltage3 + (shuntvoltage3 / 1000);

      Serial.print(bootCount);
      Serial.print(",");
      Serial.print(loadvoltage1);
      Serial.print(",");
      Serial.print(current_mA1);
      Serial.print(",");
      Serial.print(loadvoltage3);
      Serial.print(",");
      Serial.print(current_mA3);
      Serial.print(",");

      Serial.println(digitalRead(34));

      delay(250);
    }
  }

  else
  {
    digitalWrite(GPIOPIN, LOW);
    digitalWrite(23, LOW);// switch off Ina3221
  }

  bootCount++;
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");
  //Serial.println("Going to sleep in 20 sec");
  delay(delay_Switch_ON);
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop(void)
{
}

// #include <Arduino.h>

// void setup(){
//   Serial.begin(9600);
//   delay(1000); //Take some time to open up the Serial Monitor

// }

// void loop(){
//   //This is not going to be called

//   Serial.println("Guillaume");
//   delay(1000);
// }

// #include <Arduino.h>

// #define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
// #define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

// // const int GPIOPIN = 12;
// const int delay_Switch_ON=1000*40; //sec or min

// RTC_DATA_ATTR int bootCount = 0;

// /*
// Method to print the reason by which ESP32
// has been awaken from sleep
// */
// void print_wakeup_reason(){
//   esp_sleep_wakeup_cause_t wakeup_reason;

//   wakeup_reason = esp_sleep_get_wakeup_cause();

//   switch(wakeup_reason)
//   {
//     case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
//     case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
//     case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
//     case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
//     case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
//     default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
//   }
// }

// void setup(){
//   Serial.begin(115200);
//   delay(1000); //Take some time to open up the Serial Monitor

//   //Increment boot number and print it every reboot
//   ++bootCount;
//   Serial.println("Boot number: " + String(bootCount));

//   //Print the wakeup reason for ESP32
//   print_wakeup_reason();
//   // pinMode(GPIOPIN, OUTPUT);
//   //
//   // digitalWrite(GPIOPIN, HIGH);
//   delay(delay_Switch_ON);
//   /*
//   First we configure the wake up source
//   We set our ESP32 to wake up every 5 seconds
//   */
//   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
//   Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
//   " Seconds");

//   /*
//   Next we decide what all peripherals to shut down/keep on
//   By default, ESP32 will automatically power down the peripherals
//   not needed by the wakeup source, but if you want to be a poweruser
//   this is for you. Read in detail at the API docs
//   http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
//   Left the line commented as an example of how to configure peripherals.
//   The line below turns off all RTC peripherals in deep sleep.
//   */
//   //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
//   //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

//   /*
//   Now that we have setup a wake cause and if needed setup the
//   peripherals state in deep sleep, we can now start going to
//   deep sleep.
//   In the case that no wake up sources were provided but deep
//   sleep was started, it will sleep forever unless hardware
//   reset occurs.
//   */
//   Serial.println("Going to sleep now");
//   delay(1000);
//   Serial.flush();
//   esp_deep_sleep_start();
//   //Serial.println("This will never be printed");
// }

// void loop(){
//   //This is not going to be called
// }