#include <Arduino.h>
#include <vector>
#include <TwoWayESP.h>
#include <ESP32Encoder.h>
#include <Wire.h>  
#include <ESP32Servo.h> 

uint8_t broadcastAddress[] = {0x30, 0xC9, 0x22, 0xF2, 0x88, 0x40};
float coof[] = {1.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

struct SensorData {
    uint8_t angles[8];      // Массив для хранения углов с 8 датчиков
    unsigned long timestamp; // Время записи в миллисекундах
    int bt1;                 // Состояние кнопки 1
    int bt2;                 // Состояние кнопки 2
};

SensorData sensorDataArray[1000];  // Массив для хранения 1000 структур
Servo servos[8];

#define p1_1f 4
#define p1_2f 32
#define p1_3fr 33
#define p1_3st 16 // левая нога
#define p2_1f 26
#define p2_2f 27
#define p2_3fr 14
#define p2_3st 25 // правая нога

int servoPins[8] = {p1_1f, p1_2f, p1_3fr, p2_1f, p2_2f, p2_3fr, p1_3st, p2_3st};
int minUs = 1000;
int maxUs = 2000;

bool isRecording = false;   // Флаг для записи
bool isPlaying = false;     // Флаг для управления приводами
int dataIndex = 0;          // Индекс для записи в массив
int playIndex = 0;         // Индекс для воспроизведения из массива
int lastBt1State = 0;       // Последнее состояние кнопки bt1
int lastBt2State = 0;       // Последнее состояние кнопки bt2
unsigned long lastUpdate = 0; // Последнее обновление тайминга для привода
unsigned long playbackDelay = 0; // Задержка для воспроизведения

void setup() {
    Serial.begin(9600); // Инициализация последовательного порта
    TwoWayESP::Begin(broadcastAddress);

    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    // Привязываем сервомоторы к пинам и устанавливаем параметры
    for (int i = 0; i < 8; i++) {
        servos[i].attach(servoPins[i], minUs, maxUs);
        servos[i].setPeriodHertz(50); // Стандартный 50 Гц для сервоприводов
    }
}
double calculateFootAngle(double hipAngle, double kneeAngle) {
    // hipAngle и kneeAngle в градусах
    double footAngle = 90.0 - (hipAngle + kneeAngle); 
    return footAngle;
}
void drive(int i, int angle) {
  //  Serial.print(angle);
 //   Serial.print(" ");
    
    if (i == 0) angle = map(angle, 120, 210, 150, 0);      // левое колено
    if (i == 3) angle = map(angle, 143, 250, 0, 150);      // правое колено
    if (i == 1) angle = map(angle, 34, 113, 150, 0);       // левое бедро
    if (i == 4) angle = map(angle, 12, 106, 0, 150);       // правое бедро
    if (i == 2) angle = map(angle, 60, 84, 0, 150);        // левое бедро отвод
    if (i == 5) angle = map(angle, 209, 229, 0, 150);      // правое бедро отвод
    if (i == 6) {angle =calculateFootAngle(sensorDataArray[0].angles[0],sensorDataArray[0].angles[1]); // левая стопа
     Serial.print(angle);Serial.print("l ");
   angle = map(angle, -233, -111, 0, 90);      // правое бедро отвод
     Serial.println(angle);
    }
  if (i == 7) {
  angle =calculateFootAngle(sensorDataArray[0].angles[3],sensorDataArray[0].angles[4]); // левая стопа
     Serial.print(angle);Serial.print("p");
   angle = map(angle, -233, -111, 0, 90);      // правое бедро отвод
     Serial.println(angle);
  }
    servos[i].write(angle);
  //  Serial.println(angle);
}

void loop() {
    if (TwoWayESP::Available()) {
        SensorData tmp;
        TwoWayESP::GetBytes(&tmp, sizeof(SensorData));
  //Serial.println("Начат процесс.");
        // Проверка состояния bt1 для начала/остановки записи
           sensorDataArray[0] = tmp;  
        if (tmp.bt1 == 1 && lastBt1State == 0) {
            isRecording = !isRecording;  // Переключаем состояние записи
            if (isRecording) {
                Serial.println("Начата запись данных.");
            } else {
                Serial.println("Остановлена запись данных.");
            }
        }

        // Проверка состояния bt2 для начала/остановки управления приводами
        if (tmp.bt2 == 1 && lastBt2State == 0) {
            isPlaying = !isPlaying;  // Переключаем состояние управления приводами
            if (isPlaying) {
                Serial.println("Начато управление приводами.");
                playIndex = 0; // Начинаем с начала массива
                lastUpdate = millis(); // Обновляем время начала воспроизведения
            } else {
                Serial.println("Остановлено управление приводами.");
            }
        }

        lastBt1State = tmp.bt1;  // Обновляем состояние кнопки bt1
        lastBt2State = tmp.bt2;  // Обновляем состояние кнопки bt2

        // Если запись активна и не превышен размер массива
        if (isRecording && dataIndex < 1000) {
            sensorDataArray[dataIndex] = tmp;  // Записываем данные в массив
            dataIndex++;  // Увеличиваем индекс для следующей записи
            Serial.print("Записано значение: ");
            Serial.println(dataIndex);
        }

        // Управление сервоприводами
        if (isPlaying) {
            unsigned long currentTime = millis();
            if (playbackDelay == 0) {
                playbackDelay = sensorDataArray[playIndex].timestamp;
            }
            if (currentTime - lastUpdate >= playbackDelay) {
                // Обновляем состояние привода
                for (int i = 0; i < 8; i++) {
                    drive(i, sensorDataArray[playIndex].angles[i]);
                }
                lastUpdate = currentTime; // Обновляем время последнего обновления
                playbackDelay = sensorDataArray[playIndex].timestamp; // Обновляем задержку

                playIndex++; // Переходим к следующему элементу массива
                if (playIndex >= 1000) {
                    playIndex = 1; // Начинаем с начала массива
                }
            }
        } else {
for (int i = 0; i < 8; i++) {
  //    tmp.angles[i]=tmp.angles[i]*5;
   //   if(tmp.angles[i]>90)tmp.angles[i]=0;
   // int i=5;
  
     drive(i,tmp.angles[i]);
  //     i=0;
   //  drive(i,tmp.angles[i]);
    //  servos[i].write(tmp.angles[i]);//
 //   Serial.print(tmp.angles[i]);
  //  Serial.print(" ");
     
 }
//  Serial.println(" ");

        }

        delay(10); // Сокращаем задержку для быстрого реагирования на изменения
    }
}
