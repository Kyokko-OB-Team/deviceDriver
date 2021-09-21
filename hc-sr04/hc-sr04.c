#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#include "../include/hc-sr04.h"

#define DEVICE_NAME "hc-sr04"
#define DEVICE_NAMED "hc-sr04%d"
#define DEVICE_NAME_IRQ "hc-sr04_irq"
#define VERSION_MINOR (0x01)
#define VERSION_MAJOR (0x00)

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

/* 割り込み許可フラグ */
bool irq_permit;

/* 割り込み立ち上がり/立ち下がり判定フラグ */
unsigned long irq_fall_or_rise;

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
	if (gpio_get_value(gpio_num[id]) == 0) {
		input = 0;
		stored_gpio_value[id] = 0;
	} else {
		input = 1;
		stored_gpio_value[id] = 1;
	}
	return input;
}

static irqreturn_t irq_handler (int irq, void *arg) {
	unsigned long flags;
}

static void trigger_output (void)
{
	/* トリガパルス出力時間 */
	unsigned long trigger_pulse_us = 20; /* 仕様10us以上のため誤差を考慮して20us */

	_set_output(_GPIO_NUM_SENSOR_TRIGGER, 1);
	udelay(trigger_pulse_us);
	_set_output(_GPIO_NUM_SENSOR_TRIGGER, 0);
}

static int gpiodrv_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int gpiodrv_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int gpiodrv_read(struct file *filp, char *user_buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "read() is not implement\n");
	return 0;
}

static int gpiodrv_write(struct file *filp, const char *user_buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "write() is not implement\n");
	return 0;
}

static long gpiodrv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	drv_rq_t rq;
	int rt_info = 0;
	char rt_char;
	int irq;

	switch (cmd) {
	/* 距離測定開始 */
	case GPIO_HCSR04_EXEC_MEASURE_DISTANCE:
		irq = gpio_to_irq(gpio_num[_GPIO_NUM_SENSOR_ECHO]);
		if (request_irq(irq,
				(void*)irq_handler,
				IRQF_SHARED | IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				DEVICE_NAME_IRQ,
				DEVICE_NAME_IRQ) < 0)
		{
			printk(KERN_ERR "request irq error.\n");
			return -EFAULT;
		}
	}
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
					stored_gpio_value[i] = 0;
				else
					stored_gpio_value[i] = 1;
				break;

			case _GPIO_IO_OUTPUT_LOW:
				gpio_direction_output(gpio, 0);
				stored_gpio_value[i] = 0;
				break;

			case _GPIO_IO_OUTPUT_HIGH:
				gpio_direction_output(gpio, 1);
				stored_gpio_value[i] = 0;
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

	printk(KERN_INFO "%s init. ver.%d.%d\n", DEVICE_NAME,
						 VERSION_MAJOR,
						 VERSION_MINOR);

	/* ハードウェア初期化 */
	gpiodrv_hardware_init();

	/* 空いているメジャー番号を確保 */
	alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DEVICE_NAME);
	if (alloc_ret != 0) {
		printk(KERN_ERR "alloc_cdrdev_region = %d\n", alloc_ret);
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
		printk(KERN_ERR "cdev_add = %d\n", cdev_err);
		unregister_chrdev_region(dev, MINOR_NUM);
		return -1;
	}

	/* デバイスのクラス登録(/sys/class/DEVICE_NAME の作成) */
	device_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(device_class)) {
		printk(KERN_ERR "class_create err\n");
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
