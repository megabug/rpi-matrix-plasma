CXXFLAGS+=-std=c++17 -O3 -I../rpi-rgb-led-matrix/include

LDFLAGS+=-L../rpi-rgb-led-matrix/lib
LDLIBS+=-lrgbmatrix -lpthread

plasma.$(OBJEXT): CXXFLAGS += -Wall -Wextra -Wshadow -Wundef -Wconversion -Wcast-align -Wunused -Wsign-conversion -fno-common -Wlogical-op

plasma: plasma.cpp fast_hsv2rgb_32bit.c
