// path: /usr/src/linux-5.4.282/drivers/char/agp/backend.c
// 335~343 line

static int __init agp_init(void)
{
	if (!agp_off) {
		printk(KERN_INFO "os2022202065_Linux agpgart interface v%d.%d\n",
			AGPGART_VERSION_MAJOR, AGPGART_VERSION_MINOR);
		printk(KERN_INFO "os2022202065_arg in agp_init(void)\n");
	}
	return 0;
}
