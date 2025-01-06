#include <Time.h>

Time::Time(uint8_t hour, uint8_t minute) {
  this->hour = hour;
  this->minute = minute;
}

bool Time::equal(virtuabotixRTC &myRTC) {
  myRTC.updateTime();
  return (myRTC.hours == this->hour && myRTC.minutes == this->minute && myRTC.seconds == 0);
}

void Time::updateHour(uint8_t hour) {
  this->hour = hour;
}

void Time::updateMinute(uint8_t minute) {
  this->minute = minute;
}

void Time::print(LiquidCrystal_I2C &lcd) {
  this->printPadded(lcd, this->hour % 12 == 0 ? 12 : this->hour % 12);
  lcd.print(":");
  this->printPadded(lcd, this->minute);
  lcd.print(this->hour < 12 ? " AM" : " PM");
}

void Time::printPadded(LiquidCrystal_I2C &lcd, int number) {
  if (number < 10) lcd.print(0);
  lcd.print(number);
}