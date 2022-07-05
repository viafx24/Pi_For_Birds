
// test new commit
//version sans LDR ni statistic mais ajout epoch

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
            Serial.println(Data_transistor_Off[1]); // only to allow raspi to get the reboot parameter(>0)
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
          // delay(5000);
          // delay(250);
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
        else if (String_Sent == "Break Sent")
        {
          //delay(5000);
          Serial.println("Break received");
          delay(100);
          while (Serial.readString() != "Double check OK")
          {
            Serial.println("Waiting dor double check");
            delay(100);
            
          }
          Serial.println("Double check received: switch off pi in 50 secs.");
          delay(100);
          delay(50000);
          Reason_Switch_Off = 1;
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

// #include <Arduino.h>
// #include <ESP32Time.h>

// long Epoch_Now = 0;
// long Epoch_Restart = 0;

// ESP32Time rtc(0);

// void setup(void)
// {
//   //rtc.setTime(1653592210);
//   rtc.setTime(10, 53, 23, 25, 5, 2022);
//   esp_sleep_enable_timer_wakeup(10 * 1000000);
//   Serial.begin(9600);
// }

// void loop(void)
// {
//   Serial.print(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
//   Serial.print(',');
//   Serial.print(rtc.getEpoch()); //  (long)    1609459200
//   Serial.print(',');
//   Serial.print(Epoch_Now);
//   Serial.print(',');
//   Serial.println(Epoch_Restart);
//   delay(100);

//   if (Serial.available() > 0)
//   {

//     Epoch_Now = Serial.readStringUntil(',').toInt();
//     Epoch_Restart = Serial.readStringUntil(',').toInt(); // reçoit un byte de pyhon et considéré comme String
//     rtc.setTime(Epoch_Now);
//   }

//   esp_light_sleep_start();
// }

// #include <Arduino.h>
// #include <Wire.h>
// #include <SDL_Arduino_INA3221.h>
// #include <Statistic.h>

// // All constants

// const int I2C_SDA=33;
// const int I2C_SCL=32;

// const long uS_TO_S_FACTOR=1000000; /* Conversion factor for micro seconds to seconds */
// const int TIME_TO_SLEEP=1 * 30;   /* Time ESP32 will go to sleep (in seconds) */

// const int TRANSISTOR = 27; // for NPN (to switch on the base)
// const int LDRVCC = 18;  // for vcc 3.3V for LDR/light sensing
// const int INAVCC = 26;
// const int LDR_ANALOG = 15; // analog reading of ambiant light

// const int RASPI=1;
// const int SOLAR=3;

// const long Initial_Delay=60000;

// // all variables

// float Minimal_Voltage_To_Switch_On_Raspi = 15.5;  // volt
// float Minimal_Voltage_To_Switch_Off_Raspi = 14.5; //volt

// float Incoherent_Voltage = 17 ; //volt

// float busvoltage = 0;
// float current_mA_solar = 0;
// float current_mA_raspi = 0;

// float LDR = 0;
// float LDR_Average = 0; //At beginning, fix to zero to allow the loop
// float LDR_TRESHOLD = 3000;

// float Voltage_Average;

// // All objects

// Statistic LDR_Array;
// Statistic Voltage_Array;
// SDL_Arduino_INA3221 ina3221;

// void setup(void)
// {

//   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

//   pinMode(TRANSISTOR, OUTPUT); // for NPN
//   digitalWrite(TRANSISTOR, LOW);

//   pinMode(INAVCC, OUTPUT);    //Use as VCC for ina3221
//   digitalWrite(INAVCC, HIGH); // switch on Ina3221

//   pinMode(LDRVCC, OUTPUT);    //Use as VCC for LDR
//   digitalWrite(LDRVCC, HIGH); //

//   pinMode(LDR_ANALOG, INPUT);

//   delay(10); // needed to get correct first value of INA3221

//   Wire.begin(I2C_SDA, I2C_SCL);
//   Serial.begin(9600);
//   ina3221.begin();

//   LDR_Array.clear();
//   Voltage_Array.clear();

//   Serial.println((String)"Delay of "+ Initial_Delay/1000 + " seconds");
//   delay(Initial_Delay);
//   Serial.println("End of delay");
// }

// void loop(void)
// {

//   digitalWrite(INAVCC, HIGH); // switch on Ina3221
//   digitalWrite(LDRVCC, HIGH); //

//   delay(10);
//   busvoltage = ina3221.getBusVoltage_V(SOLAR);
//   current_mA_solar = ina3221.getCurrent_mA(SOLAR);
//   LDR = analogRead(LDR_ANALOG);

//   while ((busvoltage > 12.0) || (busvoltage < 17.0))
//       {
//         busvoltage = ina3221.getBusVoltage_V(SOLAR);
//       }

//   Serial.println(current_mA_solar);
//   Serial.println(LDR);

//   if ((busvoltage > Minimal_Voltage_To_Switch_On_Raspi) && (LDR < LDR_TRESHOLD)) // last condition to check daylight
//   {

//     digitalWrite(TRANSISTOR, HIGH);
//     LDR_Average=LDR;
//     Voltage_Average=busvoltage;

//     LDR_Array.clear();
//     Voltage_Array.clear();

//     // use LDR_Average rather than LDR in the loop to avoid accident of light transitory switch off
//     while ((Voltage_Average > Minimal_Voltage_To_Switch_Off_Raspi) && (LDR_Average < LDR_TRESHOLD))
//     {
//       Serial.flush();

//       while ((busvoltage < 12.0) || (busvoltage < 17.0))
//       {
//         busvoltage = ina3221.getBusVoltage_V(SOLAR);
//       }
//       current_mA_raspi = ina3221.getCurrent_mA(RASPI);
//       current_mA_solar = ina3221.getCurrent_mA(SOLAR);

//       LDR = analogRead(LDR_ANALOG);

//       // take a average all 10 measures thus for 1 min = 10 min.
//       if (LDR_Array.count() <= 10)
//       {
//         LDR_Array.add(LDR);
//         Voltage_Array.add(busvoltage);
//       }
//       else
//       {
//         LDR_Average = LDR_Array.average();
//         LDR_Array.clear();

//         Voltage_Average=Voltage_Array.average();
//         Voltage_Array.clear();
//       }

//       Serial.print(bootCount);
//       Serial.print(",");
//       Serial.print(Reason_Switch_Off);
//       Serial.print(",");
//       Serial.print(busvoltage);
//       Serial.print(",");
//       Serial.println(Voltage_Average);
//       Serial.print(",");
//       Serial.print(current_mA_raspi);
//       Serial.print(",");
//       Serial.print(current_mA_solar);
//       Serial.print(",");
//       Serial.print(LDR);
//       Serial.print(",");
//       Serial.print(LDR_Average);

//       // the goal is to know why the esp32 switch off at the previous run. 1 is voltage, 2: LDR_Average
//       // if Reason_Switch_Off=0 after reboot>0, there is a problem.
//       if (Voltage_Average <= Minimal_Voltage_To_Switch_Off_Raspi)
//       {
//         Reason_Switch_Off = 1;
//         bootCount++;
//       }

//       if (LDR_Average >= LDR_TRESHOLD)
//       {
//         Reason_Switch_Off = 2;
//         bootCount++;
//       }

//       delay(250);
//     }
//   }

//   else
//   {
//     digitalWrite(TRANSISTOR, LOW);
//     digitalWrite(INAVCC, LOW); // switch off Ina3221
//     digitalWrite(LDRVCC, LOW); //

//     Serial.flush();

//     Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
//                    " Seconds");
//     Serial.println("Going to sleep now");
//     Serial.flush();
//     esp_light_sleep_start();
//   }
// }

// #include <Arduino.h>
// #include <Wire.h>
// #include <SDL_Arduino_INA3221.h>
// #include <Statistic.h>

// #define I2C_SDA 33
// #define I2C_SCL 32

// SDL_Arduino_INA3221 ina3221;

// // the three channels of the INA3221 named for SunAirPlus Solar Power Controller channels (www.switchdoc.com)
// #define RASPI 1
// //#define ESP32 2
// #define SOLAR 3

// #define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
// #define TIME_TO_SLEEP 1 * 30   /* Time ESP32 will go to sleep (in seconds) */

// //#define TIME_TO_SLEEP 1 * 10  /* testing purpose */

// RTC_DATA_ATTR int bootCount = 0;
// RTC_DATA_ATTR int Reason_Switch_Off = 0;

// const int TRANSISTOR = 27; // for NPN (to switch on the base)
// const int LDRVCC = 18;  // for vcc 3.3V for LDR/light sensing
// const int ACCVCC = 19;  // for vcc 3.3V for accelerometer

// const int LDR_ANALOG = 15; // analog reading of ambiant light
// const float LDR_TRESHOLD = 3000;
// const int ACC_X = 2;
// const int ACC_Y = 0;
// const int ACC_Z = 4;

// //const int delay_Switch_ON=1000*20; //sec
// const int delay_Switch_ON = 0; //milli fois sec

// const float Minimal_Voltage_To_Switch_On_Raspi = 15.5;  // volt
// const float Minimal_Voltage_To_Switch_Off_Raspi = 14.5; //volt

// // for testing purpose
// //const float Minimal_Voltage_To_Switch_Off_Raspi = 14.1; //volt

// const float Incoherent_Voltage = 17 ; //volt

// // those two objects are used to remove outlier by use of the mean. (outlier of voltage and human error for LDR)
// Statistic LDR_Array;
// Statistic Voltage_Array;

// float shuntvoltage1 = 0;
// float busvoltage1 = 0;
// float current_mA_raspi = 0;
// float loadvoltage1 = 0;

// float shuntvoltage3 = 0; // to enter while loop
// float busvoltage = 0;
// float current_mA_solar = 0;
// float loadvoltage3 = 0;

// float LDR = 0;
// float LDR_Average = 0; //At beginning, fix to zero to allow the loop
// float Voltage_Average;

// int X = 0;
// int Y = 0;
// int Z = 0;

// bool PIR;
// int Count_Trigger_PIR = 0;

// void setup(void)
// {

//   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

//   pinMode(26, OUTPUT);    //Use as VCC for ina3221
//   digitalWrite(26, HIGH); // switch on Ina3221

//   pinMode(LDRVCC, OUTPUT);    //Use as VCC for LDR
//   digitalWrite(LDRVCC, HIGH); //

//   pinMode(ACCVCC, OUTPUT); //Use as VCC for acc
//   digitalWrite(ACCVCC, HIGH);

//   pinMode(LDR_ANALOG, INPUT);
//   pinMode(ACC_X, INPUT);
//   pinMode(ACC_Y, INPUT);
//   pinMode(ACC_Z, INPUT);

//   delay(10); // needed to get correct first value of INA3221

//   Wire.begin(I2C_SDA, I2C_SCL);
//   Serial.begin(9600);
//   ina3221.begin();

//   //pinMode(26, OUTPUT);      //Use as VCC for PIR (not use anymore)
//   pinMode(25, OUTPUT); //PIR mode
//   pinMode(34, INPUT);  // PIR reading
//   digitalWrite(25, 1); // mode PIR =1 (0 may be interesting as well)
//   //digitalWrite(26, 1);      //Use as VCC for PIR
//   pinMode(TRANSISTOR, OUTPUT); // for NPN
//   digitalWrite(TRANSISTOR, LOW);

//   //delay(2000);
//   LDR_Array.clear();
//   Voltage_Array.clear();
// delay(10000);
// Serial.println("Delay of 1 minute begin");
// delay(60000);
// Serial.println("End of delay");
// }

// void loop(void)
// {

//   digitalWrite(26, HIGH); // switch on Ina3221
//   digitalWrite(LDRVCC, HIGH); //
//   digitalWrite(ACCVCC, HIGH);
//   digitalWrite(25, 1); // mode PIR =1 (0 may be interesting as well)

//   delay(10);
//   busvoltage = ina3221.getBusVoltage_V(SOLAR);
//   current_mA_solar = ina3221.getCurrent_mA(SOLAR);
//   LDR = analogRead(LDR_ANALOG);

//   Serial.println(busvoltage);
//   Serial.println(current_mA_solar);
//   Serial.println(LDR);

//   if ((busvoltage > Minimal_Voltage_To_Switch_On_Raspi) && (busvoltage < Incoherent_Voltage) && (LDR < LDR_TRESHOLD)) // last condition to check daylight
//   {

//     digitalWrite(TRANSISTOR, HIGH);
//     LDR_Average=LDR;
//     Voltage_Average=busvoltage;

//     LDR_Array.clear();
//     Voltage_Array.clear();

//     // use LDR_Average rather than LDR in the loop to avoid accident of light transitory switch off
//     while ((Voltage_Average > Minimal_Voltage_To_Switch_Off_Raspi) && (LDR_Average < LDR_TRESHOLD))
//     {
//       Serial.flush();
//       //pinMode(TRANSISTOR, OUTPUT);
//       // switch on the raspberry py (base of NPN transistor)

//       busvoltage1 = ina3221.getBusVoltage_V(RASPI);
//       shuntvoltage1 = ina3221.getShuntVoltage_mV(RASPI);
//       current_mA_raspi = ina3221.getCurrent_mA(RASPI); // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
//       loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);

//       busvoltage = ina3221.getBusVoltage_V(SOLAR);
//       shuntvoltage3 = ina3221.getShuntVoltage_mV(SOLAR);
//       current_mA_solar = ina3221.getCurrent_mA(SOLAR);
//       loadvoltage3 = busvoltage + (shuntvoltage3 / 1000);

//       X = analogRead(ACC_X); //read from xpin
//       delay(1);              //
//       Y = analogRead(ACC_Y); //read from ypin
//       delay(1);
//       Z = analogRead(ACC_Z); //read from zpin

//       LDR = analogRead(LDR_ANALOG);

//       // take a average all 200 measures thus for 250ms = 50 seconds. first average is fixed at zero.
//       if (LDR_Array.count() <= 200)
//       {
//         LDR_Array.add(LDR);
//         Voltage_Array.add(busvoltage);
//       }
//       else
//       {
//         LDR_Average = LDR_Array.average();
//         LDR_Array.clear();

//         Voltage_Average=Voltage_Array.average();
//         Voltage_Array.clear();
//       }

//       PIR = digitalRead(34);

//       if (PIR == 1)
//       {
//         Count_Trigger_PIR++;
//       }

//       Serial.print(bootCount);
//       Serial.print(",");
//       Serial.print(Reason_Switch_Off);
//       Serial.print(",");
//       Serial.print(loadvoltage1);
//       Serial.print(",");
//       Serial.print(current_mA_raspi);
//       Serial.print(",");
//       Serial.print(loadvoltage3);
//       Serial.print(",");
//       Serial.print(current_mA_solar);
//       Serial.print(",");
//       Serial.print(LDR);
//       Serial.print(",");
//       Serial.print(LDR_Average);
//       Serial.print(",");
//       Serial.print(X);
//       Serial.print(",");
//       Serial.print(Y);
//       Serial.print(",");
//       Serial.print(Z);
//       Serial.print(",");
//       Serial.print(PIR);
//       Serial.print(",");
//       //Serial.println(Count_Trigger_PIR);//change to the next line to correct bug and avoid modif of python
//       Serial.println(Voltage_Average);

//       // the goal is to know why the esp32 switch off at the previous run. 1 is voltage, 2: LDR_Average
//       // if Reason_Switch_Off=0 after reboot>0, there is a problem.
//       if (Voltage_Average <= Minimal_Voltage_To_Switch_Off_Raspi)
//       {
//         Reason_Switch_Off = 1;
//         bootCount++;
//       }

//       if (LDR_Average >= LDR_TRESHOLD)
//       {
//         Reason_Switch_Off = 2;
//         bootCount++;
//       }

//       delay(250);
//     }
//   }

//   else
//   {
//     digitalWrite(TRANSISTOR, LOW);
//     digitalWrite(26, LOW); // switch off Ina3221

//     digitalWrite(LDRVCC, LOW); //
//     digitalWrite(ACCVCC, LOW);

//     Serial.flush();

//     Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
//                    " Seconds");
//     //Serial.println("Going to sleep in 20 sec");
//     delay(delay_Switch_ON);
//     Serial.println("Going to sleep now");
//     Serial.flush();
//     esp_light_sleep_start();
//   }
// }

// #include <Arduino.h>

// uint8_t Integer_1 = 0;
// uint8_t Integer_2 = 0;
// String Launch = "0";

// void setup(void)
// {

//   Serial.begin(9600);
// }

// void loop(void)
// {
//   //Serial.println("test");
//   //Serial.println("test");
//   //Serial.println(Serial.available());
//   delay(100);
//   if (Serial.available() > 0)
//   {
//     // Serial.println("test_3");
//     Launch = Serial.readString(); // reçoit un byte de pyhon et considéré comme String

//     Integer_1 = Launch.toInt();
//     //Serial.println("test"); // transformé en entier
//     //Serial.println(Integer_1);
//     if (Integer_1 == 1)
//     {
//       //Integer_2 = 1;
//       Serial.println("I received 1");
//     }
//     else if (Integer_1 == 0)
//     {
//       //Integer_2 = 0;
//       Serial.println("I received 0");
//     }
//   }
// }

// script qui a pour but de maintenir esp32 et raspi uniquement quand le voltage detecté par ina3221 reste au
// dessus d'une certaine valeur (par exemple 14V) et qu'il fait jour
// (par exemple courant positif arrivant du panneau ou négatif)
// le script gère aussi le PIR de makeblock sans la librairie. allume un NPN de puissance et rentre en deep sleep
// si le voltage passe en dessous d'un certain votlage ce qui coupe le raspi. tout est donc dans la fonction setup.A0

// #include <Arduino.h>

// void setup(void)
// {

//   pinMode(27, OUTPUT); // for NPN
//   digitalWrite(27, LOW);
//   esp_sleep_enable_timer_wakeup(1000*1000*15);

// }

// void loop(void)
// {

//   digitalWrite(27, HIGH);
//   gpio_hold_en(GPIO_NUM_27);
//   esp_light_sleep_start();
//   gpio_hold_dis(GPIO_NUM_27);
//   digitalWrite(27, LOW);
//   gpio_hold_en(GPIO_NUM_27);
//   esp_light_sleep_start();
//   gpio_hold_dis(GPIO_NUM_27);
// }
