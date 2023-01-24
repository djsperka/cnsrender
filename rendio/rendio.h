/*
 * rendio.h
 *
 *  Created on: Jul 5, 2016
 *      Author: dan
 */

#ifndef RENDIO_RENDIO_H_
#define RENDIO_RENDIO_H_

#ifndef __KERNEL__
#include <sys/ioctl.h>
#endif

struct rendio_trigger_user_struct
{
	unsigned long ltrigger;		// trigger number - use this in cmds to render
	unsigned long mask;
	unsigned long bits;
	unsigned long flags;
};
typedef struct rendio_trigger_user_struct RendioTriggerUserStruct;
//typedef struct rendio_trigger_user_struct rendio_trigger_params;

/*
 * flags for the triggers
 */

#define RENDIO_TRIGGER_TOGGLE 0x1

struct rendio_dio_output_struct
{
	unsigned long mask;
	unsigned long bits;
};
typedef struct rendio_dio_output_struct RendioDigoutStruct;

#define MAJOR_NUMBER 10
#define IOCTL_ADD_TRIGGER _IOR(MAJOR_NUMBER, 0, struct rendio_trigger_user_struct)
#define IOCTL_REM_TRIGGER _IOR(MAJOR_NUMBER, 1, unsigned long)
#define IOCTL_UPD_OUTPUT _IOR(MAJOR_NUMBER, 2, struct rendio_dio_output_struct)
#define IOCTL_NEXT_TRIGGER _IOW(MAJOR_NUMBER, 3, unsigned long)
#define IOCTL_FIRE_TRIGGER _IOR(MAJOR_NUMBER, 4, unsigned long)
#define IOCTL_ENABLE_OUTPUT _IOR(MAJOR_NUMBER, 5, unsigned long)
#define IOCTL_CLEAR_TRIGGERS _IO(MAJOR_NUMBER, 6)
#define RENDIO_TRIGGER_TOGGLE 0x1


#endif /* RENDIO_RENDIO_H_ */
