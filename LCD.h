#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd2(0x27,20,4);
LiquidCrystal_I2C lcd1(0x26,20,4);

byte bus1[] = { B00111, B00100, B00100, B00100, B00111, B01100, B00111, B00000 };
byte bus2[] = { B11111, B10001, B10001, B10001, B11111, B10111, B11101, B00111 };
byte bus3[] = { B11111, B00010, B00010, B00010, B11111, B11100, B10111, B11100 };
byte bus4[] = { B11111, B00100, B00100, B00100, B11111, B00000, B11111, B00000 };
byte bus5[] = { B11111, B10101, B10101, B10101, B10101, B10101, B11111, B00000 };
byte bus6[] = { B11111, B01000, B01000, B01000, B11111, B00111, B11101, B00111 };
byte bus7[] = { B11110, B10001, B10001, B10001, B10011, B10001, B11111, B00000 };
byte bus8[] = { B00000, B11000, B01000, B00000, B00000, B00000, B00000, B00000 };

void LCDprintBus(int displej, int x, int y);
void LCDanimateBus();

int X = 0, Y = 0;


void LCDsetup() {
  lcd1.init();
  lcd2.init();
  lcd1.backlight();
  lcd2.backlight();

  lcd1.createChar(0, bus1);
  lcd1.createChar(1, bus2);
  lcd1.createChar(2, bus3);
  lcd1.createChar(3, bus4);
  lcd1.createChar(4, bus5);
  lcd1.createChar(5, bus6);
  lcd1.createChar(6, bus7);
  lcd1.createChar(7, bus8);

  lcd2.createChar(0, bus1);
  lcd2.createChar(1, bus2);
  lcd2.createChar(2, bus3);
  lcd2.createChar(3, bus4);
  lcd2.createChar(4, bus5);
  lcd2.createChar(5, bus6);
  lcd2.createChar(6, bus7);
  lcd2.createChar(7, bus8);

  LCDprintBus(1, 6, 2);
}

void LCDprintData() {
  lcd1.setCursor(8,3);
  lcd1.print("            ");
  lcd1.setCursor(8,3);
  lcd1.print(int(vehicles[nejBus].vehicle_delay / 60));
  lcd1.print("m ");
  lcd1.print(vehicles[nejBus].vehicle_delay%60);
  lcd1.print("s   ");
  lcd2.setCursor(6,0);
  lcd2.print("              ");
  lcd2.setCursor(6,0);
  lcd2.print(removeDiacritics(vehicles[nejBus].last_stop, 14));
  lcd2.setCursor(6,1);
  lcd2.print("              ");
  lcd2.setCursor(6,1);
  lcd2.print(removeDiacritics(vehicles[nejBus].next_stop, 14));
  if(vypnout == 1) {lcd1.clear(); lcd2.clear(); lcd1.noBacklight(); lcd2.noBacklight(); esp_deep_sleep_start();}
}

void LCDbackground() {
    lcd1.clear();
    lcd2.backlight();
    lcd1.setCursor(0,0);
    lcd1.print("369 Kly,Vinice ");
    lcd1.print(removeDiacritics(vehicles[nejBus].kly_arrival_time, 5));
    lcd1.setCursor(3,1);
    lcd1.print(removeDiacritics(vehicles[nejBus].trip_headsign, 11));
    lcd1.setCursor(15,1);
    lcd1.print(removeDiacritics(vehicles[nejBus].arrival_time, 5));
    lcd1.setCursor(0,3);
    lcd1.print("Delay: ");
    lcd2.setCursor(0,0);
    lcd2.print("Last: ");
    lcd2.setCursor(0,1);
    lcd2.print("Next: ");
    getVehicleType(vehicles[nejBus]);
    lcd2.setCursor(0,3);
    lcd2.print(removeDiacritics(vehicles[nejBus].vehicle_type, 20));
    if(vypnout == 1) {lcd1.clear(); lcd2.clear(); lcd1.noBacklight(); lcd2.noBacklight(); esp_deep_sleep_start();}
}

void nejede() {
    Serial.println("Nejede :(");
    lcd1.clear();
    lcd2.clear();
    lcd2.noBacklight();
    if(vypnout == 1) {lcd1.clear(); lcd2.clear(); lcd1.noBacklight(); lcd2.noBacklight(); esp_deep_sleep_start();}
}

void LCDprintBus(int displej, int x, int y) {

  if(displej == 1) {

    lcd1.setCursor(x,y);
    lcd1.write(byte(0));
    lcd1.write(byte(1));
    lcd1.write(byte(2));
    lcd1.write(byte(3));
    lcd1.write(byte(4));
    lcd1.write(byte(5));
    lcd1.write(byte(6));
    lcd1.write(byte(7));

  }
  if(displej == 2) {

    lcd2.setCursor(x,y);
    lcd2.write(byte(0));
    lcd2.write(byte(1));
    lcd2.write(byte(2));
    lcd2.write(byte(3));
    lcd2.write(byte(4));
    lcd2.write(byte(5));
    lcd2.write(byte(6));
    lcd2.write(byte(7));

  }
  if(vypnout == 1) {lcd1.clear(); lcd2.clear(); lcd1.noBacklight(); lcd2.noBacklight(); esp_deep_sleep_start();}
}

void LCDanimateBus() {

  for(int i = 0; i < 50; i++) {
    lcd1.setCursor(X,Y);
    X++;
    lcd1.print("        ");
    lcd1.setCursor(X,Y);
    LCDprintBus(1, X, Y);
    delay(2000);
  }
}
