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
	{ 0xe338ec46, "module_layout" },
	{ 0xfe990052, "gpio_free" },
	{ 0x1926e08c, "class_destroy" },
	{ 0xcf2eb6cf, "device_create" },
	{ 0x2ae31514, "cdev_del" },
	{ 0x550754f6, "__class_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xdb4e9b54, "cdev_add" },
	{ 0xe91a98fb, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x8ceb1781, "gpiod_get_raw_value" },
	{ 0x5c5e06b4, "gpiod_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x5f754e5a, "memset" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x8114629f, "gpiod_to_irq" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x2e0988ec, "gpiod_direction_output_raw" },
	{ 0x3212d7f6, "gpio_to_desc" },
	{ 0xc5850110, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "82E8882F63877F79426BF27");
