CC = g++

LIBUSB_LIB_DIR =  /lib/x86_64-linux-gnu
LIBUSB_INCLUDE_DIR = /usr/include/libusb-1.0
COMMUNI_LIB_DIR = /usr/lib
COMMUNI_INCLUDE_DIR = /usr/include/communi
SRC_DIR = ./src
INCLUDE_DIR = ./include
EXAMPLES_SRC_DIR = ./examples


objects = usb_e2usb.o communi.o 


all: install main

usb_e2usb.o: $(SRC_DIR)/usb_e2usb.cpp
	@echo Compile $@
	$(CC) $< -c -I$(INCLUDE_DIR) -I$(SRC_DIR) -I$(LIBUSB_INCLUDE_DIR) -L$(LIBUSB_LIB_DIR) -lusb-1.0 -fPIC -o $@

communi.o: $(SRC_DIR)/communi.cpp
	@echo Compile $@
	$(CC) $< -c -I$(INCLUDE_DIR) -I$(SRC_DIR) -I$(LIBUSB_INCLUDE_DIR) -L$(LIBUSB_LIB_DIR) -lusb-1.0 -fPIC -o $@

libcommuni.so: $(objects)
	@echo "Build libcommuni.so.1.0"
	$(CC) $^ -o $@.1.0 -shared -Wl,-soname,$@.1

install: libcommuni.so
	@echo "Install libcommuni.so"
	sudo cp libcommuni.so.1.0 $(COMMUNI_LIB_DIR)
	sudo ln -sf $(COMMUNI_LIB_DIR)/libcommuni.so.1.0 $(COMMUNI_LIB_DIR)/libcommuni.so.1
	sudo ln -sf $(COMMUNI_LIB_DIR)/libcommuni.so.1.0 $(COMMUNI_LIB_DIR)/libcommuni.so
	sudo mkdir $(COMMUNI_INCLUDE_DIR)
	sudo cp $(INCLUDE_DIR)/afx_linux.h $(COMMUNI_INCLUDE_DIR)
	sudo cp $(INCLUDE_DIR)/communi.h $(COMMUNI_INCLUDE_DIR)
	sudo cp $(INCLUDE_DIR)/Communierr.h $(COMMUNI_INCLUDE_DIR)
	sudo cp $(SRC_DIR)/usb_e2usb.h $(COMMUNI_INCLUDE_DIR)
	sudo cp $(SRC_DIR)/if_obj.h $(COMMUNI_INCLUDE_DIR)
	sudo chmod 644 $(COMMUNI_INCLUDE_DIR)/*.h

uninstall:
	@echo "Uninstall libcommuni.so.1.0"
	sudo rm -f $(COMMUNI_LIB_DIR)/libcommuni.so.*
	sudo rm -f $(COMMUNI_INCLUDE_DIR)/*.h
	sudo rm -rf $(COMMUNI_INCLUDE_DIR)

clean:
	rm -f *.o
	rm -f *.so*

main.o: $(EXAMPLES_SRC_DIR)/main.cpp
	@echo Compile $@
	$(CC) $< -c -I$(COMMUNI_INCLUDE_DIR) -o $@

# build using libcommuni.so
main: main.o
	@echo Build $@
	$(CC) $< -o $@ -L$(COMMUNI_LIB_DIR) -lcommuni -lusb-1.0

clean-examples:
	rm -f main main.o

examples: main

# build direct source code (not use libcommuni.so)
build-examples: $(objects)
	g++ $(EXAMPLES_SRC_DIR)/main.cpp  -c -I$(INCLUDE_DIR) -I$(SRC_DIR) -I$(LIBUSB_INCLUDE_DIR) -L$(LIBUSB_LIB_DIR) -lusb-1.0 -fPIC
	g++ main.o $(objects) -o main -I$(INCLUDE_DIR) -I$(SRC_DIR) -I$(LIBUSB_INCLUDE_DIR) -L$(LIBUSB_LIB_DIR) -lusb-1.0
