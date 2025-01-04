#include <Arduino.h>
#include <virtuabotixRTC.h>
#include <LiquidCrystal_I2C.h>

class Time {
  public: Time(uint8_t hour, uint8_t minute);
  bool equal(virtuabotixRTC myRTC);
  void updateHour(uint8_t hour);
  void updateMinute(uint8_t minute);
  void print(LiquidCrystal_I2C lcd);
  private: void printPadded(LiquidCrystal_I2C lcd, int number);

  uint8_t hour;
  uint8_t minute;
};