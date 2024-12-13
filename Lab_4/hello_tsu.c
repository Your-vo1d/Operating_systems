#include <linux/init.h>    // Макросы для инициализации
#include <linux/module.h>  // Основные макросы модулей
#include <linux/kernel.h>  
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Likontsev Nikolay aka Your_Vo1d"); 
MODULE_DESCRIPTION("A module for TSU");
MODULE_VERSION("1.0");

// Название создаваемого файла
#define procfs_name "tsulab"

static struct proc_dir_entry *our_proc_file = NULL;

static ssize_t procfile_read(
    struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t* offset)
    {
        struct tm date;
        date.tm_year = 2024 - 1900;
        date.tm_mon = 11;
        date.tm_mday = 31;
        date.tm_hour = 23;
        date.tm_min = 59;
        date.tm_sec = 59;
        return 0; // Временно
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
    .proc_read= procfile_read,
};
#else
static const struct file_operations_proc_file_fops = {
    .read = procfile_read,
};
#endif

// Функция загрузки модуля
static int __init tsu_module_init(void) {
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    pr_info("Welcome to the Tomsk State University\n");
    return 0; 
}

// Функция выгрузки модуля
static void __exit tsu_module_exit(void) {
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed", procfs_name)
    pr_info("Tomsk State University forever!\n");
}

// Указываем функции для загрузки и выгрузки
module_init(tsu_module_init);
module_exit(tsu_module_exit);
