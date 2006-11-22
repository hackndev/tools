obj-m += gpioed.o gpioed-ng.o

all:
	make -C ../linux/ M=$(PWD) modules

clean:
	make -C ../linux/ M=$(PWD) clean
