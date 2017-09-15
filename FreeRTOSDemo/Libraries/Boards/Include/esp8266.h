#ifndef _ESP8266_H
#define _ESP8266_H

#define COM_ESP8266	COM6		/* ѡ�񴮿� */

/* ����������仰, �����յ����ַ����͵����Դ���1 */
#define ESP8266_TO_COM1_EN

/* ��ģ�鲿�ֺ����õ���������ʱ�����1��ID�� �����������ñ�ģ��ĺ���ʱ����ע��رܶ�ʱ�� TMR_COUNT - 1��
  bsp_StartTimer(3, _usTimeOut);

  TMR_COUNT �� bsp_timer.h �ļ�����
*/
#define ESP8266_TMR_ID	(TMR_COUNT - 1)

/* ���ⲿ���õĺ������� */
void bsp_InitESP8266(void);
void ESP8266_Reset(void);
void ESP8266_PowerOn(void);
void ESP8266_PowerOff(void);
void ESP8266_SendAT(char *_Cmd);

uint8_t ESP8266_WaitResponse(char *_pAckStr, uint16_t _usTimeOut);
void ESP8266_PrintRxData(uint8_t _ch);
void ESP8266_JoinAP(char *_ssid, char *_pwd);


#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/