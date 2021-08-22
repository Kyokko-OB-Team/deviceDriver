#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/fs.h>

#define DEVICE_NAME "hc-sr04"
#define VERSION_MINOR (0x01)
#define VERSION_MAJOR (0x00)

// このデバイスドライバで使うマイナー番号の開始番号と個数(=デバイス数)
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 1; // マイナー番号は0

// このデバイスドライバのメジャー番号(動的に決める)
static unsigned int device_major;

// キャラクタデバイスのオブジェクト
static struct cdev device_cdev;

// デバイスドライバのクラスオブジェクト
static struct class *device_class = NULL;

typedef enum {
  _GPIO_SENSOR_ECHO,
  _GPIO_SENSOR_TRIGGER,
  _GPIO_TYPE_MAX,
} _GPIO_TYPE;

// GPIO番号
static const int gpio_num[_GPIO_TYPE_MAX] = {
  14, // エコー入力 : GPIO14(DSen_Echo)
  15, // トリガ出力 : GPIO15(DSen_Trigger)
};

static const struct file_operations driver_fops = {
  .open       = NULL,
  .release    = NULL,
};

static void drv_hardware_init(void)
{
}

// insmod時に呼ばれる関数
static int drv_init(void)
{
  int alloc_ret = 0;
  int cdev_err = 0;
  dev_t dev;

  printk(KERN_INFO "%s init. ver.%d.%d\n", DEVICE_NAME, VERSION_MAJOR, VERSION_MINOR);

  // ハードウェア初期化
  drv_hardware_init();

  // 空いているメジャー番号を確保
  alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DEVICE_NAME);
  if (alloc_ret != 0) {
    printk(KERN_ERR "alloc_cdrdev_region = %d\n", alloc_ret);
    return -1;
  }

  // 取得したメジャー番号 + マイナー番号(dev)から、メジャー番号を取得して保持する
  device_major = MAJOR(dev);
  dev = MKDEV(device_major, MINOR_BASE);

  // cdev構造体の初期化とシステムコールハンドラテーブルの登録
  cdev_init(&device_cdev, &driver_fops);
  device_cdev.owner = THIS_MODULE;

  // このデバイスドライバをカーネルに登録
  cdev_err = cdev_add(&device_cdev, dev, MINOR_NUM);
  if (cdev_err != 0) {
    printk(KERN_ERR "cdev_add = %d\n", cdev_err);
    unregister_chrdev_region(dev, MINOR_NUM);
    return -1;
  }

  // デバイスのクラス登録(/sys/class/DEVICE_NAME の作成)
  device_class = class_create(THIS_MODULE, DEVICE_NAME);
  if (IS_ERR(device_class)) {
    printk(KERN_ERR "class_create err\n");
    cdev_del(&device_cdev);
    unregister_chrdev_region(dev, MINOR_NUM);
    return -1;
  }
  return 0;
}

// rmmod時に呼ばれる関数
static void drv_exit(void)
{
  printk(KERN_INFO "HC-SR04 module exit.\n");
}

module_init(drv_init);
module_exit(drv_exit);

MODULE_AUTHOR("gariyoshi0630@gmail.com");
MODULE_LICENSE("GPL");
