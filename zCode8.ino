#include <Arduino_FreeRTOS.h>
#include <semphr.h>

SemaphoreHandle_t xSerialSemaphore;

void TaskDo( void *pvParameters );
void TaskPID( void *pvParameters );
///////////////////////////////////////////////////////////////////////////////////////
#include <SoftwareSerial.h>
SoftwareSerial S(8, 9); //Khởi tạo cổng serial mềm, 8 = Rx, 9 = Tx

#include <MPU6050_tockn.h>
#include <Wire.h>
MPU6050 mpu6050(Wire);

float goc, mucdich;
int dem = 0;

int zone = 0, kt = 0, readA6;

int KD;

#include <SharpIR.h>
SharpIR mySensor = SharpIR(A2, 1080);

int distance_cm;
///////////////////////////////////////////////////////////////////////////////////////
void setup() {
  ///////////////////////////////////////////////////
  Serial.begin(9600);
  S.begin(9600);
  
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  pinMode(A6, INPUT);
  ////////////////////////////////////////////////////
  if ( xSerialSemaphore == NULL ) {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );
  }

  xTaskCreate(
    TaskDo
    ,  "Do"
    ,  128
    ,  NULL
    ,  1
    ,  NULL );

  xTaskCreate(
    TaskPID
    ,  "PID"
    ,  128
    ,  NULL
    ,  1
    ,  NULL );
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskDo( void *pvParameters __attribute__((unused)) ) {
  for (;;) {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      mpu6050.update();
      goc = mpu6050.getAngleZ();
      if (dem == 0) {
        mucdich = goc + 0.2;
        dem++;
      }
      readA6 = analogRead(A6);

      if (S.available() > 0) {KD = S.read();}
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      xSemaphoreGive( xSerialSemaphore );
    }
    vTaskDelay(1);
  }
}

void TaskPID( void *pvParameters __attribute__((unused)) ) {
  for (;;) {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      int PWM = 70;
      if (dem == 1 && KD == 5) {
        if (goc < mucdich) {
          analogWrite(5, PWM + 50);
          analogWrite(6, PWM);
        }
        else {
          analogWrite(5, PWM);
          analogWrite(6, PWM + 50);
        }
      }
      if (readA6 > 300 && kt == 0) {
        zone++;
        kt = 1;
      }
      if (readA6 < 60) {
        kt = 0;
      }

      S.write(zone);

      if (zone == 3 && dem == 1){
        digitalWrite(4, 1);
        analogWrite(5, 255);
        analogWrite(6, 255);
        digitalWrite(7, 1);
        
        dem = 2;
        delay(100);
        distance_cm = mySensor.distance();
      }
      if (dem == 2 && distance_cm >= 12) {
        dem = 3;
      }
      if (dem == 2 && distance_cm <= 11) {
        dem = 4;
      }
      if (dem == 3) {
        digitalWrite(4, 0);
        analogWrite(5, 113);
        analogWrite(6, 0);
        digitalWrite(7, 0);
        if (goc >= (mucdich + 90.5)){
          mucdich = mucdich + 88.5;
          dem = 5;
        }
      }
      if (dem == 4) {
        digitalWrite(4, 0);
        analogWrite(5, 0);
        analogWrite(6, 120);
        digitalWrite(7, 0);
        if (goc <= (mucdich - 90.5)){
          mucdich = mucdich - 88.5;
          dem = 5;
        }
      }
      if (dem == 5) {
        if (goc < mucdich) {
          analogWrite(5, PWM + 50);
          analogWrite(6, PWM);
        }
        else {
          analogWrite(5, PWM);
          analogWrite(6, PWM + 50);
        }
      }
      if (zone == 5 && dem == 5) {
        analogWrite(5, 0);
        analogWrite(6, 0);
        
        delay(100);
        dem = 6;
      }
      //Serial.print(zone);
      //Serial.print(" ");
      //Serial.print(dem);
      //Serial.print(" ");
      //Serial.println(distance_cm);
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      xSemaphoreGive( xSerialSemaphore );
    }
    vTaskDelay(1);
  }
}
