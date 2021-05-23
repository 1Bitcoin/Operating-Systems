#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/version.h>

#define VFS_MAGIC_NUMBER 0x13131313
#define SLABNAME "vfs_cache"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhigalkin");
MODULE_DESCRIPTION("my_vfs");

static int size = 7;
static int number = 31;
static int sco = 0;

static void **line = NULL;

static void co(void* p)
{
    *(int*)p = (int)p;
    sco++;
}

struct kmem_cache *cache = NULL;

static struct vfs_inode
{
     int i_mode;
     unsigned long i_ino;
} vfs_inode;

static struct inode *vfs_make_inode(struct super_block *sb, int mode)
{
    // Размещение новой структуры inode и заполнение её значениями времен.
    struct inode *ret = new_inode(sb);

    if (ret)
    {
        inode_init_owner(ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        ret->i_private = &vfs_inode;
    }

    return ret;
}

static void vfs_put_super(struct super_block * sb)
{
    printk(KERN_DEBUG "[vfs] super block destroyed!\n");
}

static struct super_operations const vfs_super_ops = {
    .put_super = vfs_put_super,
    // Это заглушки из libfs
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
};

// Создаст структуру dentry, представляющую корневой каталог ФС.
static int vfs_fill_sb(struct super_block *sb, void *data, int silent)
{
    // Инициализация суперблока.
    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = VFS_MAGIC_NUMBER;
    sb->s_op = &vfs_super_ops;

    // Создание inode. S_IFDIR указывает, что создаем каталог.
    struct inode* root = NULL;

    root = vfs_make_inode(sb, S_IFDIR | 0755);
    if (!root)
    {
        printk (KERN_ERR "[vfs] inode allocation failed!\n");
        return -ENOMEM;
    }

    // Взято из ядра.
    root->i_op  = &simple_dir_inode_operations;
    root->i_fop = &simple_dir_operations;

    sb->s_root = d_make_root(root);
    if (!sb->s_root)
    {
        printk(KERN_ERR "[vfs] root creation failed!\n");
        iput(root);
        return -ENOMEM;
    }

    return 0;
}

// Примонтирует устройство и вернет структуру, описывающую корневой каталог ФС.
static struct dentry* vfs_mount(struct file_system_type *type, int flags, char const *dev, void *data)
{
    struct dentry* const entry = mount_nodev(type, flags, data, vfs_fill_sb);

    if (IS_ERR(entry))
        printk(KERN_ERR "[vfs] mounting failed!\n");
    else
        printk(KERN_DEBUG "[vfs] mounted!\n");

    return entry;
}

// "Описывает" создаваемую файловую систему
static struct file_system_type vfs_type = {
    .owner = THIS_MODULE,           // отвечает за счетчик ссылок на модуль, чтобы 
                                    // его нельзя было случайно выгрузить
    .name = "vfs",                  // имя ФС, используемое при монтировании.
    .mount = vfs_mount,             // Эта функция будет вызвана при монтировании ФС.
    .kill_sb = kill_litter_super,   // Эта функция будет вызвана при размонтировании ФС.
};

static int __init vfs_init(void)
{
    int i;
    int ret;

    if (size < 0)
    {
        printk(KERN_ERR "[vfs] invalid argument\n");
        return -EINVAL;
    }

    line = kmalloc(sizeof(void*) * number, GFP_KERNEL);

    if (line == NULL)
    {
        printk(KERN_ERR "[vfs] kmalloc error\n" );
        kfree(line);
        return -ENOMEM;
    }

    for (i = 0; i < number; i++)
    {
        line[i] = NULL;
    }

    // SLAB_HWCACHE_ALIGN — расположение каждого элемента в слабе должно 
    // выравниваться по строкам процессорного кэша, это может существенно 
    // поднять производительность, но непродуктивно расходуется память;

    // со - конструктор, вызывается при размещении каждого элемента
    cache = kmem_cache_create(SLABNAME, size, 0, SLAB_HWCACHE_ALIGN, co);

    if (cache == NULL)
    {
        printk(KERN_ERR "[vfs] kmem_cache_create error\n" );
        kmem_cache_destroy(cache);
        kfree(line);
        return -ENOMEM;
    }

    for (i = 0; i < number; i++)
    {
        if (NULL == (line[i] = kmem_cache_alloc(cache, GFP_KERNEL))) 
        {
            printk(KERN_ERR "[vfs] kmem_cache_alloc error\n");

            for (i = 0; i < number; i++)
            {
                kmem_cache_free(cache, line[i]);
            }

            kmem_cache_destroy(cache);
            kfree(line);
            return -ENOMEM;
        }
    }

    printk(KERN_INFO "[vfs] allocate %d objects into slab: %s\n", number, SLABNAME);
    printk(KERN_INFO "[vfs] object size %d bytes, full size %ld bytes\n", size, (long)size * number);
    printk(KERN_INFO "[vfs] constructor called %d times\n", sco);

    ret = register_filesystem(&vfs_type);

    if (ret!= 0)
    {
        printk(KERN_ERR "[vfs] module cannot register filesystem!\n");
        return ret;
    }

    printk(KERN_DEBUG "[vfs] module loaded!\n");
    return 0;
}

static void __exit vfs_exit(void)
{
    int i;
    int ret;

    for (i = 0; i < number; i++)
        kmem_cache_free(cache, line[i]);

    kmem_cache_destroy(cache);
    kfree(line);

    ret = unregister_filesystem(&vfs_type);

    if (ret!= 0)
        printk(KERN_ERR "[vfs] module cannot unregister filesystem!\n");

    printk(KERN_DEBUG "[vfs] module unloaded!\n");
}

module_init(vfs_init);
module_exit(vfs_exit);