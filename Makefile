obj   := spi_pid.c motor_func.c rpi_func.c
obj-out := 3_motor_example.out

all :
	gcc $(obj) -o $(obj-out)
clean :
	rm *.out
	rm *.o
