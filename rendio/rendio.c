#include <linux/init.h>		/* __init and __exit macroses */
#include <linux/kernel.h>	/* KERN_INFO macros */
#include <linux/module.h>	/* required for all kernel modules */
#include <linux/moduleparam.h>	/* module_param() and MODULE_PARM_DESC() */

#include <linux/fs.h>		/* struct file_operations, struct file */
#include <linux/miscdevice.h>	/* struct miscdevice and misc_[de]register() */
#include <linux/string.h>	/* memchr() function */
#include <linux/slab.h>		/* kzalloc() function */
#include <linux/sched.h>	/* wait queues */
#include <linux/uaccess.h>	/* copy_{to,from}_user() */
#include <linux/ioctl.h>
#include <linux/list.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/comedilib.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include "rendio.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel Sperka <djsperka@ucdavis.edu>");
MODULE_DESCRIPTION("Digital IO module for render");

// module parameters

static unsigned long comedi_device_number = 0;
module_param(comedi_device_number, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(comedi_device_number, "Comedi device number (/dev/comediN) to use.");

static unsigned long comedi_subdevice_number = 0;
module_param(comedi_subdevice_number, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(comedi_subdevice_number, "Comedi subdevice number to use.");

static unsigned long input_blocks = 0x1;
module_param(input_blocks, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(input_blocks, "Bitmask indicating which 8255 block(s) to use for input.");

static unsigned long output_blocks = 0x2;
module_param(output_blocks, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(output_blocks, "Bitmask indicating which 8255 block(s) to use for output.");

static unsigned long timer_usecs = 10000;
module_param(timer_usecs, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(timer_usecs, "Usecs for dio timer - check in/post out on each timeout.");

static unsigned long rendio_debug = 0;
module_param(rendio_debug, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(rendio_debug, "Bitmask for setting debug output.");


static unsigned long rendio_wmask;
static unsigned long rendio_rmask;
static unsigned long last_wbits;

/*
 * This struct represents a single bit pattern that, when it is satisfied, will be placed
 * in the trigger stream for reading.
 */

struct rendio_trigger
{
	struct rendio_trigger_user_struct t;
	unsigned long last_value;		// for toggled triggers we have to save state from last read
	struct list_head list;
};



/*
 * Big old buffer that represents a single "open" device /dev/rendio
 * I'm only allowing a single client to open this device, but am still allocating this
 * rather than making it static global.
 *
 * The spinlock rwlock protects output_bits.
 * The callback from the timer below (rendio_hrtimer_callback) obtains the read-only
 * version of this lock. The code that modifies rendio_output (rendio_ioctl, called from
 * userspace) obtains the r/w version of the lock.
 *
 *
 * rendio_wmask and rendio_rmask bitmasks for writing and reading to/fromthe dio card.
 * There is an assumption here that there will be no more than 24 bits
 * involved - i.e. that we are dealing with an 8255 chip.
 *
 * The input parameters input_blocks and output_blocks are bit patterns
 * indicating which of the 8255 blocks are to be used for reading from
 * the 8255 (where external triggers come into the system) and writing
 * to the 8255 (output bits written for the world to see).
 *
 * In input_blocks/output_blocks...
 *    bit 0 corresponds to the first 8255 block (8 bits)
 *    bit 1 corresponds to the first 8255 block (8 bits)
 *    bit 2 corresponds to the first 8255 block (4 bits)
 *    bit 3 corresponds to the first 8255 block (4 bits)
 *
 */

struct rendio_buffer
{
	comedi_t *it;					// comedi dio device
	unsigned long subdevice;		// comedi subdevice number to be used
	wait_queue_head_t read_queue;	// read() calls sleep here
	DECLARE_KFIFO(fifo, unsigned long, 32);
	unsigned long fifo_size;		// size of 'fifo', in bytes.
	struct hrtimer timer;			// check dio for triggers on this timer callback
	ktime_t interval;
	struct list_head triggers;		// configured triggers to look for
	rwlock_t rwlock;				// this protects output_bits
	int output_enable;				// set to 1 tells timer function to write bits.
	unsigned long output_bits;
	unsigned long last_rbits;		// copy of last bits read, to look for differences
	unsigned long stop_timer;
};


// local-only prototypes
void rendio_update_output_bits(struct rendio_buffer *buffer, struct rendio_dio_output_struct* pdiout);
enum hrtimer_restart rendio_hrtimer_callback(struct hrtimer *timer);
static unsigned int rendio_do_dio(struct rendio_buffer *buffer, unsigned long wmask, unsigned long wbits, unsigned long rmask, unsigned long *prbits);

/*
 * rendio_trigger_alloc
 *
 * Allocate a struct rendio_trigger and return it (or NULL if there was a failure)
 */

static struct rendio_trigger *rendio_trigger_alloc(void)
{
	struct rendio_trigger *rbuf = NULL;
	rbuf = kzalloc(sizeof(struct rendio_trigger), GFP_KERNEL);
	return rbuf;
}


/*
 * Allocate struct_rendio_buffer and initialize
 */

static struct rendio_buffer *rendio_buffer_alloc(void)
{
	struct rendio_buffer *buf = NULL;

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (unlikely(!buf))
		goto out;

	// initialize read queue
	init_waitqueue_head(&buf->read_queue);

	// init kfifo for triggers found and not yet read
	INIT_KFIFO(buf->fifo);

	// initialize timer.
	// Callback will use container_of(timer) to get the 'struct rendio_buffer'
	// and all the goodies it contains.
	hrtimer_init(&buf->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	buf->timer.function = &rendio_hrtimer_callback;
	buf->interval = ktime_set(0, NSEC_PER_USEC * timer_usecs);

	// initialize list for triggers.
	INIT_LIST_HEAD(&buf->triggers);

	// init rw lock for output_bits
	rwlock_init(&buf->rwlock);
	buf->output_bits = 0;
	buf->last_rbits = 0;
	buf->stop_timer = 0;

 out:
	return buf;

 out_free:
	kfree(buf);
	return NULL;
}


/*
 * rendio_buffer_free
 *
 * free memory allocated in rendio_buffer_alloc
 */

static void rendio_buffer_free(struct rendio_buffer *buffer)
{
	kfifo_free(&buffer->fifo);
	kfree(buffer);
}


static unsigned int rendio_do_dio(struct rendio_buffer *buffer, unsigned long wmask, unsigned long wbits, unsigned long rmask, unsigned long *prbits)
{
	// Now set up comedi instruction and make the hw call
	// below code taken from dio.c, comedi_dio_bitfield
	comedi_insn insn;
	lsampl_t data[2];
	int status;

	memset(&insn, 0, sizeof(insn));
	insn.insn = INSN_BITS;
	insn.n = 2;
	insn.data = data;
	insn.subdev = buffer->subdevice;

	data[0] = wmask;
	data[1] = wbits;
	if (rendio_debug && wmask && ((wbits&wmask) != last_wbits))
	{
		if (rendio_debug) printk(KERN_INFO "comedi_do_dio wrote mask/bits mask %lx bits %lx status %d\n", wmask, wbits, status);
		last_wbits = (wbits&wmask);
	}

	status = comedi_do_insn(buffer->it, &insn);

	if (prbits) *prbits = data[1] & rmask;
	return 0;
}

/*
 * rendio_analyze_triggers
 *
 * Put trigger value in fifo for output to user/listener.
 * The trigger is not "fired", but a listener waiting on a IOCTL_NEXT_TRIGGER ioctl
 * call will get this trigger.
 */

static int rendio_fire_trigger(struct rendio_buffer *buffer, unsigned long trig)
{
	int status = 0;
	if (rendio_debug) printk(KERN_INFO "in rendio_fire_trigger\n");
	if (kfifo_is_full(&buffer->fifo))
	{
		if (rendio_debug) printk(KERN_INFO "rendio - trigger fifo is full! Cannot enqueue trigger %lu\n", trig);
		status = -EPERM;
	}
	else
	{
		// Add trigger to trigger output fifo
		if (rendio_debug) printk(KERN_INFO "in rendio_fire_trigger - kfifo_in\n");

		kfifo_in(&buffer->fifo, &trig, 1);
		wake_up(&buffer->read_queue);
	}
	return status;
}


/*
 * rendio_analyze_triggers
 *
 * Check bit pattern for triggers. Returns 0 on success, a negative value on error.
 * Note that I assume there is a trigger consumer! If there is none, then triggers
 * will be lost inelegantly.
 */

static int rendio_analyze_triggers(struct rendio_buffer *buffer, unsigned long rbits)
{
	int status=0;
	struct rendio_trigger *ptrig;
	list_for_each_entry(ptrig, &buffer->triggers, list)
	{
		unsigned long masked_rbits = rbits & ptrig->t.mask;	// these are the bits relevant to the trigger ONLY
		if (ptrig->t.flags & RENDIO_TRIGGER_TOGGLE)
		{
			//all bits must flip, in theory. We use a lesser requirement, where
			// we require that the bit pattern is changed since last time, AND
			// the bit pattern matches either t.bits (the established pattern for the trigger)
			// or ~t.bits (masked). For a single bit this is trivial and does what you expect.
			unsigned long flipped_bits = ~ptrig->t.bits & ptrig->t.mask;
			if (ptrig->last_value != masked_rbits)
			{
				if (masked_rbits == ptrig->t.bits || masked_rbits == flipped_bits)
				{
					rendio_fire_trigger(buffer, ptrig->t.ltrigger);
				}
			}
		}
		else
		{
			if (ptrig->last_value != masked_rbits)
			{
				if (masked_rbits == ptrig->t.bits)
				{
					rendio_fire_trigger(buffer, ptrig->t.ltrigger);
				}
			}
		}
		ptrig->last_value = masked_rbits;
	}

	buffer->last_rbits = rbits;

	return status;
}

/*
 * rendio_hrtimer_callback(struct hrtimer *timer)
 *
 * Callback from timer. This callback reads from DIO (and writes if bits are set).
 * Does trigger analysis and fills trigger read queue (wakes up sleeper, if there is one).
 *
 */


enum hrtimer_restart rendio_hrtimer_callback(struct hrtimer *timer)
{
	int status;
	struct rendio_buffer *buffer = container_of(timer, struct rendio_buffer, timer);

	unsigned long wbits=0;
	unsigned long wmask = 0;
	unsigned long rbits;


	read_lock(&buffer->rwlock);
	if (buffer->output_enable)
	{
		wbits = buffer->output_bits&rendio_wmask;
		wmask = rendio_wmask;
		buffer->output_enable = 0;
	}
	read_unlock(&buffer->rwlock);

	// do the dio.
	rendio_do_dio(buffer, wmask, wbits, rendio_rmask, &rbits);

	if (rbits != buffer->last_rbits)
	{
		if (rendio_debug) printk(KERN_INFO "changed rbits %lx\n", rbits);
	}
	buffer->last_rbits = rbits;

	// analyze rbits for triggers...
	rendio_analyze_triggers(buffer, rbits);

	//printk(KERN_INFO "rendio_hrtimer wbits %lx in_irq(): %lu in_softirq(): %lu in_interrupt(): %lu\n", wbits, in_irq(), in_softirq(), in_interrupt());

	if (!buffer->stop_timer)
	{
		status = hrtimer_forward(timer, ktime_get(), buffer->interval);
		return HRTIMER_RESTART;
	}
	else
	{
		printk(KERN_INFO "rendio hrtimer stopped.\n");
		return HRTIMER_NORESTART;
	}
}


/*
 * rendio_comedi_open
 *
 * Open comedi device using comedi_device_number as N: /dev/comediN
 * Returns 0 on success, -1 otherwise.
 */

static int rendio_comedi_open(struct rendio_buffer *buffer)
{
	char cdevice[16];
	snprintf(cdevice, 16, "/dev/comedi%lu", comedi_device_number);
	buffer->it = comedi_open(cdevice);
	if(buffer->it == NULL)
	{
		printk(KERN_ERR "Cannot open %s", cdevice);
		return -1;
	}
	printk(KERN_INFO "Comedi device %s opened.\n", cdevice);
	return 0;
}


/*
 * rendio_open
 *
 * Open /dev/rendio, initialize buffer render_buf
 * TODO: convert back to allocated buffer.
 */

static int rendio_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	int status = 0;
	struct rendio_buffer *buffer = NULL;
	// debugging
	last_wbits = 0;

	buffer = rendio_buffer_alloc();
	if (!buffer)
	{
		ret = -ENOMEM;
		goto out;
	}
	file->private_data = buffer;
	printk(KERN_INFO "rendio: open comedi device %lu\n", comedi_device_number);
	status = rendio_comedi_open(buffer);
	if (status)
	{
		/* If we couldn't open the comedi device we are sad */
		ret = -EINVAL;
		goto out;
	}
	else
	{
		// configure input/output channels by block.
		// Comedi docs state that ...
		//
		//     Depending on the characteristics of the hardware device, multiple
		//     channels might be grouped together in hardware when configuring the
		//     input/output direction. In this case, a single call to
		//     comedi_dio_config for any channel in the group will affect the entire
		//     group.
		//
		// docs also state that for the 8255 chip, direction configuration is done in
		// blocks, with channels 0-7, 8-15, 16-19, 20-23 comprising the 4 blocks.
		// Configuring any one channel in each block configures all channels in that
		// block the same way.
		//
		// The code below sets things up assuming we are dealing with an 8255 chip!

		printk(KERN_INFO "Configure dio output blocks %lx\n", output_blocks);
		if (output_blocks & 0x1)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 0, COMEDI_OUTPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 0 for COMEDI_OUTPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}
		if (output_blocks & 0x2)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 8, COMEDI_OUTPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 8 for COMEDI_OUTPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}
		if (output_blocks & 0x4)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 16, COMEDI_OUTPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 16 for COMEDI_OUTPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}
		if (output_blocks & 0x8)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 20, COMEDI_OUTPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 20 for COMEDI_OUTPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}

		printk(KERN_INFO "Configure dio input blocks %lx\n", input_blocks);
		if (input_blocks & 0x1)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 0, COMEDI_INPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 0 for COMEDI_INPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}
		if (input_blocks & 0x2)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 8, COMEDI_INPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 8 for COMEDI_INPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}
		if (input_blocks & 0x4)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 16, COMEDI_INPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 16 for COMEDI_INPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}
		if (input_blocks & 0x8)
		{
			if ((status = comedi_dio_config(buffer->it, 0, 20, COMEDI_INPUT)) < 0)
			{
				printk(KERN_ERR "Cannot set up channel 20 for COMEDI_INPUT (comedi_dio_config returned %d).\n", status);
				ret = -EINVAL;
				goto out;
			}
		}
	}

	// Start timer
	if (timer_usecs)
	{
		printk("Starting timer\n");
		buffer->stop_timer = 0;
		hrtimer_start(&buffer->timer, buffer->interval, HRTIMER_MODE_REL);
	}
	else
	{
		printk(KERN_INFO "Not starting timer\n");
	}

 out:
	return ret;
}

static int rendio_close(struct inode *inode, struct file *file)
{
	struct rendio_buffer *buffer = (struct rendio_buffer *)file->private_data;
	printk(KERN_INFO "rendio_close()\n");
	buffer->stop_timer = 1;
	rendio_update_output_bits(buffer, NULL);	// should lower all outputs before exiting
	if (timer_usecs)
	{
		hrtimer_cancel(&buffer->timer);
	}
	rendio_do_dio(buffer, rendio_wmask, 0, rendio_rmask, NULL);
	comedi_close(buffer->it);
	rendio_buffer_free(buffer);
	printk(KERN_INFO "rendio device has been closed\n");
	return 0;
}

static void rendio_print_trigger_list(struct rendio_buffer *buffer)
{
	struct rendio_trigger *ptrig;

	printk("Emit trigger list\n");
	list_for_each_entry(ptrig, &buffer->triggers, list)
	{
		printk("rendio trigger: %lu/%lx/%lx/%lu.\n", ptrig->t.ltrigger, ptrig->t.mask, ptrig->t.bits, ptrig->t.flags);
	}
}

void rendio_update_output_bits(struct rendio_buffer *buffer, struct rendio_dio_output_struct* pdiout)
{
	if (rendio_debug && pdiout) printk(KERN_INFO "rendio_update_output_bits(bits/mask: %lx/%lx)\n", pdiout->bits, pdiout->mask);
	if (!pdiout)
	{
		write_lock(&buffer->rwlock);
		buffer->output_bits = 0;
		write_unlock(&buffer->rwlock);
	}
	else
	{
		write_lock(&buffer->rwlock);
		buffer->output_bits = (buffer->output_bits&~pdiout->mask) | (pdiout->mask&pdiout->bits);
		if (rendio_debug) printk(KERN_INFO "rendio_update_output_bits: buffer->output_bits=%lx\n", buffer->output_bits);
		write_unlock(&buffer->rwlock);
	}
}

static long rendio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int status = 0;
	unsigned long new_trigger=0;
	struct rendio_trigger *ptrig;
	struct rendio_trigger *pt;
	struct list_head *pos, *q;
	struct rendio_dio_output_struct rendio_dio_output;
	struct rendio_buffer *buffer = (struct rendio_buffer *)file->private_data;

	switch(cmd)
	{
	case IOCTL_ADD_TRIGGER:
	{
		// allocate struct
		ptrig = rendio_trigger_alloc();
		if (unlikely(!ptrig))
		{
			err = -ENOMEM;
		}
		else
		{
			if (copy_from_user(&ptrig->t, (const void __user *)arg, sizeof(struct rendio_trigger_user_struct)))
			{
				err = -EFAULT;
				kfree(ptrig);
			}
			else
			{
				if (rendio_debug) printk(KERN_INFO "rendio ioctl ADD_TRIGGER called: %lu/%lx/%lx/%lu.\n", ptrig->t.ltrigger, ptrig->t.mask, ptrig->t.bits, ptrig->t.flags);

				// Make sure this isn't a duplicate trigger
				status = 0;
				list_for_each_entry(pt, &buffer->triggers, list)
				{
					if (pt->t.ltrigger == ptrig->t.ltrigger)
						err = -EINVAL;
				}

				if (err)
				{
					printk(KERN_ERR "rendio ioctl ADD_TRIGGER: duplicate trigger id (%lu), call REM_TRIGGER first.", ptrig->t.ltrigger);
				}
				else
				{
					// TODO should do error checking on the trigger args
					list_add(&ptrig->list, &buffer->triggers);
				}

				// print list
				rendio_print_trigger_list(buffer);
			}
		}
		break;
	}
	case IOCTL_REM_TRIGGER:

		printk(KERN_INFO "rendio ioctl REM_TRIGGER (%lu) called.\n", arg);
		list_for_each_safe(pos, q, &buffer->triggers)
		{
		    ptrig = list_entry(pos, struct rendio_trigger, list);
		    if (ptrig->t.ltrigger == arg)
		    {
		    	list_del(pos);
		    	kfree(ptrig);
	    	}
		}
		// print list
		rendio_print_trigger_list(buffer);

		break;
	case IOCTL_CLEAR_TRIGGERS:

		printk(KERN_INFO "rendio ioctl CLEAR_TRIGGERS called.\n", arg);
		list_for_each_safe(pos, q, &buffer->triggers)
		{
		    ptrig = list_entry(pos, struct rendio_trigger, list);
	    	list_del(pos);
	    	kfree(ptrig);
		}
		// print list
		rendio_print_trigger_list(buffer);

		break;
	case IOCTL_FIRE_TRIGGER:

		if (rendio_debug) printk(KERN_INFO "rendio ioctl FIRE_TRIGGER (%lu) called.\n", arg);

		// Make sure the trigger exists. If so, fire it.
		list_for_each_safe(pos, q, &buffer->triggers)
		{
		    ptrig = list_entry(pos, struct rendio_trigger, list);
		    if (ptrig->t.ltrigger == arg)
		    {
		    	// queue the trigger
		    	if (rendio_debug) printk(KERN_INFO "rendio ioctl call rendio_fire_trigger(%lu).\n", arg);
		    	err = rendio_fire_trigger(buffer, arg);
	    	}
		}

		break;
	case IOCTL_UPD_OUTPUT:
		if (rendio_debug) printk(KERN_INFO "rendio_ioctl(IOCTL_UPD_OUTPUT)\n");
		if (!arg)
		{
			rendio_update_output_bits(buffer, NULL);
		}
		else
		{
			if (copy_from_user(&rendio_dio_output, (const void __user *)arg, sizeof(struct rendio_dio_output_struct)))
				err = -EFAULT;
			else
			{
				rendio_update_output_bits(buffer, &rendio_dio_output);
				if (rendio_debug) printk(KERN_INFO "rendio ioctl UPD_OUTPUT bits/mask %lx/%lx, next write bits/mask %lx/%lx\n", rendio_dio_output.bits, rendio_dio_output.mask, buffer->output_bits, rendio_wmask);
			}
		}
		break;
	case IOCTL_NEXT_TRIGGER:
		// sleep until there is something available in the queue.
		// Because we're using the producer/consumer kfifo scheme, no locking is needed.
		while (kfifo_is_empty(&buffer->fifo))
		{
			if (wait_event_interruptible(buffer->read_queue, !kfifo_is_empty(&buffer->fifo)))
			{
				return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
			}
		}
		// OK there is something in the queue
		if (1 == kfifo_out(&buffer->fifo, &new_trigger, 1))
		{
			if (rendio_debug) printk(KERN_INFO "IOCTL_NEXT_TRIGGER %lu\n", new_trigger);
			if (copy_to_user((void __user *)arg, &new_trigger, sizeof(unsigned long)))
				return -EFAULT;
		}
		else
			return -EAGAIN;
		break;
	case IOCTL_ENABLE_OUTPUT:
		read_lock(&buffer->rwlock);
		buffer->output_enable = arg;
		read_unlock(&buffer->rwlock);
		break;
	default:
		printk(KERN_INFO "UNKNOWN rendio ioctl called %x.\n", cmd);
		err = -ENOTTY;
		break;
	}

	return err;
}

static struct file_operations rendio_fops = {
	.owner = THIS_MODULE,
	.open = rendio_open,
	.unlocked_ioctl = rendio_ioctl,
	.read = 0,
	.write = 0,
	.release = rendio_close,
	.llseek = 0
};

static struct miscdevice rendio_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rendio",
	.fops = &rendio_fops
};

static int __init rendio_init(void)
{

	// set up masks for read/write bits from/to dio
	if (input_blocks&output_blocks)
	{
		printk(KERN_ERR "input_blocks (%lx) and output_blocks (%lx) cannot overlap.", input_blocks, output_blocks);
		return -EINVAL;
	}

	rendio_wmask =	(output_blocks & 0x1 ?     0xff : 0) |
					(output_blocks & 0x2 ?   0xff00 : 0) |
					(output_blocks & 0x4 ?  0xf0000 : 0) |
					(output_blocks & 0x8 ? 0xf00000 : 0);
	rendio_rmask =	(input_blocks & 0x1 ?     0xff : 0) |
					(input_blocks & 0x2 ?   0xff00 : 0) |
					(input_blocks & 0x4 ?  0xf0000 : 0) |
					(input_blocks & 0x8 ? 0xf00000 : 0);

	// register misc device
	misc_register(&rendio_misc_device);
	printk(KERN_INFO
	       "rendio device has been registered\n");

	return 0;
}

static void __exit rendio_exit(void)
{
	misc_deregister(&rendio_misc_device);
	printk(KERN_INFO "rendio device has been unregistered\n");
}

module_init(rendio_init);
module_exit(rendio_exit);


