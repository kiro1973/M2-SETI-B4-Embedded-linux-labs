#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include "adxl345_t.h"
//#include <linux/fs.h>

char adxl345_counter = 0; // Counter for the connected devices

//static long adxl345_write_read_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int read_register(struct i2c_client *client,u8 reg_addr);
static ssize_t adxl345_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset);
static long adxl345_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
struct adxl345_device
{
    struct miscdevice misc_dev;
    char addr[2];// TP 3 step 3 Stores the register addresses for the selected axis
};
static const struct file_operations adxl345_fops = {
    .owner = THIS_MODULE,
    .read = adxl345_read,
    .unlocked_ioctl = adxl345_ioctl, // Add the ioctl callback
};





///////////////////////////////////////////////////////////////////
static int read_register(struct i2c_client *client, u8 reg_addr) {
    int ret;
    u8 read_value;

    // Step 1: Write the register address to the device
    ret = i2c_master_send(client, &reg_addr, 1);
    if (ret < 0) {
        printk(KERN_ERR "adxl345: Failed to write register address\n");
        return ret;
    }

    // Step 2: Read the data from the register
    ret = i2c_master_recv(client, &read_value, 1);
    if (ret < 0) {
        printk(KERN_ERR "adxl345: Failed to read register 0x%02X\n", reg_addr);
        return ret;
    }

    return read_value; // Return the value read from the register
}

///////////////////////////////////////////////////////////////////
// Function to write a register
static int write_register(struct i2c_client *client, u8 reg_addr, u8 value) {
    u8 buffer[2] ;
    
    int ret;
    buffer[0] = reg_addr; // Assign the first element
    buffer[1] = value;    // Assign the second element
    // Write the register address and value
    ret = i2c_master_send(client, buffer, 2);
    if (ret < 0) {
        printk(KERN_ERR "adxl345: Failed to write to register 0x%02X\n", reg_addr);
        return ret;
    }
    return 0; // Success
}

///////////////////////////////////////////////////////////////////

// Function to configure the ADXL345
static int configure_adxl345(struct i2c_client *client) {
    int ret;

    // Set data output rate to 100 Hz (BW_RATE register: 0x2C)
    ret = write_register(client, 0x2C, 0x0A); // 0x0A = 100 Hz
    if (ret < 0) return ret;

    // Disable all interrupts (INT_ENABLE register: 0x2E)
    ret = write_register(client, 0x2E, 0x00); // 0x00 = disable all interrupts
    if (ret < 0) return ret;

    // Set default data format (DATA_FORMAT register: 0x31)
    ret = write_register(client, 0x31, 0x00); // 0x00 = default settings
    if (ret < 0) return ret;

    // Set FIFO to bypass mode (FIFO_CTL register: 0x38)
    ret = write_register(client, 0x38, 0x00); // 0x00 = bypass mode
    if (ret < 0) return ret;

    // Enable measurement mode (POWER_CTL register: 0x2D)
    ret = write_register(client, 0x2D, 0x08); // 0x08 = measurement mode
    if (ret < 0) return ret;

    printk(KERN_INFO "adxl345: Configuration complete\n");
    return 0;
}



//////////////////////////////////////////////////////////////////

// Function to put the ADXL345 into standby mode
static int standby_adxl345(struct i2c_client *client) {
    int ret;

    // Set standby mode (POWER_CTL register: 0x2D)
    ret = write_register(client, 0x2D, 0x00); // 0x00 = standby mode
    if (ret < 0) return ret;

    printk(KERN_INFO "adxl345: Device in standby mode\n");
    return 0;
}


//////////////////////////////////////////////////////////////////

static ssize_t adxl345_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    struct adxl345_device *adxl345_dev ;
    struct i2c_client *client ;
    /* LAB 3 Step 2 -->START..........*/
    adxl345_dev = container_of(file->private_data, struct adxl345_device, misc_dev); //step 2 1.a //slide 42, i think misc defive is in the private_data because of misc_register 
        /* [Explanation]
        When you call misc_register, the kernel associates the miscdevice structure with
        the file->private_data field during the open operation. This allows you to retrieve
        it later in the read function.*/
    
    client= to_i2c_client(adxl345_dev->misc_dev.parent);//step 2->1.a//we can use here container_of or to_i2c_client(slide 43)
    u8 buffer[2]; // Buffer to hold the X-axis data (16 bits)
    int ret;

    // Read the low byte (register 0x32)
    ret = read_register(client, adxl345_dev->addr[0]);//1.b
    if (ret < 0) {
        printk(KERN_ERR "adxl345: Failed to read X-axis low byte\n");
        return ret;
    }
    buffer[0] = ret; // Store the low byte

    // Read the high byte (register 0x33)
    ret = read_register(client, adxl345_dev->addr[1]);//1.b
    if (ret < 0) {
        printk(KERN_ERR "adxl345: Failed to read X-axis high byte\n");
        return ret;
    }
    buffer[1] = ret; // Store the high byte

    // Determine how much data to return
    if (size == 1) {
        // Return the most significant byte (MSB)
        if (copy_to_user(user_buffer, &buffer[1], 1)) {//1.c
            printk(KERN_ERR "adxl345: Failed to copy data to user space\n");
            return -EFAULT;
        }
        //return 1; // Return 1 byte
    } else if (size >= 2) {
        // Return the full 16-bit value
        if (copy_to_user(user_buffer, buffer, 2)) {//1.c
            printk(KERN_ERR "adxl345: Failed to copy data to user space\n");
            return -EFAULT;
        }
        //return 2; // Return 2 bytes
    }
        /* LAB 3 Step 2 -->END..........*/


    return size; // No data read
}

///////////////////////////////////////////////////////////////////

static long adxl345_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct adxl345_device *dev = container_of(file->private_data, struct adxl345_device, misc_dev);

    switch (cmd) {
        case ADXL345_SELECT_X:
            dev->addr[0] = DATAX0; // Set register for X-axis low byte
            dev->addr[1] = DATAX1; // Set register for X-axis high byte
            printk(KERN_INFO "adxl345: X-axis selected\n");
            break;

        case ADXL345_SELECT_Y:
            dev->addr[0] = DATAY0; // Set register for Y-axis low byte
            dev->addr[1] = DATAY1; // Set register for Y-axis high byte
            printk(KERN_INFO "adxl345: Y-axis selected\n");
            break;

        case ADXL345_SELECT_Z:
            dev->addr[0] = DATAZ0; // Set register for Z-axis low byte
            dev->addr[1] = DATAZ1; // Set register for Z-axis high byte
            printk(KERN_INFO "adxl345: Z-axis selected\n");
            break;

        default:
            return -EINVAL; // Invalid command   -->#define	EINVAL		22	/* Invalid argument */
    }

    return 0; // Success
}



///////////////////////////////////////////////////////////////////



static int adxl345_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
    struct adxl345_device *adxl345_dev; //lab 3 step 1
    char *name;                         //lab 3 step 1
    int ret;
    printk(KERN_INFO "in adxl345 probe\n");

    read_register(client,(u8)0x00);
       // Configure the ADXL345
    
    ret = configure_adxl345(client);
    if (ret < 0) return ret;
/* LAB 3 Step 1 -->START..........*/
    //Dynamically allocate memory for `adxl345 device` structure 2.a and 2.b
    adxl345_dev = devm_kzalloc(&client->dev, sizeof(struct adxl345_device), GFP_KERNEL);
    if (!adxl345_dev) {
        printk(KERN_ERR "adxl345: Failed to allocate memory for device\n");
        return -ENOMEM; //#define	ENOMEM		12	/* Out of memory */
    }
    
    // Initialize the register addresses (default to X-axis)
    adxl345_dev->addr[0] = DATAX0;
    adxl345_dev->addr[1] = DATAX1;
       
        name = kasprintf(GFP_KERNEL, "adxl345-%d", adxl345_counter++);// 2.c
        if (!name) {
            printk(KERN_ERR "adxl345: Failed to allocate memory for device name\n");
            return -ENOMEM;
        }
    
    i2c_set_clientdata(client, adxl345_dev);//2.b  Associate this instance with the struct i2c client
    adxl345_dev->misc_dev.minor = MISC_DYNAMIC_MINOR; //2.c
    adxl345_dev->misc_dev.name = name;//2.c
    adxl345_dev->misc_dev.parent = &client->dev;//2.c
    adxl345_dev->misc_dev.fops = &adxl345_fops; // SECOND STEP (2) In the probe function, declare this function adxl345 read. this structure in which we have reference to the read function.
    ret = misc_register(&adxl345_dev->misc_dev);//2.d
    if (ret) {
        printk(KERN_ERR "adxl345: Failed to register misc device\n");
        kfree(name); // Free the device name
        return ret;
    }
    printk(KERN_INFO "adxl345: Device registered\n");
    printk(KERN_INFO "adxl345: Device name = %s\n", name);
    printk(KERN_INFO "adxl345: Device minor number = %d\n", adxl345_dev->misc_dev.minor);
    printk(KERN_INFO "LAB 3 step 1 RROPBE done successfully\n");
 /* LAB 3 Step 1 -->END..........*/ 
   

    
    return 0;
}

static int adxl345_remove(struct i2c_client *client)
{
    
    int ret;
    struct adxl345_device *adxl345_dev; //lab 3 step 1.3
    printk(KERN_INFO "adxl345: Device removed\n");

    // // Put the ADXL345 into standby mode
    ret = standby_adxl345(client);
    if (ret < 0) return ret;
/* LAB 3 Step 1 -->START..........*/ 
    adxl345_dev = i2c_get_clientdata(client);
    misc_deregister(&adxl345_dev->misc_dev);
    kfree(adxl345_dev);                // Free the memory allocated for the device structure
    printk(KERN_INFO "adxl345: Device unregistered\n");
    printk(KERN_INFO "LAB 3 step 1 REMOVE done successfully\n");

/* LAB 3 Step 1 -->END..........*/ 

     return 0;

}

/* La liste suivante permet l'association entre un périphérique et son
   pilote dans le cas d'une initialisation statique sans utilisation de
   device tree.

   Chaque entrée contient une chaîne de caractère utilisée pour
   faire l'association et un entier qui peut être utilisé par le
   pilote pour effectuer des traitements différents en fonction
   du périphérique physique détecté (cas d'un pilote pouvant gérer
   différents modèles de périphérique).
*/
static struct i2c_device_id adxl345_idtable[] = {
    { "adxl345", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, adxl345_idtable);

#ifdef CONFIG_OF
/* Si le support des device trees est disponible, la liste suivante
   permet de faire l'association à l'aide du device tree.

   Chaque entrée contient une structure de type of_device_id. Le champ
   compatible est une chaîne qui est utilisée pour faire l'association
   avec les champs compatible dans le device tree. Le champ data est
   un pointeur void* qui peut être utilisé par le pilote pour
   effectuer des traitements différents en fonction du périphérique
   physique détecté.
*/
static const struct of_device_id adxl345_of_match[] = {
    { .compatible = "qemu,adxl345",
      .data = NULL },
    {}
};

MODULE_DEVICE_TABLE(of, adxl345_of_match);
#endif

static struct i2c_driver adxl345_driver = {
    .driver = {
        /* Le champ name doit correspondre au nom du module
           et ne doit pas contenir d'espace */
        .name   = "adxl345",
        .of_match_table = of_match_ptr(adxl345_of_match),
    },

    .id_table       = adxl345_idtable,
    .probe          = adxl345_probe,
    .remove         = adxl345_remove,
};

// struct ice  adxl345{
//     /*........*/
//     struct miscdevice miscdev{
//         /*......*/
//     }
// }

module_i2c_driver(adxl345_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("adxl345 driver");
MODULE_AUTHOR("Kirollos Georges");

