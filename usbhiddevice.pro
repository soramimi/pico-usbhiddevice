TEMPLATE = app
TARGET = usbhiddevice
INCLUDEPATH += .

DEFINES += CFG_TUSB_MCU=OPT_MCU_RP2040
INCLUDEPATH += $$PWD/../../pico-sdk/lib/tinyusb/src
INCLUDEPATH += $$PWD/../../pico-sdk/lib/tinyusb/hw

HEADERS += tusb_config.h \
           usb_descriptors.h \
		   build/generated/pico_base/pico/config_autogen.h

SOURCES += \
		   main.cpp \
		   usb_descriptors.c
