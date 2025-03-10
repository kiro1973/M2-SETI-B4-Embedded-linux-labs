# Steps to Launch the Lab

1. **Create and enter the `pilote_i2c` directory**  
   Make sure you have created a directory named `pilote_i2c`. If not:
   ` $ mkdir pilote_i2c `  
   Then:
    cd pilote_i2c
   Make sure it has the makefile provided by this repo or in the lab description 

3. **Build the driver in `pilote_i2c`**  
    make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm KDIR=../linux-5.10.19/build/

4. **Compile the user program**  
    arm-linux-gnueabihf-gcc -Wall -o main main.c  
   If this doesn’t work, move `main.c` to another folder, compile it there, and then copy the resulting (compiled/binary/executable) file back into `pilote_i2c`.

5. **Go to the directory containing `linux-5.10.19`**  
   For example, if it’s `~/seti-b4-tp`, do:  
    cd ~/seti-b4-tp  

   Then launch QEMU:
    ./qemu-system-arm -nographic -machine vexpress-a9 -kernel linux-5.10.19/build/arch/arm/boot/zImage -dtb linux-5.10.19/build/arch/arm/boot/dts/vexpress-v2p-ca9.dtb -initrd rootfs.cpio.gz -fsdev local,path=pilote_i2c,security_model=mapped,id=mnt -device virtio-9p-device,fsdev=mnt,mount_tag=mnt

6. **Inside QEMU**  
   - Login as root:  
       root  
   - Mount the 9p filesystem:  
       mount -t 9p -o trans=virtio mnt /mnt -oversion=9p2000.L,msize=10240  
   - Insert the module:  
       insmod /mnt/adxl345.ko  
   - Check if the device node exists:  
       ls /dev/adxl345-*  
     You should see `/dev/adxl345-0`.  
   - Finally, run the program:  
       /mnt/main  

   You should see some output indicating that the ADXL345 driver is working (e.g., sensor readings).
