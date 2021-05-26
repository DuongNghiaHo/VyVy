#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <SoftwareSerial.h>
SoftwareSerial S(4,5);

char auth[] = "3XxXdPaQKaOkfmkZDkc0sdvHxADBEcRb";
char ssid[] = "_VVy.196";
char pass[] = "vyvy1998";

WidgetLED led1(V1); // led connect (blinking)
WidgetLED led2(V2); // led start/stop
WidgetLED led3(V3); // zone 1
WidgetLED led4(V4); // zone 2
WidgetLED led5(V5); // zone 3
WidgetLED led6(V6); // zone 4
WidgetLED led7(V7); // zone 5
BlynkTimer timer;
int tt, giatri, diem; // tt = zone?, giatri = 5(chay),8(dung)

void blinkLedWidget() {
  if (led1.getValue()) {led1.off();}
  else {led1.on();}
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  digitalWrite(D0, pinValue);
  if (digitalRead(D0) == 0) {
    led2.on();
    giatri = 5;
  }
  else {
    led2.off();
    giatri = 8;
  }
  S.write(giatri);
}

void setup() {
  Serial.begin(9600);
  S.begin(9600);
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(1000L, blinkLedWidget);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);
}

void loop() {
  Blynk.virtualWrite(V10, diem);
  Blynk.run();
  timer.run();
  if (S.available() > 0) {
    tt = S.read();
  }
  if (tt == 1) {led3.on(); diem = 20;}
  if (tt == 2) {led4.on(); diem = 40;}
  if (tt == 3) {led5.on(); diem = 70;}
  if (tt == 4) {led6.on(); diem = 85;}
  if (tt == 5) {led7.on(); diem = 100;}
  if (tt == 0) {
    led3.off();
    led4.off();
    led5.off();
    led6.off();
    led7.off();
    diem = 0;   
  }

  Blynk.virtualWrite(V10, diem);
  Blynk.run();
  timer.run();
}
