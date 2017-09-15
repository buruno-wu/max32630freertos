#include <math.h>
#include "IMU.h"
#include "arm_math.h"
#include "bmi160.h"
#include "board.h"

#define RtA     57.324841                //���ȵ��Ƕ�
#define AtR      0.0174533                //�ȵ��Ƕ�
#define Acc_G     0.0011963                //���ٶȱ��G
#define Gyro_G     0.0152672                //���ٶȱ�ɶ�
#define Gyro_Gr    0.0002663            //���ٶȱ�ɻ��ȣ��˲�����Ӧ����2000��ÿ��    
#define FILTER_NUM 10

int ACC_AVG_X,ACC_AVG_Y,ACC_AVG_Z;//ƽ��ֵ�˲����ACC
S_FLOAT_XYZ GYRO_I;            //�����ǻ���
S_FLOAT_XYZ EXP_ANGLE;        //�����Ƕ�
S_FLOAT_XYZ DIF_ANGLE;        //�����Ƕ���ʵ�ʽǶȲ�
S_FLOAT_XYZ Q_ANGLE;        //��Ԫ��������ĽǶ�
float GYRO_I_X,GYRO_I_Y,GYRO_I_Z;//�����ǻ���?
int16_t    ACC_X_BUF[FILTER_NUM],ACC_Y_BUF[FILTER_NUM],ACC_Z_BUF[FILTER_NUM];    //���ٶȻ��������˲�����
struct bmi160_gyro_t gyro;
struct bmi160_accel_t accel;

void Prepare_Data(void)
{
    static unsigned char filter_cnt=0;
    long int temp1=0,temp2=0,temp3=0;
    unsigned char i;
    bmi160_read_gyro_xyz(&gyro);
   // printf("gyro x:%d-y:%d-z:%d\n\r",gyro.x,gyro.y,gyro.z);
    bmi160_read_accel_xyz(&accel);
  //  printf("accel x:%d-y:%d-z:%d\n\r",accel.x,accel.y,accel.z);
    ACC_X_BUF[filter_cnt] = accel.x;//���»�����������
    ACC_Y_BUF[filter_cnt] = accel.y;
    ACC_Z_BUF[filter_cnt] = accel.z;
    for(i=0;i<FILTER_NUM;i++)
    {
        temp1 += ACC_X_BUF[i];
        temp2 += ACC_Y_BUF[i];
        temp3 += ACC_Z_BUF[i];
    }
    ACC_AVG_X = temp1 / FILTER_NUM;
    ACC_AVG_Y = temp2 / FILTER_NUM;
    ACC_AVG_Z = temp3 / FILTER_NUM;
//    printf("filter_cnt:%d\n\r",filter_cnt);
    filter_cnt++;
    if(filter_cnt==FILTER_NUM) filter_cnt=0;
    //�����ǻ���ֵ
   GYRO_I.X+=gyro.x*Gyro_G*0.02;//0.0001��ʱ����������prepare������ִ������
   GYRO_I.Y+=gyro.y*Gyro_G*0.02;//ʾ���������ĵõ���ʱ����20ms.
   GYRO_I.Z+=gyro.z*Gyro_G*0.02;
}

void Get_Attitude(void)
{
    IMUupdate(gyro.x*Gyro_Gr,
            gyro.y*Gyro_Gr,
            gyro.z*Gyro_Gr,
            ACC_AVG_X,ACC_AVG_X,ACC_AVG_X);    //*0.0174ת�ɻ���
}

////////////////////////////////////////////////////////////////////////////////
#define Kp 10.0f //�����KpKi�����ڵ������ٶȼ����������ǵ��ٶ�
#define Ki 0.008f
#define halfT 0.001f       //�������ڵ�һ�룬���������Ԫ��΢�ַ���ʱ���������

float q0 = 1, q1 = 0, q2 = 0, q3 = 0; //��ʼ��̬��Ԫ��������ƪ�����ᵽ�ı任��Ԫ����ʽ����
float exInt = 0, eyInt = 0, ezInt = 0; //��ǰ�ӼƲ�õ��������ٶ��������ϵķ���
 //���õ�ǰ��̬��������������������ϵķ��������Ļ���
void IMUupdate(float  gx, float  gy, float  gz, float  ax, float  ay, float  az)//g�������ǣ�a��Ӽ�
{
 float  q0temp,q1temp,q2temp,q3temp;//��Ԫ���ݴ���������΢�ַ���ʱҪ��
 float  norm; //ʸ����ģ����Ԫ���ķ���
 float  vx, vy, vz;//��ǰ��̬��������������������ϵķ���
 float  ex, ey, ez;//��ǰ�ӼƲ�õ��������ٶ��������ϵķ���
 //���õ�ǰ��̬��������������������ϵķ��������

 //�Ȱ���Щ�õõ���ֵ���
 float  q0q0 = q0*q0;
 float  q0q1 = q0*q1;
 float  q0q2 = q0*q2;
 float  q1q1 = q1*q1;
 float  q1q3 = q1*q3;
 float  q2q2 = q2*q2;
 float  q2q3 = q2*q3;
 float  q3q3 = q3*q3;

 if(ax*ay*az==0)//�Ӽƴ�����������״̬ʱ��������̬���㣬��Ϊ�������ĸ���������
 return;

 norm = sqrt(ax*ax + ay*ay + az*az);//��λ�����ٶȼƣ�
 ax = ax /norm;//�������������Ҳ����Ҫ�޸�KP��������Ϊ�����һ����
 ay = ay / norm;
 az = az / norm;

 //�õ�ǰ��̬������������������ϵķ�����
 //�ο�����nϵת������������bϵ������Ԫ����ʾ�ķ������Ҿ�������м��ǣ�����һ�����ᵽ��
 vx = 2*(q1q3 - q0q2);
 vy = 2*(q0q1 + q2q3);
 vz = q0q0 - q1q1 - q2q2 + q3q3 ;

 //�����õ������������������������������Ա�ʾ��һ���
 //ԭ�����������Ϊ���������ǵ�λ������sin0����0
 //����Ҫ�Ǽн���180����~�����û���
 ex = (ay*vz - az*vy) ;  
 ey = (az*vx - ax*vz) ;
 ez = (ax*vy - ay*vx) ;

 exInt = exInt + ex * Ki; //�������л���
 eyInt = eyInt + ey * Ki;
 ezInt = ezInt + ez * Ki;

 // adjusted gyroscope measurements
 gx = gx + Kp*ex + exInt; //�����PI�󲹳��������ǣ����������Ư��
 gy = gy + Kp*ey + eyInt;
 gz = gz + Kp*ez + ezInt; //�����gz����û�й۲��߽��н��������Ư�ƣ����ֳ����ľ��ǻ����������Լ�

 //���������̬�ĸ��£�Ҳ������Ԫ��΢�ַ��̵����
 q0temp=q0;//�ݴ浱ǰֵ���ڼ���
 q1temp=q1;//���ϴ�������㷨���û��ע��������⣬�ڴ˲���
 q2temp=q2;
 q3temp=q3;
 //����һ�ױϿ��ⷨ�����֪ʶ�ɲμ���������������Ե���ϵͳ��P212
 q0 = q0temp + (-q1temp*gx - q2temp*gy -q3temp*gz)*halfT;
 q1 = q1temp + (q0temp*gx + q2temp*gz -q3temp*gy)*halfT;
 q2 = q2temp + (q0temp*gy - q1temp*gz +q3temp*gx)*halfT;
 q3 = q3temp + (q0temp*gz + q1temp*gy -q2temp*gx)*halfT;

 //��λ����Ԫ���ڿռ���תʱ�������죬������ת�Ƕȣ����������Դ�����������任
 norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
 q0 = q0 / norm;
 q1 = q1 / norm;
 q2 = q2 / norm;
 q3 = q3 / norm;

 //��Ԫ����ŷ���ǵ�ת������ʽ�Ƶ�������һ
 //����YAW��������ڼ��ٶȼƶ���û���������ã���˴˴�ֱ���������ǻ��ִ���
 Q_ANGLE.Z = GYRO_I.Z; // yaw
 Q_ANGLE.Y = asin(-2 * q1 * q3 + 2 * q0* q2)*57.3; // pitch
 Q_ANGLE.X = atan2(2 * q2 * q3 + 2 * q0 * q1,-2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3; // roll
 
}
#define MXC_UARTn   MXC_UART_GET_UART(CONSOLE_UART)
#define UART_FIFO   MXC_UART_GET_FIFO(CONSOLE_UART)

static void UART1_Put_Char(uint8_t data)
{
    // Wait for TXFIFO to not be full
    while ((MXC_UARTn->tx_fifo_ctrl & MXC_F_UART_TX_FIFO_CTRL_FIFO_ENTRY) == MXC_F_UART_TX_FIFO_CTRL_FIFO_ENTRY);
    MXC_UARTn->intfl = MXC_F_UART_INTFL_TX_DONE; // clear DONE flag for UART_PrepForSleep
    UART_FIFO->tx = data;
}
/**************************ʵ�ֺ���********************************************
*����ԭ��:		void UART1_ReportIMU(int16_t yaw,int16_t pitch,int16_t roll
				,int16_t alt,int16_t tempr,int16_t press)
*��������:		����λ�����;�����������̬����
���������
		int16_t yaw ���������ĺ���Ƕȡ���λΪ0.1�� 0 -> 3600  ��Ӧ 0 -> 360.0��
		int16_t pitch ����õ��ĸ����Ƕȣ���λ 0.1�ȡ�-900 - 900 ��Ӧ -90.0 -> 90.0 ��
		int16_t roll  �����õ��ĺ���Ƕȣ���λ0.1�ȡ� -1800 -> 1800 ��Ӧ -180.0  ->  180.0��
		int16_t alt   ��ѹ�߶ȡ� ��λ0.1�ס�  ��Χһ�����ͱ���
		int16_t tempr �¶� �� ��λ0.1���϶�   ��Χ��ֱ����ĵ�·�岻����������
		int16_t press ��ѹѹ������λ10Pa  һ������ѹǿ��101300pa ����Ѿ�����һ�����͵ķ�Χ����Ҫ����10�ٷ�����λ��
		int16_t IMUpersec  ��̬�������ʡ�����IMUpersecÿ�롣
���������û��	
*******************************************************************************/
void UART1_ReportIMU(int16_t yaw,int16_t pitch,int16_t roll
,int16_t alt,int16_t tempr,int16_t press,int16_t IMUpersec)
{
 	unsigned int temp=0xaF+2;
	char ctemp;
	UART1_Put_Char(0xa5);
	UART1_Put_Char(0x5a);
	UART1_Put_Char(14+2);
	UART1_Put_Char(0xA1);

	if(yaw<0)yaw=32768-yaw;
	ctemp=yaw>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=yaw;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(pitch<0)pitch=32768-pitch;
	ctemp=pitch>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=pitch;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(roll<0)roll=32768-roll;
	ctemp=roll>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=roll;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(alt<0)alt=32768-alt;
	ctemp=alt>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=alt;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(tempr<0)tempr=32768-tempr;
	ctemp=tempr>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=tempr;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(press<0)press=32768-press;
	ctemp=press>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=press;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	UART1_Put_Char(temp%256);
	UART1_Put_Char(0xaa);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void UART1_ReportMotion(int16_t ax,int16_t ay,int16_t az,int16_t gx,int16_t gy,int16_t gz,
					int16_t hx,int16_t hy,int16_t hz)
*��������:		����λ�����͵�ǰ�����������ֵ
���������
	int16_t ax  ���ٶ� X��ADC��� ��Χ ��һ���з�������
	int16_t ay  ���ٶ� Y��ADC��� ��Χ ��һ���з�������
	int16_t az  ���ٶ� Z��ADC��� ��Χ ��һ���з�������
	int16_t gx  ������ X��ADC��� ��Χ ��һ���з�������
	int16_t gy  ������ Y��ADC��� ��Χ ��һ���з�������
	int16_t gz  ������ Z��ADC��� ��Χ ��һ���з�������
	int16_t hx  ������ X��ADC��� ��Χ ��һ���з�������
	int16_t hy  ������ Y��ADC��� ��Χ ��һ���з�������
	int16_t hz  ������ Z��ADC��� ��Χ ��һ���з�������
	
���������û��	
*******************************************************************************/
void UART1_ReportMotion(int16_t ax,int16_t ay,int16_t az,int16_t gx,int16_t gy,int16_t gz,
					int16_t hx,int16_t hy,int16_t hz)
{
 	unsigned int temp=0xaF+9;
	char ctemp;
	UART1_Put_Char(0xa5);
	UART1_Put_Char(0x5a);
	UART1_Put_Char(14+8);
	UART1_Put_Char(0xA2);

	if(ax<0)ax=32768-ax;
	ctemp=ax>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=ax;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(ay<0)ay=32768-ay;
	ctemp=ay>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=ay;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(az<0)az=32768-az;
	ctemp=az>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=az;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(gx<0)gx=32768-gx;
	ctemp=gx>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=gx;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(gy<0)gy=32768-gy;
	ctemp=gy>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=gy;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
//-------------------------
	if(gz<0)gz=32768-gz;
	ctemp=gz>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=gz;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(hx<0)hx=32768-hx;
	ctemp=hx>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=hx;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(hy<0)hy=32768-hy;
	ctemp=hy>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=hy;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	if(hz<0)hz=32768-hz;
	ctemp=hz>>8;
	UART1_Put_Char(ctemp);
	temp+=ctemp;
	ctemp=hz;
	UART1_Put_Char(ctemp);
	temp+=ctemp;

	UART1_Put_Char(temp%256);
	UART1_Put_Char(0xaa);
}

void uart_report_imu(void)
{

UART1_ReportIMU( Q_ANGLE.Z*50, Q_ANGLE.Y*10 , Q_ANGLE.X*10,10.3,30,1.5, 50);
UART1_ReportMotion(accel.x,accel.y,accel.z,gyro.x,gyro.y,gyro.z,0,0,0);
}
