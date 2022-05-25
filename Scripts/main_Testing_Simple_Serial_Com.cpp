#include <Arduino.h>

uint8_t Integer_1 = 0;
uint8_t Integer_2 = 0;
String Launch = "0";

void setup(void)
{

  Serial.begin(9600);
}

void loop(void)
{
  //Serial.println("test");
  //Serial.println("test");
  //Serial.println(Serial.available());
  delay(100);
  if (Serial.available() > 0)
  {
    // Serial.println("test_3");
    Launch = Serial.readString(); // reçoit un byte de pyhon et considéré comme String

    Integer_1 = Launch.toInt();
    //Serial.println("test"); // transformé en entier
    //Serial.println(Integer_1);
    if (Integer_1 == 1)
    {
      //Integer_2 = 1;
      Serial.println("I received 1");
    }
    else if (Integer_1 == 0)
    {
      //Integer_2 = 0;
      Serial.println("I received 0");
    }
  }
}

// // script qui a pour but de maintenir esp32 et raspi uniquement quand le voltage detecté par ina3221 reste au
// // dessus d'une certaine valeur (par exemple 14V) et qu'il fait jour
// // (par exemple courant positif arrivant du panneau ou négatif)
// // le script gère aussi le PIR de makeblock sans la librairie. allume un NPN de puissance et rentre en deep sleep
// // si le voltage passe en dessous d'un certain votlage ce qui coupe le raspi. tout est donc dans la fonction setup.A0

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

// const int GPIOPIN = 27; // for NPN (to switch on the base)
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
// float current_mA1 = 0;
// float loadvoltage1 = 0;

// float shuntvoltage3 = 0; // to enter while loop
// float busvoltage3 = 0;
// float current_mA3 = 0;
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
//   pinMode(GPIOPIN, OUTPUT); // for NPN
//   digitalWrite(GPIOPIN, LOW);

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
//   busvoltage3 = ina3221.getBusVoltage_V(SOLAR);
//   current_mA3 = ina3221.getCurrent_mA(SOLAR);
//   LDR = analogRead(LDR_ANALOG);

//   Serial.println(busvoltage3);
//   Serial.println(current_mA3);
//   Serial.println(LDR);

//   if ((busvoltage3 > Minimal_Voltage_To_Switch_On_Raspi) && (busvoltage3 < Incoherent_Voltage) && (LDR < LDR_TRESHOLD)) // last condition to check daylight
//   {

//     digitalWrite(GPIOPIN, HIGH);
//     LDR_Average=LDR;
//     Voltage_Average=busvoltage3;

//     LDR_Array.clear();
//     Voltage_Array.clear();

//     // use LDR_Average rather than LDR in the loop to avoid accident of light transitory switch off
//     while ((Voltage_Average > Minimal_Voltage_To_Switch_Off_Raspi) && (LDR_Average < LDR_TRESHOLD))
//     {
//       Serial.flush();
//       //pinMode(GPIOPIN, OUTPUT);
//       // switch on the raspberry py (base of NPN transistor)

//       busvoltage1 = ina3221.getBusVoltage_V(RASPI);
//       shuntvoltage1 = ina3221.getShuntVoltage_mV(RASPI);
//       current_mA1 = ina3221.getCurrent_mA(RASPI); // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
//       loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);

//       busvoltage3 = ina3221.getBusVoltage_V(SOLAR);
//       shuntvoltage3 = ina3221.getShuntVoltage_mV(SOLAR);
//       current_mA3 = ina3221.getCurrent_mA(SOLAR);
//       loadvoltage3 = busvoltage3 + (shuntvoltage3 / 1000);

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
//         Voltage_Array.add(busvoltage3);
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
//       Serial.print(current_mA1);
//       Serial.print(",");
//       Serial.print(loadvoltage3);
//       Serial.print(",");
//       Serial.print(current_mA3);
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
//     digitalWrite(GPIOPIN, LOW);
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
