

#include <ESP32Time.h>

//ESP32Time rtc;
//ESP32Time rtc(3600);  // offset in seconds GMT+1
ESP32Time rtc(0);  // offset in seconds GMT+1

void setup() {
  Serial.begin(9600);
  rtc.setTime(10, 53, 23, 25, 5, 2022);  // 17th Jan 2021 15:24:30

  esp_sleep_enable_timer_wakeup(30 * 1000000);
  //rtc.setTime(1609459200);  // 1st Jan 2021 00:00:00
  //rtc.offset = 7200; // change offset value

/*---------set with NTP---------------*/
//  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
//  struct tm timeinfo;
//  if (getLocalTime(&timeinfo)){
//    rtc.setTimeStruct(timeinfo); 
//  }
}

void loop() {

  
  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S")); 
//  Serial.println(rtc.getTime());          //  (String) 15:24:38
//  Serial.println(rtc.getDate());          //  (String) Sun, Jan 17 2021
//  Serial.println(rtc.getDate(true));      //  (String) Sunday, January 17 2021
//  Serial.println(rtc.getDateTime());      //  (String) Sun, Jan 17 2021 15:24:38
//  Serial.println(rtc.getDateTime(true));  //  (String) Sunday, January 17 2021 15:24:38
//  Serial.println(rtc.getTimeDate());      //  (String) 15:24:38 Sun, Jan 17 2021
//  Serial.println(rtc.getTimeDate(true));  //  (String) 15:24:38 Sunday, January 17 2021
//
//  Serial.println(rtc.getMicros());        //  (long)    723546
//  Serial.println(rtc.getMillis());        //  (long)    723
  Serial.println(rtc.getEpoch());         //  (long)    1609459200
//  Serial.println(rtc.getSecond());        //  (int)     38    (0-59)
//  Serial.println(rtc.getMinute());        //  (int)     24    (0-59)
//  Serial.println(rtc.getHour());          //  (int)     3     (0-12)
//  Serial.println(rtc.getHour(true));      //  (int)     15    (0-23)
//  Serial.println(rtc.getAmPm());          //  (String)  pm
//  Serial.println(rtc.getAmPm(true));      //  (String)  PM
//  Serial.println(rtc.getDay());           //  (int)     17    (1-31)
//  Serial.println(rtc.getDayofWeek());     //  (int)     0     (0-6)
//  Serial.println(rtc.getDayofYear());     //  (int)     16    (0-365)
//  Serial.println(rtc.getMonth());         //  (int)     0     (0-11)
//  Serial.println(rtc.getYear());          //  (int)     2021

  Serial.println(rtc.getLocalEpoch());         //  (long)    1609459200 epoch without offset
    // (String) returns time with specified format 
  // formating options  http://www.cplusplus.com/reference/ctime/strftime/


  struct tm timeinfo = rtc.getTimeStruct();
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");   //  (tm struct) Sunday, January 17 2021 07:24:38

  delay(1000);
  esp_light_sleep_start();
  
  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S")); 
  //delay(1000);
}



























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
