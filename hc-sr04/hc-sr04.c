#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>

#include "../include/hc-sr04.h"

#define DEVICE_NAME "hc-sr04"
#define DEVICE_NAMED "hc-sr04%d"
#define DEVICE_NAME_IRQ "hc-sr04_irq"
#define VERSION_MINOR (0x01)
#define VERSION_MAJOR (0x00)

#define GPIO_LOW (0)
#define GPIO_HIGH (1)

/* このデバイスドライバで使うマイナー番号の開始番号と個数(=デバイス数) */
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 1; /* マイナー番号は0 */

/* このデバイスドライバのメジャー番号(動的に決める) */
static unsigned int device_major;

/* キャラクタデバイスのオブジェクト */
static struct cdev device_cdev;

/* デバイスドライバのクラスオブジェクト */
static struct class *device_class = NULL;

/* GPIO割当 */
typedef enum {
	_GPIO_NUM_SENSOR_ECHO,
	_GPIO_NUM_SENSOR_TRIGGER,
	_GPIO_NUM_MAX,
} _GPIO_NUM;

/* GPIO設定種別 */
typedef enum {
	_GPIO_IO_INPUT,
	_GPIO_IO_OUTPUT_LOW,
	_GPIO_IO_OUTPUT_HIGH,
	_GPIO_IO_MAX,
} _GPIO_IO_TYPE;

/* GPIO番号 */
static const int gpio_num[_GPIO_NUM_MAX] = {
	14, /* エコー入力 : GPIO14(DSen_Echo) */
	15, /* トリガ出力 : GPIO15(DSen_Trigger) */
};

/* 初期値 */
static const int gpio_init_value[] = {
	_GPIO_IO_INPUT,
	_GPIO_IO_OUTPUT_LOW,
};

/* 現在の値 */
static char stored_gpio_value[_GPIO_NUM_MAX];

/* 割り込み番号 */
int irq = 0;
/* 立ち上がりのタイムスタンプ */
static u64 rising_timestamp = 0;
/* 立ち下がりのタイムスタンプ */
static u64 falling_timestamp = 0;

/* param
 * id : GPIO ID
 * value : Outputレベル
 * return :
 * 	0はlow、1はhighを設定出来ました。
 * 	エラーの場合は負の値を返します。
 */
static char _set_output (int id, char value) {
	char output = 0;

	if ((id < 0) || (_GPIO_NUM_MAX <= id)) {
		printk(KERN_ERR "Set illegal ID error.(ID:%d)\n", id);
		return -EINVAL;
	}
	switch (gpio_init_value[id]) {
	case _GPIO_IO_OUTPUT_LOW:
	case _GPIO_IO_OUTPUT_HIGH:
		break;

	case _GPIO_IO_INPUT:
	default:
		printk(KERN_ERR "Set illegal ID error.(ID:%d)\n", id);
		return -EINVAL;
	}
	if (value == 0)
		output = 0;
	else
		output = 1;
	gpio_direction_output(gpio_num[id], output);
	stored_gpio_value[id] = output;

	return output;
}

/* param
 * id : GPIO ID
 * return :
 * 	0はlow、1はhighを取得しました。
 * 	エラーの場合は負の値を返します。
 */
static char _get_input (int id) {
	char input = 0;

	if ((id < 0) || (_GPIO_NUM_MAX <= id)) {
		printk(KERN_ERR "Set illegal ID error.(ID:%d)\n", id);
		return -EINVAL;
	}
	switch (gpio_init_value[id]) {
	case _GPIO_IO_INPUT:
		break;

	case _GPIO_IO_OUTPUT_LOW:
	case _GPIO_IO_OUTPUT_HIGH:
	default:
		printk(KERN_ERR "Set illegal ID error.(ID:%d)\n", id);
		return -EINVAL;
	}
	if (gpio_get_value(gpio_num[id]) == GPIO_LOW) {
		input = GPIO_LOW;
		stored_gpio_value[id] = GPIO_LOW;
	} else {
		input = GPIO_HIGH;
		stored_gpio_value[id] = GPIO_HIGH;
	}
	return input;
}

static irqreturn_t irq_handler (int irq, void *arg) {
	char level_flag; /* 割り込みのエッジ */

	level_flag = _get_input(gpio_num[_GPIO_NUM_SENSOR_ECHO]);
	switch (level_flag) {
	case GPIO_HIGH:
		rising_timestamp = ktime_get_ns();
		stored_gpio_value[_GPIO_NUM_SENSOR_ECHO] = GPIO_HIGH;
		printk(KERN_INFO "irq_handler GPIO rising edge detected.\n");
		break;
	case GPIO_LOW:
		falling_timestamp = ktime_get_ns();
		stored_gpio_value[_GPIO_NUM_SENSOR_ECHO] = GPIO_LOW;
		printk(KERN_INFO "irq_handler GPIO falling edge detected.\n");
		break;
	default:
		printk(KERN_ERR "Get gpio input error.\n");
		break;
	}
	return IRQ_HANDLED;
}

/* param
 * return :
 *	正常に処理が出来た場合は0を返します。
 *	エラーの場合は負の値を返します。
 */
static char trigger_output (void)
{
	char out_result = 0;
	/* トリガパルス出力時間 */
	unsigned long trigger_pulse_us = 20; /* 仕様10us以上のため誤差を考慮して20us */

	if ((out_result = _set_output(_GPIO_NUM_SENSOR_TRIGGER, 1)) < 0)
		return out_result;
	udelay(trigger_pulse_us);
	if ((out_result = _set_output(_GPIO_NUM_SENSOR_TRIGGER, 0)) < 0)
		return out_result;
	return 0;
}

static int gpiodrv_open(struct inode *inode, struct file *filp)
{
	irq = gpio_to_irq(gpio_num[_GPIO_NUM_SENSOR_ECHO]);
	if (irq < 0)
	{
		printk(KERN_ERR "gpio_to_irq error.\n");
		return -EFAULT;
	}
	if (request_irq(irq,
			(void*)irq_handler,
			IRQF_SHARED | IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			DEVICE_NAME_IRQ,
			DEVICE_NAME_IRQ) < 0)
	{
		printk(KERN_ERR "request irq error.\n");
		return -EFAULT;
	}
	printk(KERN_INFO "request irq success.\n");
	return 0;
}

static int gpiodrv_release(struct inode *inode, struct file *filp)
{
	free_irq(irq, DEVICE_NAME_IRQ);
	return 0;
}

static int gpiodrv_read(struct file *filp, char *user_buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "read() is not implement.\n");
	return 0;
}

static int gpiodrv_write(struct file *filp, const char *user_buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "write() is not implement.\n");
	return 0;
}

static long gpiodrv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	drv_rq_t rq; /* ユーザ空間とのやり取りデータ */
	unsigned int distance = 0; /* 距離データ(cm) */

	rq.status = false;
	rq.value = 0;

	switch (cmd) {
	/* 距離測定開始 */
	case GPIO_HCSR04_EXEC_MEASURE_DISTANCE:
		if (trigger_output())
			return -EFAULT;
		rq.status = true;
		printk(KERN_INFO "exec measure distance.\n");
		if ((copy_to_user((void __user *)arg, &rq, sizeof(rq))))
			return -EFAULT;
		break;

	/* 測定結果取得 */
	case GPIO_HCSR04_GET_DISTANCE:
		if (rising_timestamp == 0 ||
		    falling_timestamp == 0 ||
		    rising_timestamp < falling_timestamp) {
			printk(KERN_INFO "Illegal edge timestamp.\n");
		} else {
			distance = (falling_timestamp - rising_timestamp) / 58;
			rq.status = true;
			rq.value = distance;
		}
		if ((copy_to_user((void __user *)arg, &rq, sizeof(rq))))
			return -EFAULT;
		break;
	default:
		printk(KERN_ERR "Illegal type error.\n");
		return -EFAULT;
	}
	return 0;
}

/* システムコールのハンドラテーブル */
static const struct file_operations gpiodrv_fops = {
	.open = gpiodrv_open,
	.release = gpiodrv_release,
	.read = gpiodrv_read,
	.write = gpiodrv_write,
	.unlocked_ioctl = gpiodrv_ioctl,
	.compat_ioctl = gpiodrv_ioctl,
};

/* ハードウェア初期化 */
static void gpiodrv_hardware_init(void)
{
	int i;
	int gpio;
	for (i = 0; i < _GPIO_NUM_MAX; i++) {
		gpio = gpio_num[i];
		if (gpio_is_valid(gpio)) {
			if (gpio_request(gpio, DEVICE_NAME))
				printk(KERN_ERR "gpio_request(%d) error.\n", gpio);

			switch (gpio_init_value[i]) {
			case _GPIO_IO_INPUT:
				gpio_direction_input(gpio);
				if (gpio_get_value(gpio) == 0)
					stored_gpio_value[i] = GPIO_LOW;
				else
					stored_gpio_value[i] = GPIO_HIGH;
				break;

			case _GPIO_IO_OUTPUT_LOW:
				gpio_direction_output(gpio, 0);
				stored_gpio_value[i] = GPIO_LOW;
				break;

			case _GPIO_IO_OUTPUT_HIGH:
				gpio_direction_output(gpio, 1);
				stored_gpio_value[i] = GPIO_HIGH;
				break;

			default:
				printk(KERN_ERR "Illegal initial value error.\n");
				break;
			}
		} else {
			printk(KERN_ERR "gpio_is_valid(%d) error.\n", gpio);
		}
	}
}

/* insmod時に呼ばれる関数 */
static int __init gpiodrv_init(void)
{
	int alloc_ret = 0;
	int cdev_err = 0;
	dev_t dev;

	printk(KERN_INFO "%s init. ver.%d.%d.\n", DEVICE_NAME,
						 VERSION_MAJOR,
						 VERSION_MINOR);

	/* ハードウェア初期化 */
	gpiodrv_hardware_init();

	/* 空いているメジャー番号を確保 */
	alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DEVICE_NAME);
	if (alloc_ret != 0) {
		printk(KERN_ERR "alloc_cdrdev_region = %d.\n", alloc_ret);
		return -1;
	}

	/* 取得したメジャー番号 + マイナー番号(dev)から、
	 * メジャー番号を取得して保持する */
	device_major = MAJOR(dev);
	dev = MKDEV(device_major, MINOR_BASE);

	/* cdev構造体の初期化とシステムコールハンドラテーブルの登録 */
	cdev_init(&device_cdev, &gpiodrv_fops);
	device_cdev.owner = THIS_MODULE;

	/* このデバイスドライバをカーネルに登録 */
	cdev_err = cdev_add(&device_cdev, dev, MINOR_NUM);
	if (cdev_err != 0) {
		printk(KERN_ERR "cdev_add = %d.\n", cdev_err);
		unregister_chrdev_region(dev, MINOR_NUM);
		return -1;
	}

	/* デバイスのクラス登録(/sys/class/DEVICE_NAME の作成) */
	device_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(device_class)) {
		printk(KERN_ERR "class_create error.\n");
		cdev_del(&device_cdev);
		unregister_chrdev_region(dev, MINOR_NUM);
		return -1;
	}

	/* デバイスノード作成 /sys/class/DEVICE_NAME/DEVICE_NAME */
	device_create(device_class, NULL, dev, NULL, DEVICE_NAMED, MINOR_BASE);

	return 0;
}

/* ハードウェア終了 */
static void gpiodrv_hardware_exit(void)
{
	int i;
	int gpio;
	for (i = 0; i < _GPIO_NUM_MAX; i++) {
		gpio = gpio_num[i];
		if (gpio_is_valid(gpio))
			gpio_free(gpio);
		else
			printk(KERN_ERR "gpio_is_valid(%d) is not valid.\n", gpio);
	}
}

/* rmmod時に呼ばれる関数 */
static void __exit gpiodrv_exit(void)
{
	dev_t dev;

	printk(KERN_INFO "%s module exit.\n", DEVICE_NAME);

	dev = MKDEV(device_major, MINOR_BASE);

	/* /sys/class/DEVICE_NAME/ を削除 */
	class_destroy(device_class);

	/* このデバイスドライバをカーネルから除去する */
	cdev_del(&device_cdev);

	/* このデバイスドライバで使用していたメジャー番号の登録解除 */
	unregister_chrdev_region(dev, MINOR_NUM);

	/* ハードウェア終了処理 */
	gpiodrv_hardware_exit();

	return;
}

module_init(gpiodrv_init);
module_exit(gpiodrv_exit);

MODULE_AUTHOR("gariyoshi0630@gmail.com");
MODULE_LICENSE("GPL");
