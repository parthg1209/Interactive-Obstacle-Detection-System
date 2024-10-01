#include <LiquidCrystal.h>
#include <IRremote.h>
#include <EEPROM.h>

#define LED1 12
#define LED2 11
#define LED3 10
#define RS A5
#define E A4
#define D4 6
#define D5 7
#define D6 8
#define D7 9
#define BUTTON 2
#define ECHO 3
#define TRIG 4
#define IR 5
#define PHOTO A0
#define PLAY 67
#define UP 21
#define DOWN 7
#define EQ 9
#define IR0 22

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

unsigned long ulast=millis();
unsigned long udelay=60;
volatile bool pavail=0;
volatile unsigned long pbegin;
volatile unsigned long pend;
bool unit=EEPROM.read(0);
double prevdist=400;
double prevdisti=160;
bool LED2s=0;
unsigned long bdelay=100;
unsigned long tlast=millis();
byte dis=1;
bool bs=0;
unsigned long plast=millis();
unsigned long pdelay=500;
unsigned long lastblink=millis();
unsigned long blinkdelay=300;

void lockblink()
{
  if(LED2s==HIGH)
    LED2s=LOW;
  else
    LED2s=HIGH;
  digitalWrite(LED2,LED2s);
  digitalWrite(LED1,LED2s);
}

void remote(int a)
{
  switch(a)
  {
    case UP:
    lcd.clear();
    if(dis==3)
      dis=1;
    else
      dis++;
    break;

    case DOWN:
    lcd.clear();
    if(dis==1)
      dis=3;
    else
      dis--;
    break;

    case EQ:
    if(unit)
    {
      unit=0;
      EEPROM.write(0,0);
    }
    else
    {
      unit=1;
      EEPROM.write(0,1);
    }
    break;
  }
}

void trigger()
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
}

void echointer()
{
  if(digitalRead(ECHO)==HIGH)
    pbegin=micros();
  else
  {
    pend=micros();
    pavail=1;
  }
}

void display1(double distance)
{ 
  lcd.setCursor(0,0);
  lcd.print("Dist: ");
  lcd.print(distance);
  if(unit)
  {
    lcd.print(" in  ");
    lcd.setCursor(0,1);
    if(distance>20)
      lcd.print("No Obstacle.    ");
    else if(distance<20&&distance>4)
      lcd.print("!! Warning !!   ");
  }
  else
  {
    lcd.print(" cm  ");
    lcd.setCursor(0,1);
    if(distance>50)
      lcd.print("No Obstacle.    ");
    else if(distance<50&&distance>10)
      lcd.print("!! Warning !!   ");
  }
}

void display2()
{
  lcd.setCursor(0,0);
  lcd.print("Luminosity: ");
  lcd.print(analogRead(PHOTO));
  lcd.print("  ");
}

void display3()
{
  lcd.setCursor(0,0);
  lcd.print("Press on 0 to");
  lcd.setCursor(0,1);
  lcd.print("reset settings");
  if(IrReceiver.decode())
  {
    IrReceiver.resume();
    int a=IrReceiver.decodedIRData.command;
    if(a==IR0)
    {
      lcd.clear();
      lcd.print("Initializing...");
      delay(1000);
      dis=1;
      unit=0;
      EEPROM.write(0,0);
    }
  }
}

void lock()
{
  lcd.setCursor(0,0);
  lcd.print("!!! Obstacle !!!");
  lcd.setCursor(0,1);
  lcd.print("Press to unlock");
  while(true)
  {
    unsigned long tnow=millis();
    if(IrReceiver.decode())
    {
      IrReceiver.resume();
      int a=IrReceiver.decodedIRData.command;
      if(a==PLAY)
      {
        lcd.clear();
        digitalWrite(LED1,LOW);
        break;
      }
    }
    bool bnow=digitalRead(BUTTON);
    if(bnow==0&&bs==1)
    {
      bs=0;
      digitalWrite(LED1,LOW);
      lcd.clear();
      break;
    }
    bs=bnow;
    if(tnow-lastblink>blinkdelay)
    {
      lastblink+=blinkdelay;
      lockblink();
    }  
  }
}

void blink()
{
  if(LED2s==HIGH)
    LED2s=LOW;
  else
    LED2s=HIGH;
  digitalWrite(LED2,LED2s);
}

double dist()
{
  pavail=0;
  double dur=pend-pbegin;
  double distance;
  if(!unit)
  {
    distance=dur/58.0;
    if(distance>400)
      return prevdist;
    prevdist=distance;
    bdelay=distance*4;
  }
  else
  {
    distance=dur/148.0;
    if(distance>160)
      return prevdisti;
    prevdisti=distance;
    bdelay=distance*10;
  }
  return distance;
}
void setup() {
  lcd.begin(16,2);
  pinMode(TRIG,OUTPUT);
  pinMode(ECHO,INPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(BUTTON,INPUT);
  IrReceiver.begin(IR);
  lcd.print("Initializing...");
  delay(1000);
  attachInterrupt(digitalPinToInterrupt(ECHO),echointer,CHANGE);
}

void loop() 
{
  unsigned long tnow=millis();
  if(tnow-ulast>udelay)
  {
    ulast+=udelay;
    trigger();
  }
  if(pavail)
  {
    double distance=dist();
    if(dis==1)
      display1(distance);
    else if(dis==2)
      display2();
    else
      display3();
    if(unit)
    {
      if(distance<4)
        lock();
    }
    else
    {
      if(distance<10)
        lock();
    }
  }
  if(tnow-tlast>bdelay)
  {
    tlast+=bdelay;
    blink();
  }
  if(tnow-plast>pdelay)
  {
    if(IrReceiver.decode())
    {
      plast+=pdelay;
      IrReceiver.resume();
      remote(IrReceiver.decodedIRData.command);
    }
  }
  analogWrite(10, 255-analogRead(A0)/4);
}
