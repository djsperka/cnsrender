obj-m += rendio.o
CFLAGS_rendio.o += -DDEBUG

# the default installation location for comedi includes is /usr/local
# Note that the default install (from the git version of comedi) will not
# install the kcomedilib headers anywhere. I fix that by manually
# tweaking the Makefile.am in the folder above these files. 
# TODO create a patch for comedi git 
EXTRA_CFLAGS=-I/usr/local/include

# KERNEL VERSION used in render packages
KVERSION ?= $(shell uname -r)

KBUILD_EXTRA_SYMBOLS=/usr/local/src/comedi/comedi/Module.symvers

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	