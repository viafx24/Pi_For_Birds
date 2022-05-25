#include <Arduino.h>

void setup(void)
{

  pinMode(27, OUTPUT); // for NPN
  digitalWrite(27, LOW);
  esp_sleep_enable_timer_wakeup(1000*1000*15);

}

void loop(void)
{

  digitalWrite(27, HIGH);
  gpio_hold_en(GPIO_NUM_27);
  esp_light_sleep_start();
  gpio_hold_dis(GPIO_NUM_27);
  digitalWrite(27, LOW);
  gpio_hold_en(GPIO_NUM_27);
  esp_light_sleep_start();
  gpio_hold_dis(GPIO_NUM_27);
}