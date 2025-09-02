#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/slab.h>

// 定义模块的元信息
MODULE_LICENSE("GPL");
MODULE_AUTHOR("shxuuer");
MODULE_DESCRIPTION("A simple shared memory char device");
MODULE_VERSION("0.1");

// 设备名称和内存大小
#define DEVICE_NAME "shdmem"
#define MEM_SIZE 4096

// ioctl命令定义
#define SHDMEM_CLEAR _IO('s', 0) // 清空共享内存

// 共享内存缓冲区
static char *shdmem;

// 字符设备结构体
struct cdev shdmem_cdev;
struct class *shdmem_class;

// 设备号（主设备号[12]:次设备号[20]）
static dev_t shdmem_devno;

// 初始化、清理函数和文件操作函数声明
static int __init shdmem_init(void);
static void __exit shdmem_exit(void);
static int shdmem_open(struct inode *inode, struct file *filep);
static int shdmem_release(struct inode *inode, struct file *filep);
static ssize_t shdmem_read(struct file *filep, char __user *buf, size_t count, loff_t *ppos);
static ssize_t shdmem_write(struct file *filep, const char __user *buf, size_t count, loff_t *ppos);
static loff_t shdmem_llseek(struct file *filep, loff_t offset, int whence);
static long shdmem_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

// 注入初始化和清理函数
module_init(shdmem_init);
module_exit(shdmem_exit);

// 定义文件操作结构体
static struct file_operations shdmem_fops = {
    .owner = THIS_MODULE,
    .open = shdmem_open,
    .release = shdmem_release,
    .read = shdmem_read,
    .write = shdmem_write,
    .llseek = shdmem_llseek,
    .unlocked_ioctl = shdmem_ioctl,
};

/**
 * 初始化函数
 * @return 0 成功，负值失败
 */
static int __init shdmem_init(void)
{
    int ret = 0;
    // 分配设备号
    ret = alloc_chrdev_region(&shdmem_devno, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "Failed to allocate char device region\n");
        return ret;
    }
    // 注册字符设备
    cdev_init(&shdmem_cdev, &shdmem_fops);
    ret = cdev_add(&shdmem_cdev, shdmem_devno, 1);
    if (ret)
        goto fail;

    // 创建设备类
    shdmem_class = class_create(DEVICE_NAME);
    if (IS_ERR(shdmem_class))
    {
        ret = PTR_ERR(shdmem_class);
        goto fail;
    }
    // 创建设备节点 /dev/shdmem
    device_create(shdmem_class, NULL, shdmem_devno, NULL, DEVICE_NAME);

    // 分配共享内存
    shdmem = kzalloc(MEM_SIZE, GFP_KERNEL);
    if (!shdmem)
    {
        ret = -ENOMEM;
        goto fail;
    }

    // 创建成功
    printk(KERN_INFO "shdmem module loaded, major: %d, minor: %d\n", MAJOR(shdmem_devno), MINOR(shdmem_devno));
    return 0;

fail:
    printk(KERN_ERR "Failed to load shdmem module\n");
    if (shdmem)
        kfree(shdmem);
    unregister_chrdev_region(shdmem_devno, 1);
    return ret;
}

/**
 * 清理函数
 */
static void __exit shdmem_exit(void)
{
    device_destroy(shdmem_class, shdmem_devno);
    class_destroy(shdmem_class);
    cdev_del(&shdmem_cdev);
    kfree(shdmem);
    unregister_chrdev_region(shdmem_devno, 1);
    printk(KERN_INFO "shdmem module unloaded\n");
}

/**
 * 打开函数
 * @param inode inode指针
 * @param filep 文件指针
 * @return 0 成功，负值失败
 */
static int shdmem_open(struct inode *inode, struct file *filep)
{
    filep->private_data = &shdmem_cdev;
    return 0;
}

/**
 * 释放函数
 * @param inode inode指针
 * @param filep 文件指针
 * @return 0 成功，负值失败
 */
static int shdmem_release(struct inode *inode, struct file *filep)
{
    return 0;
}

/**
 * 读取函数
 * @param filep 文件指针
 * @param buf 用户空间缓冲区
 * @param count 读取字节数
 * @param ppos 文件位置指针
 * @return 实际读取字节数，负值表示错误
 */
static ssize_t shdmem_read(struct file *filep, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    int size = count;
    // 越界访问
    if (*ppos >= MEM_SIZE)
        return 0;
    // 防止越界访问
    if (*ppos + size > MEM_SIZE)
        size = MEM_SIZE - *ppos;
    // 复制数据到用户空间
    if (copy_to_user(buf, shdmem + *ppos, size))
    {
        ret = -EFAULT;
        goto fail;
    }

    *ppos += size;
    printk(KERN_INFO "shdmem read %d bytes from %lld\n", size, *ppos - size);
    return size;

fail:
    return ret;
}

/**
 * 写入函数
 * @param filep 文件指针
 * @param buf 用户空间缓冲区
 * @param count 写入字节数
 * @param ppos 文件位置指针
 * @return 实际写入字节数，负值表示错误
 */
static ssize_t shdmem_write(struct file *filep, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    int size = count;
    // 越界访问
    if (*ppos >= MEM_SIZE)
        return 0;
    // 防止越界访问
    if (*ppos + size > MEM_SIZE)
        size = MEM_SIZE - *ppos;
    // 复制数据到内核空间
    if (copy_from_user(shdmem + *ppos, buf, size))
    {
        ret = -EFAULT;
        goto fail;
    }

    *ppos += size;
    printk(KERN_INFO "shdmem write %d bytes from %lld\n", size, *ppos - size);
    return size;

fail:
    return ret;
}

/**
 * 定位函数
 * @param filep 文件指针
 * @param offset 偏移量
 * @param whence 位置基准 {SEEK_SET, SEEK_CUR, SEEK_END}
 * @return 新的位置，负值表示错误
 */
static loff_t shdmem_llseek(struct file *filep, loff_t offset, int whence)
{
    loff_t newpos;
    switch (whence)
    {
    case SEEK_SET:
        newpos = offset;
        break;
    case SEEK_CUR:
        newpos = filep->f_pos + offset;
        break;
    case SEEK_END:
        newpos = MEM_SIZE + offset;
        break;
    default:
        return -EINVAL;
    }
    if (newpos < 0 || newpos > MEM_SIZE)
        return -EINVAL;
    filep->f_pos = newpos;
    return newpos;
}

static long shdmem_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
    case SHDMEM_CLEAR:
        memset(shdmem, 0, MEM_SIZE);
        printk(KERN_INFO "shdmem cleared\n");
        break;
    }
    return 0;
}