#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xd279b1af, "module_layout" },
	{ 0xfe990052, "gpio_free" },
	{ 0x973c40cc, "class_destroy" },
	{ 0xae4311e2, "device_destroy" },
	{ 0x49080b49, "device_create" },
	{ 0xa4247917, "cdev_del" },
	{ 0x2438fafb, "__class_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe4c9a6d0, "cdev_add" },
	{ 0xfccd2604, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xf4e5728f, "gpiod_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xb8057d40, "gpiod_direction_output_raw" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xf3dc4481, "gpiod_get_raw_value" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x67987eb2, "gpiod_to_irq" },
	{ 0xb7c24ed7, "gpio_to_desc" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xc5850110, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "4EF1ADFA339C5DE3AA419FF");
