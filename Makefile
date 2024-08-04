CC	= kos-cc

CFLAGS =  -DDREAMCAST -Isrc -std=gnu99 -Isrc -I. -flto -Iresources -Wcast-align
LDFLAGS = -lc -lm -lkmg -lkosutils -s -flto -Wl,--as-needed -Wl,--gc-sections

DEFINES = -Wall
OUTPUT = example.elf

SRCDIR		= src
VPATH		= $(SRCDIR)
SRC_C		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
OBJ_C		= $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJS		= $(OBJ_C)

all: ${OUTPUT}

${OUTPUT}: ${OBJS}
	${CC} ${CFLAGS} -o ${OUTPUT} ${OBJS} ${LDFLAGS} ${DEFINES} 

sound_convert:
	ffmpeg -i track1.wav -f s16le -acodec pcm_s16le track1.raw

#%.kmg: %.png
#	$(KOS_BASE)/utils/vqenc/vqenc -t -v -q -k $<

pack:
	rm -f IP.BIN
	sh-elf-objcopy -R .stack -O binary $(OUTPUT) main.bin
	${KOS_BASE}/utils/scramble/scramble main.bin ./cd/1ST_READ.BIN
	makeip ip.txt IP.BIN
	mkisofs -V example -G IP.BIN -joliet -rock -l -x cd -o "example.iso" ./cd
	cdi4dc example.iso disc.cdi -d
	
clean:
	rm *.o *.img src/*.o src/sdl/*.o src/dc/*.o src/enemies/*.o ${OUTPUT} *.BIN *.iso *.mds *.mdf *.elf
