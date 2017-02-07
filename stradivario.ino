#include <MS5611.h>
#include <RTClib.h>
#include <Wire.h>
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <MenuBackend.h>

//#define HAVE_SPEAKER
#define HAVE_SCREEN
//#define HAVE_SENSOR
#define HAVE_RTC

/*
 * General global variables
 */
#define SCREEN_REFRESH_TIME 500000  // In microseconds
float pressure;
float altitude;
float lastAltitude;
float velocity = 0;
float tim;
float lastRefresh = 0;
float currentTime = 0;
float lastTime = 0;
float alt;


/*
 * MS5611 Sensor
 * SCL -> pin A5
 * SDA -> pin A4
 */
MS5611 ms5611;


/*
 * RTC Module
 * SCL -> pin A5
 * SDA -> pin A4
 */
RTC_DS1307 rtc;

#define DATE_DAY 1
#define DATE_MONTH 2
#define DATE_YEAR 3
#define DATE_HOUR 4
#define DATE_MINUTE 5
uint8_t conf_date_displayed = 0;

uint16_t date_year;
uint8_t date_month;
uint8_t date_day;
uint8_t date_hour;
uint8_t date_minute;


/*
 * Nokia 5110 Screen 
 * CLK   -> pin D7
 * DIN   -> pin D6
 * DC    -> pin D5
 * CE    -> pin D4
 * RST   -> pin D3
 * LIGHT -> pin D11
 */
#define PIN_CLK 7
#define PIN_DIN 6
#define PIN_DC 5
#define PIN_CE 4
#define PIN_RST 3
#define PIN_LIGHT 11 
Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_CLK, PIN_DIN, PIN_DC, PIN_CE, PIN_RST);


/*
 * Rotary Encoder
 */
#define ENCODER_STEP 4
#define CLICK_BUTTON 12
Encoder encoder(10, 11);
long currentPosition = 0;
int currentState = HIGH;

/*
 * Menu
 */
//bool menuIsUsed = false;

/*
 * Here all use events are handled
 * This is where you define a behaviour for a menu item
 */
/*void menuUseEvent(MenuUseEvent used) {
  Serial.println("Menu use ");
  Serial.println(used.item.getName());
}*/

/*
 * Here we get a notification whenever the user changes the menu
 * That is, when the menu is navigated
 */
/*void menuChangeEvent(MenuChangeEvent changed) {
  Serial.println("Menu change ");
  Serial.println(changed.from.getName());
  Serial.println("->");
  Serial.println(changed.to.getName()); 
}*/

/* MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent);
MenuItem item1 = MenuItem(menu, "Item1", 1);
MenuItem item11 = MenuItem(menu, "Item11", 2);
MenuItem item12 = MenuItem(menu, "Item12", 2);
MenuItem item2 = MenuItem(menu, "Item2", 1);

void menuSetup() {
  menu.getRoot().add(item1);
  
  item1.addAfter(item2); 
  item2.addAfter(item1);
  
  item1.addLeft(item1); //loop back if left on settings
  
  item1.addRight(item11);     //chain settings to  pin on right

  item11.addAfter(item12);
  item12.addAfter(item11);
}


void readButtons() {
  long newPosition = encoder.read();
  int newState = digitalRead(CLICK_BUTTON);
  
  if (newPosition != currentPosition && abs(newPosition - currentPosition) >= ENCODER_STEP) {
    if (newPosition > currentPosition) { // Right
      if (menuIsUsed) {
        menu.moveDown();
      }
    }
    else { // Left
      if (menuIsUsed) {
        menu.moveUp();
      }
    }
    currentPosition = newPosition;
  }

  if (newState != currentState) {
    if (newState == LOW) { // Click
      Serial.println("Click");
      if (!menuIsUsed) {
        menu.use();
        menuIsUsed = true;
      }
      else {
        menu.moveRight();
      }
    }
    currentState = newState;
  }
}*/

/*
 * Screen brightness. AnalogWrite values from 0 to 255
 */
/*void updateBrightness() { analogWrite(PIN_LIGHT, conf.light_cpt * 51); }*/

void setup() {
    //pinMode(CLICK_BUTTON, INPUT);
    //menuSetup();
    Serial.begin(9600);
    while (! Serial); // Wait until Serial is ready

    #ifdef HAVE_SCREEN
    display.begin();
    display.setContrast(60);
    display.display();
    pinMode(PIN_LIGHT, OUTPUT);
    analogWrite(PIN_LIGHT, 128);
    #endif

    #ifdef HAVE_SENSOR
    while (!ms5611.begin()) {
        Serial.println("Could not find MS5611 sensor, check wiring!");
        delay(500);
    }

    tim = micros();
    altitude = ms5611.getAltitude(ms5611.readPressure());
    #endif

    #ifdef HAVE_RTC
    while (!rtc.begin()) {
        Serial.println("Couldn't find RTC module, check wiring!");
        delay(500);
    }
    
    if (!rtc.isrunning()) {
        Serial.println("RTC is NOT running!");
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    #endif
}


void renderVario() {
    #ifdef HAVE_RTC
    DateTime now = rtc.now();
    #endif

    #ifdef HAVE_SCREEN 
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(BLACK);

    display.setCursor(0,0);
    display.print(now.hour());
    display.print(":");
    display.print(now.minute());
    display.print(":");
    display.print(now.second());

    display.setCursor(0,10);
    display.print((int) altitude);
    display.print(" m");

    display.setCursor(0,25);
    display.print(velocity);
    display.print(" m/s");

    //Battery level
    uint8_t vccPixels = 5;
    uint8_t bat_x = 72;
    uint8_t bat_y = 0;
    display.drawRect(bat_x + 2, bat_y, 10, 6, BLACK);
    display.fillRect(bat_x, bat_y + 2, 2, 2, BLACK);
    display.fillRect(bat_x + 3 + 8 - vccPixels, bat_y + 1, vccPixels, 4, BLACK);

    display.display();
    #endif
}

void loop() {
    //readButtons();

    currentTime = micros();

    #ifdef HAVE_RTC
    /* DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(now.dayOfTheWeek());
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    delay(1000);*/
    #endif

    // Update screen
    if (currentTime - lastRefresh > SCREEN_REFRESH_TIME) {
        lastRefresh = currentTime;
        renderVario();
    }

    #ifdef HAVE_SENSOR
    pressure = 0.95 * pressure + 0.05 * ms5611.readPressure();
    altitude = 0.85 * altitude + 0.15 * ms5611.getAltitude(pressure);
    velocity = 0.85 * velocity + 150000 * (altitude - lastAltitude) / (currentTime - lastTime);
    lastAltitude = altitude;
    lastTime = currentTime; 
    #endif
    
}
