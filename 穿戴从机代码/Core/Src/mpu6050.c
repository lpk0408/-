#include "mpu6050.h"
#include "i2c.h"
#include "stdio.h"
#include "math.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "i2c_hal_mpu6050.h"

static uint8_t check_val = 0;
HAL_StatusTypeDef i2c_status;

#define mpu_i2c	hi2c1

#ifndef ERROR
#define ERROR 0
#endif

//#define mpu_i2c	hi2c3

//void mpu6050_Init(void)
//{
//	printf("\r\n========== MPU6050 DMP Init Start ==========\r\n\r\n");

//	/* [1/8] Wake up MPU6050 */
//	printf("[1/9] Waking up MPU6050 (PWR_MGMT_1 = 0x00)...\r\n");
//	uint8_t wakeup = 0x00;
//	i2c_status = HAL_I2C_Mem_Write(&mpu_i2c, MPU6050_I2C_ADDR, 0x6B, 1, &wakeup, 1, 100);
//	if (i2c_status != HAL_OK)
//	{
//		printf("  [FAIL] I2C write PWR_MGMT_1 failed, status=%d\r\n", i2c_status);
//		printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//		return;
//	}
//	printf("  [ OK ] PWR_MGMT_1 = 0x00, chip awake\r\n\r\n");
//	HAL_Delay(50);

//	/* [2/8] Read WHO_AM_I register */
//	printf("[2/9] Reading WHO_AM_I register (expected: 0x68)...\r\n");
//	i2c_status = HAL_I2C_Mem_Read(&mpu_i2c, MPU6050_I2C_ADDR, MPU6050_WHO_AM_I,
//	                               I2C_MEMADD_SIZE_8BIT, &check_val, 1, 100);
//	HAL_Delay(50);
//	if (i2c_status == HAL_OK && check_val == 0x68)
//	{
//		printf("  [ OK ] WHO_AM_I = 0x%02X, MPU6050 confirmed on I2C3\r\n\r\n", check_val);
//	}
//	else
//	{
//		printf("  [FAIL] WHO_AM_I read failed, status=%d, value=0x%02X\r\n", i2c_status, check_val);
//		printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//		return;
//	}

//	/* [3/8] Full MPU6050 initialization via inv_mpu library */
//	printf("[3/9] Running mpu_init() (reset + gyro FSR=2000dps, accel FSR=2g, LPF=42Hz)...\r\n");
//	{
//		struct int_param_s int_param = {0};
//		int ret = mpu_init(&int_param);
//		if (ret != 0)
//		{
//			printf("  [FAIL] mpu_init() returned %d\r\n", ret);
//			printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//			return;
//		}
//		printf("  [ OK ] mpu_init() succeeded\r\n\r\n");
//	}

//	/* [4/9] Enable sensors (required before DMP firmware load) */
//	printf("[4/9] Enabling sensors (GYRO + ACCEL)...\r\n");
//	{
//		int ret = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
//		if (ret != 0)
//		{
//			printf("  [FAIL] mpu_set_sensors() returned %d\r\n", ret);
//			printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//			return;
//		}
//		printf("  [ OK ] Sensors enabled: GYRO + ACCEL\r\n\r\n");
//	}

//	/* [5/9] Load DMP motion driver firmware */
//	printf("[5/9] Loading DMP motion driver firmware (3062 bytes)...\r\n");
//	{
//		int ret = dmp_load_motion_driver_firmware();
//		if (ret != 0)
//		{
//			printf("  [FAIL] dmp_load_motion_driver_firmware() returned %d\r\n", ret);
//			printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//			return;
//		}
//		printf("  [ OK ] DMP firmware loaded successfully\r\n\r\n");
//	}

//	/* [6/9] Enable DMP features */
//	printf("[6/9] Enabling DMP features: 6X_LP_QUAT + RAW_ACCEL + RAW_GYRO...\r\n");
//	{
//		unsigned short features = DMP_FEATURE_6X_LP_QUAT
//		                        | DMP_FEATURE_SEND_RAW_ACCEL
//		                        | DMP_FEATURE_SEND_RAW_GYRO;
//		int ret = dmp_enable_feature(features);
//		if (ret != 0)
//		{
//			printf("  [FAIL] dmp_enable_feature(0x%04X) returned %d\r\n", features, ret);
//			printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//			return;
//		}
//		printf("  [ OK ] Features enabled: 0x%04X\r\n\r\n", features);
//	}

//	/* [7/9] Set DMP FIFO rate */
//	printf("[7/9] Setting DMP FIFO output rate...\r\n");
//	{
//		unsigned short fifo_rate = 5;
//		int ret = dmp_set_fifo_rate(fifo_rate);
//		if (ret != 0)
//		{
//			printf("  [FAIL] dmp_set_fifo_rate(%d) returned %d\r\n", fifo_rate, ret);
//			printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//			return;
//		}
//		printf("  [ OK ] DMP FIFO rate = %d Hz\r\n\r\n", fifo_rate);
//	}

//	/* [8/9] Enable DMP */
//	printf("[8/9] Enabling DMP...\r\n");
//	{
//		int ret = mpu_set_dmp_state(1);
//		if (ret != 0)
//		{
//			printf("  [FAIL] mpu_set_dmp_state(1) returned %d\r\n", ret);
//			printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
//			return;
//		}
//		printf("  [ OK ] DMP enabled, FIFO streaming data\r\n\r\n");
//	}

//	/* [9/9] Init complete */
//	printf("[9/9] MPU6050 DMP initialization complete!\r\n");
//	printf("      Ready to read quaternion + accel + gyro data.\r\n");
//	printf("\r\n========== MPU6050 DMP Init SUCCESS ==========\r\n\r\n");
//}

void mpu6050_read_and_print(void)
{
	short gyro[3], accel[3], sensors;
	long quat[4];
	unsigned long timestamp;
	unsigned char more;

	int ret = dmp_read_fifo(gyro, accel, quat, &timestamp, &sensors, &more);
	if (ret != 0)
		return;

	/* Convert quaternion (q30 fixed-point) to float */
	float q0 = quat[0] / 1073741824.0f;
	float q1 = quat[1] / 1073741824.0f;
	float q2 = quat[2] / 1073741824.0f;
	float q3 = quat[3] / 1073741824.0f;

	/* Quaternion to Euler angles (roll, pitch, yaw) in degrees */
	float roll  = atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * 57.29578f;
	float pitch = asinf(2.0f * (q0 * q2 - q3 * q1)) * 57.29578f;
	float yaw   = atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * 57.29578f;

	/* Convert raw accel to g (FSR=2g: 16384 LSB/g) */
	float accel_x = accel[0] / 16384.0f;
	float accel_y = accel[1] / 16384.0f;
	float accel_z = accel[2] / 16384.0f;

	/* Convert raw gyro to dps (FSR=2000dps: 16.4 LSB/dps) */
	float gyro_x = gyro[0] / 16.4f;
	float gyro_y = gyro[1] / 16.4f;
	float gyro_z = gyro[2] / 16.4f;

	printf("Ang(deg): X=%7.1f Y=%7.1f Z=%7.1f | ", roll, pitch, yaw);
//	printf("Acc(g): X=%6.2f Y=%6.2f Z=%6.2f | ", accel_x, accel_y, accel_z);
//	printf("Gyr(dps): X=%7.1f Y=%7.1f Z=%7.1f\r\n", gyro_x, gyro_y, gyro_z);
}

/* ========== Step Counter (state-machine, displacement-based) ========== */

#define STEP_STATE_STATIC   0
#define STEP_STATE_MOVING   1

static float          step_distance_cm = 10.0f;
unsigned long 				step_count = 0;
static float          disp_x = 0, disp_y = 0, disp_z = 0;
static float          vel_x = 0, vel_y = 0, vel_z = 0;
static unsigned long  last_ts = 0;
static int            first_reading = 1;
static int            state = STEP_STATE_STATIC;
static int            static_cnt = 0;

void mpu6050_set_step_distance(float cm)
{
	if (cm < 1.0f) cm = 1.0f;
	if (cm > 100.0f) cm = 100.0f;
	step_distance_cm = cm;
	mpu6050_reset_step_count();
//	printf("[STEP] Threshold set to %.1f cm\r\n", cm);
}

unsigned long mpu6050_get_step_count(void)
{
	return step_count;
}

void mpu6050_reset_step_count(void)
{
	step_count = 0;
	disp_x = disp_y = disp_z = 0;
	vel_x = vel_y = vel_z = 0;
	first_reading = 1;
	state = STEP_STATE_STATIC;
	static_cnt = 0;
//	printf("[STEP] Step count reset to 0\r\n");
}


static float acc_filter = 0.0f;
static unsigned long last_step_time = 0;

//// 1. 适当降低一点门槛（防止软件I2C采样率低时漏掉波峰）
//#define STEP_HIGH_THRES   11.4f  // 从 11.8f 下调至 11.4f
//#define STEP_LOW_THRES    8.5f   // 从 8.0f 提高至 8.5f，更容易触发状态回退
//#define STEP_INTERVAL_MS  250

//static void mpu6050_process_step_peak(short *accel, unsigned long ts)
//{
//    float ax = accel[0] / 16384.0f * 9.81f;
//    float ay = accel[1] / 16384.0f * 9.81f;
//    float az = accel[2] / 16384.0f * 9.81f;
//    float raw_mag = sqrtf(ax*ax + ay*ay + az*az);

//    // 2. 调整滤波占比：加大当前原始值的权重（0.3 -> 0.5），降低对历史的依赖
//    // 这样哪怕采样率下降了，波峰依然能够敏锐地显现出来
//    acc_filter = acc_filter * 0.5f + raw_mag * 0.5f;

//    switch (state) {
//    case STEP_STATE_STATIC:
//        if (acc_filter > STEP_HIGH_THRES) {
//            state = STEP_STATE_MOVING; 
//        }
//        break;

//    case STEP_STATE_MOVING:
//        if (acc_filter < STEP_LOW_THRES) {
//            if ((ts - last_step_time) > STEP_INTERVAL_MS) {
//                step_count++;
//                last_step_time = ts;
//            }
//            state = STEP_STATE_STATIC; 
//        }
//        break;
//    }
//}

static void mpu6050_process_step(short *accel, long *quat, unsigned long ts)
{	
	
	
	
	/* Convert quaternion (q30) to floats */
	float q0 = quat[0] / 1073741824.0f;
	float q1 = quat[1] / 1073741824.0f;
	float q2 = quat[2] / 1073741824.0f;
	float q3 = quat[3] / 1073741824.0f;

	/* Accel in g units */
	float ax = accel[0] / 16384.0f;
	float ay = accel[1] / 16384.0f;
	float az = accel[2] / 16384.0f;

	/* Gravity vector in sensor frame (from quaternion) */
	float gx = 2.0f * (q1 * q3 - q0 * q2);
	float gy = 2.0f * (q2 * q3 + q0 * q1);
	float gz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

	/* Linear acceleration = measured - gravity (g → m/s²) */
	float lax = (ax - gx) * 9.81f;
	float lay = (ay - gy) * 9.81f;
	float laz = (az - gz) * 9.81f;

	/* dt in seconds */
	float dt;
	if (first_reading) {
		dt = 0.0f;
		first_reading = 0;
	} else {
		dt = (ts - last_ts) / 1000.0f;
		if (dt <= 0 || dt > 1.0f) dt = 0.2f;
	}
	last_ts = ts;

	float acc_mag = sqrtf(lax * lax + lay * lay + laz * laz);
	
//	printf("acc=%.2f state=%d\r\n", acc_mag, state);
	
	switch (state) {

	case STEP_STATE_STATIC:
		vel_x = vel_y = vel_z = 0;
		disp_x = disp_y = disp_z = 0;
		static_cnt = 0;

		/* Start moving when linear accel clearly exceeds baseline noise (~1.3) */
		if (acc_mag > 2.0f) {
			state = STEP_STATE_MOVING;
			vel_x = vel_y = vel_z = 0;
			disp_x = disp_y = disp_z = 0;
		}
		break;

	case STEP_STATE_MOVING:
		/* Returning to rest? threshold must be above stationary baseline */
		if (acc_mag < 1.8f) {
			static_cnt++;
			if (static_cnt >= 3) {
				/* Confirmed rest, discard unfinished sub-threshold displacement */
				state = STEP_STATE_STATIC;
				vel_x = vel_y = vel_z = 0;
				disp_x = disp_y = disp_z = 0;
				static_cnt = 0;
				break;
			}
		} else {
			static_cnt = 0;
		}

		/* Integrate acceleration -> velocity (mild decay) */
		vel_x = vel_x * 0.95f + lax * dt;
		vel_y = vel_y * 0.95f + lay * dt;
		vel_z = vel_z * 0.95f + laz * dt;

		/* Integrate velocity -> displacement */
		disp_x += vel_x * dt;
		disp_y += vel_y * dt;
		disp_z += vel_z * dt;

		/* Check step threshold DURING movement (don't wait for rest) */
		{
			float total_disp = sqrtf(disp_x * disp_x + disp_y * disp_y + disp_z * disp_z);
			float threshold_m = step_distance_cm / 100.0f;

			if (total_disp >= threshold_m) {
				step_count++;
//				printf("[STEP] #%lu detected | disp=%.1f cm | acc=%.1f\r\n",
//				       step_count, total_disp * 100.0f, acc_mag);
				/* Reset for next step, stay in MOVING */
				disp_x = disp_y = disp_z = 0;
				vel_x = vel_y = vel_z = 0;
			}
		}
		break;
	}
}

void mpu6050_step_detect_and_print(void)
{
	short gyro[3], accel[3], sensors;
	long quat[4];
	unsigned long timestamp;
	unsigned char more;

	int ret = dmp_read_fifo(gyro, accel, quat, &timestamp, &sensors, &more);
	if (ret != 0) {
        // FIFO 溢出或其他错误 → 恢复
//        printf("[DMP] FIFO error %d, resetting...\r\n", ret);
//        mpu_set_dmp_state(0);          // 关闭 DMP
//        dmp_load_motion_driver_firmware(); // 重新加载固件
//        mpu_set_dmp_state(1);          // 重新开启 DMP
//        return;
//		
//		mpu_reset_fifo();           // 只清空 FIFO（如果有这个函数）
//    // 或者仅重启 DMP，不重载固件
//    mpu_set_dmp_state(0);
//    mpu_set_dmp_state(1);
		return;
    }

	/* Convert quaternion (q30 fixed-point) to float */
	float q0 = quat[0] / 1073741824.0f;
	float q1 = quat[1] / 1073741824.0f;
	float q2 = quat[2] / 1073741824.0f;
	float q3 = quat[3] / 1073741824.0f;

	/* Euler angles */
	float roll  = atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * 57.29578f;
	float pitch = asinf(2.0f * (q0 * q2 - q3 * q1)) * 57.29578f;
	float yaw   = atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3)) * 57.29578f;
	
	
	
	/* Run step detection */
	mpu6050_process_step(accel, quat, timestamp);
	
//	mpu6050_process_step_peak(accel, timestamp);
		
	/* Print angles + step count */
//	printf("Ang: X=%6.1f Y=%6.1f Z=%6.1f | Steps: %lu\r\n", roll, pitch, yaw, step_count);
//	
//	printf("steps:%lu\r\n",step_count);
}

/* I2C read callback for inv_mpu library */
//int mpu6050_read(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* buf)
//{
//	if (addr == 0x68 || addr == 0x69)
//	{
//		addr <<= 1;
//	}

//	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&mpu_i2c, addr, reg,
//	                                             I2C_MEMADD_SIZE_8BIT, buf,
//	                                             len, 100);
//	return (status == HAL_OK) ? 0 : 1;
//}

///* I2C write callback for inv_mpu library */
//int mpu6050_write(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data)
//{
//	if (addr == 0x68 || addr == 0x69) {
//		addr <<= 1;
//	}

//	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&mpu_i2c, addr, reg,
//	                                              I2C_MEMADD_SIZE_8BIT, data,
//	                                              len, 100);
//	return (status == HAL_OK) ? 0 : 1;
//}

int mpu6050_read(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* buf)
{
    // 如果传入的是7位地址(0x68或0x69)，将其左移1位腾出读写位
    if (addr == 0x68 || addr == 0x69)
    {
        addr <<= 1;
    }

    // 1. 发送起始信号
    I2CStart_mp();

    // 2. 发送设备写地址 (准备写寄存器地址)
    I2CSendByte_mp(addr | 0x00); 
    if (I2CWaitAck_mp() == ERROR) 
    {
        I2CStop_mp();
        return 1; // 失败返回1
    }

    // 3. 发送目标寄存器地址
    I2CSendByte_mp(reg);
    if (I2CWaitAck_mp() == ERROR) 
    {
        I2CStop_mp();
        return 1;
    }

    // 4. 发送重复起始信号 (Restart) 准备读数据
    I2CStart_mp();

    // 5. 发送设备读地址
    I2CSendByte_mp(addr | 0x01); 
    if (I2CWaitAck_mp() == ERROR) 
    {
        I2CStop_mp();
        return 1;
    }

    // 6. 循环读取数据
    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = I2CReceiveByte_mp();
        
        // 如果是最后一个字节，发送 NACK，否则发送 ACK
        if (i == (len - 1))
        {
            I2CSendNotAck_mp(); 
        }
        else
        {
            I2CSendAck_mp();
        }
    }

    // 7. 发送停止信号
    I2CStop_mp();

    return 0; // 成功返回0
}

/* I2C write callback for inv_mpu library */
int mpu6050_write(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data)
{
    // 如果传入的是7位地址(0x68或0x69)，将其左移1位腾出读写位
    if (addr == 0x68 || addr == 0x69) {
        addr <<= 1;
    }

    // 1. 发送起始信号
    I2CStart_mp();

    // 2. 发送设备写地址
    I2CSendByte_mp(addr | 0x00);
    if (I2CWaitAck_mp() == ERROR)
    {
        I2CStop_mp();
        return 1; // 失败返回1
    }

    // 3. 发送目标寄存器地址
    I2CSendByte_mp(reg);
    if (I2CWaitAck_mp() == ERROR)
    {
        I2CStop_mp();
        return 1;
    }

    // 4. 循环发送数据
    for (uint8_t i = 0; i < len; i++)
    {
        I2CSendByte_mp(data[i]);
        if (I2CWaitAck_mp() == ERROR)
        {
            I2CStop_mp();
            return 1;
        }
    }

    // 5. 发送停止信号
    I2CStop_mp();

    return 0; // 成功返回0
}


void mpu6050_Init(void)
{
    int status; // 用于接收 mpu6050_read/write 的返回值 (0代表成功, 1代表失败)

    printf("\r\n========== MPU6050 DMP Init Start ==========\r\n\r\n");

    /* [1/8] Wake up MPU6050 */
    printf("[1/9] Waking up MPU6050 (PWR_MGMT_1 = 0x00)...\r\n");
    uint8_t wakeup = 0x00;
    
    // 使用软件I2C封装函数平替 HAL_I2C_Mem_Write
    // 传入的 MPU6050_I2C_ADDR 通常是 0x68
    status = mpu6050_write(MPU6050_I2C_ADDR, 0x6B, 1, &wakeup);
    if (status != 0) // 0 为成功，非0 (1) 为失败
    {
        printf("  [FAIL] I2C write PWR_MGMT_1 failed, status=%d\r\n", status);
        printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
        return;
    }
    printf("  [ OK ] PWR_MGMT_1 = 0x00, chip awake\r\n\r\n");
    HAL_Delay(50);

    /* [2/8] Read WHO_AM_I register */
    printf("[2/9] Reading WHO_AM_I register (expected: 0x68)...\r\n");
    
    // 使用软件I2C封装函数平替 HAL_I2C_Mem_Read
    status = mpu6050_read(MPU6050_I2C_ADDR, MPU6050_WHO_AM_I, 1, &check_val);
    HAL_Delay(50);
    if (status == 0 && check_val == 0x68)
    {
        printf("  [ OK ] WHO_AM_I = 0x%02X, MPU6050 confirmed\r\n\r\n", check_val);
    }
    else
    {
        printf("  [FAIL] WHO_AM_I read failed, status=%d, value=0x%02X\r\n", status, check_val);
        printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
        return;
    }

    /* [3/8] Full MPU6050 initialization via inv_mpu library */
    printf("[3/9] Running mpu_init() (reset + gyro FSR=2000dps, accel FSR=2g, LPF=42Hz)...\r\n");
    {
        struct int_param_s int_param = {0};
        int ret = mpu_init(&int_param);
        if (ret != 0)
        {
            printf("  [FAIL] mpu_init() returned %d\r\n", ret);
            printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
            return;
        }
        printf("  [ OK ] mpu_init() succeeded\r\n\r\n");
    }

    /* [4/9] Enable sensors (required before DMP firmware load) */
    printf("[4/9] Enabling sensors (GYRO + ACCEL)...\r\n");
    {
        int ret = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
        if (ret != 0)
        {
            printf("  [FAIL] mpu_set_sensors() returned %d\r\n", ret);
            printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
            return;
        }
        printf("  [ OK ] Sensors enabled: GYRO + ACCEL\r\n\r\n");
    }

    /* [5/9] Load DMP motion driver firmware */
    printf("[5/9] Loading DMP motion driver firmware (3062 bytes)...\r\n");
    {
        int ret = dmp_load_motion_driver_firmware();
        if (ret != 0)
        {
            printf("  [FAIL] dmp_load_motion_driver_firmware() returned %d\r\n", ret);
            printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
            return;
        }
        printf("  [ OK ] DMP firmware loaded successfully\r\n\r\n");
    }

    /* [6/9] Enable DMP features */
    printf("[6/9] Enabling DMP features: 6X_LP_QUAT + RAW_ACCEL + RAW_GYRO...\r\n");
    {
        unsigned short features = DMP_FEATURE_6X_LP_QUAT
                                | DMP_FEATURE_SEND_RAW_ACCEL
                                | DMP_FEATURE_SEND_RAW_GYRO;
        int ret = dmp_enable_feature(features);
        if (ret != 0)
        {
            printf("  [FAIL] dmp_enable_feature(0x%04X) returned %d\r\n", features, ret);
            printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
            return;
        }
        printf("  [ OK ] Features enabled: 0x%04X\r\n\r\n", features);
    }

    /* [7/9] Set DMP FIFO rate */
    printf("[7/9] Setting DMP FIFO output rate...\r\n");
    {
        unsigned short fifo_rate = 5;
        int ret = dmp_set_fifo_rate(fifo_rate);
        if (ret != 0)
        {
            printf("  [FAIL] dmp_set_fifo_rate(%d) returned %d\r\n", fifo_rate, ret);
            printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
            return;
        }
        printf("  [ OK ] DMP FIFO rate = %d Hz\r\n\r\n", fifo_rate);
    }

    /* [8/9] Enable DMP */
    printf("[8/9] Enabling DMP...\r\n");
    {
        int ret = mpu_set_dmp_state(1);
        if (ret != 0)
        {
            printf("  [FAIL] mpu_set_dmp_state(1) returned %d\r\n", ret);
            printf("========== MPU6050 DMP Init ABORTED ==========\r\n\r\n");
            return;
        }
        printf("  [ OK ] DMP enabled, FIFO streaming data\r\n\r\n");
    }

    /* [9/9] Init complete */
    printf("[9/9] MPU6050 DMP initialization complete!\r\n");
    printf("      Ready to read quaternion + accel + gyro data.\r\n");
    printf("\r\n========== MPU6050 DMP Init SUCCESS ==========\r\n\r\n");
}

/* GPIO interrupt callback */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
}