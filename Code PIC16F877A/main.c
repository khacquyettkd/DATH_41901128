#include <16F877A.h>
#include <string.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP
#use delay(crystal=20000000)
#use i2c(Master, sda = PIN_C4, scl=PIN_C3)
#use rs232(UART1, baud=9600, xmit=PIN_C6, rcv=PIN_C7, stream=STREAM_UART1, parity=N, stop=1)
#BIT TMR1IF = 0x0C.0

#define bt1 PIN_D0
#define bt2 PIN_D1
#define bt3 PIN_D2
#define bt4 PIN_D3
#define bt5 PIN_D4
#define bt6 PIN_D5
#define bt7 PIN_D6
#define bt8 PIN_D7

#define ledtest PIN_B4
#define debugPin PIN_E2

const int ch1 = 8;
const int ch2 = 6;
const int ch3 = 16;
const int ch4 = 7;
const int ch5 = 9;
const int ch6 = 13;
const int ch7 = 11;
const int ch8 = 10;
const int ch9 = 1;
const int ch10 = 2;
const int ch11 = 5;
const int ch12 = 3;
const int ch13 = 4;
const int ch14 = 15;
const int ch15 = 14;
const int ch16 = 12;

//!char buffer[50];
unsigned int8 angle1,angle2,angle3,angle4,angle5,angle6,angle7,angle8,angle9,angle10,angle11,angle12,angle13,angle14,angle15,angle16;
int1 debugMode = false;
int1 echoMode =false;
char receivedData[50];  
int index = 0;
void PCA_init_()
{
   i2c_init(100000); //F=100kHz

   i2c_start();
   i2c_write(0x80); 
   i2c_write(0xFE);  //Prescale
   i2c_write(0x7A);  //0x7A=122-->50Hz
   i2c_stop();       

   i2c_start();
   i2c_write(0x80);
   i2c_write(0x00);  //Mode 1
   i2c_write(0x21); // Auto-Increment , ALL CALL
   i2c_stop();
}
void PCA_write(unsigned int channel,unsigned int16 ON, unsigned int16 OFF)
{
   if(channel>15)
      return;
      
   int channel_address=channel*4+6;
   int8 ON_L= ON & 0xFF;
   int8 ON_H= (ON & 0xF00)>>8;
   int8 OFF_L= OFF & 0xFF;
   int8 OFF_H= (OFF & 0xF00)>>8;
   
   i2c_start();
   i2c_write(0x80);
   i2c_write(channel_address);
   i2c_write(ON_L);
   i2c_write(ON_H);
   i2c_write(OFF_L);
   i2c_write(OFF_H);
   i2c_stop();
}
void servo_angle(int servo_number, int angle)//angle 35-175
{
   if((servo_number<1)||(servo_number>16))
      return;
   if((angle<35)||(angle>175))
      return;
   float x=(angle*(450-90)/180); //max pulse 500(but obstructed)
   int16 off_count=(int16)x+90;
   PCA_write(servo_number-1,0,off_count);
}
void stringToNum(char *str) {
   if(str[0] - '0'==1){
      angle1 = (str[2] - '0') * 100 + (str[3] - '0') * 10 + (str[4] - '0');
      angle2 = (str[5] - '0') * 100 + (str[6] - '0') * 10 + (str[7] - '0');
      angle3 = (str[8] - '0') * 100 + (str[9] - '0') * 10 + (str[10] - '0');
      angle4 = (str[11] - '0') * 100 + (str[12] - '0') * 10 + (str[13] - '0');
      angle5 = (str[14] - '0') * 100 + (str[15] - '0') * 10 + (str[16] - '0');
      angle6 = (str[17] - '0') * 100 + (str[18] - '0') * 10 + (str[19] - '0');
      angle7 = (str[20] - '0') * 100 + (str[21] - '0') * 10 + (str[22] - '0');
      angle8 = (str[23] - '0') * 100 + (str[24] - '0') * 10 + (str[25] - '0');
      servo_angle(ch1,angle1);
      servo_angle(ch2,angle2);
      servo_angle(ch3,angle3);
      servo_angle(ch4,angle4);
      servo_angle(ch5,angle5);
      servo_angle(ch6,angle6);
      servo_angle(ch7,angle7);
      servo_angle(ch8,angle8);
   } else if(str[0] - '0'==2)
   {
      angle9 = (str[2] - '0') * 100 + (str[3] - '0') * 10 + (str[4] - '0');
      angle10 = (str[5] - '0') * 100 + (str[6] - '0') * 10 + (str[7] - '0');
      angle11 = (str[8] - '0') * 100 + (str[9] - '0') * 10 + (str[10] - '0');
      angle12 = (str[11] - '0') * 100 + (str[12] - '0') * 10 + (str[13] - '0');
      angle13 = (str[14] - '0') * 100 + (str[15] - '0') * 10 + (str[16] - '0');
      angle14 = (str[17] - '0') * 100 + (str[18] - '0') * 10 + (str[19] - '0');
      angle15 = (str[20] - '0') * 100 + (str[21] - '0') * 10 + (str[22] - '0');
      angle16 = (str[23] - '0') * 100 + (str[24] - '0') * 10 + (str[25] - '0'); 
      servo_angle(ch9,angle9);
      servo_angle(ch10,angle10);
      servo_angle(ch11,angle11);
      servo_angle(ch12,angle12);
      servo_angle(ch13,angle13);
      servo_angle(ch14,angle14);
      servo_angle(ch15,angle15);
      servo_angle(ch16,angle16);
   }else{
   printf("Sai cu phap");
   }
}
#INT_RDA
void serial_isr() {

   char currentChar = getc(); 
   if(currentChar=='(') //ky tu bat dau
   {
      index=0; 
   }
   else if(currentChar==')')
   {
      receivedData[index] = '\0';
      if(debugMode==1)
      {
         printf(receivedData);
      }
      stringToNum(receivedData);
      for(int j=0;j<=index;j++)
         {
            receivedData[j]=0;
         }
      index=0;
   }
   else if (index < sizeof(receivedData) - 1) {
        receivedData[index] = currentChar;
        if(echoMode==1)
        {
         putc(currentChar);
         printf("i: %d\n", index);
        }
        index++;
    }
 
}

void main() {
    delay_ms(1000);  // Delay for a moment for the UART to stabilize
   
    set_tris_d(0xff);
    
    PCA_init_();
    
    output_high(ledtest);
    enable_interrupts(INT_RDA);
    enable_interrupts(GLOBAL);

    int1 multiButtonState=0;
    while(true) 
    {
         if(input(bt1)==0)
         {
            delay_ms(20);
            if(input(bt1)==0)//xac nhan nut 1 thuc su duoc nhan
            {  
               while(input(bt1)==0)//trong khi nut 1 van duoc nhan
               { 
                  if(input(bt7)==0) //neu nut 7 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt7)==0) //xac nhan nut 7 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("1&7");
                           while((input(bt1)==0)||(input(bt7)==0));//cho den ca trong 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("1");
               }
               multiButtonState=0;
            }
         }
         if(input(bt2)==0)
         {
            delay_ms(20);
            if(input(bt2)==0)//xac nhan nut 1 thuc su duoc nhan
            {  
               while(input(bt2)==0)//trong khi nut 1 van duoc nhan
               { 
                  if(input(bt7)==0) //neu nut 7 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt7)==0) //xac nhan nut 7 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("2&7");
                           while((input(bt2)==0)||(input(bt7)==0));//cho den ca trong 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("2");
               }
               multiButtonState=0;
            }
         }

         if(input(bt3)==0)
         {
            delay_ms(20);
            if(input(bt3)==0)//xac nhan nut 3 thuc su duoc nhan
            {  
               while(input(bt3)==0)//trong khi nut 3 van duoc nhan
               { 
                  if(input(bt4)==0) //neu nut 3 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt4)==0) //xac nhan nut 4 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("3&4");
                           while((input(bt3)==0)||(input(bt4)==0));//cho den ca trong 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("3");
               }
               multiButtonState=0;
            }
         }
         if(input(bt4)==0)
         {
            delay_ms(20);
            if(input(bt4)==0)//xac nhan nut 4 thuc su duoc nhan
            {  
               while(input(bt4)==0)//trong khi nut 4 van duoc nhan
               { 
                  if(input(bt3)==0) //neu nut 3 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt3)==0) //xac nhan nut 3 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("4&3");
                           while((input(bt3)==0)||(input(bt4)==0));//cho den khi ca 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("4");
               }
               multiButtonState=0;
            }
         }

         if(input(bt5)==0)
         {
            delay_ms(20);
            if(input(bt5)==0)//xac nhan nut 5 thuc su duoc nhan
            {  
               while(input(bt5)==0)//trong khi nut 5 van duoc nhan
               { 
                  if(input(bt6)==0) //neu nut 6 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt6)==0) //xac nhan nut 6 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("5&6");
                           while((input(bt5)==0)||(input(bt6)==0));//cho den ca trong 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("5");
               }
               multiButtonState=0;
            }
         }
         if(input(bt6)==0)
         {
            delay_ms(20);
            if(input(bt6)==0)//xac nhan nut 6 thuc su duoc nhan
            {  
               while(input(bt6)==0)//trong khi nut 6 van duoc nhan
               { 
                  if(input(bt5)==0) //neu nut 5 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt5)==0) //xac nhan nut 5 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("6&5");
                           while((input(bt5)==0)||(input(bt6)==0));//cho den khi ca 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("6");
               }
               multiButtonState=0;
            }
         }

         if(input(bt7)==0)
         {
            delay_ms(20);
            if(input(bt7)==0)//xac nhan nut 7 thuc su duoc nhan
            {  
               while(input(bt7)==0)//trong khi nut 7 van duoc nhan
               { 
                  if(input(bt1)==0) //neu nut 1 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt1)==0) //xac nhan nut 1 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("7&1");
                           while((input(bt1)==0)||(input(bt7)==0));//cho den ca trong 2 nut nha ra
                        }
                     }
                    if(input(bt2)==0) //neu nut 1 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt2)==0) //xac nhan nut 1 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           printf("7&2");
                           while((input(bt2)==0)||(input(bt7)==0));//cho den ca trong 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("7");
               }
               multiButtonState=0;
            }
         }
         if(input(bt8)==0)
         {
            delay_ms(20);
            if(input(bt8)==0)//xac nhan nut 6 thuc su duoc nhan
            {  
               while(input(bt8)==0)//trong khi nut 6 van duoc nhan
               { 
                  if(input(bt7)==0) //neu nut 5 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt7)==0) //xac nhan nut 5 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           //hien thi mode
                           if(debugMode==0){
                              debugMode=1;
                              printf("debug Mode On\n");
                              printf("data:");
                              printf(receivedData);
                              printf("\nindex: %d\n", index);
                              }
                           else if(debugMode==1){
                              debugMode=0;
                              printf("debug Mode Off\n");
                              }
                           for(int i=0;i<=index;i++)
                           {
                              receivedData[i]=0;
                           }
                           index=0; 
                           while((input(bt7)==0)||(input(bt8)==0));//cho den khi ca 2 nut nha ra
                        }
                     }
                  if(input(bt6)==0) //neu nut 1 duoc nhan
                     {
                        delay_ms(20);
                        if(input(bt6)==0) //xac nhan nut 1 thuc su duoc nhan
                        { 
                           multiButtonState=1;
                           if(echoMode==0){
                              echoMode=1;
                              printf("Echo Mode On\n");
                              }
                           else if(echoMode==1){
                              echoMode=0;
                              printf("Echo Mode Off\n");
                              }
                           while((input(bt6)==0)||(input(bt7)==0));//cho den ca trong 2 nut nha ra
                        }
                     }
               }
               if(multiButtonState==0)
               {
                 printf("8");
               }
               multiButtonState=0;
            }
         }
   }
}



