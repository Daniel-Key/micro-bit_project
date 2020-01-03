HEXFILE=microbit/build/bbc-microbit-classic-gcc/source/jh-project-b1-combined.hex
MICROBIT=/run/media/$(USER)/MICROBIT
TEST=test -d $(MICROBIT)
CONFIG=microbit/source/config/ProtocolConfig


.PHONY: set-target build flash id1 id2 id3 py build-flash1 build-flash2 build-flash3

set-target:
	cd microbit; \
	yotta target bbc-microbit-classic-gcc

build:
	cd microbit; yotta build

flash:
	cp $(HEXFILE) $(MICROBIT)
py:
	python3 -m serial_comms.serial_computer

id1:
	cp $(CONFIG)0.h $(CONFIG).h

id2:
	cp $(CONFIG)1.h $(CONFIG).h

id3:
	cp $(CONFIG)2.h $(CONFIG).h

build-flash1: id1 build flash
build-flash2: id2 build flash
build-flash3: id3 build flash

line-count:
	tokei -e website/node_modules -e *.json -e yotta_* -e website/spam-filter/datasets

install-packages:
	pip3 install -r requirements.txt
	cd website;npm install;npm audit fix
