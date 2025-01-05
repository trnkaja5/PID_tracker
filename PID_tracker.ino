
#include "PID_API.h"
#include "LCD.h"

#define T 26

void IRAM_ATTR vypni();

void setup() {

  Serial.begin(115200);
  pinMode(T, INPUT_PULLUP);
  vypnout = 0;
  LCDsetup();
  wifiSetup();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,0);
  attachInterrupt(T, vypni, FALLING);

  nejBus = najdiBus();
}

void loop() {

  if(nejBus != 666) {

    LCDbackground();                          // vypis pozadi

    X = 0;

    while(getZone(vehicles[nejBus]) > 3) {

      LCDprintData();                         // vypis dat

      delay(10000);
      getVehicleInfo(vehicles[nejBus]);   // aktualizace
    }
    nejBus = najdiBus();
  }
  if(nejBus == 666) {

    nejede();
  }

  while(nejBus == 666) {
    LCDanimateBus();        // to zabere asi 100 sekund
    nejBus = najdiBus();
  }
}


void IRAM_ATTR vypni() {
  if(vypnout == 0) {
    Serial.println("vypinam...");
    vypnout = 1;
  }
}




