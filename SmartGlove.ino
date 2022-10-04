#include <SD.h>
#define SD_ChipSelectPin 4  //using digital pin 4 on arduino nano 328, can use other pins
#include <TMRpcm.h>           //  also need to include this library...
#include <SPI.h>
#include <avr/pgmspace.h>
TMRpcm tmrpcm;

#include <LiquidCrystal.h>     //lcd library

const int rs = 2, en = 3, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define finger1_flex A6       //flex sensor 1 connected to analog pin no A6
#define finger2_flex A5       //flex sensor 2 connected to analog pin no A5
#define finger3_flex A4       //flex sensor 3 connected to analog pin no A4
#define finger4_flex A3       //flex sensor 4 connected to analog pin no A3
#define thumb_flex A7         //flex sensor 5 connected to analog pin no A7

//#define none_bend_min 0       //minimum value of adc when finger is not bend 0
//#define none_bend_max 100     //maximum value of adc when finger is not bend 360
#define half_bend_min 350     //minimum value of adc when finger is little bend 361
#define half_bend_max 400     //maximum value of adc when finger is little bend 661
#define full_bend_min 405     //minimum value of adc when finger is full bend 723
#define full_bend_max 1023    //maximum value of adc when finger is full bend 1023

unsigned int f1 = 0;          //variable to store adc raw value of flex sensor 1
unsigned int f2 = 0;          //variable to store adc raw value of flex sensor 2
unsigned int f3 = 0;          //variable to store adc raw value of flex sensor 3
unsigned int f4 = 0;          //variable to store adc raw value of flex sensor 4
unsigned int t = 0;           //variable to store adc raw value of flex sensor 5

unsigned char f1_state = 0;   //variable to store finger-1 bend state 0=no bend or dead band, 1=little or half bend, 2=full bend
unsigned char f2_state = 0;   //variable to store finger-2 bend state 0=no bend or dead band, 1=little or half bend, 2=full bend
unsigned char f3_state = 0;   //variable to store finger-3 bend state 0=no bend or dead band, 1=little or half bend, 2=full bend
unsigned char f4_state = 0;   //variable to store finger-4 bend state 0=no bend or dead band, 1=little or half bend, 2=full bend
unsigned char t_state = 0;    //variable to store thumb bend state 0=no bend or dead band, 1=little or half bend, 2=full bend

unsigned int value = 0;       //value calculated from state of finger 1-4 (0-80)

char buf[16];

bool audio_played = 0;

#define max_msg_count 20

const char msg1[16] PROGMEM = "message-1";
const char msg2[16] PROGMEM = "message-2";
const char msg3[16] PROGMEM = "message-3";
const char msg4[16] PROGMEM = "message-4";
const char msg5[16] PROGMEM = "message-5";
const char msg6[16] PROGMEM = "message-6";
const char msg7[16] PROGMEM = "message-7";
const char msg8[16] PROGMEM = "message-8";
const char msg9[16] PROGMEM = "message-9";
const char msg10[16] PROGMEM = "message-10";
const char msg11[16] PROGMEM = "message-11";
const char msg12[16] PROGMEM = "message-12";
const char msg13[16] PROGMEM = "message-13";
const char msg14[16] PROGMEM = "message-14";
const char msg15[16] PROGMEM = "message-15";
const char msg16[16] PROGMEM = "message-16";
const char msg17[16] PROGMEM = "message-17";
const char msg18[16] PROGMEM = "message-18";
const char msg19[16] PROGMEM = "message-19";
const char msg20[16] PROGMEM = "message-20";

const char *const msg[max_msg_count] PROGMEM = {  
                              msg1,
                              msg2,
                              msg3,
                              msg4,
                              msg5,
                              msg6,
                              msg7,
                              msg8,
                              msg9,
                              msg10,
                              msg11,
                              msg12,
                              msg13,
                              msg14,
                              msg15,
                              msg16,
                              msg17,
                              msg18,
                              msg19,
                              msg20
                             };

char state(unsigned int st)   //function to convert raw adc data to bend state (return bend state) 
{
  if(st >= half_bend_min && st<= half_bend_max)
  {
    return 1;
  }
  else if(st >= full_bend_min && st<= full_bend_max)
  {
    return 2;
  }
  
  return 0;
}

void calculate_value()        //function to calculate value from finger's bend state
{
  value = f1_state + (f2_state * 3) + (f3_state * 9) + (f4_state * 27);
  
  return;
}

void setup()                  //for setup of pins and other peripherals (one time only at startup)
{
  pinMode(finger1_flex,INPUT);//define pin as input to take analog reading
  pinMode(finger2_flex,INPUT);
  pinMode(finger3_flex,INPUT);
  pinMode(finger4_flex,INPUT);
  pinMode(thumb_flex,INPUT);

  analogReference(INTERNAL);   //set adc reference value to 1.1v which is internally generated 

  lcd.begin(16, 2);            //16x2 lcd initialize

  tmrpcm.speakerPin = 9;
  if (!SD.begin(SD_ChipSelectPin)) 
  { 
    Serial.println("SD fail");  
    return;
  }
  tmrpcm.volume(10);
}

void loop()                   //for run logic(code) continuesly like while(1) loop
{  
  f1 = analogRead(finger1_flex); //take analog value of flex sensor to variables
  f2 = analogRead(finger2_flex);
  f3 = analogRead(finger3_flex);
  f4 = analogRead(finger4_flex);
  t = analogRead(thumb_flex);

  f1_state = state(f1);          //define states from analog value of flex sensor
  f2_state = state(f2);
  f3_state = state(f3);
  f4_state = state(f4);
  t_state = state(t);

  if(t_state != 0)              //check thumb is bend or not if bend calculate value else not
  {
    calculate_value();          //find value from finger state

    if(value > max_msg_count)    //count more than message array size
    {
      value = 0;
    }

    if(value != 0)                //some movement occurs give the output according to value
    {
      strcpy_P(buf, (char *)pgm_read_word(&(msg[value-1])));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(buf);
    }

    audio_played = 0;
  }
  else if(audio_played == 0)
  {
    switch(value)
    {
      case 1:
        tmrpcm.play("1.wav");
        break;
      case 2:
        tmrpcm.play("2.wav");
        break;
      case 3:
        tmrpcm.play("3.wav");
        break;
      case 4:
        tmrpcm.play("4.wav");
        break;
      case 5:
        tmrpcm.play("5.wav");
        break;
      case 6:
        tmrpcm.play("6.wav");
        break;
      case 7:
        tmrpcm.play("7.wav");
        break;
      case 8:
        tmrpcm.play("8.wav");
        break;
      case 9:
        tmrpcm.play("9.wav");
        break;
      case 10:
        tmrpcm.play("10.wav");
        break;
      case 11:
        tmrpcm.play("11.wav");
        break;
      case 12:
        tmrpcm.play("12.wav");
        break;
      case 13:
        tmrpcm.play("13.wav");
        break;
      case 14:
        tmrpcm.play("14.wav");
        break;
      case 15:
        tmrpcm.play("15.wav");
        break;
      case 16:
        tmrpcm.play("16.wav");
        break;
      case 17:
        tmrpcm.play("17.wav");
        break;
      case 18:
        tmrpcm.play("18.wav");
        break;
      case 19:
        tmrpcm.play("19.wav");
        break;
      case 20:
        tmrpcm.play("20.wav");
        break;
      default:
        break;
    }
    audio_played = 1;
  }
  
  delay(100);                   //some delay for proper output on lcd
}
