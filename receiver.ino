#include <XBee.h>
#include <Servo.h>

int servoPin = 2;
int URPWM = 8; // pin 4 sensor
int URTRIG = 9; // pin 6 snesor
int Vitesse_M1 = 5;     //M1 Speed Control
int Vitesse_M2 = 6;     //M2 Speed Control
int Direction_M1 = 4;     //M1 Direction Control
int Direction_M2 = 7;     //M1 Direction Control

int i = 0;
bool Done;
Servo Servo1;
int pause = 500;
int Rssi[5] , RssiF;
XBee xbee = XBee();
unsigned int Distance = 0;
Rx16Response rx16 = Rx16Response();
uint8_t EnPwmCmd[4] = {0x44, 0x02, 0xbb, 0x01};


int sort_desc(const void *cmp1, const void *cmp2)
{
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  return a > b ? -1 : (a < b ? 1 : 0);

}

void setup()
{
  xbee.setSerial(Serial);
  Servo1.attach(servoPin);
  Serial.begin(9600);
  PWM_Mode_Setup();
}

void loop()
{

  for  (int i = 0; i <= 5; i++)
  {
    xbee.readPacket(5);
    if (xbee.getResponse().isAvailable())
    {
      if (xbee.getResponse().getApiId() == RX_16_RESPONSE)
      {
        xbee.getResponse().getRx16Response(rx16);

        Rssi[i] = rx16.getRssi();
        Serial.print("Compteur a :");
        Serial.println(i);
        Serial.println(Rssi[i]);

        Done = HIGH;
        delay(125);
      }

    }

  }



  if (Done == HIGH)
  {

    int Rssi_L = sizeof(Rssi) / sizeof(Rssi[0]);
    qsort(Rssi, Rssi_L, sizeof(Rssi[0]), sort_desc);

    Serial.print(Rssi[0]);
    Serial.print(" ");
    Serial.print(Rssi[1]);
    Serial.print(" ");
    Serial.print(Rssi[2]);
    Serial.print(" ");
    Serial.print(Rssi[3]);
    Serial.print(" ");
    Serial.print(Rssi[4]);
    Serial.print(" ");
    Serial.println(Rssi[5]);

    RssiF = (Rssi[2] +  Rssi[3] +  Rssi[4]) / 3;
    Serial.print("Rssi Final" );
    Serial.println(RssiF);
    Done = LOW;
  }

  for (int i = 90; i <= 135; i = i + 15) {
    if (PWM_Mode()){
      break;
    }
      Serial.println(i);
      Servo1.write(i);
      delay(pause);

  }
  for (int i = 135; i >= 45; i = i - 15) {
    if(PWM_Mode()){
      break;
    }
      Serial.println(i);
      Servo1.write(i);
      delay(pause);

  }
  for (int i = 45; i <= 90; i = i + 15) {
    if(PWM_Mode()){
      break;
    }
      Serial.println(i);
      Servo1.write(i);
      delay(pause);
  }

  if (RssiF >= 55 && !PWM_Mode())
  {
    Serial.println("En avant: ");
    carAdvance(250, 250);
  }
  else
  {
    carStop();
  }
}

void PWM_Mode_Setup()
{
  pinMode(URTRIG, OUTPUT);
  digitalWrite(URTRIG, HIGH);

  pinMode(URPWM, INPUT);

  for (int i = 0; i < 4; i++)
  {
    Serial.write(EnPwmCmd[i]);
  }
}

bool PWM_Mode()
{
  digitalWrite(URTRIG, LOW);
  digitalWrite(URTRIG, HIGH);

  unsigned long DistanceMeasured = pulseIn(URPWM, LOW);

  if (DistanceMeasured >= 10200)
  {
    Serial.println("Invalid");
  }
  else
  {
    Distance = DistanceMeasured / 50;
    Serial.print("Distance=");
    Serial.print(Distance);
    Serial.println("cm");
    if (Distance <= 30)
    {
      return true;
    } else {
      return false;
    }
  }

}

void carAdvance(int leftSpeed,int rightSpeed){       //Move forward
  analogWrite (Vitesse_M2,leftSpeed);
  digitalWrite(Direction_M1,HIGH);   
  analogWrite (Vitesse_M1,rightSpeed);    
  digitalWrite(Direction_M2,LOW);
}

void carStop(){                 //  Motor Stop
  digitalWrite(Vitesse_M2,0); 
  digitalWrite(Direction_M1,LOW);    
  digitalWrite(Vitesse_M1,0);   
  digitalWrite(Direction_M2,LOW);    
}  
