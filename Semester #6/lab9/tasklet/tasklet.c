#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhigalkin");

#define KEYBOARDIRQ 1

char my_tasklet_data[] = "KEYBOARD IRQ";
struct tasklet_struct my_tasklet;

// Bottom Half Function
void my_tasklet_function(unsigned long data)
{
  printk(KERN_INFO "Tasklet: state - %d, count - %d, data - %ld\n",
    my_tasklet.state, my_tasklet.count, my_tasklet.data);
}

// Обработчик прерывания
irqreturn_t irq_handler(int irq, void *dev, struct pt_regs *regs)
{
  // Проверка, что произошло именно нужное 1-е прерывание
  if(irq == KEYBOARDIRQ)
  {
    // Постановка тасклета в очередь на выполнение
    tasklet_schedule(&my_tasklet);
    printk(KERN_INFO "Interrupt by KEYBOARD!\n");

    return IRQ_HANDLED; // прерывание обработано
  }
  else
    return IRQ_NONE; // прерывание не обработано
}

// Инициализация модуля
static int __init my_module_init(void)
{
  printk(KERN_DEBUG "MODULE loaded!\n");

  // номер irq
  // наш обработчик
  // флаг разделение(совместное использование) линии IRQ с другими устройствами
  // имя устройства
  // идентификатор устройства, нужен для для отключения с линии прерваний с помощью free_irq
  int ret = request_irq(KEYBOARDIRQ, (irq_handler_t)irq_handler, IRQF_SHARED,
				"my_irq_handler_tasklet", (void *)(irq_handler));

  if (ret != 0)
  {
    printk(KERN_ERR "KEYBOARD IRQ handler wasn't registered");
    return ret;
  }

  printk(KERN_INFO "KEYBOARD IRQ handler was registered successfully");

  tasklet_init(&my_tasklet, my_tasklet_function, (void *)(irq_handler));
  printk(KERN_INFO "xx");
  return ret;
}

// Выход загружаемого модуля
static void __exit my_module_exit(void)
{
  // Освобождение линии прерывания
  free_irq(KEYBOARDIRQ, (void *)(irq_handler));

  // Удаление тасклета
  tasklet_disable(&my_tasklet);
  tasklet_kill(&my_tasklet);
  printk(KERN_DEBUG "MODULE unloaded!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);