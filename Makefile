BUILD := build



gdbshell:
	mkdir -p $(BUILD)
	cd $(BUILD) && cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain_x86.cmake && make VERBOSE=1

.PHONY: clean
clean:
	rm -rf build

all:gdbshell
	$(MAKE) -C . gdbshell

