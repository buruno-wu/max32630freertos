/****************************************************************************************
* �ļ�����LOADBIT.C
* ���ܣ���ʾ��ɫͼ�μ�������ʾ���Ƚ�ͼ��ת��Ϊ��Ӧ�ĵ������飬Ȼ�󼴿ɵ��ô��ļ��ĺ������������
* ���ߣ����ܱ�
* ���ڣ�2004.02.26
* ��ע��ʹ��GUI_SetBackColor()����������ʾ��ɫ������ɫ��
****************************************************************************************/
#include  "config.h"

//һЩ�õ��ı���
#define HZ_FONT_ADDR 0x26000 //�ֿ��ŵ���ʼ��ַ������ʵ���������
uint8 *hz_addr;//�����ֿ����ݵ�ַָ��
uint8 hz_width1,hz_width2;//�����ֿ�ȣ������ֿ��
uint8 hz_height; //����ͳһ�߶�
uint8 hz_code_mode;//���뷽ʽ
uint8 hz_bytes1,hz_bytes2;//������ռ���ֽ������������ֽ���
uint8 font_lib_valid_flag;//�ֿ���Ч��

#if  (GUI_LoadPic_EN==1)|(GUI_MenuIco_EN==1)|(GUI_PutHZ_EN==1)
/****************************************************************************
* ���ƣ�GUI_LoadLine()
* ���ܣ������ɫͼ�ε�һ�����ݡ�
* ��ڲ����� x		ָ����ʾλ�ã�x����
*           y		ָ����ʾλ�ã�y����
*           dat		Ҫ�����ʾ�����ݡ�
*           no      Ҫ��ʾ���еĵ����
* ���ڲ���������ֵΪ1ʱ��ʾ�����ɹ���Ϊ0ʱ��ʾ����ʧ�ܡ�
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
uint8  GUI_LoadLine(uint32 x, uint32 y, uint8 *dat, uint32 no)
{  uint8   bit_dat=0;
   uint8   i;
   TCOLOR  bakc;

   /* �������� */
   if(x>=GUI_LCM_XMAX) return(0);
   if(y>=GUI_LCM_YMAX) return(0);
   
   for(i=0; i<no; i++)
   {  /* �ж��Ƿ�Ҫ��ȡ�������� */
      if( (i%8)==0 ) bit_dat = *dat++;
     
      /* ������Ӧ�ĵ�Ϊcolor��Ϊback_color */
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
* ���ƣ�GUI_LoadPic()
* ���ܣ������ɫͼ�����ݡ�
* ��ڲ����� x		ָ����ʾλ�ã�x����
*           y		ָ����ʾλ�ã�y����
*           dat		Ҫ�����ʾ������
*           hno     Ҫ��ʾ���еĵ����
*           lno     Ҫ��ʾ���еĵ����
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_LoadPic(uint32 x, uint32 y, uint8 *dat, uint32 hno, uint32 lno)
{  uint32  i;

   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);				// ���һ������
      y++;										// ��ʾ��һ��
      dat += (hno>>3);							// ������һ�е�����
      if( (hno&0x07)!=0 ) dat++;
   }
}




/****************************************************************************
* ���ƣ�GUI_LoadPic1()
* ���ܣ������ɫͼ�����ݣ�������ʾ��
* ��ڲ����� x		ָ����ʾλ�ã�x����
*           y		ָ����ʾλ�ã�y����
*           dat		Ҫ�����ʾ�����ݡ�
*           hno     Ҫ��ʾ���еĵ����
*           lno     Ҫ��ʾ���еĵ����
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_LoadPic1(uint32 x, uint32 y, uint8 *dat, uint32 hno, uint32 lno)
{  uint32  i;
   
   GUI_ExchangeColor();									// ��ʾɫ�뱳��ɫ����
   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);						// ���һ������
      y++;												// ��ʾ��һ��
      dat += (hno>>3);									// ������һ�е�����
      if( (hno&0x07)!=0 ) dat++;
   }
   GUI_ExchangeColor();
   
}
#endif


#if  GUI_PutHZ_EN==1
/****************************************************************************
* ���ƣ�GUI_PutHZ()
* ���ܣ���ʾ���֡�
* ��ڲ����� x		ָ����ʾλ�ã�x����
*           y		ָ����ʾλ�ã�y����
*           dat		Ҫ�����ʾ�ĺ��ֵ������ݡ�
*           hno     Ҫ��ʾ���еĵ����
*           lno     Ҫ��ʾ���еĵ����
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_PutHZ(uint32 x, uint32 y, uint8 *dat, uint8 hno, uint8 lno)
{  uint8  i;

   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);						// ���һ������
      y++;												// ��ʾ��һ��
      dat += (hno>>3);									// ������һ�е�����
      if( (hno&0x07)!=0 ) dat++;
   }
}

void GUI_PutHZ1(uint32 x, uint32 y, uint8 *chr)
{
uint32 tmp;
if(!font_lib_valid_flag)//�ֿ��Ƿ���Ч
return;
if(*chr<128)
{
hz_addr=(uint8 *)HZ_FONT_ADDR;
tmp=(*chr)*hz_bytes1;//������ƫ����
GUI_PutHZ(x, y, hz_addr+tmp, hz_width1, hz_height);//���������
}
else
{
hz_addr=(uint8 *)HZ_FONT_ADDR+128*hz_bytes1;
 tmp=((*(chr)-0xa0-1)*94+(*(chr+1)-0xa0-1))*hz_bytes2;//������ƫ����
GUI_PutHZ(x, y, hz_addr+tmp, hz_width2 ,hz_height);//���������
}
}
//�������������ʾˮƽ��������ַ������ɻ��������ֺ�ASCIi�ַ������Զ�ʶ������Ŀ��
//����ʾ����GUI_PutHZStringH(0,0,"ɽ����̬Ū���ͣ�ĪΪ���������");
//GUI_PutHZStringH(0,100,"abcdefһ������");
void GUI_PutHZStringH(uint32 x, uint32 y, uint8 *str)
{
 if(!font_lib_valid_flag)//�ֿ��Ƿ���Ч
return;
 while(1)
 { if( (*str)=='\0' ) break;
 
 if(*str > 127)
 {
  GUI_PutHZ1(x, y, str);
 str+=2;//ÿ���������������ֽ��������
 x+=hz_width2;//��������
 }
 else
 {
  GUI_PutHZ1(x, y, str);
 str+=1;
 x+=hz_width1;//��������
 }
 }
}
//�������������ʾ��ֱ��������ַ������ɻ��������ֺ�ASCII�ַ������Զ�ʶ������Ŀ��
//����ʾ����GUI_PutHZStringV(0,0,"ɽ����̬Ū���ͣ�ĪΪ���������");
//GUI_PutHZStringV(0,100,"abcdefһ������");
void GUI_PutHZStringV(uint32 x, uint32 y, uint8 *str)
{
if(!font_lib_valid_flag)//�ֿ��Ƿ���Ч
return;
 while(1)
 { if( (*str)=='\0' ) break;
 
 if(*str > 127)
 {
  GUI_PutHZ1(x, y, str);
 str+=2;
 y+=hz_height;//��������
 }
 else
 {
  GUI_PutHZ1(x, y, str);
 str+=1;
 y+=hz_height;//��������
 }
 }
}

//�ֿ������ȡ����������ͳһ�߶ȡ����������ȡ����������ȡ����뷽ʽ��У���ֽ�����
//�����GB2312�ֿ�����֮ǰ�������ȵ�������������������һ�μ��ɣ�������ȡ�ֿ�Ĳ����磺����ͳһ�߶ȡ���ȡ����뷽ʽ���������ֻ֧��GB2312����У��ȡ�
void GUI_HZParameterInit(void)
{
uint8 tmp;
hz_addr=(uint8 *)HZ_FONT_ADDR;
 hz_height=*hz_addr;//����ͳһ�߶�
hz_width1=*(hz_addr+1);//����������
hz_width2=*(hz_addr+2);//����������
hz_code_mode=*(hz_addr+3);//������뷽ʽ

tmp=~(hz_height+hz_width1+hz_width2+hz_code_mode);//����У��
if(*(hz_addr+4)==tmp)//�Ƚ�У���Ƿ���Ч
{
font_lib_valid_flag=0xff;
hz_bytes1=((hz_width1%8) ?hz_width1/8+1:hz_width1/8)*hz_height; //�������������ֽ���
hz_bytes2=((hz_width2%8) ?hz_width2/8+1:hz_width2/8)*hz_height;//�������������ֽ���
}
else
font_lib_valid_flag=0x00;//��Ч���ֿ�
}

#endif






