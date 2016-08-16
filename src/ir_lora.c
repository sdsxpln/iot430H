#include "ir_lora.h"
#include "uart.h"
#include "protocol.h"
#include "utils.h"
#include "power_clk.h"
#include <msp430.h>
void ir_write(unsigned char *buf,int len)
{
    uart1_write(buf,len);
}

void ir_read(unsigned char *buf,int len,int * read)
{
    uart1_read(buf,len,read);
}

void lora_write(unsigned char *buf,int len)
{
    uart2_write(buf,len);
}

void lora_read(unsigned char *buf,int len,int * read)
{
    uart2_read(buf,len,read);
}

//01
#define LORA_MODE_WAKEUP 1
//10
#define LORA_MODE_LOWPOWER 2
//00
#define LORA_MODE_NORMAL 0
//
#define LORA_MODE_SLEEP 3

#define  M0_PIN_low   (P9OUT &= ~(BIT3))
#define  M0_PIN_high  (P9OUT |= (BIT3))
#define  M1_PIN_low  (P9OUT &= ~(BIT2))
#define  M1_PIN_high (P9OUT |= (BIT2))

#define LORA_CHANNEL 0x17
static unsigned char lora_config[6]={0xfe};
void lora_gpio_init(void);
unsigned short g_hub_addr=0xffff;
unsigned short g_hub_channel=0;
unsigned char g_sn[SN_LEN];
unsigned short g_dongle_addr=0;
//ok
void init_lora_device(void)
{
  int ret=0;
  ret=read_sn(g_sn);
  if(ret<0)
  {
    g_dongle_addr=0;
  }
  else
  {
    printf("SN:%s\r\n",g_sn);
    g_dongle_addr=get_addr_from_sn(g_sn,SN_LEN);
  }
# if 0
   memcpy(g_sn,"FE-FE00-A1-160712-00001",strlen("FE-FE00-A1-160712-00001"));
   g_dongle_addr=get_addr_from_sn(g_sn,SN_LEN);

   printf("test dongle_addr=0x%02x\r\n",(unsigned short)g_dongle_addr);
   # endif  
   lora_gpio_init();
  // lora_mode_config(LORA_MODE_SLEEP);
   //get_lora_param(lora_config);
   lora_mode_config(LORA_MODE_NORMAL);
}

void lora_gpio_init(void)
{
  P9DIR |= BIT2;  //P6.6设置为输出 M1
  P9DIR |= BIT3;  //P6.7设置为输出 M0
  P8DIR&=~BIT0;//p6.5设置为输入 AUX
  P8REN&=~BIT0;//no pull
  delay(1000);
}

//only use mode LORA_MODE_NORMAL LORA_MODE_SLEEP 3
//睡眠模式只在配置网络参数的时候用到。
void lora_mode_config(char mode)
{
  int ret=0;
  while(!ret)
  {
    ret=lora_is_ready();
    delay(1000);
  }
  switch (mode)
  {
    case LORA_MODE_WAKEUP:M0_PIN_low;M1_PIN_high;printf("enter wakeup mode\r\n");
        break;
    case LORA_MODE_LOWPOWER:M0_PIN_high;M1_PIN_low;printf("enter low power mode\r\n");
        break;
    case LORA_MODE_NORMAL:M0_PIN_low;M1_PIN_low;printf("enter normal mode\r\n");
        break;
    case LORA_MODE_SLEEP:M0_PIN_high;M1_PIN_high;printf("enter sleep mode\r\n");
      break;
  } 
}
//ok
int lora_is_ready(void)
{
    return (P8IN&BIT0);
}

void lora_reset(void)
{
  unsigned char buf[3]={0xc4,0xc4,0xc4};
  lora_write(buf,3);
//  check aux is high
}

//ok
void  get_lora_param(unsigned char * rbuf)
{
    int len=0xff,count=120;
    unsigned char buf[3]={0xc1,0xc1,0xc1};
    uart2_clear();
    lora_write(buf,3);
get:
    lora_read(rbuf,6,&len);
    if(len<6)
    {
      uart2_clear();
      lora_write(buf,3);
      count--;
      delay(3000);
      if(count==0)
      {
        printf("%s timeout\r\n",__func__);
        return ;
      }
      goto get;
    }
    printf("module:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",rbuf[0],rbuf[1],rbuf[2],rbuf[3],rbuf[4],rbuf[5]);
    return ;
}

/*enter sleep mode first*/
void  set_lora_addr(unsigned short addr)
{
    int len;
    unsigned char get_buf[6];
    unsigned char buf[6];
    lora_gpio_init();
    lora_mode_config(LORA_MODE_SLEEP);
    buf[0]=0xc0;
    buf[1]=(addr>>8)&0xff;
    buf[2]=addr&0xff;
    buf[3]=0x18;//2.4k,9600
    buf[4]=LORA_CHANNEL;
    buf[5]=0xc4;
    
    printf("write config :0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",buf[0],buf[1],buf[2],buf[3],buf[4], buf[5]);
    uart2_clear();
    lora_write(buf,6);
    delay(1000);
    
    get_lora_param(get_buf);
    /*compare*/
    lora_mode_config(LORA_MODE_SLEEP);
    lora_reset();
    lora_mode_config(LORA_MODE_NORMAL);
}
#define  HUB_FLASH_ADDR 0x0
#define  SN_ADDR (0x0+0x0)

//ok
int write_sn(char * buf)
{
  int i=0;
    unsigned  char tmp[SN_LEN+4];
    tmp[0]='{';
    tmp[1]='{';
    memcpy(&tmp[2],buf,SN_LEN);
    tmp[SN_LEN+2]='}';
    tmp[SN_LEN+3]='}';
    ucb0_i2c_init();    
    delay_ms(100);
    at24c02_writeMultBytes(SN_ADDR, tmp, SN_LEN+4) ; 
    delay_ms(100);
    read_sn(g_sn);
    g_dongle_addr=get_addr_from_sn(g_sn,SN_LEN);
    printf("node_addr=%d 0x%02x\r\n",(unsigned short)g_dongle_addr,(unsigned short)g_dongle_addr); 
    set_lora_addr(g_dongle_addr);
    flash_led0_fast(20);
}
//ok
int read_sn(unsigned char * buf)
{
  unsigned char  i=0;
  ucb0_i2c_init();   
  delay_ms(200);
  unsigned char tmp[SN_LEN+4];

  at24c02_readMultBytes(SN_ADDR, tmp, SN_LEN+4);
        
  if((tmp[0]=='{')&&(tmp[1]=='{')&&(tmp[SN_LEN+3]=='}')&&(tmp[SN_LEN+2]=='}'))
  {
    memcpy(buf,tmp+2,SN_LEN);
  }
  else
  {
    memset(buf,0xff,SN_LEN);
    printf("Invalid SN\r\n");
    return -1;
  }
  return 0;
}

void lora_send(unsigned short node_addr,unsigned char * buf,int len)
{
  unsigned char head[3];
  char * tmp_buf=buf;
  int send_len=len;
  head[1]=node_addr&0xff;
  head[0]=(node_addr>>8)&0xff;
  head[2]=LORA_CHANNEL;
  while(send_len>0)
  {
    if(send_len<58)
    {  lora_write(head,3);
       lora_write(tmp_buf,send_len);
       break;
    }
    else
    {
      lora_write(head,3);
      lora_write(tmp_buf,58);
      tmp_buf=tmp_buf+58;
      send_len=send_len-58;
      delay(20);
    }
  }
   delay_ms(10);
 // lora_write(buf,len);
  printf("lora wrtie 0x%02x,0x%02x,0x%02x\r\n",head[0],head[1],head[2]);
}

//P5.4
void ir_power_on(void)
{
   P5DIR |= BIT4;  
   P5OUT |= (BIT4);
}

void ir_power_off(void)
{
   P5DIR |= BIT4;  
   P5OUT &= ~(BIT4);
}

//P5.5
void lora_power_on(void)
{
   P5DIR |= BIT4;  
   P5OUT |= (BIT4);
}

void lora_power_off(void)
{
   P5DIR |= BIT4; 
   P5OUT &= ~(BIT4);
}