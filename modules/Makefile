obj-m += gpioed.o palmt3_i2c.o

all:
	make -C ../../linux/ M=$(PWD) modules

clean:
	make -C ../../linux/ M=$(PWD) clean
