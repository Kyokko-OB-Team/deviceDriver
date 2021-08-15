#include <linux/module.h>

#define DRIVER_NAME "hc-sr94"
#define VERSION_MINOR (0x01)
#define VERSION_MAJOR (0x00)

typedef enum {
  GPIO_SENSOR_ECHO,
  GPIO_SENSOR_TRIGGER,
  GPIO_TYPE_MAX,
} GPIO_TYPE;

static const int gpio_num[GPIO_TYPE_MAX] = {
  14, // GPIO14
  15, // GPIO15
};

static int driver_hardware_init(void) {
  printk(KERN_INFO "HC-SR04 module initialization.\n");
  return 0;
}

static void driver_hardware_exit(void) {
  printk(KERN_INFO "HC-SR04 module exit.\n");
}

module_init(driver_hardware_init);
module_exit(driver_hardware_exit);


MODULE_AUTHOR("gariyoshi0630@gmail.com");
MODULE_LICENSE("GPL");
