#include <msp430x54x.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//ok
int send_search_ready=0;
void process_key(void)
{
    int debounce=10;
    if(!(P9IN&BIT6))
    {
      while(debounce--)
      {
        if((P9IN&BIT6))
        {
          printf("no thing\r\n");
          break;
        }
        delay(20);
      }
      if(debounce<=0)
      {
        printf("key pressed\r\n");
        send_search_ready=1;
        flash_led0_slow(20);//begin to flash
      }
    }
}
//ok
void key_init(void)
{
    P1DIR&=~BIT4;//p5.5设置为输入 AUX
    P1REN |=  BIT4;                //上拉下拉电阻使能
    P1OUT |=  BIT4;                //P5.5 为上拉
    delay(1000);
}

#if 0
//only port1 and port2 可以中断配置
void key_init(void)
{
    P5DIR&=~BIT5;//p5.5设置为输入 AUX
    P5REN |=  BIT5;                //上拉下拉电阻使能
    P5OUT |=  BIT5;                //P5.5 为上拉
 
    P5IE |= BIT5;  // P5.5 interrupt enabled
 
    P5IES |= BIT5; // P5.5 Hi/Lo edge
 
    P5IFG &= ~BIT5;// P5.5 IFG cleared
    delay(1000);
}



static char toggle=1;
#pragma vector=PORT5_VECTOR

__interrupt void Port_5(void)

{
  
    int debounce=10;
    if(P5IN&BIT5)
    {
      while(debounce--)
      {
        if(P5IN&BIT5)
        {
          
        }
        delay(500);
      }
      if(toggle==1)
      {
             led1_off();
             led2_off();
      }
      else
      {
             led1_on();
             led2_on();
      }
      toggle=!toggle;
    }   
    P5IFG &= ~BIT5; 
}
#endif