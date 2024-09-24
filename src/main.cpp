#include <Arduino.h>
#include <vector>
#include <TwoWayESP.h>
#include <ESP32Encoder.h>
#include <Wire.h>  
#include <ESP32Servo.h> 
//48:E7:29:B3:F9:FC//30:C9:22:F2:88:40
uint8_t broadcastAddress[] = {0x30,0xC9,0x22,0xF2,0x88,0x40};
float coof[] = {1.5,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
struct SensorData {
    uint8_t angles[8];      // Массив для хранения углов с 8 датчиков
    unsigned long timestamp; // Время записи в миллисекундах
};
SensorData tmp;
std::vector<SensorData> sensorDataVector; // Вектор для хранения структур
Servo servos[8];

#define p1_1f 4
#define p1_2f 32
#define p1_3fr 33
#define p1_3st 16 //левая
#define p2_1f 26
#define p2_2f 27
#define p2_3fr 14
#define p2_3st 25//правая
/*std::vector<SensorData> sensorDataVector; // Вектор для хранения структур
const int sensorPins[] = {p1_1f, p1_2f, p1_3fr, p2_1f, p2_2f, p2_3fr}; // Пины для подключения датчиков*/
// Пины для сервомоторов
int servoPins[8] = {p1_1f, p1_2f, p1_3fr, p2_1f, p2_2f, p2_3fr, p1_3st,p2_3st};
int tmpx=0;
int pos = 0;      // position in degrees
// Published values for SG90 servos; adjust if needed
int minUs = 1000;
int maxUs = 2000;
// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600); // Инициализация последовательного порта
	TwoWayESP::Begin(broadcastAddress);
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  // Привязываем сервомоторы к пинам и устанавливаем параметры
  for (int i = 0; i < 8; i++) {
    servos[i].attach(servoPins[i], minUs, maxUs);
    servos[i].setPeriodHertz(50); // Standard 50hz servo
  }
}
void drive(int i,int angle){
  Serial.print(angle);Serial.print(" ");
  if (i==0)angle=map(angle,120,210,150,0);//round(angle*coof[i]); левое колено
   if (i==3)angle=map(angle,143,250,0,150);// правое колено
      if (i==1)angle=map(angle,34,113,150,0);//левое бедро
           if (i==4)angle=map(angle,12,106,0,150);// правое бедро
                 if (i==2)angle=map(angle,60,84,0,150);// правое бедро
                     if (i==5)angle=map(angle,209,229,0,150);// правое бедро
                     
                          if (i==6)angle=-( tmp.angles[0]+ tmp.angles[1]);// левая стопа
                                 //if (i==5)angle=map(angle,209,229,0,150);// правое бедро
 
 
 servos[i].write(angle);//
 Serial.println(angle);
 tmp.angles[i]=angle;
}
void loop() {
 
/*if(TwoWayESP::Available()) {    

    
		TwoWayESP::GetBytes(&tmp, sizeof(SensorData));
    servo1.write(tmp.angles[1]);
      servo2.write(tmp.angles[0]);
         servo3.write(tmp.angles[3]);
}
delay(500);
Serial.print(tmp.angles[0]);Serial.print(" ");
Serial.print(tmp.angles[1]);Serial.print(" ");
Serial.print(tmp.angles[2]);Serial.print(" ");
Serial.print(tmp.angles[3]);Serial.print(" ");
Serial.print(tmp.angles[4]);Serial.print(" ");
Serial.println(tmp.angles[5]);Serial.print(" ");
*/
 if (TwoWayESP::Available()) 
{
    TwoWayESP::GetBytes(&tmp, sizeof(SensorData));
 for (int i = 0; i < 8; i++) {
  //    tmp.angles[i]=tmp.angles[i]*5;
   //   if(tmp.angles[i]>90)tmp.angles[i]=0;
   // int i=5;
  
     drive(i,tmp.angles[i]);
  //     i=0;
   //  drive(i,tmp.angles[i]);
    //  servos[i].write(tmp.angles[i]);//
    Serial.print(" ");
     
 }
  }
  delay(100);
 
 
  Serial.println();
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}