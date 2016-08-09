#include "protocol.h"
#include "ir_lora.h"
#include <msp430.h>
#include "power_clk.h"
unsigned char msg[256];
int node_addr=0;
int  parse_data_pc(unsigned char * buf,int len)
{
  int total_len=0;
  int dest_addr_len,msg_len;

  unsigned char dest_addr[32]={0},port=0;
  unsigned short cal_crc=0,data_crc=0;
   
 // dump_data(buf,len);
  if(buf==NULL)
    return -1;
  //check head,end
  if((buf[0]=='#')&&(buf[1]=='$')&&(buf[len-1]=='!')&&(buf[len-2]=='@'))
  {
    //check len
    total_len=buf[2]+(buf[3]<<8);
   // printf("total_len=%d,len=%d\r\n",total_len,len);
    if(total_len!=len)
    {
      printf("error packet len total=%d,len=%d\r\n",total_len,len);
      return -1;
    }
    //check crc
    data_crc=buf[len-4]+(buf[len-3]<<8);
    cal_crc=CRC16(buf,len-4);
    if(data_crc!=cal_crc)
    {
      printf("crc error,data_crc=0x%02x,cal_crc=0x%02x\r\n",data_crc,cal_crc);
      //return -1;
    }
    //get addr info
    dest_addr_len=buf[4];
    // printf("dest addr len=%d\r\n",dest_addr_len);
    memcpy(dest_addr,&buf[5],dest_addr_len);
    if(dest_addr_len)
    {   
      //  printf("dest_addr %s\r\n",dest_addr);
    }

    port=buf[HEAD_F_SIZE+TOTAL_F_SIZE+ADDR_LEN_F_SIZE+dest_addr_len];

    if(port==PORT_MCU)
    {
        msg_len=buf[HEAD_F_SIZE+TOTAL_F_SIZE+ADDR_LEN_F_SIZE+dest_addr_len+PORT_F_SIZE];
        printf("port=%d,msg_len=%d\r\n",port,msg_len);
        //#$[totoal_len][dest_addr_len][dest_addr][port][msg_len][msg][crc]@!
        memcpy(msg,&buf[HEAD_F_SIZE+TOTAL_F_SIZE+ADDR_LEN_F_SIZE+dest_addr_len+PORT_F_SIZE+MSG_LEN_F_SIZE],msg_len);
        process_mcu_msg(msg,msg_len);//ÐòÁÐºÅ¶ÁÐ´
    }
    else
    {
      //parse addr,adn send
       node_addr=get_addr_from_sn(dest_addr,dest_addr_len);
      // printf("send data to  Lora node:%d,0x%02x\r\n",node_addr,node_addr);
       lora_send(node_addr,buf,len);
    }
    return 0;
  }
  printf("error packet head or end %c %c %c %c\r\n",buf[0],buf[1],buf[2],buf[3]);
  return -1;
}

int  parse_data_lora(unsigned char * buf,int len)
{
   // printf("lora get data:%d\r\n",len);
    uart1_write(buf,len);
    uart2_clear();
    return 0;
}
void showMsg(char * buf,int len);
int  parse_data_config(unsigned char * buf,int len)
{
  int total_len=0;
  int dest_addr_len,msg_len;
  unsigned char dest_addr[32]={0},port=0;
  unsigned short cal_crc=0,data_crc=0;
  if(buf==NULL)
    return -1;
  //check head,end
  printf("%s %d\r\n",__func__,__LINE__);
  if((buf[0]=='#')&&(buf[1]=='$')&&(buf[len-1]=='!')&&(buf[len-2]=='@'))
  {
    printf("get data\r\n");
    //check len
    total_len=buf[2]+(buf[3]<<8);
    printf("total_len=%d,len=%d\r\n",total_len,len);
    if(total_len!=len)
    {
      printf("error packet len\r\n");
      return -1;
    }
    //check crc
    data_crc=buf[len-4]+(buf[len-3]<<8);
    cal_crc=CRC16(buf,len-4);
    if(data_crc!=cal_crc)
    {
      printf("crc error,data_crc=0x%02x,cal_crc=0x%02x\r\n",data_crc,cal_crc);
      //return -1;
    }
    //get addr info
    dest_addr_len=buf[4];
    printf("dest addr len=%d\r\n",dest_addr_len);
    if(dest_addr_len)
    {
      memcpy(dest_addr,&buf[5],dest_addr_len);
    }
    printf("dest_addr %s\r\n",dest_addr);

    port=buf[HEAD_F_SIZE+TOTAL_F_SIZE+ADDR_LEN_F_SIZE+dest_addr_len];
    msg_len=buf[HEAD_F_SIZE+TOTAL_F_SIZE+ADDR_LEN_F_SIZE+dest_addr_len+PORT_F_SIZE];
    printf("port=%d,msg_len=%d\r\n",port,msg_len);
    //#$[totoal_len][dest_addr_len][dest_addr][port][msg_len][msg][crc]@!
    memcpy(msg,&buf[HEAD_F_SIZE+TOTAL_F_SIZE+ADDR_LEN_F_SIZE+dest_addr_len+PORT_F_SIZE+MSG_LEN_F_SIZE],msg_len);
    showMsg(msg,msg_len);
    if(port==PORT_MCU)
    {
        process_mcu_msg(msg,msg_len);//ÐòÁÐºÅ¶ÁÐ´
    }
    return 0;
  }
  printf("error packet head or end %c %c %c %c\r\n",buf[0],buf[1],buf[2],buf[3]);
  return -1;
}

void showMsg(char * buf,int len)
{
  int i=0;
  for(i=0;i<len;i++)
  {
    printf("%c,0x%x ",buf[i]);
  }
  printf("\r\n");
}

/*mcu*/
int process_mcu_msg(unsigned char *buf,int len)
{
  char * SN=NULL;
  char sn[40];
  char sn_tmp[30];
  printf("%s %d\r\n",__func__,__LINE__);
  memset(sn,0,sizeof(sn));
  if(buf)
  {
    //is ctrl msg
    if(strstr(buf,"ctrl")||strstr(buf,"Ctrl"))
    {
        printf("%s\r\n",buf);
        if(strstr(buf,"SN_SET="))//write SN,ok
        { 
          SN=strstr(buf,"=");
          memcpy(sn,SN+1,SN_LEN);
          printf("to write SN:%s\r\n",sn);
          //update addr;
          write_sn(sn);
          //Òª·µ»ØOK£»
          send_msg("ctrl:ack",strlen("ctrl:ack"),PORT_MCU,sn,SN_LEN,UART_CHANNEL_CONFIG_3);
        }
        if(strstr(buf,"SN_GET"))//get SN,ok
        {
           strcat(sn,"ctrl:SN_GET=");
           read_sn(sn_tmp);
           memcpy(&sn[strlen("ctrl:SN_GET=")],sn_tmp,SN_LEN);
           send_msg(sn,SN_LEN+strlen("ctrl:SN_GET="),PORT_MCU,sn_tmp,SN_LEN,UART_CHANNEL_CONFIG_3);
        }
        if(strstr(buf,"g_dev_info"))//get dev info,ok
        {
           send_msg(NULL,0,PORT_MCU,g_sn,SN_LEN,UART_CHANNEL_PC_1);
        }
        //write sn;
        return 0;
    }
  }
//  send_msg("ctrl:ack",strlen("ctrl:ack"),PORT_MCU,sn,SN_LEN);
  return -1;
}

int send_msg(unsigned char *msg,int msg_len,char port,unsigned char * sn,int sn_len,char uart_channel)
{
	unsigned short buf_len=0,crc=0;
        int dongle_len_opt=0;
        if(UART_CHANNEL_PC_1==uart_channel)
        {
       //   dongle_len_opt=strlen(g_sn)+1;
        }
	unsigned char * msg_buf=NULL;
	buf_len=HEAD_F_SIZE+END_F_SIZE+TOTAL_F_SIZE+ADDR_LEN_F_SIZE+PORT_F_SIZE+MSG_LEN_F_SIZE+sn_len+msg_len+CRC_F_SIZE+dongle_len_opt;
	
	msg_buf=malloc(buf_len);
	if(msg_buf==NULL)
	{
		return -1;
	}
	/*head and end*/printf("send msg len=%d\r\n",buf_len);
	msg_buf[0]='#';msg_buf[1]='$';msg_buf[buf_len-2]='@';msg_buf[buf_len-1]='!';
	msg_buf[2]=buf_len&0xff;msg_buf[3]=(buf_len>>8)&0xff;
        //for dongle addr
        if(UART_CHANNEL_PC_1==uart_channel)
        {
           msg_buf[4]=strlen(g_sn);
           memcpy(&msg_buf[5],g_sn,strlen(g_sn));
        }   
	msg_buf[4+dongle_len_opt]=sn_len;
	memcpy(&msg_buf[5+dongle_len_opt],sn,sn_len);
	msg_buf[5+sn_len+dongle_len_opt]=port;
	msg_buf[5+sn_len+1+dongle_len_opt]=msg_len&0xff;
	msg_buf[5+sn_len+2+dongle_len_opt]=(msg_len>>8)&0xff;
	memcpy(&msg_buf[5+sn_len+3+dongle_len_opt],msg,msg_len);
        
	//crc
        crc=CRC16(msg_buf,buf_len-4);
	msg_buf[5+sn_len+2+1+msg_len+dongle_len_opt]=crc&0xff;
	msg_buf[5+sn_len+2+1+msg_len+1+dongle_len_opt]=(crc>>8)&0xff;
    //mabey lora_send
 
    switch(uart_channel)
    {
      case UART_CHANNEL_PC_1:uart1_write(msg_buf,buf_len);break;
      case UART_CHANNEL_LORA_2:lora_send(node_addr,msg_buf,buf_len); break;
      case UART_CHANNEL_CONFIG_3:uart3_write(msg_buf,buf_len); break;
      default:printf("error\r\n");break;
    }
	free(msg_buf);
        delay_ms(50);
	return 0;
}

int send_data_query(void)
{
 send_msg("ctrl:query",strlen("ctrl:query"),PORT_LORA,g_sn,SN_LEN,UART_CHANNEL_LORA_2);
}

int send_ack(void)
{
  send_msg("ctrl:ack",strlen("ctrl:ack"),PORT_LORA,g_sn,SN_LEN,UART_CHANNEL_LORA_2);
}

int send_search(void)
{
    int tmp=node_addr;  
    node_addr=0xffff;
    send_msg("ctrl:search",strlen("ctrl:search"),PORT_LORA,g_sn,SN_LEN,UART_CHANNEL_LORA_2);
    node_addr=tmp;
}

int send_join(void)
{
    send_msg("ctrl:join",strlen("ctrl:join"),PORT_LORA,g_sn,SN_LEN,UART_CHANNEL_LORA_2);
}


int send_fail(void)
{
    send_msg("ctrl:fail",strlen("ctrl:fail"),PORT_LORA,g_sn,SN_LEN,UART_CHANNEL_LORA_2);
}

int char2int(char c);
//you bug
unsigned short get_addr_from_sn(unsigned char * sn,int len)
{
  int a,b,c,d,e;
  e=char2int(sn[len-1]);
  if(e<0)
    return -1;
  d=char2int(sn[len-2]);
  if(d<0)
    return -1;  
  c=char2int(sn[len-3]);

  if(c<0)
    return -1;
  b=char2int(sn[len-4]);
  if(b<0)
    return -1;
  a=char2int(sn[len-5]);
  if(a<0)
    return -1;
  return a*10000+b*1000+c*100+d*10+e;
}

int char2int(char c)
{
  char num[10]={'0','1','2','3','4','5','6','7','8','9'};
  int i=0;
  for (i=0;i<10;i++)
  {
    if(c==num[i])
      return i;
  }
  return -1;
}