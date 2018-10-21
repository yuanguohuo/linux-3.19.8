#ifndef __LINUX__AIO_H
#define __LINUX__AIO_H

#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/aio_abi.h>
#include <linux/uio.h>
#include <linux/rcupdate.h>

#include <linux/atomic.h>

struct kioctx;
struct kiocb;

#define KIOCB_KEY		0

/*
 * We use ki_cancel == KIOCB_CANCELLED to indicate that a kiocb has been either
 * cancelled or completed (this makes a certain amount of sense because
 * successful cancellation - io_cancel() - does deliver the completion to
 * userspace).
 *
 * And since most things don't implement kiocb cancellation and we'd really like
 * kiocb completion to be lockless when possible, we use ki_cancel to
 * synchronize cancellation and completion - we only set it to KIOCB_CANCELLED
 * with xchg() or cmpxchg(), see batch_complete_aio() and kiocb_cancel().
 */
#define KIOCB_CANCELLED		((void *) (~0ULL))

typedef int (kiocb_cancel_fn)(struct kiocb *);

struct kiocb {
  //Yuanguo: pointer to the file object associated with the ongoing I/O operation.
	struct file		*ki_filp;

  //Yuanguo: pointer to the asynchronous I/O context descriptor for this operation.
  //     ki_ctx=NULL indicates that it's sync op;
	struct kioctx		*ki_ctx;	/* NULL for sync ops */

  //Yuanguo: method invoked when canceling an asynchronous I/O operation.
	kiocb_cancel_fn		*ki_cancel;

	void			*private;

  //Yuanguo: 
  //  for synchronous operations  : pointer to the process descriptor that issued the I/O operation; 
  //  for asynchronous operations : pointer to the iocb User Mode data structur;
	union {
		void __user		*user;
		struct task_struct	*tsk;
	} ki_obj;

  //Yuanguo: value to be returned to the User Mode process.
	__u64			ki_user_data;	/* user's data for completion */

  //Yuanguo: current file position (file pos) of the ongoing I/O operation.
	loff_t			ki_pos;
  //Yuanguo: number of bytes to be transferred (how many bytes to transferred starting from ki_pos)
	size_t			ki_nbytes;	/* copy of iocb->aio_nbytes */

  //Yuanguo: pointers for the list of active ongoing I/O operation on an asynchronous I/O context.
	struct list_head	ki_list;	/* the aio core uses this
						 * for cancellation */

	/*
	 * If the aio_resfd field of the userspace iocb is not zero,
	 * this is the underlying eventfd context to deliver events to.
	 */
	struct eventfd_ctx	*ki_eventfd;
};

static inline bool is_sync_kiocb(struct kiocb *kiocb)
{
	return kiocb->ki_ctx == NULL;
}

static inline void init_sync_kiocb(struct kiocb *kiocb, struct file *filp)
{
	*kiocb = (struct kiocb) {
			.ki_ctx = NULL,    //Yuanguo: ki_ctx == NULL indicates it's a sync op
			.ki_filp = filp,
			.ki_obj.tsk = current,
		};
}

/* prototypes */
#ifdef CONFIG_AIO
extern ssize_t wait_on_sync_kiocb(struct kiocb *iocb);
extern void aio_complete(struct kiocb *iocb, long res, long res2);
struct mm_struct;
extern void exit_aio(struct mm_struct *mm);
extern long do_io_submit(aio_context_t ctx_id, long nr,
			 struct iocb __user *__user *iocbpp, bool compat);
void kiocb_set_cancel_fn(struct kiocb *req, kiocb_cancel_fn *cancel);
#else
static inline ssize_t wait_on_sync_kiocb(struct kiocb *iocb) { return 0; }
static inline void aio_complete(struct kiocb *iocb, long res, long res2) { }
struct mm_struct;
static inline void exit_aio(struct mm_struct *mm) { }
static inline long do_io_submit(aio_context_t ctx_id, long nr,
				struct iocb __user * __user *iocbpp,
				bool compat) { return 0; }
static inline void kiocb_set_cancel_fn(struct kiocb *req,
				       kiocb_cancel_fn *cancel) { }
#endif /* CONFIG_AIO */

static inline struct kiocb *list_kiocb(struct list_head *h)
{
	return list_entry(h, struct kiocb, ki_list);
}

/* for sysctl: */
extern unsigned long aio_nr;
extern unsigned long aio_max_nr;

#endif /* __LINUX__AIO_H */
