#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/rtmutex.h>
#include <linux/lockdep.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhigalkin");

#define KEYBOARDIRQ 1

char my_workqueue_data[] = "KEYBOARD IRQ";
// Очередь работ
static struct workqueue_struct *my_wq;

typedef struct
{
  struct work_struct work;
  int number;
} my_work_t;

static my_work_t *work1;
static my_work_t *work2;

// Разделяемая переменная   
static int irq_call_n = 0;

// Mutex
struct rt_mutex my_mutex;

struct lock_class_key key;

// Bottom Half Function
void my_work_function(struct work_struct *foo)
{
  rt_mutex_lock(&my_mutex);
  my_work_t* w = (my_work_t*) foo; 

  printk(KERN_INFO "Worker%d: status %d\n", w->number, work_pending(&(w->work)));
  if (w->number == 1)
    printk(KERN_INFO "Worker2: status %d\n", work_pending(&(work2->work)));
  else
    printk(KERN_INFO "Worker1: status %d\n", work_pending(&(work1->work)));

  int sum = 0;
  int i = 0;
  for (; i < 300000000; ++i)
  {
    sum += i;
  }
  printk(KERN_INFO "Sum: %d\n", sum);

  printk(KERN_INFO "Workqueue: counter %d\n", ++irq_call_n);
  rt_mutex_unlock(&my_mutex);
}

irqreturn_t irq_handler(int irq, void *dev)
{
  // Проверка, что произошло именно нужное 1-е прерывание
  if(irq == KEYBOARDIRQ)
  {
    work1 = (my_work_t*)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    work2 = (my_work_t*)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    if (work2)
    {
      // Инициализация структуры work12
      INIT_WORK((struct work_struct*)work2, my_work_function);
      work2->number = 2;
      // Помещаем задачу в очередь работ  
      queue_work(my_wq, (struct work_struct*)work2);

      if (work1)
      {
        // Инициализация структуры work1 
        INIT_WORK((struct work_struct*)work1, my_work_function);
        work1->number = 1;
        // Помещаем задачу в очередь работ  
        queue_work(my_wq, (struct work_struct*)work1);
      }
    }

    return IRQ_HANDLED; // прерывание обработано
  }
  else
    return IRQ_NONE; // прерывание не обработано
}

// Инициализация модуля
static int __init my_module_init(void)
{
  printk(KERN_DEBUG "MODULE loaded!\n");

  // Регистрация обработчика прерывания
  // Разделение(совместное использование) линии IRQ с другими устройствами
  int ret = request_irq(KEYBOARDIRQ, irq_handler, IRQF_SHARED,
				"my_irq_handler_workqueue", irq_handler);

  if (ret != 0)
  {
    printk(KERN_ERR "KEYBOARD IRQ handler wasn't registered");
    return -ENOMEM;
  }

  // Cоздание очереди работ
  // Флаг UNBOUND - по умолчанию - непривязанная к конкретному процессору очередь
  my_wq = create_workqueue("my_queue");

  if (my_wq)
  {
    printk(KERN_INFO "Workqueue was allocated successfully");
    __rt_mutex_init(&my_mutex, NULL, &key);
  }
  else
  {
    free_irq(KEYBOARDIRQ, irq_handler);
    printk(KERN_ERR "Workqueue wasn't allocated");
    return -ENOMEM;
  }

  printk(KERN_INFO "KEYBOARD IRQ handler was registered successfully");
  return ret;
}

// Выход загружаемого модуля
static void __exit my_module_exit(void)
{
  // Освобождение линии прерывания
  free_irq(KEYBOARDIRQ, irq_handler);
  // Удаление очереди работ
  flush_workqueue(my_wq);
  destroy_workqueue(my_wq);
  rt_mutex_destroy(&my_mutex);
  printk(KERN_DEBUG "MODULE unloaded!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
