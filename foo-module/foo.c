#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

static DEFINE_SPINLOCK(foo_lock);
static struct request_queue * foo_queue;

static int FOO_MAJOR = 0;

#define FOO_SECTOR_SIZE (512)

struct foo_disk
{
  unsigned int heads;     //number of heads;
  unsigned int sectors;   //number of sectors per track;
  unsigned int cylinders; //number of cylinders;
  unsigned int sector_size;

  struct gendisk*  gd;
  unsigned char* data;
};

static struct foo_disk foo_disk0 = 
{
  //disk size = 1 * 1024 * 32 * 512 = 16MB;
  .heads       = 1,
  .sectors     = 200,
  .cylinders   = 32,
  .sector_size = FOO_SECTOR_SIZE,

  .gd   = NULL,
  .data = NULL,
};

static int foo_getgeo(struct block_device* bdev, struct hd_geometry* geo)
{
	struct foo_disk* disk = (struct foo_disk*)bdev->bd_disk->private_data;
  geo->heads     = disk->heads;
  geo->sectors   = disk->sectors;
  geo->cylinders = disk->cylinders;
  return 0;
}
 
static struct block_device_operations foo_fops =
{
  .owner  = THIS_MODULE,
  .getgeo = foo_getgeo,
};

static void do_foo_request(struct request_queue* q)
{
  struct request* req;
  struct foo_disk* disk;

  unsigned int pos;
  unsigned int nsect;
  unsigned int capacity;

  //static unsigned long read_cnt = 0;
  //static unsigned long write_cnt = 0;

  unsigned char* disk_pos;
  unsigned int len;

  unsigned char* mem_pos;
	struct bio_vec bv;
	struct req_iterator iter;

  while((req = blk_fetch_request(q)) != NULL)
  {
    disk = (struct foo_disk*)req->rq_disk->private_data;

    pos = blk_rq_pos(req);
    nsect = blk_rq_sectors(req);
    capacity = get_capacity(req->rq_disk);

    printk("INFO: YuanguoFoo do_foo_request(): %s: pos=%d nsect=%d capacity=%d\n", req->rq_disk->disk_name, pos, nsect, capacity);

    if(pos >= capacity)
    {
      printk("ERROR: YuanguoFoo do_foo_request(): %s: bad access: pos=%d capacity=%d\n", req->rq_disk->disk_name, pos, capacity);
      continue;
    }

    if((pos+nsect)>capacity)
    {
      printk("ERROR: YuanguoFoo do_foo_request(): %s: bad access: pos+nsect=%d capacity=%d\n", req->rq_disk->disk_name, pos+nsect, capacity);
      continue;
    }

    disk_pos = disk->data + pos * disk->sector_size;
    len = nsect * disk->sector_size;

	  if (req->cmd_type == REQ_TYPE_FS)
    {
		  switch (rq_data_dir(req))
      {
        case READ:
        {
          printk("INFO: YuanguoFoo do_foo_request(): %s read: disk_pos=%p len=%d\n", 
              req->rq_disk->disk_name, disk_pos, len);

          rq_for_each_segment(bv, req, iter)
          {
            mem_pos = page_address(bv.bv_page) + bv.bv_offset;
            memcpy(mem_pos, disk_pos, bv.bv_len);
            disk_pos = disk_pos + bv.bv_len;
          }
          __blk_end_request(req, 0, len);
          break;
        }
        case WRITE:
        {
          printk("INFO: YuanguoFoo do_foo_request(): %s write: disk_pos=%p len=%d\n", 
              req->rq_disk->disk_name, disk_pos, len);

          rq_for_each_segment(bv, req, iter)
          {
            mem_pos = page_address(bv.bv_page) + bv.bv_offset;
            memcpy(disk_pos, mem_pos, bv.bv_len);
            disk_pos = disk_pos + bv.bv_len;
          }
          __blk_end_request(req, 0, len);
          break;
        }
      }
    }
  }
}

static int __init foo_init(void)
{
  size_t num_secotrs;
  size_t disk_size; 

  //1.register_blkdev:
  //  alloc FOO_MAJOR, and then put FOO_MAJOR => foo in major_names (like a hashtable)
  //  the register major numbers can be seen in: /proc/devices 
  FOO_MAJOR = register_blkdev(0, "foo");
  if(FOO_MAJOR<0)
  {
    printk("ERROR: YuanguoFoo foo_init(): register_blkdev failed\n");
    return -1;
  }
  printk("INFO: YuanguoFoo foo_init(): register_blkdev succeeded. FOO_MAJOR=%d do_foo_request=%p\n", FOO_MAJOR, do_foo_request);

  //2.init queue
  foo_queue = blk_init_queue(do_foo_request, &foo_lock);
  if(!foo_queue)
  {
    printk("ERROR: YuanguoFoo foo_init(): blk_init_queue failed\n");
    unregister_blkdev(FOO_MAJOR, "foo");
    return -ENOMEM;
  }
  printk("INFO: YuanguoFoo foo_init(): blk_init_queue succeeded. foo_queue=%p\n", foo_queue);

	blk_queue_max_hw_sectors(foo_queue, 256);
  blk_queue_max_segments(foo_queue,256);
	blk_queue_logical_block_size(foo_queue, 512);

  //3.alloc gendisk;
  foo_disk0.gd = alloc_disk(4);
  if(!foo_disk0.gd)
  {
    printk("ERROR: YuanguoFoo foo_init(): alloc_disk failed\n");
    unregister_blkdev(FOO_MAJOR, "foo");
    blk_cleanup_queue(foo_queue);
    return -ENOMEM;
  }
  printk("INFO: YuanguoFoo foo_init(): alloc_disk succeeded\n");

  //4.gendisk settings;
  strcpy(foo_disk0.gd->disk_name, "foo_disk0");

  num_secotrs = foo_disk0.heads * foo_disk0.sectors * foo_disk0.cylinders;
  disk_size = num_secotrs * foo_disk0.sector_size;
  foo_disk0.data = kzalloc(disk_size+512, GFP_KERNEL);
  if(!foo_disk0.data)
  {
    printk("ERROR: YuanguoFoo foo_init(): %s: kzalloc failed\n", foo_disk0.gd->disk_name);

    unregister_blkdev(FOO_MAJOR, "foo");
    del_gendisk(foo_disk0.gd);
    put_disk(foo_disk0.gd);
    blk_cleanup_queue(foo_queue);

    return -ENOMEM;
  }
  printk("INFO: YuanguoFoo foo_init(): %s: kzalloc succeeded. disk_data=%p disk_size=%ld\n", foo_disk0.gd->disk_name, foo_disk0.data, disk_size);

  foo_disk0.gd->private_data = &foo_disk0;
  foo_disk0.gd->major = FOO_MAJOR;
  foo_disk0.gd->first_minor = 0;
  foo_disk0.gd->minors = 4;
  foo_disk0.gd->queue = foo_queue;
  set_capacity(foo_disk0.gd, num_secotrs);
  foo_disk0.gd->fops = &foo_fops;

  add_disk(foo_disk0.gd);

  return 0;
}

static void __exit foo_exit(void)
{
  printk("INFO: YuanguoFoo foo_exit()\n");
  unregister_blkdev(FOO_MAJOR, "foo");
  del_gendisk(foo_disk0.gd);
  put_disk(foo_disk0.gd);
  blk_cleanup_queue(foo_queue);
}

module_init(foo_init);
module_exit(foo_exit);
MODULE_LICENSE("GPL");
