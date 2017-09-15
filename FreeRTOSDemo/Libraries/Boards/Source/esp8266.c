/*
*********************************************************************************************************
*
*	模块名称 : ESP8266 串口WIFI模块驱动程序
*	文件名称 : esp8266.c
*	版    本 : V1.0
*	说    明 : 封装 ESP8266 模块相关的AT命令
*
*********************************************************************************************************
*/

#include "board.h"

/* ESP8266 模块接线图
	ESP8266模块    STM32-V5开发板


		UTXD   ---  PG14/USART6_TX
		GND    ---  GND
		CH_PD  ---  PI0  (接3.3V 或 IO控制模块掉电）
		GPIO2
		GPIO16 ---  PB7   (wifi 硬件复位)
		GPIO0
		VCC    ---  3.3  (供电)
		URXD   ---  PC7/USART6_RX


	模块缺省波特率 9600;  支持的范围：110~460800bps
	在板子上电初始跑boot rom的一段log，需要在 74880 的波特率下正常打印。下面是打印出来的内容.

	----------- PD = 1 之后 74880bps 打印如下内容 ------

	 ets Jan  8 2013,rst cause:1, boot mode:(3,6)

	load 0x40100000, len 25052, room 16
	tail 12
	chksum 0x0b
	ho 0 tail 12 room 4
	load 0x3ffe8000, len 3312, room 12
	tail 4
	chksum 0x53
	load 0x3ffe8cf0, len 6576, room 4
	tail 12
	chksum 0x0d
	csum 0x0d

	----------- 之后是 9600bps 打印 ---------------

	[Vendor:www.ai-thinker.com Version:0.9.2.4]

	ready


	使用串口超级终端软件时，需要设置 终端 - 仿真 - 模式 页面勾选“新行模式”.


	【修改波特率】
	AT+CIOBAUD=?     ---- 查询命令参数
	+CIOBAUD:(9600-921600)

	OK

	AT+CIOBAUD=115200
	BAUD->115200

	【选择 WIFI 应用模式 】
	AT+CWMODE=1
		1   Station 模式
		2   AP 模式
		3   AP 兼 Station 模式

	【列出当前可用 AP】
	AT+CWLAP=<ssid>,< mac >,<ch>
	AT+CWLAP

	【AT+CWJAP加入 AP】
	AT+CWJAP=<ssid>,< pwd >

*/

#define AT_CR		'\r'
#define AT_LF		'\n'

#include "mxc_config.h"
#include "uart.h"
#include "board.h"
#include "tmr.h"

#define ESP_MXC_UARTn   MXC_UART_GET_UART(ESP8266_UART)
#define ESP_UART_FIFO   MXC_UART_GET_FIFO(ESP8266_UART)
#define  ESP8266_TMR_ID  1
static void ESP_PutChar(const uint8_t data)
{
    // Wait for TXFIFO to not be full
    while ((ESP_MXC_UARTn->tx_fifo_ctrl & MXC_F_UART_TX_FIFO_CTRL_FIFO_ENTRY) == MXC_F_UART_TX_FIFO_CTRL_FIFO_ENTRY);
    ESP_MXC_UARTn->intfl = MXC_F_UART_INTFL_TX_DONE; // clear DONE flag for UART_PrepForSleep
    ESP_UART_FIFO->tx = data;
}

static uint8_t ESP_GetChar(void)
{
    while (!(ESP_MXC_UARTn->rx_fifo_ctrl & MXC_F_UART_RX_FIFO_CTRL_FIFO_ENTRY));
    return ESP_UART_FIFO->rx;
}

void bsp_StartTimer(int ESP8266_TMR_ID, unsigned long _usTimeOut)
{
mxc_tmr_regs_t *t_timer;
t_timer=MXC_TMR_GET_TMR(ESP8266_TMR_ID);
TMR_TO_Start(t_timer, _usTimeOut)
}
/*
*********************************************************************************************************
*	函 数 名: ESP8266_PowerOn
*	功能说明: 给ESP8266模块上电
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ESP8266_PowerOn(void)
{
	ESP8266_WaitResponse("csum", 2000);
	ESP8266_WaitResponse("ready", 5000);

}

/*
*********************************************************************************************************
*	函 数 名: ESP8266_PowerOff
*	功能说明: 控制ESP8266模块关机
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ESP8266_PowerOff(void)
{

}

/*
*********************************************************************************************************
*	函 数 名: ESP8266_PowerOn
*	功能说明: 给ESP8266模块上电
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ESP8266_Reset(void)
{

}

/*
*********************************************************************************************************
*	函 数 名: ESP8266_WaitResponse
*	功能说明: 等待ESP8266返回指定的应答字符串. 比如等待 OK
*	形    参: _pAckStr : 应答的字符串， 长度不得超过255
*			 _usTimeOut : 命令执行超时，0表示一直等待. >０表示超时时间，单位1ms
*	返 回 值: 1 表示成功  0 表示失败
*********************************************************************************************************
*/
uint8_t ESP8266_WaitResponse(char *_pAckStr, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint8_t ucRxBuf[256];
	uint16_t pos = 0;
	uint32_t len;
	uint8_t ret;

	len = strlen(_pAckStr);
	if (len > 255)
	{
		return 0;
	}

	/* _usTimeOut == 0 表示无限等待 */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(ESP8266_TMR_ID, _usTimeOut);		/* 使用软件定时器3，作为超时控制 */
	}
	while (1)
	{
	if (_usTimeOut > 0)
		{
			if (bsp_CheckTimer(ESP8266_TMR_ID))
			{
				ret = 0;	/* 超时 */
				break;
			}
		}

		if (ucData=ESP_GetChar())
		{
		if (ucData == '\n')
			{
				if (pos > 0)	/* 第2次收到回车换行 */
				{
					if (memcmp(ucRxBuf, _pAckStr,  len) == 0)
					{
						ret = 1;	/* 收到指定的应答数据，返回成功 */
						break;
					}
					else
					{
						pos = 0;
					}
				}
				else
				{
					pos = 0;
				}
			}
			else
			{
				if (pos < sizeof(ucRxBuf))
				{
					/* 只保存可见字符 */
					if (ucData >= ' ')
					{
						ucRxBuf[pos++] = ucData;
					}
				}
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: ESP8266_ReadResponse
*	功能说明: 读取ESP8266返回应答字符串。该函数根据字符间超时判断结束。 本函数需要紧跟AT命令发送函数。
*	形    参: _pBuf : 存放模块返回的完整字符串
*			  _usBufSize : 缓冲区最大长度
*			 _usTimeOut : 命令执行超时，0表示一直等待. >0 表示超时时间，单位1ms
*	返 回 值: 0 表示错误（超时）  > 0 表示应答的数据长度
*********************************************************************************************************
*/
uint16_t ESP8266_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t ret;
	uint8_t status = 0;		/* 接收状态 */

	/* _usTimeOut == 0 表示无限等待 */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(ESP8266_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	}
	while (1)
	{
		bsp_Idle();				/* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

		if (status == 2)		/* 正在接收有效应答阶段，通过字符间超时判断数据接收完毕 */
		{
			if (bsp_CheckTimer(ESP8266_TMR_ID))
			{
				_pBuf[pos]	 = 0;	/* 结尾加0， 便于函数调用者识别字符串结束 */
				ret = pos;		/* 成功。 返回数据长度 */
				break;
			}
		}
		else
		{
			if (_usTimeOut > 0)
			{
				if (bsp_CheckTimer(ESP8266_TMR_ID))
				{
					ret = 0;	/* 超时 */
					break;
				}
			}
		}

		if (comGetChar(COM_ESP8266, &ucData))
		{
			ESP8266_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */

			switch (status)
			{
				case 0:			/* 首字符 */
					if (ucData == AT_CR)		/* 如果首字符是回车，表示 AT命令不会显 */
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
						status = 2;	 /* 认为收到模块应答结果 */
					}
					else	/* 首字符是 A 表示 AT命令回显 */
					{
						status = 1;	 /* 这是主机发送的AT命令字符串，不保存应答数据，直到遇到 CR字符 */
					}
					break;

				case 1:			/* AT命令回显阶段, 不保存数据. 继续等待 */
					if (ucData == AT_CR)
					{
						status = 2;
					}
					break;

				case 2:			/* 开始接收模块应答结果 */
					/* 只要收到模块的应答字符，则采用字符间超时判断结束，此时命令总超时不起作用 */
					bsp_StartTimer(ESP8266_TMR_ID, 5);
					if (pos < _usBufSize - 1)
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
					}
					break;
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: ESP8266_SendAT
*	功能说明: 向模块发送AT命令。 本函数自动在AT字符串口增加<CR>字符
*	形    参: _Str : AT命令字符串，不包括末尾的回车<CR>. 以字符0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void ESP8266_SendAT(char *_Cmd)
{
	comSendBuf(COM_ESP8266, (uint8_t *)_Cmd, strlen(_Cmd));
	comSendBuf(COM_ESP8266, "\r\n", 2);
}

/*
*********************************************************************************************************
*	函 数 名: ESP8266_JoinAP
*	功能说明: 加入AP
*	形    参: _ssid : AP名字字符串
*			  _pwd : 密码字符串
*	返 回 值: 无
*********************************************************************************************************
*/
void ESP8266_JoinAP(char *_ssid, char *_pwd)
{
	char buf[128];

	sprintf(buf, "AT+CWJAP=\"%s\",\"%s\"", _ssid, _pwd);
	ESP8266_SendAT(buf);
}



/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
