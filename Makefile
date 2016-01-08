obj-m := hellofs.o
hellofs-objs := khellofs.o super.o inode.o dir.o file.o
CFLAGS_khellofs.o := -DDEBUG
CFLAGS_super.o := -DDEBUG
CFLAGS_inode.o := -DDEBUG
CFLAGS_dir.o := -DDEBUG
CFLAGS_file.o := -DDEBUG

all: ko mkfs-hellofs

ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

mkfs-hellofs_SOURCES:
	mkfs-hellofs.c hellofs.h

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm mkfs-hellofs
