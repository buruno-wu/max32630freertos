#ifndef _ESP8266_H
#define _ESP8266_H

#define COM_ESP8266	COM6		/* 选择串口 */

/* 定义下面这句话, 将把收到的字符发送到调试串口1 */
#define ESP8266_TO_COM1_EN

/* 本模块部分函数用到了软件定时器最后1个ID。 因此主程序调用本模块的函数时，请注意回避定时器 TMR_COUNT - 1。
  bsp_StartTimer(3, _usTimeOut);

  TMR_COUNT 在 bsp_timer.h 文件定义
*/
#define ESP8266_TMR_ID	(TMR_COUNT - 1)

/* 供外部调用的函数声明 */
void bsp_InitESP8266(void);
void ESP8266_Reset(void);
void ESP8266_PowerOn(void);
void ESP8266_PowerOff(void);
void ESP8266_SendAT(char *_Cmd);

uint8_t ESP8266_WaitResponse(char *_pAckStr, uint16_t _usTimeOut);
void ESP8266_PrintRxData(uint8_t _ch);
void ESP8266_JoinAP(char *_ssid, char *_pwd);


#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
