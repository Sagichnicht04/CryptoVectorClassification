#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <linux/printk.h>
#include <crypto/drbg.h>
 
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Herold");

static int get_random_numbers(u8 *buf, unsigned int len);

static int hex_dump(u8 *buf, unsigned int len);

static int __init hello_init(void)
{
	u8 *rng_buf = NULL;
	unsigned int buffer_length = 512;

	rng_buf = kzalloc(buffer_length * sizeof(u8), GFP_KERNEL);
	printk(KERN_ALERT "hello, world - this is the kernel speaking!\n");
	
	rng_buf = "x\00x\01x\02x\03x\04x\05x\06x\07"
		  "x\08x\09x\0ax\0bx\0cx\0dx\0ex\0f";
	hex_dump(rng_buf, 16);

	//get_random_numbers(rng_buf, buffer_length);
	//hex_dump(rng_buf, buffer_length);
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_ALERT "short is the life of a kernel module!\n");
}

module_init(hello_init);
module_exit(hello_exit);

static int get_random_numbers(u8 *buf, unsigned int len)
{
	struct crypto_rng *rng = NULL;
	char *drbg = "drbg_nopr_sha256"; /* Hash DRBG with SHA-256, no PR */
	int ret;

	if (!buf || !len) {
		pr_debug("No output buffer provided\n");
		return -EINVAL;
	}

	rng = crypto_alloc_rng(drbg, 0, 0);
	if (IS_ERR(rng)) {
		pr_debug("could not allocate RNG handle for %s\n", drbg);
		return PTR_ERR(rng);
	}

	ret = crypto_rng_get_bytes(rng, buf, len);
	if (ret < 0)
		pr_debug("generation of random numbers failed\n");
	else if (ret == 0)
		pr_debug("RNG returned no data");
	else
		pr_debug("RNG returned %d bytes of data\n", ret);

	out:
	crypto_free_rng(rng);
	return ret;
}

static int hex_dump(u8 *buf, unsigned int len)
{
	int i;
	int loop;
	loop = len/8;
	pr_err("Harold in KERNEL, START DUMP!");
	for (i = 0; i < loop * 8; i+=8) {
		pr_err("%+02x %+02x %+02x %+02x %+02x %+02x %+02x %+02x\n ",
			*(buf+i), *(buf+i+1), *(buf+i+2), *(buf+i+3), *(buf+i+4), *(buf+i+5), *(buf+i+6), *(buf+i+7)
		);
	}

	for (i = loop*8; i < len; i++) {
		pr_err("%+02x ", *(buf+i));
	}
	pr_err("\nHarold reach end\n");
	return 0;
}
