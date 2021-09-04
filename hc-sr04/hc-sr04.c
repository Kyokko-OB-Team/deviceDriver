#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gpio.h>

#include "../include/hc-sr04.h"

#define DEVICE_NAME "hc-sr04"
#define DEVICE_NAMED "hc-sr04%d"
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

typedef enum {
	_GPIO_SENSOR_ECHO,
	_GPIO_SENSOR_TRIGGER,
	_GPIO_TYPE_MAX,
} _GPIO_TYPE;

/* GPIO番号 */
static const int gpio_num[_GPIO_TYPE_MAX] = {
	14, /* エコー入力 : GPIO14(DSen_Echo) */
	15, /* トリガ出力 : GPIO15(DSen_Trigger) */
};

static char stored_gpio_value[_GPIO_TYPE_MAX];

static int drv_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int drv_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int drv_read(struct inode *inode, struct file *filp)
{
	return 0;
}

static int drv_write(struct inode *inode, struct file *filp)
{
	return 0;
}

static long drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	drv_rq_t rq;
	int rt_info = 0;
	char rt_char;
}

/* システムコールのハンドラテーブル */
static const struct file_operations driver_fops = {
	.open = drv_open,
	.release = drv_release,
	.read = drv_read,
	.write = drv_write,
};

/* ハードウェア初期化 */
static void drv_hardware_init(void)
{
	int i;
	int gpio;
	for (i = 0; i < _GPIO_TYPE_MAX; i++) {
		gpio = gpio_num[i];
		if (gpio_is_valid(gpio)) {
			if (gpio_request(gpio, DEVICE_NAME))
				printk(KERN_ERR "gpio_request(%d) error.\n", gpio);
			gpio_direction_input(gpio);
			if (gpio_get_value(gpio) == 0)
				stored_gpio_value[i] = 0;
			else
				stored_gpio_value[i] = 1;
		} else {
			printk(KERN_ERR "gpio_is_valid(%d) error.\n", gpio);
		}
	}
}

/* insmod時に呼ばれる関数 */
static int __init drv_init(void)
{
	int alloc_ret = 0;
	int cdev_err = 0;
	dev_t dev;

	printk(KERN_INFO "%s init. ver.%d.%d\n", DEVICE_NAME,
						 VERSION_MAJOR,
						 VERSION_MINOR);

	/* ハードウェア初期化 */
	drv_hardware_init();

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
	cdev_init(&device_cdev, &driver_fops);
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
static void drv_hardware_exit(void)
{
	int i;
	int gpio;
	for (i = 0; i < _GPIO_TYPE_MAX; i++) {
		gpio = gpio_num[i];
		if (gpio_is_valid(gpio))
			gpio_free(gpio);
		else
			printk(KERN_ERR "gpio_is_valid(%d) is not valid.\n", gpio);
	}
}

/* rmmod時に呼ばれる関数 */
static void __exit drv_exit(void)
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
	drv_hardware_exit();

	return;
}

module_init(drv_init);
module_exit(drv_exit);

MODULE_AUTHOR("gariyoshi0630@gmail.com");
MODULE_LICENSE("GPL");
