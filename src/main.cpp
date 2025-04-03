#include <Arduino.h>
#include <Ds1302.h>
#include <time.h>

#define PUMP1_PIN 9
#define PUMP2_PIN 10
#define PUMP3_PIN 11
#define NUM_OF_PUMPS 3

struct ActivationData {
    int start_hour;
    int start_min;
    int working_duration_in_sec;
    ActivationData(int start_hour, int start_min, int working_duration_in_sec)
        : start_hour(start_hour), start_min(start_min), working_duration_in_sec(working_duration_in_sec) {}
};

struct PumpStatus {
    int pump_pin;
    int start_ms = 0;
    bool is_working_now = false;
    ActivationData activation_data;
    PumpStatus(int pump_pin, int start_hour, int start_min, int working_duration_in_sec)
        : pump_pin(pump_pin), activation_data(ActivationData(start_hour, start_min, working_duration_in_sec)) {}
};

// init pump status array
PumpStatus pumps_status[3] = {
    PumpStatus(PUMP1_PIN, 21, 30, 120),
    PumpStatus(PUMP2_PIN, 21, 35, 80),
    PumpStatus(PUMP3_PIN, 21, 40, 240)
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
        PumpSwitch(i, LOW);
    }

    delay(100);
    // test if clock is halted and set a date-time (see example 2) to start it
    if (0) {
        Serial.println("RTC is halted. Setting time...");

        Ds1302::DateTime dt = {
            .year = 25,
            .month = Ds1302::MONTH_MAR,
            .day = 30,
            .hour = 14,
            .minute = 4,
            .second = 30,
            .dow = Ds1302::DOW_TUE};

        RTC.setDateTime(&dt);
    }
}

void loop() {
    // get the current time
    Ds1302::DateTime now;
    RTC.getDateTime(&now);
    printTime(&now);

    Serial.println("Motor test mode OFF!");

    bool is_any_pump_works = false;

    for (int i = 0; i < NUM_OF_PUMPS; i++) {
        if (pumps_status[i].is_working_now) {
            int active_time_in_ms = millis() - pumps_status[i].start_ms;
            if (active_time_in_ms * 1000 > pumps_status[i].activation_data.working_duration_in_sec) {
                PumpSwitch(i, LOW);
            } else {
                is_any_pump_works = true;
            }
        }

        if (!is_any_pump_works) {
            // Check if it's time to start the pumps
            for (int i = 0; i < NUM_OF_PUMPS; i++) {
                if (!pumps_status[i].is_working_now && now.hour == pumps_status[i].activation_data.start_hour && now.minute == pumps_status[i].activation_data.start_min) {
                    Serial.print("Starting pump ");
                    Serial.print(i + 1);
                    Serial.println("...");
                    PumpSwitch(i, HIGH);
                }
            }
        }
    }
    delay(1000);
}

void PumpSwitch(int pump_id, int state) {
    digitalWrite(LED_BUILTIN, state);
    digitalWrite(pumps_status[pump_id].pump_pin, !state);
    if (state == HIGH) {
        pumps_status[pump_id].start_ms = millis();
        pumps_status[pump_id].is_working_now = true;
    } else {
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