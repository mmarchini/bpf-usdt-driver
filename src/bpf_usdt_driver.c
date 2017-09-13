#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include "bpf_usdt_driver.h"

MODULE_LICENSE("GPL");


int dev_major = 0;
int dev_minor = 0;
int dev_count = 1;

struct cdev cdev_;

BpfUsdtProbe probesList[MAX_PROBES];

int bpf_usdt_open(struct inode *inode, struct file *filp);
int bpf_usdt_release(struct inode *inode, struct file *filp);
static long bpf_usdt_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations bpf_usdt_proc_ops = {
  .owner   = THIS_MODULE,
	.open    = bpf_usdt_open,
  .release = bpf_usdt_release,
  .unlocked_ioctl = bpf_usdt_ioctl
};

static long bpf_usdt_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  // printk(KERN_ALERT "god have mercy\n");
  return 0;
}


int bpf_usdt_open(struct inode *inode, struct file *filp) {
  return 0;
}

int bpf_usdt_release(struct inode *inode, struct file *filp) {
  return 0;
}

static int __init bpf_usdt_init(void) {
  int result;
  dev_t dev=0;

  result = alloc_chrdev_region(&dev, dev_minor, dev_count, "bpf_usdt");
  if(result != 0) {
   printk(KERN_ALERT "Deu ruim\n");
   return result;
  }
  dev_major = MAJOR(dev);

  cdev_init(&cdev_, &bpf_usdt_proc_ops);
  cdev_.owner = THIS_MODULE;
  cdev_.ops = &bpf_usdt_proc_ops;
  result = cdev_add(&cdev_, dev, 1);
  if(result != 0) {
   printk(KERN_ALERT "Deu ruim 2\n");
   return result;
  }

  printk(KERN_ALERT "Hello, USDT: %d %d\n", dev_major, dev_minor);
return 0;
}

static void bpf_usdt_exit(void) {
  unregister_chrdev_region(MKDEV(dev_major, dev_minor), dev_count);

  printk(KERN_ALERT "Goodbye, eBPF: %d %d\n", dev_major, dev_minor);
}

module_init(bpf_usdt_init);
module_exit(bpf_usdt_exit);
