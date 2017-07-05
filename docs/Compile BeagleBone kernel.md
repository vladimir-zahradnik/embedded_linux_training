# Compiling the BeagleBone Black Kernel

Following guide contains brief instructions on building a linux kernel for BeagleBone boards.

## Kernel source

In older HOWTOs is often mentioned [BeagleBone Kernel repository](https://github.com/beagleboard/kernel). This repository contains patches for vanilla linux kernel and a script to apply them properly. Other actions need to be done as well, e.g. downloading proper firmware files. This method of building linux kernel for BeagleBone is deprecated, as is mentioned in the repository itself. Instead, use [official BeagleBone Linux repository](https://github.com/beagleboard/linux), which contains already modified kernel sources as well as firmware blobs.

## Prerequisites

As a reference for building the kernel, see [this article](http://wiki.beyondlogic.org/index.php/BeagleBoneBlack_Building_Kernel). Dependencies may differ based on your linux distribution, hence you will need to figure things out. You will however definitely need _GIT_, _uboot-tools_ and a cross-compiler for ARM platform. Above mentioned article uses _gcc-arm-linux-gnueabi_ toolchain. My linux distro did not contain such package, but it contained _arm-linux-gnueabihf-gcc_. To understand difference between _armeabi_ and _armeabihf_, look at [this article](https://blogs.oracle.com/jtc/is-it-armhf-or-armel). The former seems to be legacy EABI nowadays. For our purposes we need to know if SoC in our system supports newer _armhf_. This can be found directly from embedded system after you write following command:

```bash
root@arm:~# readelf -A /proc/self/exe | grep Tag_ABI_VFP_args
  Tag_ABI_VFP_args: VFP registers
root@arm:~#
```

If you were able to get some result, you are good to go with _armhf_ toolchain, otherwise you need to get _armel_ toolchain.

### Pre-compiled toolchains
If you go to [Linaro Project](https://www.linaro.org/downloads/) website, you can get binary toolchains for all common ARM platforms. You can download them, extract and by setting proper system variables, use them while building kernel or kernel module.

## Compiling the kernel

Here is a step-by-step guide on how to do this. First clone modified BeagleBone kernel sources and checkout to proper branch.

```bash
git clone git://github.com/beagleboard/linux.git
cd linux
git checkout 4.1
```

Then you need to specify BeagleBoard defconfig file. This kernel has such file already included.

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bb.org_defconfig 
```

Now we compile kernel image using following commands:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage dtbs LOADADDR=0x80008000 -j4
```

Finally, to build kernel modules, use following command:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
```

Generated kernel image can be found under

```bash
linux/arch/arm/boot
[hyperion@Blade boot (3.14)]$ ls
bootp  compressed  dts  Image  install.sh  Makefile  uImage  zImage
[hyperion@Blade boot (3.14)]$
```
