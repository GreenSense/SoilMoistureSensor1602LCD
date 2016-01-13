#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

long second = 1000;
long minute = second * 60;
long hour = minute * 60;

long lastReadingTime = 0;
long readingInterval = hour;

#define moisturePin 0
#define sensorPowerPin 2
#define ledPin 13
#define buttonPin 4

int moistureLevel = 0;

// Start these values in the middle, they'll be calibrated on the fly
int lowestReading = 500;
int highestReading = 600;

int lowestReadingAddress = 0;
int highestReadingAddress = 1;

long lastDebounceTime = 0;
long debounceDelay = 50;

void setup()
{
  Serial.begin(9600);

  pinMode(2, OUTPUT);

  lcd.init();

  lcd.backlight();

  lowestReading = getLowestReading();
  highestReading = getHighestReading();

  Serial.println("Starting soil moisture sensor");
  
  Serial.print("Lowest reading: ");
  Serial.println(lowestReading);
  Serial.print("Highest reading: ");
  Serial.println(highestReading);
  
}

void loop()
{
  checkButton();
  
  takeReading();

  delay(10);
}

void checkButton()
{
  int reading = digitalRead(buttonPin);

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == HIGH) {

      lastDebounceTime = millis();
      
      Serial.println("Button pressed");

      digitalWrite(ledPin, HIGH);

      // Reset the last reading time to force another reading
      lastReadingTime = 0;
      takeReading();
      
      digitalWrite(ledPin, LOW);
    }
  }
}

void takeReading()
{
  if (lastReadingTime + readingInterval < millis()
    || lastReadingTime == 0)
  {
    Serial.println("Taking reading");

    lastReadingTime = millis();

    sensorOn();

    delay(2000);

    int readingSum  = 0;
    int totalReadings = 10;

    for (int i = 0; i < totalReadings; i++)
    {
      int reading = analogRead(moisturePin);

      readingSum += reading;
    }

    int averageReading = readingSum / totalReadings;
    
    Serial.print("Average raw reading: ");
    Serial.println(averageReading);
    
    if (averageReading > highestReading)
    {
      highestReading = averageReading;
      setHighestReading(highestReading);
    }

    if (averageReading < lowestReading)
    {
      lowestReading = averageReading;
      setLowestReading(lowestReading);
    }
    
    moistureLevel = map(averageReading, highestReading, lowestReading, 0, 100);

    Serial.print("Moisture level: ");
    Serial.print(moistureLevel);
    Serial.println("%");
    
    sensorOff();

    displayReading();
  }
  else
  {
    outputTimeRemaining();
  }
}

void outputTimeRemaining()
{
  long timeSinceLastReading = millis();
  if (lastReadingTime > 0)
    timeSinceLastReading = millis() - lastReadingTime;
  long millisecondsRemaining = readingInterval - timeSinceLastReading;
  int secondsRemaining = millisecondsRemaining / 1000;
  
  Serial.println(secondsRemaining);
}

void displayReading()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moistureLevel);
  lcd.print("%");
}

void sensorOn()
{
  //Serial.println("Turning sensor on");

  digitalWrite(sensorPowerPin, HIGH);
}

void sensorOff()
{
  //Serial.println("Turning sensor off");

  digitalWrite(sensorPowerPin, LOW);
}

void setLowestReading(int reading)
{
  Serial.print("Setting lowest reading: ");
  Serial.println(reading);
  
  EEPROM.write(lowestReadingAddress, reading/4); // Must divide by 4 to make it fit in eeprom
}

void setHighestReading(int reading)
{
  Serial.print("Setting highest reading: ");
  Serial.println(reading);
  
  EEPROM.write(highestReadingAddress, reading/4); // Must divide by 4 to make it fit in eeprom
}

int getLowestReading()
{
  int value = EEPROM.read(lowestReadingAddress);

  if (value == 255)
    return lowestReading;
  else
    return value*4; // Must multiply by 4 to get the original value
}

int getHighestReading()
{
  int value = EEPROM.read(highestReadingAddress);
  
  if (value == 255)
    return highestReading;
  else
    return value*4; // Must multiply by 4 to get the original value
}
