ARCHS = arm64

TARGET = iphone:clang:latest:14.0

THEOS_DEVICE_IP = iphoneX.local

THEOS_PLATFORM_DEB_COMPRESSION_TYPE = gzip

DEBUG=0
STRIP=1
FINALPACKAGE=1

include $(THEOS)/makefiles/common.mk


TWEAK_NAME = H5GG

H5GG_FILES = Tweak.mm ldid-master/ldid.cpp ldid-master/lookup2.c
H5GG_CFLAGS = -fobjc-arc -fvisibility=hidden -DH5GG_RELEASE=1 -Wno-module-import-in-extern-c
H5GG_CCFLAGS = -fobjc-arc -fvisibility=hidden -std=c++11 -DH5GG_RELEASE=1 -Wno-module-import-in-extern-c -Wno-macro-redefined -Wno-unused-but-set-variable
H5GG_LOGOS_DEFAULT_GENERATOR = internal

include $(THEOS_MAKE_PATH)/tweak.mk

clean::
	rm -rf ./packages/*

