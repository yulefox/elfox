all:
	cd build/linux && ./mk.sh -i elf.mk

clean:
	cd build/linux && ./mk.sh -c elf.mk

test:
	cd build/linux && ./mk.sh -d test.mk

