#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("author name");
MODULE_DESCRIPTION("A simple character device driver with interrupt.");

#define DEVICE_NAME "hello_chardev"
#define BUFFER_SIZE 1024

static int major_number; // 模块的主设备号
static char device_buffer[BUFFER_SIZE];
static int buffer_len = 0;
static struct timer_list my_timer;
static struct class *hello_class = NULL;
static struct device *hello_device = NULL;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write};

static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "hello_chardev: device opened.\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "hello_chardev: device closed.\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    // 将数据从内核缓冲区拷贝到用户缓冲区
    ssize_t bytes_read = 0;
    if (*offset < buffer_len)
    {
        if (len > buffer_len - *offset)
        {
            len = buffer_len - *offset;
        }
        bytes_read = len - copy_to_user(buffer, device_buffer + *offset, len);
        *offset += bytes_read;
    }
    printk(KERN_INFO "hello_chardev: read %ld bytes.\n", bytes_read);
    return bytes_read;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    // 将数据从用户缓冲区拷贝到内核缓冲区
    ssize_t bytes_written = 0;
    if (len > BUFFER_SIZE)
    {
        len = BUFFER_SIZE;
    }
    bytes_written = len - copy_from_user(device_buffer, buffer, len);
    buffer_len = bytes_written; // 更新缓冲区长度
    printk(KERN_INFO "hello_chardev: wrote %ld bytes.\n", bytes_written);
    return bytes_written;
}

static void my_timer_callback(struct timer_list *timer)
{
    printk(KERN_INFO "Timer expired! Current jiffies: %lu\n", jiffies);
    mod_timer(&my_timer, jiffies + HZ);
}

// 模块初始化函数
static int __init chardev_init(void)
{
    // 动态分配主设备号
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
    {
        printk(KERN_ALERT "hello_chardev failed to register a major number\n");
        return major_number;
    }

    hello_class = class_create("hello_class");
    if (IS_ERR(hello_class))
    {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create class.\n");
        return PTR_ERR(hello_class);
    }

    hello_device = device_create(hello_class, NULL, MKDEV(major_number, 0), NULL, "hello_dev");
    if (IS_ERR(hello_device))
    {
        class_destroy(hello_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create device.\n");
        return PTR_ERR(hello_device);
    }

    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + HZ);

    printk(KERN_INFO "hello_chardev: registered with major number %d\n", major_number);

    return 0;
}

// 模块退出函数
static void __exit chardev_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    device_destroy(hello_class, MKDEV(major_number, 0));
    class_destroy(hello_class);

    del_timer_sync(&my_timer);

    printk(KERN_INFO "hello_chardev: dev destroyed.\n");
}

module_init(chardev_init);
module_exit(chardev_exit);