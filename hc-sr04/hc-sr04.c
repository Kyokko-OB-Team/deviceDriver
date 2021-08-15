#include <linux/module.h>

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
