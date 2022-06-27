#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

static const int RXPin = 3, TXPin = 4, HDOPPin = 12;
static const uint32_t GPSBaud = 9600;

const int en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;
const int i2c_addr = 0x26, i2c_addr2 = 0x27;

LiquidCrystal_I2C lcd(i2c_addr, en,rw,rs,d4,d5,d6,d7,bl,POSITIVE);
LiquidCrystal_I2C lcd2(i2c_addr2, en,rw,rs,d4,d5,d6,d7,bl,POSITIVE);

byte customChar[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B01110,
  B00100,
  B00100
};

char tape[107] = "N---ne---E---se---S---sw---W---nw---N---ne---E---se---S---sw---W---nw---N---ne---E---se---S---sw---W---nw---";

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

char dt[32];

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  lcd.begin(20,4);
  lcd2.begin(20,4);
  
  lcd.setCursor(0,0);
  lcd2.setCursor(0,0);

  lcd2.createChar(1, customChar);

  pinMode(HDOPPin, OUTPUT);
}

void loop()
{
  lcd.clear();

  lcd.print("Lat:");
  lcd.print(gps.location.rawLat().negative ? "S " : "N ");
  lcd.print(gps.location.lat(),6);
  if(gps.location.isValid()) lcd.print(" GO"); else lcd.print(" NOGO");
  lcd.setCursor(0,1);
  lcd.print("Lng:");
  lcd.print(gps.location.rawLng().negative ? "W" : "E");
  lcd.print(gps.location.lng(),6);
  if(gps.time.isValid()) lcd.print(" TFIX"); else lcd.print(" NOGO");
  lcd.setCursor(0,2);

  sprintf(dt, "T%02d/%02d/%04d %02d:%02d:%02d",
  gps.date.month(), gps.date.day(), gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
  lcd.print(dt);
  
  lcd.setCursor(0,3);
  lcd.print("SATS:");
  lcd.print(gps.satellites.value());
  lcd.print(" Alt:");
  lcd.print(gps.altitude.meters());

  lcd2.clear();

  lcd2.print("HDOP:");
  lcd2.print(gps.hdop.hdop());
  lcd2.print(" AGE:" );
  lcd2.print(gps.location.age());
  lcd2.setCursor(0,1);
  lcd2.print("CRS:");
  lcd2.print(gps.course.deg());
  lcd2.print(" SPD:");
  lcd2.print(gps.speed.kmph());

  int mapped = map(gps.course.deg(),0,359.99,0,36) + 27;
  Serial.println(mapped);

  for(int i=0;i<20;i++){
    lcd2.setCursor(i,2);
    lcd2.print(tape[mapped+i]);
  }
  
  lcd2.setCursor(0,3);
  lcd2.print("         ");
  lcd2.write(1);
  
  if(gps.hdop.hdop()<2){
    digitalWrite(HDOPPin, HIGH);
  } else digitalWrite(HDOPPin, LOW);
 
  smartDelay(1000);
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
