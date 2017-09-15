#ifndef _LQOLED_H
#define _LQOLED_H

typedef  unsigned char byte;
typedef  unsigned long word;
typedef  unsigned char u8;
 int LCD_Init(void);
 void LCD_CLS(void);
 void LCD_P6x8Str(byte x,byte y,byte ch[]);
 void LCD_P8x16Str(byte x,byte y,byte ch[]);
 void LCD_P16x16Str(byte x,byte y,byte ch[]);
 void LCD_Print(byte x, byte y, byte ch[]);
 void LCD_PutPixel(byte x,byte y);
 void LCD_Rectangle(byte x1,byte y1,byte x2,byte y2,byte gif);
 void Draw_LibLogo(void);
 void Draw_BMP(byte x0,byte y0,byte x1,byte y1,byte bmp[]); 
 void LCD_Fill(byte dat);
 void OLED_Refresh_Gram(void);
 void OLED_DrawPoint(byte x,byte y,byte t);
 void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot) ;
 void OLED_Clear(void)  ;
#endif

