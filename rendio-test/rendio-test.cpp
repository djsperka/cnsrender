/*
 * rendio-test.cpp
 *
 *  Created on: May 27, 2016
 *      Author: dan
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <cstring>
using namespace std;

#include "comedilib.h"
#include "../rendio/rendio.h"


unsigned long ulNextTrigger = 201;	// in rendio mode, first trigger number created

int configure_blocks(comedi_t *cdev, unsigned int input_blocks, unsigned int *read_mask, unsigned int output_blocks, unsigned int *write_mask)
{
	int ret=0;
	int status;
	*write_mask = 0;
	*read_mask = 0;

	cout <<  "Configure dio output blocks " << hex << output_blocks << endl;
	if (output_blocks & 0x1)
	{
		if ((status = comedi_dio_config(cdev, 0, 0, COMEDI_OUTPUT)) < 0)
		{
			cout << "Cannot set up channel 0 for COMEDI_OUTPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*write_mask |= 0xff;
		}
	}
	if (output_blocks & 0x2)
	{
		if ((status = comedi_dio_config(cdev, 0, 8, COMEDI_OUTPUT)) < 0)
		{
			cout << "Cannot set up channel 8 for COMEDI_OUTPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*write_mask |= 0xff00;
		}
	}
	if (output_blocks & 0x4)
	{
		if ((status = comedi_dio_config(cdev, 0, 16, COMEDI_OUTPUT)) < 0)
		{
			cout << "Cannot set up channel 16 for COMEDI_OUTPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*write_mask |= 0xf0000;
		}
	}
	if (output_blocks & 0x8)
	{
		if ((status = comedi_dio_config(cdev, 0, 20, COMEDI_OUTPUT)) < 0)
		{
			cout << "Cannot set up channel 20 for COMEDI_OUTPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*write_mask |= 0xf00000;
		}
	}

	cout << "Configure dio input blocks " << hex << input_blocks << endl;
	if (input_blocks & 0x1)
	{
		if ((status = comedi_dio_config(cdev, 0, 0, COMEDI_INPUT)) < 0)
		{
			cout << "Cannot set up channel 0 for COMEDI_INPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*read_mask |= 0xff;
		}
	}
	if (input_blocks & 0x2)
	{
		if ((status = comedi_dio_config(cdev, 0, 8, COMEDI_INPUT)) < 0)
		{
			cout << "Cannot set up channel 8 for COMEDI_INPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*read_mask |= 0xff00;
		}
	}
	if (input_blocks & 0x4)
	{
		if ((status = comedi_dio_config(cdev, 0, 16, COMEDI_INPUT)) < 0)
		{
			cout << "Cannot set up channel 16 for COMEDI_INPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*read_mask |= 0xf0000;
		}
	}
	if (input_blocks & 0x8)
	{
		if ((status = comedi_dio_config(cdev, 0, 20, COMEDI_INPUT)) < 0)
		{
			cout << "Cannot set up channel 20 for COMEDI_INPUT comedi_dio_config returned " << status << endl;
			ret = -1;
		}
		else
		{
			*read_mask |= 0xf00000;
		}
	}

	return ret;
}


void dump_configuration(comedi_t *cdev, unsigned int subdevice)
{
	unsigned int dir;
	int status;
	for (int i=0; i<24; i++)
	{
		status = comedi_dio_get_config(cdev, subdevice, i, &dir);
		if (dir == COMEDI_INPUT)
			cout << "(" << status << ") " << i << " input" << endl;
		else if (dir == COMEDI_OUTPUT)
			cout << "(" << status << ") " << i << " output" << endl;
		else
			cout << "(" << status << ") " << i << " ??????" << endl;
	}
}


int main(int argc, char **argv)
{
	int status=0;
	int fd;
	string dummy;
	string dummy2;

	RendioTriggerUserStruct trig;
	RendioDigoutStruct diout;



	if (argc == 4 && !strcmp(argv[1], "raw"))
	{
		comedi_t *cdev;
		cdev = comedi_open("/dev/comedi0");
		if (cdev)
		{
			cout << "opened /dev/comedi0" << endl;
			cout << comedi_get_n_subdevices(cdev) << " subdevices" << endl;
			cout << "board name " << comedi_get_board_name(cdev) << endl;
			cout << "driver name " << comedi_get_driver_name(cdev) << endl;
			unsigned int input_blocks = atoi(argv[2]);
			unsigned int output_blocks = atoi(argv[3]);
			unsigned int write_mask = 0;
			unsigned int read_mask = 0;
			cout << "configure blocks input " << input_blocks << " output " << output_blocks << endl;
			if (!configure_blocks(cdev, input_blocks, &read_mask, output_blocks, &write_mask))
			{
				cout << "Successfully opened rendio device" << endl;

				dump_configuration(cdev, 0);

				cout << "hit b to key bits, or <enter> to quit ";
				getline(cin, dummy);
				while (dummy == "b" || dummy == "r")
				{
					if (dummy == "b")
					{
						unsigned int bits;
						cout << endl << "Enter bit pattern for output: ";
						getline(cin, dummy2);

						istringstream buffer(dummy2);
						buffer >> hex >> bits;
						cout << endl << "Attempt to write bits : " << hex << bits << endl;

						int status = comedi_dio_bitfield2(cdev, 0, write_mask, &bits, 0);
						cout << "comedi_dio_bitfield2 returned " << status << " bits " << hex << bits << endl;
					}
					else
					{
						unsigned int bits=0;
						int status = comedi_dio_bitfield2(cdev, 0, 0, &bits, 0);
						cout << "comedi_dio_bitfield2 (read) returned " << status << endl;

					}
					cout << endl << "hit b|r or <enter> to quit ";
					getline(cin, dummy);
				}
			}
			close(fd);
		}
		else
		{
			perror("open /dev/comedi0?");
		}
	}
	else
	{


		/*
		 * Open device
		 */

		fd = open("/dev/rendio", O_RDONLY, 0);

		if (fd > 0)
		{
			cout << "Successfully opened rendio device" << endl;
			cout << "hit b to key bits, r to read trigger, or <enter> to quit ";
			getline(cin, dummy);
			while (dummy == "b" || dummy == "r" || dummy == "t" || dummy=="o")
			{
				if (dummy == "b")
				{
					unsigned int bits;
					unsigned int mask;
					cout << endl << "Enter bit pattern for output: ";
					getline(cin, dummy);

					istringstream buffer(dummy);
					buffer >> hex >> bits;
					cout << endl << "Write bits : " << hex << bits << endl;

					cout << endl << "Enter bit pattern for output mask: ";
					getline(cin, dummy);

					buffer.clear();
					buffer.str(dummy);
					buffer >> hex >> mask;
					cout << endl << "mask bits : " << hex << mask << endl;

					diout.bits = bits;
					diout.mask = mask;
					status = ioctl(fd, IOCTL_UPD_OUTPUT, &diout);
					cout << "ioctl status " << status << endl;
				}
				else if (dummy == "r")
				{
					unsigned int trigger;
					cout << "Waiting for input trigger...";
					status = ioctl(fd, IOCTL_NEXT_TRIGGER, &trigger);
					cout << "status " << status << " trigger " << trigger << endl;
				}
				else if (dummy == "o")
				{
					// enable dig output
					unsigned long enable = 1;
					ioctl(fd, IOCTL_ENABLE_OUTPUT, enable);
				}
				else if (dummy == "t")
				{
					RendioTriggerUserStruct triggerStruct;

					unsigned int bits;
					unsigned int mask;
					cout << endl << "Enter bit pattern for trigger mask: ";
					getline(cin, dummy);

					triggerStruct.ltrigger = ulNextTrigger++;
					cout << endl << "trigger number " << triggerStruct.ltrigger;

					istringstream buffer(dummy);
					buffer >> hex >> triggerStruct.mask;
					cout << endl << "trigger mask : " << hex << triggerStruct.mask << endl;

					cout << endl << "Enter bit pattern for trigger bits: ";
					getline(cin, dummy);

					buffer.clear();
					buffer.str(dummy);
					buffer >> hex >> triggerStruct.bits;
					cout << endl << "trigger bits : " << hex << triggerStruct.bits << endl;

					ioctl(fd, IOCTL_ADD_TRIGGER, &triggerStruct);
				}
				cout << endl << "hit b|r|o|t or <enter> to quit ";
				getline(cin, dummy);
			}
			close(fd);
		}
		else
		{
			perror("rendio-testX");
			status = -1;
		}
	}

	return status;
}



