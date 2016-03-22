TOP=../..
include $(TOP)/mk/env.mk

BUILD_DIR=$(TOP)/build/$(ARCH)
SOURCES:=$(wildcard *.c)
OBJS:=$(addprefix $(BUILD_DIR)/,$(subst .c,.o,$(SOURCES)))
DEPS:=$(addprefix $(BUILD_DIR)/,$(subst .c,.d,$(SOURCES)))

TARGET=$(TOP)/lib/$(ARCH)/liblinker.a

.PHONY: all clean

all:$(TARGET)


include $(TOP)/mk/rule.mk