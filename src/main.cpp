#include <Arduino.h>
#include <Wire.h>
#include <SDL_Arduino_INA3221.h>
#include <ESP32Time.h>

// All constants

const int I2C_SDA = 33;
const int I2C_SCL = 32;

const long uS_TO_S_FACTOR = 1000000; /* Conversion factor for micro seconds to seconds */
const int TIME_TO_SLEEP_DAY = 1 * 10;    /* Time ESP32 will go to sleep (in seconds) */
const int TIME_TO_SLEEP_NIGHT = 1 * 300;

const int TRANSISTOR = 27; // for NPN (to switch on the base)
const int INAVCC = 26;

const int RASPI = 1;
const int SOLAR = 3;

const long Initial_Delay = 30000;

const int ARRAYSIZE = 3100;

// all variables

float Minimal_Voltage_To_Switch_On_Raspi = 15;
float Initial_Voltage_To_Switch_On_Raspi = 15;  // volt
float Minimal_Voltage_To_Switch_Off_Raspi = 14; //volt

float busvoltage = 0;
float current_mA_solar = 0;
float current_mA_raspi = 0;

long Epoch_Now = 1600000000; // functions getEpoch too slow if set time below that (bug)
long Epoch_Restart = 1600000000;

int Reason_Switch_Off = 0;
int Transistor_State = 0;

int Summer_Time = 0; //7200

unsigned long it0 = 0; // iteration to save date during "zero transistor state"
unsigned long it1 = 0;
unsigned long it2 = 0;

// All objects

SDL_Arduino_INA3221 ina3221;
String Data_transistor_Off[ARRAYSIZE];
ESP32Time rtc(Summer_Time);

void setup(void)
{

  rtc.setTime(Epoch_Now);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_DAY * uS_TO_S_FACTOR);

  pinMode(TRANSISTOR, OUTPUT); // for NPN
  digitalWrite(TRANSISTOR, LOW);

  pinMode(INAVCC, OUTPUT);    //Use as VCC for ina3221
  digitalWrite(INAVCC, HIGH); // switch on Ina3221

  delay(10); // needed to get correct first value of INA3221

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(9600);
  ina3221.begin();

  // Serial.println((String) "Delay of " + Initial_Delay / 1000 + " seconds");
  // delay(Initial_Delay);
  // Serial.println("End of delay");
}

void loop(void)
{

  digitalWrite(INAVCC, HIGH); // switch on Ina3221
  delay(10);

  busvoltage = ina3221.getBusVoltage_V(SOLAR);

  while ((busvoltage < 12.0) || (busvoltage > 17.0))
  {
    busvoltage = ina3221.getBusVoltage_V(SOLAR);
  }

  current_mA_raspi = ina3221.getCurrent_mA(RASPI);
  current_mA_solar = ina3221.getCurrent_mA(SOLAR);

  // we save data during Transistor OFF only after receiving epoch
  // and only if iteration is inferior to arraysize to avoid overflow

  Data_transistor_Off[it0] = String(Transistor_State);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(Reason_Switch_Off);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(busvoltage);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(current_mA_raspi);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(current_mA_solar);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(rtc.getEpoch());
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(Epoch_Restart);

  Serial.println(Data_transistor_Off[it0]); //always print [0] if beginning
  delay(100);

  if ((Reason_Switch_Off > 0) && (it0 < ARRAYSIZE))
  {
    it0++; // the way to keep track of Data
  }

  if ((busvoltage <= Minimal_Voltage_To_Switch_On_Raspi) && (it2 == 0))

  {
    Minimal_Voltage_To_Switch_On_Raspi=Minimal_Voltage_To_Switch_On_Raspi + 0.1;
    Serial.print("New Minimal Voltage to switch on: ");
    Serial.println(Minimal_Voltage_To_Switch_On_Raspi);
    it2=1;
    // to add hysteresis
  }

  if (busvoltage > Minimal_Voltage_To_Switch_On_Raspi) // to get hysteresis
  {
    Minimal_Voltage_To_Switch_On_Raspi = Initial_Voltage_To_Switch_On_Raspi;
    it2=0;
    it1 = 0;

    while ((busvoltage > Minimal_Voltage_To_Switch_Off_Raspi) && (rtc.getEpoch() > Epoch_Restart)) // last condition to check daylight
    {
      if (it1 == 0)
      {
        digitalWrite(TRANSISTOR, HIGH);

        Transistor_State = 1;

        delay(100);

        //Serial.flush(); // maybe useless

        if (Reason_Switch_Off > 0)
        {
          digitalWrite(INAVCC, LOW); // switch on Ina3221
          while (Serial.readString() != "Raspi Ready sent")
          {
            //test with index at 0
            Serial.println(Data_transistor_Off[0]); // only to allow raspi to get the reboot parameter(>0)
          };
          digitalWrite(INAVCC, HIGH); // switch on Ina3221
          delay(100);

          Serial.println("ESP32 Ready received");
          delay(100);
          for (int i = 0; i < it0; i++)
          {
            Serial.println(Data_transistor_Off[i]);
          }
          delay(100); // critical
          Serial.println("End of transmission");
          delay(100); // critical
          it0 = 0;    // to avoid overflow
        
        }
      } // wait for raspi send data

      busvoltage = ina3221.getBusVoltage_V(SOLAR);

      while ((busvoltage < 12.0) || (busvoltage > 17.0))
      {
        busvoltage = ina3221.getBusVoltage_V(SOLAR);
      }

      current_mA_raspi = ina3221.getCurrent_mA(RASPI);
      current_mA_solar = ina3221.getCurrent_mA(SOLAR);

      Serial.print(Transistor_State);
      Serial.print(",");
      Serial.print(Reason_Switch_Off);
      Serial.print(",");
      Serial.print(busvoltage);
      Serial.print(",");
      Serial.print(current_mA_raspi);
      Serial.print(",");
      Serial.print(current_mA_solar);
      Serial.print(",");
      Serial.print(rtc.getEpoch());
      Serial.print(",");
      Serial.println(Epoch_Restart);

      delay(1000);// 100 ms was not enough; maybe 250 neither. 1000 should be ok.

      if (Serial.available() > 0)
      {
        String String_Sent=Serial.readStringUntil(',');

        if (String_Sent == "Epoch Sent")
        {
          Epoch_Now = Serial.readStringUntil(',').toInt();
          Epoch_Restart = Serial.readStringUntil(',').toInt(); // + Summer_Time; // reçoit un byte de pyhon et considéré comme String
          rtc.setTime(Epoch_Now);
          Serial.println("Epoch received");
          delay(100);
          while (Serial.readString() != "Double check OK")
          {
            Serial.println("Waiting dor double check");
            delay(100);
            
          }
          Serial.println("Double check received: switch off pi in 50 secs.");
          delay(50000);
          Reason_Switch_Off = 2;
          break;
        }
        else
        {
          Serial.print("Corrupted data: "); 
          Serial.println(String_Sent);
          delay(100);        
        }
      }

      if (busvoltage < Minimal_Voltage_To_Switch_Off_Raspi)
      {
        Reason_Switch_Off = 1;
        delay(50000);
        break;
      }

      // if (rtc.getEpoch() <= Epoch_Restart)
      // {

      // }

      //Serial.flush();
      it1++;
      gpio_hold_en(GPIO_NUM_27);
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_DAY * uS_TO_S_FACTOR);
      esp_light_sleep_start();
      gpio_hold_dis(GPIO_NUM_27);
    }
  }

  //Serial.flush();
  digitalWrite(TRANSISTOR, LOW);
  Transistor_State = 0;
  gpio_hold_en(GPIO_NUM_27);
  if (it0 > 5)
  {
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_NIGHT * uS_TO_S_FACTOR);
  }
  esp_light_sleep_start();
  gpio_hold_dis(GPIO_NUM_27);
}










#include <Arduino.h>
#include <Wire.h>
#include <SDL_Arduino_INA3221.h>
#include <ESP32Time.h>

// All constants

const int I2C_SDA = 33;
const int I2C_SCL = 32;

const long uS_TO_S_FACTOR = 1000000; /* Conversion factor for micro seconds to seconds */
const int TIME_TO_SLEEP_DAY = 1 * 10;    /* Time ESP32 will go to sleep (in seconds) */
const int TIME_TO_SLEEP_NIGHT = 1 * 300;

const int TRANSISTOR = 27; // for NPN (to switch on the base)
const int INAVCC = 26;

const int RASPI = 1;
const int SOLAR = 3;

const long Initial_Delay = 30000;

const int ARRAYSIZE = 3100;

// all variables

float Minimal_Voltage_To_Switch_On_Raspi = 15;
float Initial_Voltage_To_Switch_On_Raspi = 15;  // volt
float Minimal_Voltage_To_Switch_Off_Raspi = 14; //volt

float busvoltage = 0;
float current_mA_solar = 0;
float current_mA_raspi = 0;

long Epoch_Now = 1600000000; // functions getEpoch too slow if set time below that (bug)
long Epoch_Restart = 1600000000;

int Reason_Switch_Off = 0;
int Transistor_State = 0;

int Summer_Time = 0; //7200

unsigned long it0 = 0; // iteration to save date during "zero transistor state"
unsigned long it1 = 0;
unsigned long it2 = 0;

// All objects

SDL_Arduino_INA3221 ina3221;
String Data_transistor_Off[ARRAYSIZE];
ESP32Time rtc(Summer_Time);

void setup(void)
{

  rtc.setTime(Epoch_Now);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_DAY * uS_TO_S_FACTOR);

  pinMode(TRANSISTOR, OUTPUT); // for NPN
  digitalWrite(TRANSISTOR, LOW);

  pinMode(INAVCC, OUTPUT);    //Use as VCC for ina3221
  digitalWrite(INAVCC, HIGH); // switch on Ina3221

  delay(10); // needed to get correct first value of INA3221

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(9600);
  ina3221.begin();

  // Serial.println((String) "Delay of " + Initial_Delay / 1000 + " seconds");
  // delay(Initial_Delay);
  // Serial.println("End of delay");
}

void loop(void)
{

  digitalWrite(INAVCC, HIGH); // switch on Ina3221
  delay(10);

  busvoltage = ina3221.getBusVoltage_V(SOLAR);

  while ((busvoltage < 12.0) || (busvoltage > 17.0))
  {
    busvoltage = ina3221.getBusVoltage_V(SOLAR);
  }

  current_mA_raspi = ina3221.getCurrent_mA(RASPI);
  current_mA_solar = ina3221.getCurrent_mA(SOLAR);

  // we save data during Transistor OFF only after receiving epoch
  // and only if iteration is inferior to arraysize to avoid overflow

  Data_transistor_Off[it0] = String(Transistor_State);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(Reason_Switch_Off);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(busvoltage);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(current_mA_raspi);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(current_mA_solar);
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(rtc.getEpoch());
  Data_transistor_Off[it0].concat(",");
  Data_transistor_Off[it0].concat(Epoch_Restart);

  Serial.println(Data_transistor_Off[it0]); //always print [0] if beginning
  delay(100);

  if ((Reason_Switch_Off > 0) && (it0 < ARRAYSIZE))
  {
    it0++; // the way to keep track of Data
  }

  if ((busvoltage <= Minimal_Voltage_To_Switch_On_Raspi) && (it2 == 0))

  {
    Minimal_Voltage_To_Switch_On_Raspi=Minimal_Voltage_To_Switch_On_Raspi + 0.1;
    Serial.print("New Minimal Voltage to switch on: ");
    Serial.println(Minimal_Voltage_To_Switch_On_Raspi);
    it2=1;
    // to add hysteresis
  }

  if (busvoltage > Minimal_Voltage_To_Switch_On_Raspi) // to get hysteresis
  {
    Minimal_Voltage_To_Switch_On_Raspi = Initial_Voltage_To_Switch_On_Raspi;
    it2=0;
    it1 = 0;

    while ((busvoltage > Minimal_Voltage_To_Switch_Off_Raspi) && (rtc.getEpoch() > Epoch_Restart)) // last condition to check daylight
    {
      if (it1 == 0)
      {
        digitalWrite(TRANSISTOR, HIGH);

        Transistor_State = 1;

        delay(100);

        //Serial.flush(); // maybe useless

        if (Reason_Switch_Off > 0)
        {
          digitalWrite(INAVCC, LOW); // switch on Ina3221
          while (Serial.readString() != "Raspi Ready sent")
          {
            //test with index at 0
            Serial.println(Data_transistor_Off[0]); // only to allow raspi to get the reboot parameter(>0)
          };
          digitalWrite(INAVCC, HIGH); // switch on Ina3221
          delay(100);

          Serial.println("ESP32 Ready received");
          delay(100);
          for (int i = 0; i < it0; i++)
          {
            Serial.println(Data_transistor_Off[i]);
          }
          delay(100); // critical
          Serial.println("End of transmission");
          delay(100); // critical
          it0 = 0;    // to avoid overflow
        
        }
      } // wait for raspi send data

      busvoltage = ina3221.getBusVoltage_V(SOLAR);

      while ((busvoltage < 12.0) || (busvoltage > 17.0))
      {
        busvoltage = ina3221.getBusVoltage_V(SOLAR);
      }

      current_mA_raspi = ina3221.getCurrent_mA(RASPI);
      current_mA_solar = ina3221.getCurrent_mA(SOLAR);

      Serial.print(Transistor_State);
      Serial.print(",");
      Serial.print(Reason_Switch_Off);
      Serial.print(",");
      Serial.print(busvoltage);
      Serial.print(",");
      Serial.print(current_mA_raspi);
      Serial.print(",");
      Serial.print(current_mA_solar);
      Serial.print(",");
      Serial.print(rtc.getEpoch());
      Serial.print(",");
      Serial.println(Epoch_Restart);

      delay(1000);// 100 ms was not enough; maybe 250 neither. 1000 should be ok.

      if (Serial.available() > 0)
      {
        String String_Sent=Serial.readStringUntil(',');

        if (String_Sent == "Epoch Sent")
        {
          Epoch_Now = Serial.readStringUntil(',').toInt();
          Epoch_Restart = Serial.readStringUntil(',').toInt(); // + Summer_Time; // reçoit un byte de pyhon et considéré comme String
          rtc.setTime(Epoch_Now);
          Serial.println("Epoch received");
          delay(100);
          while (Serial.readString() != "Double check OK")
          {
            Serial.println("Waiting dor double check");
            delay(100);
            
          }
          Serial.println("Double check received: switch off pi in 50 secs.");
          delay(50000);
          Reason_Switch_Off = 2;
          break;
        }
        else
        {
          Serial.print("Corrupted data: "); 
          Serial.println(String_Sent);
          delay(100);        
        }
      }

      if (busvoltage < Minimal_Voltage_To_Switch_Off_Raspi)
      {
        Reason_Switch_Off = 1;
        delay(50000);
        break;
      }

      // if (rtc.getEpoch() <= Epoch_Restart)
      // {

      // }

      //Serial.flush();
      it1++;
      gpio_hold_en(GPIO_NUM_27);
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_DAY * uS_TO_S_FACTOR);
      esp_light_sleep_start();
      gpio_hold_dis(GPIO_NUM_27);
    }
  }

  //Serial.flush();
  digitalWrite(TRANSISTOR, LOW);
  Transistor_State = 0;
  gpio_hold_en(GPIO_NUM_27);
  if (it0 > 5)
  {
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_NIGHT * uS_TO_S_FACTOR);
  }
  esp_light_sleep_start();
  gpio_hold_dis(GPIO_NUM_27);
}
