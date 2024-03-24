#include <asm-generic/errno-base.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Substitution Cipher");
MODULE_AUTHOR("Pratham Rao U N");

static char plaintext[] = "abcd";
static int key = 1;

module_param_string(plaintext, plaintext, sizeof(plaintext), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(plaintext, "Text to encrypt");
module_param(key, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(key, "Key for encryption");

static char *encryption(const char *ptr1, int key) {
    int length = strlen(ptr1);
    char *ciphertext = (char *)kmalloc((length + 1) * sizeof(char), GFP_KERNEL);
    if (!ciphertext) {
	printk(KERN_ERR "Failed to allocate memory for ciphertext.\n");
	return NULL;
    }
    for (int i=0;i<length;i++) {
        if (isalpha(ptr1[i])) {
            if (islower(ptr1[i])) {
                ciphertext[i] = 'a' + (((ptr1[i] - 'a') + key) % 26);
            } else {
                ciphertext[i] = 'A' + (((ptr1[i] - 'A') + key) % 26);
            }
        } else {
            ciphertext[i] = ptr1[i];
        }
    }
    ciphertext[length] = '\0';
    return ciphertext;
}

static char *decryption(const char *ptr2, int key) {
    int length = strlen(ptr2);
    char *decrypt= (char *)kmalloc((length + 1) * sizeof(char), GFP_KERNEL);
    if (!decrypt) {
	printk(KERN_ERR "Failed to allocate memory for plaintext.\n");
	return NULL;
    }
    for (int i=0;i<length;i++) {
        if (isalpha(ptr2[i])) {
            if (islower(ptr2[i])) {
                decrypt[i] = 'z' - ((('z' - ptr2[i]) + key) % 26);
            } else {
                decrypt[i] = 'Z' - ((('Z' - ptr2[i]) + key) % 26);
            }
        } else {
            decrypt[i] = ptr2[i];
        }
    }
    decrypt[length] = '\0';
    return decrypt;
}

static int child_function(void *data) {
    printk(KERN_INFO "Child process. PID: %d, State: %u, Name: %s\n", current->pid, current->flags, current->comm);
    char *encrypt = encryption(plaintext, key);
    if (!encrypt) {
	printk(KERN_ERR "Encryption failed.\n");
	return -ENOMEM;
    }
    printk(KERN_INFO "Encrypted Text after encryption: %s\n", encrypt);
    kfree(encrypt);
    kthread_should_stop();
    return 0;
}

static int __init cipher_init(void) {
    printk(KERN_INFO "Substitution Cipher kernel module loaded.\n");
    printk(KERN_INFO "Plaintext before encryption: %s\n", plaintext);
    printk(KERN_INFO "Parent process. PID: %d, State: %u\n", current->pid, current->flags);
    struct task_struct *child_task;
    child_task = kthread_run(child_function, NULL, "cipher_child");
    if (IS_ERR(child_task)) {
        printk(KERN_ERR "Failed to create child process.\n");
        return PTR_ERR(child_task);
    }
    msleep(100);
    return 0;
}

static void __exit cipher_exit(void) {
    printk(KERN_INFO "Substitution Cipher kernel module unloaded.\n");
    char *re_encrypt = encryption(plaintext, key);
    if (!re_encrypt) {
	printk(KERN_ERR "Encryption failed.\n");
    }
    char *re_decrypt = decryption(re_encrypt, key);
    if (!re_decrypt) {
	printk(KERN_ERR "Decryption failed.\n");
    }
    printk(KERN_INFO "Decrypted Text after decryption: %s\n", re_decrypt);
    kfree(re_encrypt);
    kfree(re_decrypt);
}

module_init(cipher_init);
module_exit(cipher_exit);
