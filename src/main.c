/******************************************************************************
	            MSP430F5438A最小系统V3.0

功能：串口透传
串口波特率：115200

硬件原理图：
            TXD --> P3.4
            RXD --> P3.5

//   MSP430F543xA Demo - USCI_A0, 115200 UART Echo ISR, DCO SMCLK

//   Description: Echo a received character, RX ISR used. Normal mode is LPM0.

//   USCI_A0 RX interrupt triggers TX Echo.

//   Baud rate divider with 1048576hz = 1048576/115200 = ~9.1 (009h|01h)

//   ACLK = REFO = ~32768Hz, MCLK = SMCLK = default DCO = 32 x ACLK = 1048576Hz

//   See User Guide for baud rate divider table

//                 MSP430F5438A

//             -----------------

//         /|\|                 |

//          | |                 |

//          --|RST              |

//            |                 |

//            |     P3.4/UCA0TXD|------------>

//            |                 | 115200 - 8N1

//            |     P3.5/UCA0RXD|<------------

//   Built with IAR Embedded Workbench Version: 5.3

Time：2014、11、9
by：梦想电子工作室
淘宝：mcu-dream.taobao.com
******************************************************************************/
#include <msp430x54x.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include  <uart.h>
void watchDog_close(void);

void board_init(void)
{
  watchDog_close();
  led_init();
  lora_power_on();
  log_uart0_init();
  pc_uart1_init();
  lora_uart2_init();
  config_uart3_init();
  key_init();
  timer1_init();
}
int send_search_cnt=0;
extern int send_search_ready;
void main(void)
{
  //set_xt2_to_source();
  board_init();  
  printf("I am dongle\r\n"); 
  init_lora_device();
  flash_led0_slow(4);//show we begin to run
  while(1)
  {  
    process_key();//ok,send search
    process_pc_uart1();//ok,一个用于获取devinfo，其他透传给节点，根据port来区分。
    delay(200);
    process_lora_uart2();//接收lora的数据，打包自身的序列号，透传给网关
    delay(200);
    process_config_uart3();//ok,//用于读写序列号和hub地址等配置信息，MCU PORT
    delay(200); 
    if(send_search_ready==1)
    {
       send_search_cnt++;
       send_search();
       delay(3000);
      //led;
       if(send_search_cnt==10)
       {
         printf("stop search!\r\n");
         send_search_cnt=0;
         send_search_ready=0;
         flash_led0_off();
       }
    }
  }
}
