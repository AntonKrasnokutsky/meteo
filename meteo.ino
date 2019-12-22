// Переменные, создаваемые процессом сборки,
// когда компилируется скетч
extern int __bss_end;
extern void *__brkval;

// Функция, возвращающая количество свободного ОЗУ (RAM)
int memoryFree()
{
   int freeValue;
   if((int)__brkval == 0)
      freeValue = ((int)&freeValue) - ((int)&__bss_end);
   else
      freeValue = ((int)&freeValue) - ((int)__brkval);
   return freeValue;
}


//экран
#include <OLED_I2C.h>

OLED  myOLED(SDA, SCL, 8);

extern uint8_t EnFontMedium[];

//часы
#include <Wire.h>
#include "RTClib.h"
DateTime now;

RTC_DS1307 rtc;

//влажность
#include "DHT.h"
#define DHTPIN 2

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float t;

//датчик давления
#include <BMP085.h>
//#include <SFE_BMP180.h>
long Pressure = 0, Humidity=0;


BMP085 dps = BMP085();
//SFE_BMP180 pressure;
//#define ALTITUDE 26.0
//double t, P;

//флешка

#include <SPI.h>
#include <SD.h>

bool sdOK=false;//инициализаци флешки
File myFile;

int e=2019,m=5,d=15,h=12,mm=0;//год, месяц, день, час, минута
#define setBtn 3 
#define plsBtn 4
#define TimeOut 1000 //задержка (циклов) перехода в другой режим
int TimeOutBtn=0; //счетчик задерщки изменения режима
byte pos=0;//позиция курсора 0 - часы 1 минуты 2 день 3 месяц 4 год
bool set=0;//0 - обычнй режим 1 - режим настройки даты времени, в этом режиме длительное нажатие перемещает курсор вправо, короткое нажатие работает как инкримент. после перемещения курсора за пределы экрана режим меняет на 0 "обычный"
#define saveInt 1 //интервал записи на SD в минутах
#define blinkInt 2 //интервал моргания в секундах
byte lastMin=0,lastBlink=0;//время последнего сохранения
bool saveOK=false;//тригер начала/конца записи на SD

void setup()
{
  //экран
  myOLED.begin();
  myOLED.setFont(EnFontMedium);
  setupOLED();

  //часы
  rtc.begin();
  setTime();
  //rtc.adjust(DateTime(2019, 5, 10, 12, 0, 0));

  //датчик давления
  Wire.begin();
  dps.init(MODE_ULTRA_HIGHRES, 2600, true);
  //pressure.begin();

  //датчик влажности и давления
  dht.begin();

  //флешка
  if (SD.begin(53))
  {
    myOLED.update();
    sdOK=true;
  }
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(setBtn, INPUT);
  pinMode(plsBtn, INPUT);
}

void setTime()
{
  rtc.adjust(DateTime(e, m, d, h, mm, 0));
}

void setupOLED()
{
  
  myOLED.clrScr();
  myOLED.print(":", 21, 0);
  myOLED.print( "no", 0, 0); //вывод времени (часы)
  myOLED.print("no", 30, 0); //вывод времени (минуты)

  myOLED.print(":", 9, 16);
  myOLED.print("mm", 56, 16);
  myOLED.print("p", 0, 16);
  myOLED.print("no", 18, 16); //вывод давления

  myOLED.print(":", 9, 32);
  myOLED.print("no.n", 18, 32); //вывод температуры
  myOLED.print("C", 68, 32);
  myOLED.print("T", 0, 32);
  

  myOLED.print(":", 9, 48);
  myOLED.print("f", 0, 48);
  myOLED.print("%", 54, 48);
  myOLED.print("no", 30, 48); //вывод влажности

  if(sdOK&&!saveOK)
  {
    for (byte i=0; i<5; i++)
    {
      myOLED.drawLine(123, i, 127, i);
    };
  };
  
  myOLED.update();
}

void setOLED()
{
  myOLED.print(":", 21, 0);
  
  myOLED.print(".", 25, 25);
  myOLED.print(".", 55, 25);
  if(d<10)
  {
    myOLED.printNumI(0, 0, 24); //день
    myOLED.printNumI(d, 12, 24); //день
  }
  else
    myOLED.printNumI(d, 0, 24); //день
  if(m<10)
  {
    myOLED.printNumI(0, 30, 24); //день
    myOLED.printNumI(m, 42, 24); //день
  }
  else
    myOLED.printNumI(m, 30, 24); //месяц
  myOLED.printNumI(e, 60, 24); //год

  if (h<10)
  {
     myOLED.printNumI(0, 0, 0); //вывод времени (часы)
     myOLED.printNumI(h, 12, 0); //вывод времени (часы)
  }
  else
  {
     myOLED.printNumI(h, 0, 0); //вывод времени (часы)
  }
  if (mm<10)
  {
     myOLED.printNumI(0, 30, 0); //вывод времени (часы)
     myOLED.printNumI(mm, 42, 0); //вывод времени (часы)
  }
  else
  {
     myOLED.printNumI(mm, 30, 0); //вывод времени (часы)
  }

  
  //myOLED.update();
}

void printTime()
{
  //DateTime now = rtc.now();
  if (now.hour()<10)
  {
     myOLED.printNumI(0, 0, 0); //вывод времени (часы)
     myOLED.printNumI(now.hour(), 12, 0); //вывод времени (часы)
  }
  else
  {
     myOLED.printNumI(now.hour(), 0, 0); //вывод времени (часы)
  }
  if (now.minute()<10)
  {
     myOLED.printNumI(0, 30, 0); //вывод времени (часы)
     myOLED.printNumI(now.minute(), 42, 0); //вывод времени (часы)
  }
  else
  {
     myOLED.printNumI(now.minute(), 30, 0); //вывод времени (часы)
  }
}

void printPressure()
{
  dps.getPressure(&Pressure);
  Pressure=Pressure/133.322;
  myOLED.printNumI(Pressure, 18, 16); //вывод давления
}

void printTemperature()
{
  if (t<10)
  {
    myOLED.printNumI(0, 18, 32);
    myOLED.printNumF(t, 1, 30, 32); //вывод температуры
  }
  else
  {
    myOLED.printNumF(t, 1, 18, 32); //вывод температуры
  }  
}

void printHumidity()
{
  Humidity=8192-memoryFree();
myOLED.printNumI(Humidity, 30, 48);
}

bool savePriod()
{
  byte tmp=0;
  if(now.minute()-saveInt<0)
  {
    tmp=60+(now.minute()-saveInt);
  }
  else tmp=now.minute()-saveInt;
  if(lastMin==tmp)
  {
    lastMin=now.minute();
    return true;
    //break;
  }
  else
    return false;
  //break;
}

void saveData()
{
  if(savePriod())
  {
    String nameFile = "";
    nameFile += now.year();
    if (now.month()<10)
    {
      nameFile += "0";
      nameFile += now.month();
    }
    else
      nameFile += now.month();
    if (now.day()<10)
    {
      nameFile += "0";
      nameFile += now.day();
    }
    else
      nameFile += now.day();
    nameFile += ".txt";
  
    myFile = SD.open(nameFile, FILE_WRITE);
    if (myFile)
    {
      myFile.print(now.year());
      myFile.print("-");
      if (now.month()<10)
      {
        myFile.print("0");
        myFile.print(now.month());
      }
      else
        myFile.print(now.month());
      myFile.print("-");
      if (now.day()<10)
      {
        myFile.print("0");
        myFile.print(now.day());
      }
      else
        myFile.print(now.day());
      myFile.print(";");
      if (now.hour()<10)
      {
        myFile.print("0"); //вывод времени (часы)
        myFile.print(now.hour()); //вывод времени (часы)
      }
      else
      {
        myFile.print(now.hour()); //вывод времени (часы)
      }
      myFile.print(":");
      if (now.minute()<10)
      {
        myFile.print("0"); //вывод времени (минуты)
        myFile.print(now.minute()); //вывод времени (минуты)
      }
      else
      {
        myFile.print(now.minute()); //вывод времени (минуты)
      }
      myFile.print(":");
      if (now.second()<10)
      {
        myFile.print("0"); //вывод времени (секунды)
        myFile.print(now.second()); //вывод времени (секунды)
      }
      else
      {
        myFile.print(now.second()); //вывод времени (секунды)
      }
      myFile.print(";");
      myFile.print(t);
      myFile.print(";");
      myFile.print(Pressure);
      myFile.print(";");
      myFile.println(Humidity);
      myFile.close();
/*      for(byte z=0;z<2;z++)
        for(byte x=0;x<3;x++)
        {
          myOLED.invPixel(124+x, 1+z);
        }
      myOLED.update();*/
    }
    else
    {
      sdOK=SD.begin(53);
      saveOK=false;
    }
  }
}

void pushBtn()
{
  #define y1 17
  #define y2 43
  
  if (digitalRead(setBtn) == HIGH)
  {
    if(set==0)
    {
      while(digitalRead(setBtn) == HIGH)
      {
        TimeOutBtn++;
        delay(1);
        if(TimeOutBtn>=TimeOut)
        {
          set=1;
          TimeOutBtn=0;
          myOLED.clrScr();
          setOLED();
          myOLED.drawLine(0,y1,24,y1);
          myOLED.drawLine(0,y1+1,24,y1+1);
          myOLED.update();
          while(digitalRead(setBtn) == HIGH){}
        }
      }
    }
    if(set==1)
    {
      while(digitalRead(setBtn) == HIGH)
      {
        TimeOutBtn++;
        delay(1);
        if(TimeOutBtn>=TimeOut)
        {
          pos++;
          TimeOutBtn=0;
          switch (pos) {
            case 0:
              myOLED.drawLine(0,y1,24,y1);
              myOLED.drawLine(0,y1+1,24,y1+1);
              myOLED.update();
            case 1:
              myOLED.clrLine(0,y1,24,y1);
              myOLED.clrLine(0,y1+1,24,y1+1);
              myOLED.drawLine(30,y1,54,y1);
              myOLED.drawLine(30,y1+1,54,y1+1);
              myOLED.update();
              break;
            case 2:
              myOLED.clrLine(30,y1,54,y1);
              myOLED.clrLine(30,y1+1,54,y1+1);
              myOLED.drawLine(0,y2,24,y2);
              myOLED.drawLine(0,y2+1,24,y2+1);
              myOLED.update();
              break;
            case 3:
              myOLED.clrLine(0,y2,24,y2);
              myOLED.clrLine(0,y2+1,24,y2+1);
              myOLED.drawLine(30,y2,54,y2);
              myOLED.drawLine(30,y2+1,54,y2+1);
              myOLED.update();
              break;
            case 4:
              myOLED.clrLine(30,y2,54,y2);
              myOLED.clrLine(30,y2+1,54,y2+1);
              myOLED.drawLine(60,y2,108,y2);
              myOLED.drawLine(60,y2+1,108,y2+1);
              myOLED.update();
              break;
            default:
              pos=0;
              set=0;
              myOLED.clrScr();
              rtc.adjust(DateTime(e, m, d, h, mm, 0));
              setupOLED();
          }

          /*if(pos>4)
          {
            
          }*/
          while(digitalRead(setBtn) == HIGH){}
        }
      }
      if(TimeOutBtn!=0)
      {
        switch (pos) {
          case 0:
            h++;
            if(h>23)h=0;
            break;
          case 1:
            mm++;
            if(mm>59)mm=0;
            break;
          case 2:
            d++;
            if(m==1||m==3||m==5||m==7||m==8||m==10||m==12) if(d>31)d=1;
            if(m==2){
              if(e%4==0&&e%100!=0||e%400==0) 
                if(d>29)d=1;//високосный год
              else 
                if(d>28)d=1;//невисокосный год
              }
            if(m==4||m==6||m==9||m==11) if(d>30)d=1;
            break;
          case 3:
            m++;
            if(m>12)m=1;
            break;
          case 4:
            e++;
            if(m==2)
            {
              if(!(e%4==0&&e%100!=0||e%400==0))
                if(d==29)d=28;//невисокосный год
            }
            break;
        }
      }
    }
  }

  if(digitalRead(plsBtn) == HIGH)
  {
    delay(1);
    if(set==1)
    {
      while(digitalRead(plsBtn) == HIGH)
      {
        TimeOutBtn++;
        delay(1);
      }
      if(TimeOutBtn!=0)
      {
        switch (pos) {
          case 0:
            h--;
            if(h<0)h=23;
            break;
          case 1:
            mm--;
            if(mm<0)mm=59;
            break;
          case 2:
            d--;
            if(m==1||m==3||m==5||m==7||m==8||m==10||m==12) if(d<1)d=31;
            if(m==2){
            if(e%4==0&&e%100!=0||e%400==0) 
              if(d<1)d=29;//високосный год
            else 
              if(d<1)d=28;//невисокосный год
            }
            if(m==4||m==6||m==9||m==11) if(d<1)d=30;
            break;
          case 3:
            m--;
            if(m<1)m=12;
            break;
          case 4:
            e--;
            break;
        }
      }
    }
    if(sdOK&&set!=1)
    {
      while(digitalRead(plsBtn) == HIGH);
      saveOK=!saveOK;
      if(now.second()-saveInt<0)
      {
        lastMin=60+(now.second()-saveInt);
      }
      else lastMin=now.second()-saveInt;
      saveData();
      
      lastBlink=now.second();
    }
    
    if(!sdOK&&set!=1)
    {
      if (SD.begin(53))
      {
        for (int i=0; i<5; i++)
        {
          myOLED.drawLine(123, i, 127, i);
        }
        myOLED.update();
        sdOK=true;
        for (byte i=5; i<10; i++)
        {
          myOLED.clrLine(123, i, 127, i);
        };
      }
      while(digitalRead(plsBtn) == HIGH);
    }
  }
        
  TimeOutBtn=0;
}

void loop()
{
  now = rtc.now();
  t = dht.readTemperature();

  switch(set){
    case 0:
      e=now.year();
      m=now.month();
      d=now.day();
      h=now.hour();
      mm=now.minute();
      printTime();
      printPressure();
      printTemperature();
      printHumidity();
      break;
    case 1:
      setOLED();
      break;
  }

  if (sdOK&&saveOK) saveData();
  else
  {
//    if(sdOK) sdOK=SD.begin(53);
    if(!sdOK&&set==0)
    {
//      sdOK=SD.begin(53);
      for (byte i=0; i<5; i++)
      {
        myOLED.clrLine(123, i, 127, i);
      };
    };
    if(sdOK&&!saveOK&&set==0)
    {
      for (byte i=0; i<5; i++)
      {
        myOLED.drawLine(123, i, 127, i);
      };
    }
  }
    
  if(saveOK&&set==0)
  {
    if((lastBlink==now.second()-blinkInt)||(lastBlink==60+(now.second()-blinkInt)))
    {
      lastBlink=now.second();
      for(byte z=0;z<5;z++)
        for(byte x=0;x<4;x++)
        {
          myOLED.invPixel(123+x, z);
        }
    }
  }
  pushBtn();
  
  myOLED.update();
  //delay(60000);
}
