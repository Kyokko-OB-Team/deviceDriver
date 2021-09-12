#ifndef HC_SR04_H
#define HC_SR04_H

#include <linux/ioctl.h>

typedef struct drv_rq {
	unsigned int value;
	unsigned int status;
} drv_rq_t;

#define GPIO_HCSR04_IOC_TYPE 'S'

/* 距離計測 */
#define GPIO_HCSR04_EXEC_MEASURE_DISTANCE _IOR(GPIO_HCSR04_IOC_TYPE, 1, drv_rq_t)

#endif // HC_SR04_H