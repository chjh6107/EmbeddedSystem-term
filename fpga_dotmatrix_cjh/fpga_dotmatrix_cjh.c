#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>

#define DRIVER_AUTHOR		"Hanback Electronics"
#define DRIVER_DESC		"dotmatrix program"

#define DOT_NAME 		"fpga_dotmatrix_cjh"
#define DOT_MODULE_VERSION 	"DOTMATRIX V1.0"
#define DOT_PHY_ADDR		0x05000000
#define DOT_ADDR_RANGE 		0x1000

#define NUMSIZE			4
#define DELAY			2

static int dot_usage = 0;
static unsigned long  dot_ioremap;
static unsigned short *dot_row_addr,*dot_col_addr;

void m_delay(int num){
	volatile int i,j;
	for(i=0 ; i<num ; i++)
		for(j = 0 ; j < 16384 ; j++);
}
	
static int fpga_dotmatrix_cjh_open(struct inode * inode, struct file * file){
	if(dot_usage != 0) return -EBUSY;

	dot_ioremap=(unsigned long)ioremap(DOT_PHY_ADDR,DOT_ADDR_RANGE);
	
	dot_row_addr =(unsigned short *)(dot_ioremap+0x40);
	dot_col_addr =(unsigned short *)(dot_ioremap+0x42);
	*dot_row_addr =0;
	*dot_col_addr =0;

	if(!check_mem_region(dot_ioremap, DOT_ADDR_RANGE)) {
		request_mem_region(dot_ioremap, DOT_ADDR_RANGE, DOT_NAME);
	}
	else	printk("driver: unable to register this!\n");

	dot_usage = 1;
	return 0;
}

static int fpga_dotmatrix_cjh_release(struct inode * inode, struct file * file){
	iounmap((unsigned long*)dot_ioremap);

	release_mem_region(dot_ioremap, DOT_ADDR_RANGE);
	dot_usage = 0;
	return 0;
}
/*not use
static ssize_t fpga_dotmatrix_cjh_read(struct file * file, char * buf, size_t length, loff_t * ofs){
	printk("fpga_dotmatrix_cjh_read, \n");
	
	return 0;
}
*/

int htoi(const char hexa){
	int ch = 0;
	if('0' <= hexa && hexa <= '9')
		ch = hexa - '0';
	if('A' <= hexa && hexa <= 'F')
		ch = hexa - 'A' + 10;
	if('a' <= hexa && hexa <= 'f')
		ch = hexa - 'a' + 10;

	return ch;
}

static ssize_t fpga_dotmatrix_cjh_write(struct file * file, const char * buf, size_t length, loff_t * ofs){
	int ret=0, i;
	char data[20];
	unsigned short result[10] = { 0 };
	unsigned int init=0x001;
	unsigned int n1, n2;

	ret = copy_from_user(data, buf, length);
	if(ret<0) return -1;

	for (i=0; i < 10; i++) {
		n1 = htoi( data[2*i] );
		n2 = htoi( data[2*i+1] );

		result[i] = n1*16+n2;

		*dot_row_addr = init << i;
		*dot_col_addr = 0x8000 | result[ i ];
		m_delay(3);
	}
	
	return length;
}
/*not use
static DEFINE_MUTEX(fpga_dotmatrix_cjh_mutex);
static long fpga_dotmatrix_cjh_ioctl(struct file * file, unsigned int cmd, unsigned long arg){
	printk("fpga_dotmatrix_cjh_ioctl, \n");
	
	switch(cmd){
		default:
			mutex_unlock(&fpga_dotmatrix_cjh_mutex);
			return ENOTTY;
	}
	
	mutex_unlock(&fpga_dotmatrix_cjh_mutex);
	return 0;
}
*/

static struct file_operations fpga_dotmatrix_cjh_fops = {
	.owner = THIS_MODULE,
	.open = fpga_dotmatrix_cjh_open,
	.release = fpga_dotmatrix_cjh_release,
//	.read = fpga_dotmatrix_cjh_read,
	.write = fpga_dotmatrix_cjh_write,
//	.unlocked_ioctl = fpga_dotmatrix_cjh_ioctl,
};

static struct miscdevice fpga_dotmatrix_cjh_driver = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fpga_dotmatrix_cjh",
	.fops = &fpga_dotmatrix_cjh_fops,
};

static int fpga_dotmatrix_cjh_init(void){
	printk("fpga_dotmatrix_cjh_init, \n");
	
	return misc_register(&fpga_dotmatrix_cjh_driver);
}

static void fpga_dotmatrix_cjh_exit(void){
	printk("fpga_dotmatrix_cjh_exit, \n");

	misc_deregister(&fpga_dotmatrix_cjh_driver);
	
}

module_init(fpga_dotmatrix_cjh_init);
module_exit(fpga_dotmatrix_cjh_exit);

MODULE_AUTHOR("Hanback");
MODULE_DESCRIPTION("fpga_dotmatrix_cjh driver test");
MODULE_LICENSE("Dual BSD/GPL");
