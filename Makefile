all:
	cd build/linux && ./mk.sh -i elf.mk

test:
	cd build/linux && ./mk.sh -d test.mk

