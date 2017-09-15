#include <math.h>
#include "IMU.h"
#include "arm_math.h"
#include "bmi160.h"
#include "board.h"

#define RtA     57.324841                //弧度到角度
#define AtR      0.0174533                //度到角度
#define Acc_G     0.0011963                //加速度变成G
#define Gyro_G     0.0152672                //角速度变成度
#define Gyro_Gr    0.0002663            //角速度变成弧度，此参数对应陀螺2000度每秒    
#define FILTER_NUM 10

int ACC_AVG_X,ACC_AVG_Y,ACC_AVG_Z;//平均值滤波后的ACC
S_FLOAT_XYZ GYRO_I;            //陀螺仪积分
S_FLOAT_XYZ EXP_ANGLE;        //期望角度
S_FLOAT_XYZ DIF_ANGLE;        //期望角度与实际角度差
S_FLOAT_XYZ Q_ANGLE;        //四元数计算出的角度
float GYRO_I_X,GYRO_I_Y,GYRO_I_Z;//陀螺仪积分?
int16_t    ACC_X_BUF[FILTER_NUM],ACC_Y_BUF[FILTER_NUM],ACC_Z_BUF[FILTER_NUM];    //加速度滑动窗口滤波数组
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
    ACC_X_BUF[filter_cnt] = accel.x;//更新滑动窗口数组
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
    //以下是积分值
   GYRO_I.X+=gyro.x*Gyro_G*0.02;//0.0001是时间间隔，两次prepare函数的执行周期
   GYRO_I.Y+=gyro.y*Gyro_G*0.02;//示波器测量的得到的时间是20ms.
   GYRO_I.Z+=gyro.z*Gyro_G*0.02;
}

void Get_Attitude(void)
{
    IMUupdate(gyro.x*Gyro_Gr,
            gyro.y*Gyro_Gr,
            gyro.z*Gyro_Gr,
            ACC_AVG_X,ACC_AVG_X,ACC_AVG_X);    //*0.0174转成弧度
}

////////////////////////////////////////////////////////////////////////////////
#define Kp 10.0f //这里的KpKi是用于调整加速度计修正陀螺仪的速度
#define Ki 0.008f
#define halfT 0.001f       //采样周期的一半，用于求解四元数微分方程时计算角增量

float q0 = 1, q1 = 0, q2 = 0, q3 = 0; //初始姿态四元数，由上篇博文提到的变换四元数公式得来
float exInt = 0, eyInt = 0, ezInt = 0; //当前加计测得的重力加速度在三轴上的分量
 //与用当前姿态计算得来的重力在三轴上的分量的误差的积分
void IMUupdate(float  gx, float  gy, float  gz, float  ax, float  ay, float  az)//g表陀螺仪，a表加计
{
 float  q0temp,q1temp,q2temp,q3temp;//四元数暂存变量，求解微分方程时要用
 float  norm; //矢量的模或四元数的范数
 float  vx, vy, vz;//当前姿态计算得来的重力在三轴上的分量
 float  ex, ey, ez;//当前加计测得的重力加速度在三轴上的分量
 //与用当前姿态计算得来的重力在三轴上的分量的误差

 //先把这些用得到的值算好
 float  q0q0 = q0*q0;
 float  q0q1 = q0*q1;
 float  q0q2 = q0*q2;
 float  q1q1 = q1*q1;
 float  q1q3 = q1*q3;
 float  q2q2 = q2*q2;
 float  q2q3 = q2*q3;
 float  q3q3 = q3*q3;

 if(ax*ay*az==0)//加计处于自由落体状态时不进行姿态解算，因为会产生分母无穷大的情况
 return;

 norm = sqrt(ax*ax + ay*ay + az*az);//单位化加速度计，
 ax = ax /norm;//这样变更了量程也不需要修改KP参数，因为这里归一化了
 ay = ay / norm;
 az = az / norm;

 //用当前姿态计算出重力在三个轴上的分量，
 //参考坐标n系转化到载体坐标b系的用四元数表示的方向余弦矩阵第三列即是（博文一中有提到）
 vx = 2*(q1q3 - q0q2);
 vy = 2*(q0q1 + q2q3);
 vz = q0q0 - q1q1 - q2q2 + q3q3 ;

 //计算测得的重力与计算得重力间的误差，向量外积可以表示这一误差
 //原因我理解是因为两个向量是单位向量且sin0等于0
 //不过要是夹角是180度呢~这个还没理解
 ex = (ay*vz - az*vy) ;  
 ey = (az*vx - ax*vz) ;
 ez = (ax*vy - ay*vx) ;

 exInt = exInt + ex * Ki; //对误差进行积分
 eyInt = eyInt + ey * Ki;
 ezInt = ezInt + ez * Ki;

 // adjusted gyroscope measurements
 gx = gx + Kp*ex + exInt; //将误差PI后补偿到陀螺仪，即补偿零点漂移
 gy = gy + Kp*ey + eyInt;
 gz = gz + Kp*ez + ezInt; //这里的gz由于没有观测者进行矫正会产生漂移，表现出来的就是积分自增或自减

 //下面进行姿态的更新，也就是四元数微分方程的求解
 q0temp=q0;//暂存当前值用于计算
 q1temp=q1;//网上传的这份算法大多没有注意这个问题，在此补上
 q2temp=q2;
 q3temp=q3;
 //采用一阶毕卡解法，相关知识可参见《惯性器件与惯性导航系统》P212
 q0 = q0temp + (-q1temp*gx - q2temp*gy -q3temp*gz)*halfT;
 q1 = q1temp + (q0temp*gx + q2temp*gz -q3temp*gy)*halfT;
 q2 = q2temp + (q0temp*gy - q1temp*gz +q3temp*gx)*halfT;
 q3 = q3temp + (q0temp*gz + q1temp*gy -q2temp*gx)*halfT;

 //单位化四元数在空间旋转时不会拉伸，仅有旋转角度，这类似线性代数里的正交变换
 norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
 q0 = q0 / norm;
 q1 = q1 / norm;
 q2 = q2 / norm;
 q3 = q3 / norm;

 //四元数到欧拉角的转换，公式推导见博文一
 //其中YAW航向角由于加速度计对其没有修正作用，因此此处直接用陀螺仪积分代替
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
/**************************实现函数********************************************
*函数原型:		void UART1_ReportIMU(int16_t yaw,int16_t pitch,int16_t roll
				,int16_t alt,int16_t tempr,int16_t press)
*功　　能:		向上位机发送经过解算后的姿态数据
输入参数：
		int16_t yaw 经过解算后的航向角度。单位为0.1度 0 -> 3600  对应 0 -> 360.0度
		int16_t pitch 解算得到的俯仰角度，单位 0.1度。-900 - 900 对应 -90.0 -> 90.0 度
		int16_t roll  解算后得到的横滚角度，单位0.1度。 -1800 -> 1800 对应 -180.0  ->  180.0度
		int16_t alt   气压高度。 单位0.1米。  范围一个整型变量
		int16_t tempr 温度 。 单位0.1摄氏度   范围：直到你的电路板不能正常工作
		int16_t press 气压压力。单位10Pa  一个大气压强在101300pa 这个已经超过一个整型的范围。需要除以10再发给上位机
		int16_t IMUpersec  姿态解算速率。运算IMUpersec每秒。
输出参数：没有	
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

/**************************实现函数********************************************
*函数原型:		void UART1_ReportMotion(int16_t ax,int16_t ay,int16_t az,int16_t gx,int16_t gy,int16_t gz,
					int16_t hx,int16_t hy,int16_t hz)
*功　　能:		向上位机发送当前传感器的输出值
输入参数：
	int16_t ax  加速度 X轴ADC输出 范围 ：一个有符号整型
	int16_t ay  加速度 Y轴ADC输出 范围 ：一个有符号整型
	int16_t az  加速度 Z轴ADC输出 范围 ：一个有符号整型
	int16_t gx  陀螺仪 X轴ADC输出 范围 ：一个有符号整型
	int16_t gy  陀螺仪 Y轴ADC输出 范围 ：一个有符号整型
	int16_t gz  陀螺仪 Z轴ADC输出 范围 ：一个有符号整型
	int16_t hx  磁罗盘 X轴ADC输出 范围 ：一个有符号整型
	int16_t hy  磁罗盘 Y轴ADC输出 范围 ：一个有符号整型
	int16_t hz  磁罗盘 Z轴ADC输出 范围 ：一个有符号整型
	
输出参数：没有	
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
