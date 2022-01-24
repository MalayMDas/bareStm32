PROGRAM=barestm32
OBJECTS=main.o startup_stm32f446re.o

TARGET_FLAGS=\
	--specs=nosys.specs\
	-mcpu=cortex-m4\
	-mthumb\
	-mlittle-endian\
	-mfpu=fpv4-sp-d16\
	-mfloat-abi=hard\
	-mthumb-interwork

CFLAGS=\
	-g -Wall -Wextra -Werror\
	$(TARGET_FLAGS)

LDFLAGS=\
	$(TARGET_FLAGS)\
	-Wl,-Tstm32f446re.ld

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c -o $@ $<

%.o: %.s
	arm-none-eabi-gcc $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJECTS)	
	arm-none-eabi-gcc $(LDFLAGS) -o $@ $(OBJECTS)

flash:
	openocd -f openocd.cfg -c "program $(PROGRAM) verify reset exit"

debug:
	arm-none-eabi-gdb -ex "target remote localhost:4242" $(PROGRAM)

clean:
	rm *.o $(PROGRAM)

gdb:
	arm-none-eabi-gdb -q $(PROGRAM) -x gdb.cfg

.PHONY: flash debug clean gdb	