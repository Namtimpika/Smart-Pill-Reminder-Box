#define BLYNK_TEMPLATE_ID "TMPL6Hkp03kgM"
#define BLYNK_TEMPLATE_NAME "SmartBox"
#define BLYNK_AUTH_TOKEN "6iary4SqhdvIbDEf3W3Gekwd-jLU9Y2U"

// -----------------------------
// PITCH DEFINE (ความถี่ของโน้ต)
// -----------------------------
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define BUZZER_PIN D5
#define BUTTON_PIN D6
#define IR_PIN     D4

char ssid[] = "panpanhandsome";
char pass[] = "45e45aedd562";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // GMT+7

BlynkTimer timer;

struct AlarmTime {
  int hour;
  int minute;
  bool triggered;
};

AlarmTime alarms[3] = {
  {7, 0, false},   // เช้า
  {12, 0, false},  // กลางวัน
  {18, 0, false}   // เย็น
};

// -----------------------------
// Harry Potter Melody (บางส่วน)
// -----------------------------
int melody[] = {
  NOTE_G4, NOTE_G4, NOTE_C5, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_B4,
  NOTE_A4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5,
  NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_C5,
  NOTE_D5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5,
  NOTE_G5, NOTE_E5, NOTE_D5, NOTE_C5
};

// ระยะเวลาแต่ละโน้ต (4 = quarter note, 8 = eighth note)
int notedurations[] = {
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 4, 4, 2,
  4, 4, 4, 4, 2,
  4, 4, 4, 4, 4, 4,
  2, 4, 4, 2
};

// -----------------------------
// ฟังก์ชันเล่นเพลง Harry Potter
// -----------------------------
void play() {
  Serial.println("🎵 เล่นเพลง");
  int notes = sizeof(melody) / sizeof(int);

  for (int i = 0; i < notes; i++) {
    int duration = (1000 / notedurations[i]) * 1.5 ;  // ความยาวของแต่ละโน้ต
    tone(BUZZER_PIN, melody[i], duration);
    delay(duration * 1.3);  // เว้นช่วงเล็กน้อย
    noTone(BUZZER_PIN);
  }
}

// รับค่าจาก Time Input (V0 = เช้า, V1 = เที่ยง, V2 = เย็น)
BLYNK_WRITE(V0) {
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    alarms[0].hour = t.getStartHour();
    alarms[0].minute = t.getStartMinute();
    Serial.printf("ตั้งเวลาเช้า: %02d:%02d\n", alarms[0].hour, alarms[0].minute);
  }
}

BLYNK_WRITE(V1) {
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    alarms[1].hour = t.getStartHour();
    alarms[1].minute = t.getStartMinute();
    Serial.printf("ตั้งเวลากลางวัน: %02d:%02d\n", alarms[1].hour, alarms[1].minute);
  }
}

BLYNK_WRITE(V2) {
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    alarms[2].hour = t.getStartHour();
    alarms[2].minute = t.getStartMinute();
    Serial.printf("ตั้งเวลาเย็น: %02d:%02d\n", alarms[2].hour, alarms[2].minute);
  }
}

bool alarmActive = false;

BLYNK_WRITE(V3) {
  if (alarmActive) {
    alarmActive = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("ปิดเสียงเตือนผ่าน Blynk");
  }
}

// ปุ่ม V4 สำหรับเล่นเพลง Harry Potter
BLYNK_WRITE(V4) {
  int pressed = param.asInt();
  if (pressed == 1 && !alarmActive) {
    play();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(IR_PIN, INPUT);

  digitalWrite(BUZZER_PIN, LOW);

  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timeClient.begin();

  timer.setInterval(10000L, checkAlarms); // เช็คทุก 30 วินาที
}

void loop() {
  Blynk.run();
  timeClient.update();
  timer.run();

  // ปิดเสียงด้วยปุ่มหรือ IR
  static bool lastButtonState = LOW;
  static bool lastIRState = LOW;
  bool buttonState = digitalRead(BUTTON_PIN);
  bool IRState = digitalRead(IR_PIN);

  if (alarmActive && ((buttonState == HIGH && lastButtonState == LOW) || (IRState == HIGH && lastIRState == LOW))) {
    alarmActive = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("ปิดเสียงเตือนแล้ว");
  }

  lastButtonState = buttonState;
  lastIRState = IRState;
  Serial.println(IRState);
}

void checkAlarms() {
  int h = timeClient.getHours();
  int m = timeClient.getMinutes();

  Serial.printf("ตอนนี้เวลา: %02d:%02d\n", h, m);

  for (int i = 0; i < 3; i++) {
    if (!alarms[i].triggered && h == alarms[i].hour && m == alarms[i].minute) {
      alarms[i].triggered = true;
      alarmActive = true;
      digitalWrite(BUZZER_PIN, HIGH);
      Blynk.logEvent("pill", "⏰ ถึงเวลาทานยาแล้ว!");
      Serial.printf("เตือนช่วงที่ %d\n", i + 1);
    }
    
    // รีเซ็ต flag หากพ้นเวลานั้นไปแล้ว
    if (alarms[i].triggered && !(h == alarms[i].hour && m == alarms[i].minute)) {
      alarms[i].triggered = false;
    }
  }
}
