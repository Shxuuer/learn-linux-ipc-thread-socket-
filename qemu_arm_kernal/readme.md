# 在 qemu 中打包运行 linux arm kernel

该文件仅做流程记录，具体细节请参考[详细解析](./详细解析.md)

## 编译 linux kernel

### 下载交叉编译工具链

```bash
sudo apt install crossbuild-essential-arm64
```

### 下载 linux kernel 源码

```bash
git clone git@github.com:torvalds/linux.git --depth=1
cd linux
```

### 配置&编译 kernel

使用默认配置即可

```bash
make menuconfig ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
```

## busybox（如果失败可以直接尝试下一节自己编写初始化程序）

### 源码安装 busybox

配置中选择静态编译(settings->Build Options->Build BusyBox as a static binary (no shared libs))

```bash
git clone git@github.com:mirror/busybox.git --depth=1
cd busybox
make menuconfig ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
make -j20 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
make install
```

### 补充文件结构

```bash
cd _install
mkdir -p etc/init.d mnt tmp sys dev proc
```

### 创建 fstab

将进程、临时文件系统、sysfs 挂载到对应目录

```bash
cd etc
touch fstab
vim fstab
cd ..

proc	/proc	proc	defaults	0	0
tmpfs	/tmp	tmpfs	defaults	0	0
sysfs	/sys	sysfs	defaults	0	0
```

### 创建设备文件

```bash
cd dev
sudo mknod -m 622 console c 5 1
sudo mknod -m 666 tty1 c 4 1
sudo mknod -m 666 null c 1 3
cd ..
```

### 设置初始化脚本

```bash
cd etc/init.d
vim rcS
chmod +x rcS

echo -e "Welcome to tinyLinux"
/bin/mount -a # 按照/etc/fstab 挂载文件系统
mount  -o  remount,rw  / # 重新以读写方式挂载根文件系统，默认挂载为只读
mkdir -p /dev/pts # 创建目录
mount -t devpts devpts /dev/pts # 挂载伪终端
echo /sbin/mdev > /proc/sys/kernel/hotplug # 设置热插拔程序
mdev -s # 扫描/sys并创建/dev节点
cd ../..
```

### 设置初始化配置

```bash
vim etc/inittab

::sysinit:/etc/init.d/rcS # 系统初始化脚本
::respawn:-/bin/sh # 系统崩溃后重启 shell
::askfirst:-/bin/sh # 启动时进入 shell
::ctrlaltdel:/bin/umount -a -r # 按下 ctrl+alt+del 时卸载所有文件系统并重新挂载根文件系统为只读
```

### 打包 rootfs

```bash
cd .. # 回到 busybox 目录
dd if=/dev/zero of=./rootfs.ext4 bs=1M count=32 #创建一个空的 32M 的文件
mkfs.ext4 rootfs.ext4 # 格式化为 ext4 文件系统
mkdir fs
sudo mount -o loop rootfs.ext4 ./fs # 将常规文件rootfs.ext4 挂载到 fs 目录
sudo cp -rf ./_install/* ./fs
sudo umount ./fs
gzip --best -c rootfs.ext4 > rootfs.img.gz
```

## 自己编写

### 编译该目录下的两个文件

```bash
aarch64-linux-gnu-gcc -static -o linuxrc mini_init.c
aarch64-linux-gnu-gcc -static -o mini_sh mini_sh.c
```

### 打包 rootfs

```bash
dd if=/dev/zero of=./rootfs.ext4 bs=1M count=32 #创建一个空的 32M 的文件
mkfs.ext4 rootfs.ext4 # 格式化为 ext4 文件系统
mkdir fs
sudo mount -o loop rootfs.ext4 ./fs # 将常规文件rootfs.ext4 挂载到 fs 目录

cd fs
sudo cp ../linuxrc .
sudo cp ../mini_sh .
sudo mkdir -p etc mnt tmp sys dev proc

sudo umount ./fs
gzip --best -c rootfs.ext4 > rootfs.img.gz
```

## qemu 启动

```bash
qemu-system-aarch64 -machine virt -cpu cortex-a57 \
-append "root=/dev/ram init=/linuxrc console=ttyAMA0 \
earlycon=pl011,0x9000000" -nographic \
-initrd path/to/your/rootfs.img.gz
-kernel  path/to/your/arch/arm64/boot/Image
```

输出 mini_init: starting...或 busybox Welcome to tinyLinux 即表示成功
