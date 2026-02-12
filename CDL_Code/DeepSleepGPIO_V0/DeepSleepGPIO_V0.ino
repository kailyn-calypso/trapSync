    /////////////////////////////////////////////////////////////////
   //             ESP32 Deep Sleep Example 1             v1.00    //
  //       Get the latest version of the code here:              //
 //       http://educ8s.tv/esp32-deep-sleep-tutorial            //
 ////////////////////////////////////////////////////////////////
        //\\
       //||\\
      // || \\
     //__()__\\
  
 //     Make sure to reupload by connecting ground to bootloader (B1) 
 //and pressing on reset, unplugging while reset is pressed, 
 //and wait till the green light is pulsing (ONLY GREEN NOT OTHER COLORS)
/////////////////////////////////////////////////////////////////

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3        /* Time ESP32 will go to sleep (in seconds) */
#define WAKEUP_GPIO 2     // Only RTC IO are allowed
RTC_DATA_ATTR int bootCount = 0;

int GREEN_LED_PIN = 6;
int YELLOW_LED_PIN = 7;

void setup(){

  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(YELLOW_LED_PIN,OUTPUT);
  delay(500);
  
  if(bootCount == 0) //Run this only the first time
  {
      digitalWrite(YELLOW_LED_PIN,HIGH);
      bootCount = bootCount+1;
  }else
  {
      digitalWrite(GREEN_LED_PIN,HIGH);
  }
  
  delay(3000);

  digitalWrite(GREEN_LED_PIN,LOW);
  digitalWrite(YELLOW_LED_PIN,LOW);

  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 0);  // 1 = High, 0 = Low
  rtc_gpio_pullup_dis(WAKEUP_GPIO);
  esp_deep_sleep_start();
}

void loop(){
  
}
