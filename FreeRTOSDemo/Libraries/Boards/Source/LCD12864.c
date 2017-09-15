
#include "LCD12864.h"
#include  "board.h"
#include <string.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "board.h"
#include "gpio.h"
#include "spim.h"


#define XLevelL		0x00
#define XLevelH		0x10
#define XLevel		((XLevelH&0x0F)*16+XLevelL)
#define Max_Column	128
#define Max_Row		  64
#define	Brightness	0xcF 

#define X_WIDTH     131
#define Y_WIDTH     64
//======================================
const byte F6x8[][6] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // sp
    { 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
    { 0x00, 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
    { 0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
    { 0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
    { 0x00, 0x62, 0x64, 0x08, 0x13, 0x23 },   // %
    { 0x00, 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
    { 0x00, 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
    { 0x00, 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
    { 0x00, 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
    { 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
    { 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
    { 0x00, 0x00, 0x00, 0xA0, 0x60, 0x00 },   // ,
    { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08 },   // -
    { 0x00, 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
    { 0x00, 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
    { 0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
    { 0x00, 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
    { 0x00, 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
    { 0x00, 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
    { 0x00, 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
    { 0x00, 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
    { 0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
    { 0x00, 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
    { 0x00, 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
    { 0x00, 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
    { 0x00, 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
    { 0x00, 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
    { 0x00, 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
    { 0x00, 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
    { 0x00, 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
    { 0x00, 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
    { 0x00, 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
    { 0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C },   // A
    { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
    { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
    { 0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
    { 0x00, 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
    { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
    { 0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
    { 0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
    { 0x00, 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
    { 0x00, 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
    { 0x00, 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
    { 0x00, 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
    { 0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
    { 0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
    { 0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
    { 0x00, 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
    { 0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
    { 0x00, 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
    { 0x00, 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
    { 0x00, 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
    { 0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
    { 0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
    { 0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
    { 0x00, 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
    { 0x00, 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
    { 0x00, 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
    { 0x00, 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
    { 0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // 55
    { 0x00, 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
    { 0x00, 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
    { 0x00, 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
    { 0x00, 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
    { 0x00, 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
    { 0x00, 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
    { 0x00, 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
    { 0x00, 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
    { 0x00, 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
    { 0x00, 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
    { 0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C },   // g
    { 0x00, 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
    { 0x00, 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
    { 0x00, 0x40, 0x80, 0x84, 0x7D, 0x00 },   // j
    { 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
    { 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
    { 0x00, 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
    { 0x00, 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
    { 0x00, 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
    { 0x00, 0xFC, 0x24, 0x24, 0x24, 0x18 },   // p
    { 0x00, 0x18, 0x24, 0x24, 0x18, 0xFC },   // q
    { 0x00, 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
    { 0x00, 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
    { 0x00, 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
    { 0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
    { 0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
    { 0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
    { 0x00, 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
    { 0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C },   // y
    { 0x00, 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
    { 0x14, 0x14, 0x14, 0x14, 0x14, 0x14 }    // horiz lines
};
void delay_nus(int i)
{
  int j;
for(;i>0;i--)
     for(j=0;j<5;j++);
}
void LCD_WrDat(uint8_t data)
{
   uint8_t i=8;
  GPIO_OutSet(&nhd12832_dc);
  GPIO_OutClr(&nhd12832_scl) ; 
//  delay_nus(5);   
  while(i--)
  {
    if(data&0x80){GPIO_OutSet(&nhd12832_sda);}
    else{GPIO_OutClr(&nhd12832_sda);}
    GPIO_OutSet(&nhd12832_scl); 
   // delay_nus(10);            
    GPIO_OutClr(&nhd12832_scl) ; 
    data<<=1;    
  }
 // delay_nus(10);  
}
void LCD_WrCmd(uint8_t cmd)
{
   uint8_t i=8;
   GPIO_OutClr(&nhd12832_dc) ; 
   GPIO_OutClr(&nhd12832_scl) ; 
 //  delay_nus(5);     
  while(i--)
  {
    if(cmd&0x80){GPIO_OutSet(&nhd12832_sda);}
    else{GPIO_OutClr(&nhd12832_sda);}
     GPIO_OutSet(&nhd12832_scl); 
   //  delay_nus(10);              
    GPIO_OutClr(&nhd12832_scl) ;   
    cmd<<=1;;   
  }
 // delay_nus(5);  
}
//SPI写入数据********************************************************************* 
//int LCD_WrDat(byte data)
//{
//    GPIO_OutSet(&nhd12832_dc);
//    spim_req_t req;
//    req.ssel = NHD12832_SSEL;
//    req.deass = 1;
//    req.tx_data = &data;
//    req.rx_data = NULL;
//    req.width = SPIM_WIDTH_1;
//    req.len = 1;
//
//    if (SPIM_Trans(NHD12832_SPI, &req) != 1) {
//        return E_COMM_ERR;
//    }
//    // Wait for transaction to complete
//    while(SPIM_Busy(NHD12832_SPI) != E_NO_ERROR) {}
//    return E_NO_ERROR;
//}
////SPI写入指令*********************************************************************
//int LCD_WrCmd(byte cmd)
//{
//    GPIO_OutClr(&nhd12832_dc);
//    spim_req_t req;
//    req.ssel = NHD12832_SSEL;
//    req.deass = 1;
//    req.tx_data = &cmd;
//    req.rx_data = NULL;
//    req.width = SPIM_WIDTH_1;
//    req.len = 1;
//
//    if (SPIM_Trans(NHD12832_SPI, &req) != 1) {
//        return E_COMM_ERR;
//    }
//    // Wait for transaction to complete
//    while(SPIM_Busy(NHD12832_SPI) != E_NO_ERROR) {}
//    return E_NO_ERROR;
//    }

void LCD_Set_Pos(byte x, byte y)
{ 
    LCD_WrCmd( 0xb0+y );
    LCD_WrCmd((( x & 0xf0 ) >> 4) | 0x10 );
    LCD_WrCmd( ( x & 0x0f ) | 0x01 ); 
} 
void LCD_Fill(byte bmp_data)
{
	byte y,x;	
	for(y=0;y<8;y++)
	{
		LCD_WrCmd(0xb0+y);
		LCD_WrCmd(0x01);
		LCD_WrCmd(0x10);
		for(x=0;x<X_WIDTH;x++)
			LCD_WrDat(bmp_data);
	}
}
void LCD_CLS(void)
{
	byte y,x;	
	for(y=0;y<8;y++)
	{
		LCD_WrCmd(0xb0+y);
		LCD_WrCmd(0x01);
		LCD_WrCmd(0x10); 
		for(x=0;x<X_WIDTH;x++)
			LCD_WrDat(0);
	}
}
void LCD_DLY_ms(word ms)
{                         
    word a;
    while(ms)
    {
        a=1335;
        while(a--);
        ms--;
    }
    return;
}
//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 		   
u8 OLED_GRAM[131][8];	 

//更新显存到LCD		 
void OLED_Refresh_Gram(void)
{
	u8 i,n;	
      //  LCD_Set_Pos(0,0);  
	for(i=0;i<8;i++)  
	{  
	LCD_WrCmd (0xb0+i);    //设置页地址（0~7）		
	LCD_WrCmd (0x01);      //设置显示位置—列高地址  
        LCD_WrCmd (0x10);      //设置显示位置—列低地址
	for(n=0;n<131;n++)LCD_WrDat(OLED_GRAM[n][i]); 
	}   
}
	  	  
//开启OLED显示    
void OLED_Display_On(void)
{
	LCD_WrCmd(0X8D);  //SET DCDC命令
	LCD_WrCmd(0X14);  //DCDC ON
	LCD_WrCmd(0XAF);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
	LCD_WrCmd(0X8D);  //SET DCDC命令
	LCD_WrCmd(0X10);  //DCDC OFF
	LCD_WrCmd(0XAE);  //DISPLAY OFF
}		   			 
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;  
	for(i=0;i<8;i++)for(n=0;n<131;n++)OLED_GRAM[n][i]=0X00;  
	OLED_Refresh_Gram();//更新显示
}

//画点 
//x:0~127
//y:0~63
//t:1 填充 0,清空				   
void OLED_DrawPoint(byte x,byte y,byte t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//超出范围了.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x+1][pos]|=temp;
	else OLED_GRAM[x+1][pos]&=~temp;	    
}
//x1,y1,x2,y2 填充区域的对角坐标
//确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//dot:0,清空;1,填充	  
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)  
{  
	u8 x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
	}													    
	OLED_Refresh_Gram();//更新显示
}

void adjust(byte a)
{
    LCD_WrCmd(a);	//指令数据0x0000~0x003f  
}

void SetStartColumn(byte d)
{
	LCD_WrCmd(0x00+d%16);		// Set Lower Column Start Address for Page Addressing Mode
						        //   Default => 0x00
	LCD_WrCmd(0x10+d/16);		// Set Higher Column Start Address for Page Addressing Mode
					            //   Default => 0x10
}

void SetAddressingMode(byte d)
{
	LCD_WrCmd(0x20);			// Set Memory Addressing Mode
	LCD_WrCmd(d);			    // Default => 0x02
						        // 0x00 => Horizontal Addressing Mode
						        // 0x01 => Vertical Addressing Mode
						        // 0x02 => Page Addressing Mode
}

void SetColumnAddress(byte a, byte b)
{
	LCD_WrCmd(0x21);			// Set Column Address
	LCD_WrCmd(a);			    // Default => 0x00 (Column Start Address)
	LCD_WrCmd(b);			    // Default => 0x7F (Column End Address)
}

void SetPageAddress(byte a, byte b)
{
	LCD_WrCmd(0x22);			// Set Page Address
	LCD_WrCmd(a);			    // Default => 0x00 (Page Start Address)
	LCD_WrCmd(b);		    	// Default => 0x07 (Page End Address)
}

void SetStartLine(byte d)
{
	LCD_WrCmd(0x40|d);			// Set Display Start Line
						        // Default => 0x40 (0x00)
}

void SetContrastControl(byte d)
{
	LCD_WrCmd(0x81);			// Set Contrast Control
	LCD_WrCmd(d);		    	// Default => 0x7F
}

void Set_Charge_Pump(byte d)
{
	LCD_WrCmd(0x8D);			// Set Charge Pump
	LCD_WrCmd(0x10|d);			// Default => 0x10
						        // 0x10 (0x00) => Disable Charge Pump
						        // 0x14 (0x04) => Enable Charge Pump
}

void Set_Segment_Remap(byte d)
{
	LCD_WrCmd(0xA0|d);			// Set Segment Re-Map
						        // Default => 0xA0
					    	    // 0xA0 (0x00) => Column Address 0 Mapped to SEG0
						        // 0xA1 (0x01) => Column Address 0 Mapped to SEG127
}

void Set_Entire_Display(byte d)
{
	LCD_WrCmd(0xA4|d);			// Set Entire Display On / Off
						        // Default => 0xA4
						        // 0xA4 (0x00) => Normal Display
						        // 0xA5 (0x01) => Entire Display On
}

void Set_Inverse_Display(byte d)
{
	LCD_WrCmd(0xA6|d);			// Set Inverse Display On/Off
						        // Default => 0xA6
						        // 0xA6 (0x00) => Normal Display
						        // 0xA7 (0x01) => Inverse Display On
}

void Set_Multiplex_Ratio(byte d)
{
	LCD_WrCmd(0xA8);			// Set Multiplex Ratio
	LCD_WrCmd(d);			    // Default => 0x3F (1/64 Duty)
}

void Set_Display_On_Off(byte d)
{
	LCD_WrCmd(0xAE|d);			// Set Display On/Off
					        	// Default => 0xAE
						        // 0xAE (0x00) => Display Off
						        // 0xAF (0x01) => Display On
}

void SetStartPage(byte d)
{
	LCD_WrCmd(0xB0|d);			// Set Page Start Address for Page Addressing Mode
						        // Default => 0xB0 (0x00)
}

void Set_Common_Remap(byte d)
{
	LCD_WrCmd(0xC0|d);			// Set COM Output Scan Direction
						        // Default => 0xC0
						        // 0xC0 (0x00) => Scan from COM0 to 63
						        // 0xC8 (0x08) => Scan from COM63 to 0
}

void Set_Display_Offset(byte d)
{
	LCD_WrCmd(0xD3);			// Set Display Offset
	LCD_WrCmd(d);			    // Default => 0x00
}

void Set_Display_Clock(byte d)
{
	LCD_WrCmd(0xD5);			// Set Display Clock Divide Ratio / Oscillator Frequency
	LCD_WrCmd(d);			             // Default => 0x80
						        // D[3:0] => Display Clock Divider
						        // D[7:4] => Oscillator Frequency
}

void Set_Precharge_Period(byte d)
{
	LCD_WrCmd(0xD9);			// Set Pre-Charge Period
	LCD_WrCmd(d);			    // Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
						        // D[3:0] => Phase 1 Period in 1~15 Display Clocks
						        // D[7:4] => Phase 2 Period in 1~15 Display Clocks
}

void Set_Common_Config(byte d)
{
	LCD_WrCmd(0xDA);			// Set COM Pins Hardware Configuration
	LCD_WrCmd(0x02|d);			// Default => 0x12 (0x10)
						        // Alternative COM Pin Configuration
						        // Disable COM Left/Right Re-Map
}

void Set_VCOMH(byte d)
{
	LCD_WrCmd(0xDB);			// Set VCOMH Deselect Level
	LCD_WrCmd(d);			    // Default => 0x20 (0.77*VCC)
}

void Set_NOP(void)
{
	LCD_WrCmd(0xE3);			// Command for No Operation
}

int LCD_Init(void)        
{
    int err;

    // Sets GPIO to desired level for the board
    Board_nhd12832_Init();
   // GPIO_OutClr(&nhd12832_res);
    if ((err = GPIO_Config(&nhd12832_res)) != E_NO_ERROR) {
        return err;
    }
    //GPIO_OutClr(&nhd12832_dc);
    if ((err =  GPIO_Config(&nhd12832_dc)) != E_NO_ERROR) {
        return err;
    }
    if ((err =  GPIO_Config(&nhd12832_spip)) != E_NO_ERROR) {
      return err;}
  if ((err =  GPIO_Config(&nhd12832_scl)) != E_NO_ERROR) {
      return err;}
    if ((err =  GPIO_Config(&nhd12832_sda)) != E_NO_ERROR) {
        return err;
    }
     //Configure SPI interface
//    if ((err = SPIM_Init(NHD12832_SPI, &nhd12832_spim_cfg, &nhd12832_sys_cfg)) != E_NO_ERROR) {
//        return err;
//    }
    GPIO_OutClr(&nhd12832_res);
    LCD_DLY_ms(30);
    // Reset RES pin for 200us
    GPIO_OutSet(&nhd12832_res);
    LCD_DLY_ms(20);
    ////////////////////////从上电到下面开始初始化要有足够的时间，即等待RC复位完毕
//    Set_Display_On_Off  (0x00);		    // Display Off (0x00/0x01)
//    Set_Display_Clock   (0x80);		    // Set Clock as 100 Frames/Sec
//    Set_Multiplex_Ratio (0x3F);		    // 1/64 Duty (0x0F~0x3F)
//    Set_Display_Offset  (0x00);		    // Shift Mapping RAM Counter (0x00~0x3F)
//    SetStartLine        (0x00);		    // Set Mapping RAM Display Start Line (0x00~0x3F)
//    Set_Charge_Pump     (0x04);		    // Enable Embedded DC/DC Converter (0x00/0x04)
//    SetAddressingMode   (0x02);		    // Set Page Addressing Mode (0x00/0x01/0x02)
//    Set_Segment_Remap   (0x01);		    // Set SEG/Column Mapping     0x00左右反置 0x01正常
//    Set_Common_Remap    (0x08);		    // Set COM/Row Scan Direction 0x00上下反置 0x08正常
//    Set_Common_Config   (0x10);		    // Set Sequential Configuration (0x00/0x10)
//    SetContrastControl  (Brightness);	    // Set SEG Output Current
//    Set_Precharge_Period(0xF1);	    	    // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
//    Set_VCOMH           (0x40);		    // Set VCOM Deselect Level
//    Set_Entire_Display  (0x00);		    // Disable Entire Display On (0x00/0x01)
//    Set_Inverse_Display (0x00);		    // Disable Inverse Display On (0x00/0x01)  
//    Set_Display_On_Off  (0x01);		    // Display On (0x00/0x01)
//    LCD_Fill            (0x00);             //初始清屏
//    LCD_Set_Pos         (0, 0); 	
    
    
  LCD_WrCmd(0xae);//--turn off oled panel
  LCD_WrCmd(0x00);//---set low column address
  LCD_WrCmd(0x10);//---set high column address
  LCD_WrCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  LCD_WrCmd(0x81);//--set contrast control register
  LCD_WrCmd(0xcf); // Set SEG Output Current Brightness
  LCD_WrCmd(0xa1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
  LCD_WrCmd(0xc0);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
  LCD_WrCmd(0xa6);//--set normal display
  LCD_WrCmd(0xa8);//--set multiplex ratio(1 to 64)
  LCD_WrCmd(0x3f);//--1/64 duty
  LCD_WrCmd(0xd3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
  LCD_WrCmd(0x00);//-not offset
  LCD_WrCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  LCD_WrCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
  LCD_WrCmd(0xd9);//--set pre-charge period
  LCD_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  LCD_WrCmd(0xda);//--set com pins hardware configuration
  LCD_WrCmd(0x12);
  LCD_WrCmd(0xdb);//--set vcomh
  LCD_WrCmd(0x40);//Set VCOM Deselect Level
  LCD_WrCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
  LCD_WrCmd(0x02);//
  LCD_WrCmd(0x8d);//--set Charge Pump enable/disable
  LCD_WrCmd(0x14);//--set(0x10) disable
  LCD_WrCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
  LCD_WrCmd(0xa6);// Disable Inverse Display On (0xa6/a7) 
  LCD_WrCmd(0xaf);//--turn on oled panel
  LCD_Fill(0x00);  //初始清屏
  LCD_Set_Pos(0,0);  
  OLED_Clear()  ;
    return 0;
} 
//==============================================================
//函数名： void LCD_PutPixel(byte x,byte y)
//功能描述：绘制一个点（x,y）
//参数：真实坐标值(x,y),x的范围0～127，y的范围0～64
//返回：无
//==============================================================
void LCD_PutPixel(byte x,byte y)
{
	byte data1;  //data1当前点的数据 
	 
        LCD_Set_Pos(x,y); 
	data1 = 0x01<<(y%8); 	
	LCD_WrCmd(0xb0+(y>>3));
	LCD_WrCmd(((x&0xf0)>>4)|0x10);
	LCD_WrCmd((x&0x0f)|0x00);
	LCD_WrDat(data1); 	 	
}
//==============================================================
//函数名： void LCD_Rectangle(byte x1,byte y1,
//                   byte x2,byte y2,byte color,byte gif)
//功能描述：绘制一个实心矩形
//参数：左上角坐标（x1,y1）,右下角坐标（x2，y2）
//      其中x1、x2的范围0～127，y1，y2的范围0～63，即真实坐标值
//返回：无
//==============================================================
void LCD_Rectangle(byte x1,byte y1,byte x2,byte y2,byte gif)
{
	byte n; 
		
	LCD_Set_Pos(x1,y1>>3);
	for(n=x1;n<=x2;n++)
	{
		LCD_WrDat(0x01<<(y1%8)); 			
		if(gif == 1) 	
		    LCD_DLY_ms(50);
	}  
	LCD_Set_Pos(x1,y2>>3);
    
    for(n=x1;n<=x2;n++)
	{
		LCD_WrDat(0x01<<(y2%8)); 			
		if(gif == 1) 	
		    LCD_DLY_ms(5);
	}
	
}  
//==============================================================
//函数名：LCD_P6x8Str(byte x,byte y,byte *p)
//功能描述：写入一组标准ASCII字符串
//参数：显示的位置（x,y），y为页范围0～7，要显示的字符串
//返回：无
//==============================================================  
void LCD_P6x8Str(byte x,byte y,byte ch[])
{
    byte c=0,i=0,j=0;      
    while (ch[j]!='\0')
    {    
        c =ch[j]-32;
        if(x>126)
        {
            x=0;
            y++;
        }
        LCD_Set_Pos(x,y);    
  	    for(i=0;i<6;i++)     
  	        LCD_WrDat(F6x8[c][i]);  
  	    x+=6;
  	    j++;
    }
}
////==============================================================
////函数名：LCD_P8x16Str(byte x,byte y,byte *p)
////功能描述：写入一组标准ASCII字符串
////参数：显示的位置（x,y），y为页范围0～7，要显示的字符串
////返回：无
////==============================================================  
//void LCD_P8x16Str(byte x,byte y,byte ch[])
//{
//    byte c=0,i=0,j=0;
//        
//    while (ch[j]!='\0')
//    {    
//        c =ch[j]-32;
//        if(x>120)
//        {
//            x=0;
//            y++;
//        }
//        LCD_Set_Pos(x,y);    
//  	    for(i=0;i<8;i++)     
//  	        LCD_WrDat(F8X16[c*16+i]);
//    	LCD_Set_Pos(x,y+1);    
//  	    for(i=0;i<8;i++)     
//  	        LCD_WrDat(F8X16[c*16+i+8]);  
//        x+=8;
//  	    j++;
//    }
//}
////输出汉字字符串
//void LCD_P16x16Str(byte x,byte y,byte ch[])
//{
//	byte wm=0,ii = 0;
//	word adder=1; 
//	
//	while(ch[ii] != '\0')
//	{
//  	    wm = 0;
//  	    adder = 1;
//  	    while(F16x16_Idx[wm] > 127)
//  	    {
//  		    if(F16x16_Idx[wm] == ch[ii])
//  		    {
//  			    if(F16x16_Idx[wm + 1] == ch[ii + 1])
//  			    {
//  				    adder = wm * 16;
//  				    break;
//  			    }
//  		 }
//  		wm += 2;			
//    }
//    
//  	if(x>112)
//  	{
//  	    x=0;
//  	    y+=2;
//  	}
//  	LCD_Set_Pos(x , y); 
//  	if(adder != 1)// 显示汉字					
//  	{
//  		LCD_Set_Pos(x , y);
//  		for(wm = 0;wm < 16;wm++)               
//  			LCD_WrDat(F16x16[adder++]);	
//      
//  		LCD_Set_Pos(x,y + 1); 
//  		for(wm = 0;wm < 16;wm++)          
//  			LCD_WrDat(F16x16[adder++]);		
//  	}
//  	else			  //显示空白字符			
//  	{
//  		ii += 1;
//        LCD_Set_Pos(x,y);
//  		for(wm = 0;wm < 16;wm++)
//  			LCD_WrDat(0);
//  			
//  		LCD_Set_Pos(x,y + 1);
//  		for(wm = 0;wm < 16;wm++) 		
//  			LCD_WrDat(0);	
//  	}
//  	x += 16;
//  	ii += 2;
//	}
//}
////输出汉字和字符混合字符串
//void LCD_Print(byte x, byte y, byte ch[])
//{
//	byte ch2[3];
//	byte ii=0;        
//	while(ch[ii] != '\0')
//	{
//		if(ch[ii] > 127)
//		{
//			ch2[0] = ch[ii];
//	 		ch2[1] = ch[ii + 1];
//			ch2[2] = '\0';			    //汉字为两个字节
//			LCD_P16x16Str(x , y, ch2);	//显示汉字
//			x += 16;
//			ii += 2;
//		}
//		else
//		{
//			ch2[0] = ch[ii];	
//			ch2[1] = '\0';			    //字母占一个字节
//			LCD_P8x16Str(x , y , ch2);	//显示字母
//			x += 8;
//			ii+= 1;
//		}
//	}
//} 
//
////==============================================================
////函数名： void Draw_BMP(byte x,byte y)
////功能描述：显示BMP图片128×64
////参数：起始点坐标(x,y),x的范围0～127，y为页的范围0～7
////返回：无
////==============================================================
//void Draw_BMP(byte x0,byte y0,byte x1,byte y1,byte bmp[])
//{ 	
//    word ii=0;
//    byte x,y;
//  
//    if(y1%8==0) 
//        y=y1/8;      
//    else 
//        y=y1/8+1;
//	for(y=y0;y<=y1;y++)
//	{
//		LCD_Set_Pos(x0,y);				
//        for(x=x0;x<x1;x++)     
//	    	LCD_WrDat(bmp[ii++]);	    	
//
//	}
//}
//
//void Draw_LibLogo(void)
//{ 	
//    word ii=0;
//    byte x,y;       
//  
//	for(y=0;y<8;y++)
//	{
//		LCD_Set_Pos(34,y);				
//        for(x=34;x<94;x++)     
//	    	LCD_WrDat(LIBLOGO60x58[ii++]);	    	
//
//	}
//} 
