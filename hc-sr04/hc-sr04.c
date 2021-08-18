#include <linux/module.h>

#define DEVICE_NAME "hc-sr94"
#define VERSION_MINOR (0x01)
#define VERSION_MAJOR (0x00)

// このデバイスドライバで使うマイナー番号の開始番号と個数(=デバイス数)
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 1; // マイナー番号は0

// このデバイスドライバのメジャー番号(動的に決める)
//static unsigned int device_major;

// キャラクタデバイスのオブジェクト
//static struct cdev device_cdev;

// デバイスドライバのクラスオブジェクト
//static struct class *device_class = NULL;

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

// 初期化
static int driver_hardware_init(void) {
  int alloc_ret = 0;
  //int cdev_err = 0;
  dev_t dev;

  printk(KERN_INFO "%s init. ver.%d.%d\n", DEVICE_NAME, VERSION_MAJOR, VERSION_MINOR);

  alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DEVICE_NAME);
  if (alloc_ret != 0) {
    printk(KERN_ERR "alloc_cdrdev_region = %d\n", alloc_ret);
    return -1;
  }

  return 0;
}

// 終了
static void driver_hardware_exit(void) {
  printk(KERN_INFO "HC-SR04 module exit.\n");
}

module_init(driver_hardware_init);
module_exit(driver_hardware_exit);

MODULE_AUTHOR("gariyoshi0630@gmail.com");
MODULE_LICENSE("GPL");
