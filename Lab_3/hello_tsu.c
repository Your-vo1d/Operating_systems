#include <linux/init.h>    // Макросы для инициализации
#include <linux/module.h>  // Основные макросы модулей
#include <linux/kernel.h>  // KERN_INFO и другие уровни логов

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Likontsev Nikolay aka Your_Vo1d"); 
MODULE_DESCRIPTION("A module for TSU");
MODULE_VERSION("1.0");

// Функция загрузки модуля
static int __init tsu_module_init(void) {
    printk(KERN_INFO "Welcome to the Tomsk State University\n");
    return 0; 
}

// Функция выгрузки модуля
static void __exit tsu_module_exit(void) {
    printk(KERN_INFO "Tomsk State University forever!\n");
}

// Указываем функции для загрузки и выгрузки
module_init(tsu_module_init);
module_exit(tsu_module_exit);
