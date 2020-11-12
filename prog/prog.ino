//#include <avr/power.h>
//#include <avr/sleep.h>
//#include <avr/interrupt.h>
#include <SoftwareSerial.h>



SoftwareSerial Serial_2(PD4, PD2);

#define X0 3
#define X1 6
#define X2 5
#define X3 10
#define ConvVCC 7
#define PntL 17
#define PntR 1

#define Frst 15
#define Scnd 16
#define Thrd 0

#define Btn 2

#define DS_Sens 4

#define PWM_pin 9

#define lambda_pin 14

#define OpenAir 5
#define OpenAirVout 1024
double ConvK = 0.0048828125; //OpenAir/OpenAirVout;

//Вид таблицы AFR: Vout, A, B
const PROGMEM float afrtable [30][3] = {
  { 1.4, 3, 5.88 }, //10.08
  { 1.45, 3, 5.88 }, //10.23
  { 1.5, 3, 5.88 }, //10.38
  { 1.55, 3.2, 5.57 }, //10.53
  { 1.6, 3.4, 5.25 }, //10.69
  { 1.65, 3.4,5.25 }, //10.86
  { 1.7, 3.4, 5.25 }, //11.03
  { 1.75, 3.6, 4.9 }, //11.2
  { 1.8, 3.8, 4.54 }, //11.38
  { 1.85, 3.8, 4.54 }, //11.57
  { 1.9, 4, 4.16 }, //11.76
  { 1.95, 4.2, 3.77 }, //11.96
  { 2, 4.2, 3.77 }, //12.17
  { 2.05, 4.4, 3.36 }, //12.38
  { 2.1, 4.6, 2.94 }, //12.6
  { 2.15, 4.8, 2.51 }, //12.83
  { 2.2, 4.8, 2.51 }, //13.07
  { 2.25, 5.2, 1.61 }, //13.31
  { 2.3, 5.4, 1.15 }, //13.57
  { 2.35, 5.4, 1.15 }, //13.84
  { 2.4, 5.8, 0.19 }, //14.11
  { 2.45, 6, -0.3 }, //14.4
  { 2.5, 11, -12.8 }, //14.7
  { 2.55, 11.8, -14.84 }, //15.25
  { 2.6, 12.8, -17.44 }, //15.84
  { 2.65, 14, -20.62 }, //16.48
  { 2.7, 15, -23.32 }, //17.18
  { 2.75, 16.6, -27.72 }, //17.93
  { 2.8, 18, -31.64 }, //18.76
  { 2.85, 20, -37.34 }, //19.66
  }; 

int AFR = 123;
double vout = 0;
double voutADC = 0;
int32_t vouttmp = 0;
int voutprv = 1;
#define voutsmpls 20
int voutn = 0;
#define ShwnTm 1
int Shw = 0;                     
//int btn = 0;
//bool slp = false;
int dltmp = 0;
int dl = 25;

int FrstL = 0, ScndL = 0, ThrdL = 0;

bool Chk = false;

void tabl(int numr = 0){ //ДАННАЯ ТАБЛИЦА ИСТИННОСТИ ВЕРНА ТОЛЬКО ДЛЯ ДАННОЙ ПЛАТЫ ТЕРМОМЕТРА
  switch(numr){
    case 0: 
      digitalWrite(X0, LOW); 
      digitalWrite(X1, LOW); 
      digitalWrite(X2, LOW); 
      digitalWrite(X3, LOW);      
    break;
    case 7:
      digitalWrite(X0, HIGH); 
      digitalWrite(X1, LOW); 
      digitalWrite(X2, LOW); 
      digitalWrite(X3, LOW);   
    break;
    case 1:
      digitalWrite(X0, LOW); 
      digitalWrite(X1, HIGH); 
      digitalWrite(X2, LOW); 
      digitalWrite(X3, LOW); 
    break;
    case 3:
      digitalWrite(X0, HIGH); 
      digitalWrite(X1, HIGH); 
      digitalWrite(X2, LOW); 
      digitalWrite(X3, LOW); 
    break;
    case 4:
      digitalWrite(X0, LOW); 
      digitalWrite(X1, LOW); 
      digitalWrite(X2, HIGH); 
      digitalWrite(X3, LOW); 
    break;
    case 5:
      digitalWrite(X0, HIGH); 
      digitalWrite(X1, LOW); 
      digitalWrite(X2, HIGH); 
      digitalWrite(X3, LOW); 
    break;
    case 6:
      digitalWrite(X0, LOW); 
      digitalWrite(X1, HIGH); 
      digitalWrite(X2, HIGH); 
      digitalWrite(X3, LOW); 
    break;
    case 2:
      digitalWrite(X0, HIGH); 
      digitalWrite(X1, HIGH); 
      digitalWrite(X2, HIGH); 
      digitalWrite(X3, LOW); 
    break;
    case 8:
      digitalWrite(X0, LOW); 
      digitalWrite(X1, LOW); 
      digitalWrite(X2, LOW); 
      digitalWrite(X3, HIGH); 
    break;
    case 9:
      digitalWrite(X0, HIGH); 
      digitalWrite(X1, LOW); 
      digitalWrite(X2, LOW); 
      digitalWrite(X3, HIGH); 
    break;   
  }  
}

void AFRSHWN(int a = 0){
  for(int i = 0; i < a; i++)
    {
      digitalWrite(Frst, HIGH);
      tabl(FrstL);
      delay(2); 
      digitalWrite(Frst, LOW);
      delay(2);  
             
      digitalWrite(Scnd, HIGH);
      if(Chk == false)
        digitalWrite(PntR, HIGH);
      tabl(ScndL);
      delay(2); 
      digitalWrite(Scnd, LOW);
      if(Chk == false)
        digitalWrite(PntR, LOW);
      delay(2); 
      
      digitalWrite(Thrd, HIGH);
      if(Chk == true)
        digitalWrite(PntR, HIGH);
      tabl(ThrdL);
      delay(2); 
      digitalWrite(Thrd, LOW);
      if(Chk == true)
        digitalWrite(PntR, LOW);
      delay(2); 
    } 
}

void setup() {
 /* ADCSRA &= ~(1 << ADEN); 
  power_usart0_disable();
  power_twi_disable();  */
 // attachInterrupt(Btn, ButInter, RISING); 
 
  Serial_2.begin(19200);
  Serial_2.println("R");
  
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00000001;
  pinMode(Frst, OUTPUT);  
  pinMode(Scnd, OUTPUT);
  pinMode(Thrd, OUTPUT);

  pinMode(X0, OUTPUT);
  pinMode(X1, OUTPUT); 
  pinMode(X2, OUTPUT); 
  pinMode(X3, OUTPUT); 
  pinMode(ConvVCC, OUTPUT);  
  pinMode(PWM_pin, OUTPUT);  
  //analogReference(DEFAULT);
  pinMode(lambda_pin, INPUT);

  analogWrite(PWM_pin, 140);
  digitalWrite(ConvVCC, HIGH);
  digitalWrite(Frst, HIGH); 
  digitalWrite(Scnd, LOW); 
  digitalWrite(Thrd, LOW);  
 
  //Lamp_anti_poisoning
 for(int i = 0; i < 10; i++){
    FrstL = i;
    ScndL = i;
    ThrdL = i;
    AFRSHWN(120);
    }  
}


/*void ButInter(){    
    btn++;   
  }*/
void loop(){ 
  dltmp++;
  if(dltmp >= dl){
      dltmp = 0;
      Serial_2.println(AFR);
    }
  vouttmp += analogRead(lambda_pin);
  voutn++;
  if(voutn == voutsmpls - 1 ){
    vout = (vouttmp / voutsmpls) * ConvK;
    voutADC = vouttmp / voutsmpls;
    vouttmp = 0;
    voutn = 0;
    }
  for(int i = 0; i < 30; i++){    
    if(vout >= pgm_read_float(&afrtable[i][0]) && vout < pgm_read_float(&afrtable[i + 1][0])){
        AFR = (int)((pgm_read_float(&afrtable[i][1]) * vout + pgm_read_float(&afrtable[i][2]))*10);
      }
    /*if(vout >= afrtable[i][0] && vout < afrtable[i + 1][0]){
        AFR = (int)((afrtable[i][1] * vout + afrtable[i][2])*100);
      }*/
    }
    
  //Вывод напряжения без преобразования
 /* FrstL = (int)voutADC / 100;
  ScndL = ((int)(voutADC) % 100)/10;
  ThrdL = (int)(voutADC) % 10;*/
  //Вывод напряжения с платы лямбды
 /* FrstL = ((int)vout * 100) / 100;
  ScndL = ((int)(vout * 100) % 100)/10;
  ThrdL = (int)(vout * 100) % 10;*/
  
  //Вывод состава смеси
  FrstL = AFR / 100;
  ScndL = (AFR % 100)/10;
  ThrdL = AFR % 10;

 /* if(Shw == 0){    
    
    digitalWrite(ConvVCC, HIGH);          
    analogWrite(PWM_pin, 155); // Переменная ШИМ
    delay(500);     
       
  }  */     

 // if(Shw == 1){             
    AFRSHWN(ShwnTm);    
    /*analogWrite(5, 0);
    digitalWrite(0, LOW); 
    TmShw = 0; 
    Chk = false; 
    btn = 0; 
    sleep_mode();  */
 // } 

 
}
 
