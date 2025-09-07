#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/poll.h>

MODULE_AUTHOR("shxuuer");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple FIFO char device");
MODULE_VERSION("0.1");

#define DEVICE_NAME "fifo"
#define BUFFER_SIZE 1024

static dev_t fifo_devno = 0;
static struct class *fifo_class;

static struct cdev fifo_cdev;
static struct mutex fifo_mutex;
static wait_queue_head_t read_queue;
static wait_queue_head_t write_queue;
static char *fifo_buffer;
static int length = 0;
static struct fasync_struct *fifo_fasync_struct;

static int __init fifo_init(void);
static void __exit fifo_exit(void);
static int fifo_open(struct inode *, struct file *);
static int fifo_release(struct inode *, struct file *);
static ssize_t fifo_read(struct file *filep, char __user *buf, size_t count, loff_t *ppos);
static ssize_t fifo_write(struct file *filep, const char __user *buf, size_t count, loff_t *ppos);
__poll_t fifo_poll(struct file *filep, struct poll_table_struct *pt);
int fifo_fasync(int fd, struct file *filep, int mode);

static struct file_operations fifo_fops = {
    .owner = THIS_MODULE,
    .open = fifo_open,
    .release = fifo_release,
    .read = fifo_read,
    .write = fifo_write,
    .poll = fifo_poll,
    .fasync = fifo_fasync,
};

module_init(fifo_init);
module_exit(fifo_exit);

static int __init fifo_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&fifo_devno, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "fifo: failed to allocate char device region\n");
        goto alloc_chrdev_region_fail;
    }

    cdev_init(&fifo_cdev, &fifo_fops);
    ret = cdev_add(&fifo_cdev, fifo_devno, 1);
    if (ret < 0)
    {
        printk(KERN_ERR "fifo: failed to add cdev\n");
        goto cdev_add_fail;
    }

    fifo_class = class_create(DEVICE_NAME);
    if (IS_ERR(fifo_class))
    {
        printk(KERN_ERR "fifo: failed to create class\n");
        ret = PTR_ERR(fifo_class);
        goto class_create_fail;
    }

    device_create(fifo_class, NULL, fifo_devno, NULL, DEVICE_NAME);

    init_waitqueue_head(&read_queue);
    init_waitqueue_head(&write_queue);
    mutex_init(&fifo_mutex);

    fifo_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

    return 0;

class_create_fail:
    cdev_del(&fifo_cdev);
cdev_add_fail:
    unregister_chrdev_region(fifo_devno, 1);
alloc_chrdev_region_fail:
    return ret;
}

static void __exit fifo_exit(void)
{
    device_destroy(fifo_class, fifo_devno);
    class_destroy(fifo_class);
    cdev_del(&fifo_cdev);
    kfree(fifo_buffer);
    unregister_chrdev_region(fifo_devno, 1);
    printk(KERN_INFO "fifo: module unloaded\n");
}

static int fifo_open(struct inode *inode, struct file *filep)
{
    filep->private_data = &fifo_cdev;
    return 0;
}

static int fifo_release(struct inode *inode, struct file *filep)
{
    fifo_fasync(-1, filep, 0);
    return 0;
}

static ssize_t fifo_read(struct file *filep, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    int size = count;
    DECLARE_WAITQUEUE(wait, current);

    if (size <= 0)
        return 0;

    mutex_lock(&fifo_mutex);
    add_wait_queue(&read_queue, &wait);

    while (length == 0)
    {
        if (filep->f_flags & O_NONBLOCK)
        {
            ret = -EAGAIN;
            goto out_with_unlock;
        }
        set_current_state(TASK_INTERRUPTIBLE);
        mutex_unlock(&fifo_mutex);
        schedule();
        // 被信号中断或被wait queue唤醒
        if (signal_pending(current))
        {
            // 被信号唤醒
            ret = -ERESTARTSYS;
            goto out;
        }
        mutex_lock(&fifo_mutex);
    }
    if (size > length)
        size = length;
    if (copy_to_user(buf, fifo_buffer, size))
    {
        ret = -EFAULT;
        goto out_with_unlock;
    }
    length -= size;
    if (length > 0)
        memmove(fifo_buffer, fifo_buffer + size, length);
    ret = size;
    wake_up_interruptible(&write_queue);

out_with_unlock:
    mutex_unlock(&fifo_mutex);
out:
    finish_wait(&read_queue, &wait); // 等价于修改running状态并从等待队列移除
    return ret;
}

static ssize_t fifo_write(struct file *filep, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    int size = count;
    DECLARE_WAITQUEUE(wait, current);

    if (size <= 0)
        return 0;
    if (size > BUFFER_SIZE)
        size = BUFFER_SIZE;

    mutex_lock(&fifo_mutex);
    add_wait_queue(&write_queue, &wait);

    while (length == BUFFER_SIZE)
    {
        if (filep->f_flags & O_NONBLOCK)
        {
            ret = -EAGAIN;
            goto out_with_unlock;
        }
        set_current_state(TASK_INTERRUPTIBLE);
        mutex_unlock(&fifo_mutex);
        schedule();
        // 被信号中断或被wait queue唤醒
        if (signal_pending(current))
        {
            ret = -ERESTARTSYS;
            goto out;
        }
        mutex_lock(&fifo_mutex);
    }
    if (size > BUFFER_SIZE - length)
        size = BUFFER_SIZE - length;
    if (copy_from_user(fifo_buffer + length, buf, size))
    {
        ret = -EFAULT;
        goto out_with_unlock;
    }
    length += size;
    ret = size;
    wake_up_interruptible(&read_queue);

    if (fifo_fasync_struct)
        kill_fasync(&fifo_fasync_struct, SIGIO, POLL_IN);

out_with_unlock:
    mutex_unlock(&fifo_mutex);
out:
    finish_wait(&write_queue, &wait);
    return ret;
}

__poll_t fifo_poll(struct file *filep, struct poll_table_struct *pt)
{
    __poll_t mask = 0;

    mutex_lock(&fifo_mutex);
    poll_wait(filep, &read_queue, pt);
    poll_wait(filep, &write_queue, pt);

    if (length > 0)
        mask |= POLLIN | POLLRDNORM; // 可读
    if (length < BUFFER_SIZE)
        mask |= POLLOUT | POLLWRNORM; // 可写

    mutex_unlock(&fifo_mutex);
    return mask;
}

int fifo_fasync(int fd, struct file *filep, int mode)
{
    return fasync_helper(fd, filep, mode, &fifo_fasync_struct);
}