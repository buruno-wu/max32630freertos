/****************************************************************************************
* 文件名：LOADBIT.C
* 功能：显示单色图形及汉字显示。先将图形转换为对应的点阵数组，然后即可调用此文件的函数输出驱动。
* 作者：黄绍斌
* 日期：2004.02.26
* 备注：使用GUI_SetBackColor()函数设置显示颜色及背景色。
****************************************************************************************/
#include  "config.h"

//一些用到的变量
#define HZ_FONT_ADDR 0x26000 //字库存放的起始地址，根据实际情况处理
uint8 *hz_addr;//访问字库数据地址指针
uint8 hz_width1,hz_width2;//西文字宽度，中文字宽度
uint8 hz_height; //字体统一高度
uint8 hz_code_mode;//编码方式
uint8 hz_bytes1,hz_bytes2;//西文字占用字节数，中文字字节数
uint8 font_lib_valid_flag;//字库有效性

#if  (GUI_LoadPic_EN==1)|(GUI_MenuIco_EN==1)|(GUI_PutHZ_EN==1)
/****************************************************************************
* 名称：GUI_LoadLine()
* 功能：输出单色图形的一行数据。
* 入口参数： x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的数据。
*           no      要显示此行的点个数
* 出口参数：返回值为1时表示操作成功，为0时表示操作失败。
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
uint8  GUI_LoadLine(uint32 x, uint32 y, uint8 *dat, uint32 no)
{  uint8   bit_dat=0;
   uint8   i;
   TCOLOR  bakc;

   /* 参数过滤 */
   if(x>=GUI_LCM_XMAX) return(0);
   if(y>=GUI_LCM_YMAX) return(0);
   
   for(i=0; i<no; i++)
   {  /* 判断是否要读取点阵数据 */
      if( (i%8)==0 ) bit_dat = *dat++;
     
      /* 设置相应的点为color或为back_color */
      if( (bit_dat&DCB2HEX_TAB[i&0x07])==0 ) GUI_CopyColor(&bakc, back_color); 
         else  GUI_CopyColor(&bakc, disp_color);
      GUI_Point(x, y, bakc);       
     
      if( (++x)>=GUI_LCM_XMAX ) return(0);
   }
   
   return(1);
}
#endif


#if  (GUI_LoadPic_EN==1)|(GUI_MenuIco_EN==1)
/****************************************************************************
* 名称：GUI_LoadPic()
* 功能：输出单色图形数据。
* 入口参数： x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的数据
*           hno     要显示此行的点个数
*           lno     要显示此列的点个数
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_LoadPic(uint32 x, uint32 y, uint8 *dat, uint32 hno, uint32 lno)
{  uint32  i;

   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);				// 输出一行数据
      y++;										// 显示下一行
      dat += (hno>>3);							// 计算下一行的数据
      if( (hno&0x07)!=0 ) dat++;
   }
}




/****************************************************************************
* 名称：GUI_LoadPic1()
* 功能：输出单色图形数据，反相显示。
* 入口参数： x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的数据。
*           hno     要显示此行的点个数
*           lno     要显示此列的点个数
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_LoadPic1(uint32 x, uint32 y, uint8 *dat, uint32 hno, uint32 lno)
{  uint32  i;
   
   GUI_ExchangeColor();									// 显示色与背景色交换
   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);						// 输出一行数据
      y++;												// 显示下一行
      dat += (hno>>3);									// 计算下一行的数据
      if( (hno&0x07)!=0 ) dat++;
   }
   GUI_ExchangeColor();
   
}
#endif


#if  GUI_PutHZ_EN==1
/****************************************************************************
* 名称：GUI_PutHZ()
* 功能：显示汉字。
* 入口参数： x		指定显示位置，x坐标
*           y		指定显示位置，y坐标
*           dat		要输出显示的汉字点阵数据。
*           hno     要显示此行的点个数
*           lno     要显示此列的点个数
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_PutHZ(uint32 x, uint32 y, uint8 *dat, uint8 hno, uint8 lno)
{  uint8  i;

   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);						// 输出一行数据
      y++;												// 显示下一行
      dat += (hno>>3);									// 计算下一行的数据
      if( (hno&0x07)!=0 ) dat++;
   }
}

void GUI_PutHZ1(uint32 x, uint32 y, uint8 *chr)
{
uint32 tmp;
if(!font_lib_valid_flag)//字库是否有效
return;
if(*chr<128)
{
hz_addr=(uint8 *)HZ_FONT_ADDR;
tmp=(*chr)*hz_bytes1;//西文字偏移量
GUI_PutHZ(x, y, hz_addr+tmp, hz_width1, hz_height);//输出西文字
}
else
{
hz_addr=(uint8 *)HZ_FONT_ADDR+128*hz_bytes1;
 tmp=((*(chr)-0xa0-1)*94+(*(chr+1)-0xa0-1))*hz_bytes2;//中文字偏移量
GUI_PutHZ(x, y, hz_addr+tmp, hz_width2 ,hz_height);//输出中文字
}
}
//这个函数用来显示水平输出汉字字符串，可混合输出汉字和ASCIi字符，能自动识别字体的宽度
//调用示例：GUI_PutHZStringH(0,0,"山光物态弄春晖，莫为轻阴便拟归");
//GUI_PutHZStringH(0,100,"abcdef一二三四");
void GUI_PutHZStringH(uint32 x, uint32 y, uint8 *str)
{
 if(!font_lib_valid_flag)//字库是否有效
return;
 while(1)
 { if( (*str)=='\0' ) break;
 
 if(*str > 127)
 {
  GUI_PutHZ1(x, y, str);
 str+=2;//每个中文字由两个字节数据组成
 x+=hz_width2;//中文字体
 }
 else
 {
  GUI_PutHZ1(x, y, str);
 str+=1;
 x+=hz_width1;//西文字体
 }
 }
}
//这个函数用来显示垂直输出汉字字符串，可混合输出汉字和ASCII字符，能自动识别字体的宽度
//调用示例：GUI_PutHZStringV(0,0,"山光物态弄春晖，莫为轻阴便拟归");
//GUI_PutHZStringV(0,100,"abcdef一二三四");
void GUI_PutHZStringV(uint32 x, uint32 y, uint8 *str)
{
if(!font_lib_valid_flag)//字库是否有效
return;
 while(1)
 { if( (*str)=='\0' ) break;
 
 if(*str > 127)
 {
  GUI_PutHZ1(x, y, str);
 str+=2;
 y+=hz_height;//中文字体
 }
 else
 {
  GUI_PutHZ1(x, y, str);
 str+=1;
 y+=hz_height;//西文字体
 }
 }
}

//字库参数读取及处理：字体统一高度、西文字体宽度、中文字体宽度、编码方式、校验字节数据
//在输出GB2312字库数据之前，必须先调用这个函数，仅需调用一次即可，用来读取字库的参数如：字体统一高度、宽度、编码方式（输出函数只支持GB2312）、校验等。
void GUI_HZParameterInit(void)
{
uint8 tmp;
hz_addr=(uint8 *)HZ_FONT_ADDR;
 hz_height=*hz_addr;//字体统一高度
hz_width1=*(hz_addr+1);//西文字体宽度
hz_width2=*(hz_addr+2);//中文字体宽度
hz_code_mode=*(hz_addr+3);//字体编码方式

tmp=~(hz_height+hz_width1+hz_width2+hz_code_mode);//计算校验
if(*(hz_addr+4)==tmp)//比较校验是否有效
{
font_lib_valid_flag=0xff;
hz_bytes1=((hz_width1%8) ?hz_width1/8+1:hz_width1/8)*hz_height; //计算西文字体字节数
hz_bytes2=((hz_width2%8) ?hz_width2/8+1:hz_width2/8)*hz_height;//计算中文字体字节数
}
else
font_lib_valid_flag=0x00;//无效的字库
}

#endif






