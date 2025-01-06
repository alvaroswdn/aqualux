#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <virtuabotixRTC.h>
#include <Time.h>
#include <SD.h>

#include <build_time.h>
#include <pins.h>
#include <adjustments.h>

OneWire tempWire(TEMP_PIN);
DallasTemperature tempSensor(&tempWire);
LiquidCrystal_I2C lcd(0x27, 20, 4);
virtuabotixRTC myRTC(RTC_CLK_PIN, RTC_DAT_PIN, RTC_RST_PIN);
Servo servo;

// Sensor reading data values
int ldrValue, tempValue, tdsValue, potentioValue;
bool buttonPressed = false;
Time feedingTime(0, 0);

// Current page for LCD menu
uint8_t menuPage = 1;

// Last time the LCD was updated
unsigned long lastUpdate = -1;

File dataFile;

// Define utility functions
int readPotentio(int max);
int readTDS();
int readTemp();
void readSensors();
void printData();
void handleButtonPress();
void handleLight();
void handleFeeding();

void setup() {
  Serial.begin(9600);

  //Initialize input and output devices
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //Initialize the servo
  servo.attach(SERVO_PIN);
  servo.write(0); //Initial Position

  //Initialize the LCD
  lcd.init();
  lcd.backlight();

  if (!SD.begin(SD_CHIP_SELECT_PIN)) {
    lcd.setCursor(0, 1);
    lcd.print("Can't load SD card");
    lcd.setCursor(0, 2);
    lcd.print("Insert SD card..."); 
    while (true);
  }

  dataFile = SD.open(FILE_NAME, FILE_WRITE);

  myRTC.setDS1302Time(
    BUILD_SEC + 9, BUILD_MIN, BUILD_HOUR, 
    0, BUILD_DAY, BUILD_MONTH, BUILD_YEAR
  );
}

void loop() {
  if (buttonPressed != !digitalRead(BUTTON_PIN)) {
    buttonPressed = !digitalRead(BUTTON_PIN);
    if (buttonPressed) handleButtonPress();
  }

  if (menuPage == 1 && (millis() - lastUpdate >= 1000)) {
    readSensors();
    printData();
    handleLight();

    lastUpdate = millis();
  } else if (menuPage == 2) {
    if (potentioValue != readPotentio(23)) {
      potentioValue = readPotentio(23);
      feedingTime.updateHour(potentioValue);
    }

    lcd.setCursor(0, 1);
    lcd.print("Set Hour");
    lcd.setCursor(0, 2);
    feedingTime.print(lcd);
  } else if (menuPage == 3) {
    if (potentioValue != readPotentio(59)) {
      potentioValue = readPotentio(59);
      feedingTime.updateMinute(potentioValue);
    }

    lcd.setCursor(0, 1);
    lcd.print("Set Minute");
    lcd.setCursor(0, 2);
    feedingTime.print(lcd);
  }

  handleFeeding();
  dataFile.flush();
}

int readPotentio(int max) {
  return map(
    analogRead(POTENTIO_PIN), 
    POTENTIO_MIN, POTENTIO_MAX, 
    0, max
  );
}

int readTDS() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = (analogValue / AN_DC) * VOLT;
  return (voltage * CAL_FAC) * 1000;
}

int readTemp() {
  tempSensor.requestTemperatures();
  return tempSensor.getTempCByIndex(0);
}

void readSensors() {
  tempValue = readTemp();
  tdsValue = readTDS();
  ldrValue = analogRead(LDR_PIN);
}

void printData() {
  lcd.setCursor(6, 0);
  lcd.print("AQUALUX");

  lcd.setCursor(0, 1);
  lcd.print("Temperature: ");
  lcd.print(tempValue);
  lcd.print(" C  ");

  lcd.setCursor(0, 2);
  lcd.print("Murkiness: ");
  lcd.print(tdsValue);
  lcd.print(" ppm  ");

  lcd.setCursor(0, 3);
  lcd.print("Feeding at ");
  feedingTime.print(lcd);
  lcd.print("    ");

  myRTC.updateTime();
  dataFile.print("[");
  dataFile.print(myRTC.hours);
  dataFile.print(":");
  dataFile.print(myRTC.minutes);
  dataFile.print(":");
  dataFile.print(myRTC.seconds);
  dataFile.print("] Temperature=");
  dataFile.print(tempValue);
  dataFile.print(" Murkiness=");
  dataFile.print(tdsValue);
  dataFile.println();
}

void handleButtonPress() {
  if (menuPage == 1) {
    potentioValue = readPotentio(23);
  } else if (menuPage == 2) {
    potentioValue = readPotentio(59);
  } else if (menuPage >= 3) {
    menuPage = 0;
  }

  menuPage++;
  lcd.clear();
}

void handleLight() {
  if (ldrValue < LIGHT_BASE) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}

void handleFeeding() {
  if (feedingTime.equal(myRTC)) {
    for (int i = 1; i <= SERVO_REPEAT; i++) {
      servo.write(SERVO_ANGLE);
      delay(SERVO_DELAY);
      servo.write(0);
      delay(SERVO_DELAY);
    }
  }
}
