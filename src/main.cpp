#include <Arduino.h>
#include <Ds1302.h>
#include <time.h>

#define PUMP1_PIN 9
#define PUMP2_PIN 10
#define PUMP3_PIN 11
#define NUM_OF_PUMPS 3
const int ON = LOW;
const int OFF = HIGH;

struct ActivationData {
    int start_hour;
    int start_min;
    int working_duration_in_sec;
    ActivationData(int start_hour, int start_min, int working_duration_in_sec)
        : start_hour(start_hour), start_min(start_min), working_duration_in_sec(working_duration_in_sec) {}

    unsigned long WorkingDurationInMs(){
      return ((unsigned long)working_duration_in_sec) * 1000;
    }
};


struct PumpStatus {
    int pump_pin;
    unsigned long start_ms = 0;
    bool is_working_now = false;
    ActivationData activation_data;
    PumpStatus(int pump_pin, int start_hour, int start_min, int working_duration_in_sec)
        : pump_pin(pump_pin), activation_data(ActivationData(start_hour, start_min, working_duration_in_sec)) {}
};

const int t1 = 55;
const int t2 = t1 + 3;
const int t3 = t2 + 3;
// init pump status array
PumpStatus pumps_status[3] = {
    PumpStatus(PUMP1_PIN, 21, t1, 120),
    PumpStatus(PUMP2_PIN, 21, t2, 85),
    PumpStatus(PUMP3_PIN, 21, t3, 240)
  };

const static char* WeekDays[] = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
  };

void PumpSwitch(int pump_id, int state);
void printTime(const Ds1302::DateTime* theTime);

// DS1302 RTC instance
Ds1302 RTC(4, 2, 3);

void setup() {
    Serial.begin(9600);

    // initialize the RTC
    RTC.init();
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PUMP1_PIN, OUTPUT);
    pinMode(PUMP2_PIN, OUTPUT);
    pinMode(PUMP3_PIN, OUTPUT);
    // Turn of all pumps
    for (int i = 0; i < NUM_OF_PUMPS; i++) {
        PumpSwitch(i, OFF);
    }

    delay(100);
    // test if clock is halted and set a date-time (see example 2) to start it
    if (0) {
        Serial.println("RTC is halted. Setting time...");

        Ds1302::DateTime dt = {
            .year = 25,
            .month = Ds1302::MONTH_APR,
            .day = 3,
            .hour = 22,
            .minute = 21,
            .second = 00};

        RTC.setDateTime(&dt);
    }
}

void loop() {
    // get the current time
    Ds1302::DateTime now;
    RTC.getDateTime(&now);
    printTime(&now);

    bool is_any_pump_works = false;

    for (int i = 0; i < NUM_OF_PUMPS; i++) {
        if (pumps_status[i].is_working_now) {
            unsigned long active_time_in_ms = millis() - pumps_status[i].start_ms;
            Serial.println(active_time_in_ms);
            Serial.println(pumps_status[i].activation_data.WorkingDurationInMs());
            if (active_time_in_ms > pumps_status[i].activation_data.WorkingDurationInMs()) {
                PumpSwitch(i, OFF);
            } else {
                is_any_pump_works = true;
            }
        }
    }
    Serial.print("Is any motor work: ");
    Serial.println(is_any_pump_works);

    if (!is_any_pump_works) {
        for (int i = 0; i < NUM_OF_PUMPS; i++) {
            // Check if it's time to start the pumps
            if (!pumps_status[i].is_working_now
                && now.hour == pumps_status[i].activation_data.start_hour
                && now.minute == pumps_status[i].activation_data.start_min
                && now.second == 0) { // Start only at the exact start of the minute
                PumpSwitch(i, ON);
                break; // Start only one pump at a time
            }
        }
    }
    delay(1000);
}

void PumpSwitch(int pump_id, int state) {
    digitalWrite(LED_BUILTIN, state);
    digitalWrite(pumps_status[pump_id].pump_pin, state);
    if (state == ON) {
        Serial.print("Starting pump ");
        Serial.print(pump_id + 1);
        Serial.println("...");
        pumps_status[pump_id].start_ms = millis();
        pumps_status[pump_id].is_working_now = true;
    } else {
        Serial.print("Stopping pump ");
        Serial.print(pump_id + 1);
        Serial.println("...");
        pumps_status[pump_id].is_working_now = false;
    }
};

void printTime(const Ds1302::DateTime* theTime) {
    Serial.print("The date is 20");
    Serial.print(theTime->year);  // 00-99
    Serial.print('-');
    if (theTime->month < 10) Serial.print('0');
    Serial.print(theTime->month);  // 01-12
    Serial.print('-');
    if (theTime->day < 10) Serial.print('0');
    Serial.print(theTime->day);  // 01-31
    Serial.print(' ');
    Serial.print(WeekDays[theTime->dow - 1]);  // 1-7
    Serial.print(' ');
    if (theTime->hour < 10) Serial.print('0');
    Serial.print(theTime->hour);  // 00-23
    Serial.print(':');
    if (theTime->minute < 10) Serial.print('0');
    Serial.print(theTime->minute);  // 00-59
    Serial.print(':');
    if (theTime->second < 10) Serial.print('0');
    Serial.print(theTime->second);  // 00-59
    Serial.println();
};