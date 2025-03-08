# Pilote I2C (ADXL345) Setup Instructions

This README explains how to build and run the ADXL345 I2C driver on an ARM emulated environment using QEMU.

## 1. Directory Setup
1. Create a directory named `pilote_i2c` (if you haven’t already):
    mkdir pilote_i2c
2. Move into `pilote_i2c`:
    cd pilote_i2c

## 2. Build the Driver (Kernel Module)
From inside `pilote_i2c`, run:
    make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm KDIR=../linux-5.10.19/build/

- **CROSS_COMPILE=arm-linux-gnueabihf-**: Specifies the ARM toolchain prefix.
- **ARCH=arm**: Sets the target architecture to ARM.
- **KDIR=../linux-5.10.19/build/**: Path to the Linux kernel build folder.

This command should generate the `adxl345.ko` module file.

## 3. Compile the User Application
Still inside `pilote_i2c` (or wherever `main.c` is located), run:
    arm-linux-gnueabihf-gcc -Wall -o main main.c

- If this fails, try moving `main.c` to another folder, compile there, and copy the resulting executable (binary) `main` back into `pilote_i2c`.

## 4. Launch QEMU
1. Navigate to the directory that contains your `linux-5.10.19` folder. For example:
    cd ~/seti-b4-tp
2. Launch QEMU:
    ./qemu-system-arm -nographic -machine vexpress-a9 \
      -kernel linux-5.10.19/build/arch/arm/boot/zImage \
      -dtb linux-5.10.19/build/arch/arm/boot/dts/vexpress-v2p-ca9.dtb \
      -initrd rootfs.cpio.gz \
      -fsdev local,path=pilote_i2c,security_model=mapped,id=mnt \
      -device virtio-9p-device,fsdev=mnt,mount_tag=mnt

Explanation:
- `-nographic`: No graphical output, only a console.
- `-machine vexpress-a9`: Emulated machine type.
- `-kernel` / `-dtb`: Provide the kernel image and device tree blob.
- `-initrd`: Points to your initial root filesystem (`rootfs.cpio.gz`).
- `-fsdev` / `-device`: Mount your local `pilote_i2c` directory in QEMU via virtio-9p.

## 5. Inside the QEMU Environment
1. **Login**: When prompted with `# buildroot login:`, type:
    root
2. **Mount the 9p Filesystem**:
    mount -t 9p -o trans=virtio mnt /mnt -oversion=9p2000.L,msize=10240
3. **Insert the Driver Module**:
    insmod /mnt/adxl345.ko
4. **Check Device Node**:
    ls /dev/adxl345-*
   You should see:
    /dev/adxl345-0
5. **Run the User-Space Program**:
    /mnt/main

You should now see output from the ADXL345 sensor driver (e.g., x, y, z readings).

## Notes
- Ensure your kernel (`linux-5.10.19`) is built for ARM before running QEMU.
- Make sure the `arm-linux-gnueabihf-` cross-toolchain is installed and available in your PATH.
- If you encounter permissions issues, consider using `sudo` or adjusting file ownership.

**Happy Coding!**
