PREFIX = /usr/local
ARCH = h8300-elf
BINDIR = $(PREFIX)/bin
ADDNAME = $(ARCH)-

AR = $(BINDIR)/$(ADDNAME)ar
AS = $(BINDIR)/$(ADDNAME)as
CC = $(BINDIR)/$(ADDNAME)gcc
LD = $(BINDIR)/$(ADDNAME)ld
NM = $(BINDIR)/$(ADDNAME)nm
OBJCOPY = $(BINDIR)/$(ADDNAME)objcopy
OBJDUMP = $(BINDIR)/$(ADDNAME)objdump
RANLIB = $(BINDIR)/$(ADDNAME)ranlib
STRIP = $(BINDIR)/$(ADDNAME)strip


H8WRITE = $(BINDIR)/h8write
H8WRITE_SERDEV = com1

OBJS = vector.o startup.o main.o
OBJS += lib.o 
OBJS += serial.o
OBJS += xmodem.o
OBJS += elf.o
OBJS += dram.o

TARGET = kzload

CFLAGS = -Wall -mh -nostdinc -nostdlib -fno-builtin
CFLAGS += -I. -I./include
CFLAGS += -Os
CFLAGS += -DKZLOAD

LFLAGS = -static -T ld.scr -L.


.SUFFIXES: .c .o
.SUFFIXES: .s .o
.SUFFIXES: .S .o

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LFLAGS)
	cp $(TARGET) $(TARGET).elf
	$(STRIP) $(TARGET)
	readelf -a $(TARGET).elf > readelf.log

.c.o:	$<
	$(CC) -c $(CFLAGS) $<

.s.o:	$<
	$(CC) -c $(CFLAGS) $<

.S.o:	$<
	$(CC) -c $(CFLAGS) $<

$(TARGET).mot:	$(TARGET)
	$(OBJCOPY) -O srec $(TARGET) $(TARGET).mot

image:	$(TARGET).mot

write:	$(TARGET).mot
	$(H8WRITE) -3069 -f20 $(TARGET).mot $(H8WRITE_SERDEV)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).elf $(TARGET).mot

