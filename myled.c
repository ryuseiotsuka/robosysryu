#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>

MODULE_AUTHOR("Ryusei Otsuka");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

void ton(void){
  gpio_base[7] = 1 << 25;
  msleep(100);
  gpio_base[10] = 1 << 25;
  msleep(100);
}
void tsu(void){
  gpio_base[7] = 1 << 25;
  msleep(300);
  gpio_base[10] = 1 << 25;
  msleep(100);
}

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
  char c;
  if(copy_from_user(&c,buf,sizeof(char)))
  return -EFAULT;

  if('A'<=c && c <= 'Z'){
    switch(c){
      case 'A':ton();tsu();break;
      case 'B':tsu();ton();ton();ton();break;
      case 'C':tsu();ton();tsu();ton();break;
      case 'D':tsu();ton();ton();break;
      case 'E':ton();break;
      case 'F':ton();ton();tsu();ton();break;
      case 'G':tsu();tsu();ton();break;
      case 'H':ton();ton();ton();ton();break;
      case 'I':ton();ton();break;
      case 'J':ton();tsu();tsu();tsu();break;
      case 'K':tsu();ton();tsu();break;
      case 'L':ton();tsu();ton();ton();break;
      case 'M':tsu();tsu();break;
      case 'N':tsu();ton();break;
      case 'O':tsu();tsu();tsu();break;
      case 'P':ton();tsu();tsu();ton();break;
      case 'Q':tsu();tsu();ton();tsu();break;
      case 'R':ton();tsu();ton();break;
      case 'S':ton();ton();ton();break;
      case 'T':tsu();break;
      case 'U':ton();ton();tsu();break;
      case 'V':ton();ton();ton();tsu();break;
      case 'W':ton();tsu();tsu();break;
      case 'X':tsu();ton();ton();tsu();break;
      case 'Y':tsu();ton();tsu();tsu();break;
      case 'Z':tsu();tsu();ton();ton();break;
    }
    msleep(200);
  }
  else if(c==' '){
    msleep(400);
  }

  printk(KERN_INFO "recive:%c\n",c);
  gpio_base[10] = 1 << 25;
  return 1;
}

static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t* pos)
{
  int size = 0;
  char sushi[] = {0xF0,0x9F,0x8D,0xA3,0x0A}; //寿司の絵文字のバイナリ
  if(copy_to_user(buf+size,(const char *)sushi, sizeof(sushi))){
    printk( KERN_INFO "sushi : copy_to_user failed\n" );
    return -EFAULT;
  }
  size += sizeof(sushi);
  return size;
}


static struct file_operations led_fops = {
  .owner = THIS_MODULE,
  .write = led_write,
  .read = sushi_read
};

static int __init init_mod(void)
{
  int retval;

  const u32 led = 25;
  const u32 index = led/10;
  const u32 shift = (led%10)*3;
  const u32 mask = ~(0x7 << shift);
  gpio_base = ioremap_nocache(0x3f200000,0xA0);
  gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);

  retval = alloc_chrdev_region(&dev,0,1,"myled");
  if (retval<0){
    printk(KERN_ERR "alloc region failed.\n");
    return retval;
  }
  printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));
  cdev_init(&cdv, &led_fops);
  retval = cdev_add(&cdv, dev, 1);
  if(retval < 0){
    printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
    return retval;
  }
  cls = class_create(THIS_MODULE,"myled");   
  if(IS_ERR(cls)){
    printk(KERN_ERR "class_create failed.");
    return PTR_ERR(cls);
  }
  device_create(cls,NULL,dev,NULL,"myled%d",MINOR(dev));
  return 0;
}

static void __exit cleanup_mod(void) 
{
  cdev_del(&cdv);
  device_destroy(cls,dev);
  class_destroy(cls);
  unregister_chrdev_region(dev, 1);
  printk(KERN_INFO "%s is unloaded. major:%d\n",__FILE__,MAJOR(dev));
}

module_init(init_mod);     
module_exit(cleanup_mod); 
