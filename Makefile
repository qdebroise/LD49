# Distribution notes:
# Compile with -Wl,rpath on Linux and include the SDL.so lib ?
# Or maybe compile statically for Linux and dynamically for Windows ?

#------------------------------------------------------------------------------
# Make environment configuration.
#------------------------------------------------------------------------------
# Remove make built-in variables and implicit rules. It improves performance
# and debugging.
MAKEFLAGS += -Rr
MAKEFLAGS += --warn-undefined-variables

# Default target.
.DEFAULT_GOAL := all

# Use a single shell instance for better performance and exit as soon as any
# error happens.
#SHELL := /bin/bash
.ONESHELL:
#.SHELLFLAGS += -eu -o pipefail -c

# Delete the target file when the recipe fails.
.DELETE_ON_ERROR:

#------------------------------------------------------------------------------
# Build type and platform.
#------------------------------------------------------------------------------ 
SUPPORTED_PLATFORMS := linux windows
SUPPORTED_BUILDS := debug release

TARGET_PLATFORM ?= linux
TARGET_BUILD ?= debug
TARGET_NAME ?= a.out

ifeq ($(filter $(TARGET_PLATFORM),$(SUPPORTED_PLATFORMS)),)
    $(error Unsupported platform '$(TARGET_PLATFORM)')
endif

ifeq ($(filter $(TARGET_BUILD),$(SUPPORTED_BUILDS)),)
    $(error Unsupported build type '$(TARGET_BUILD)')
endif

$(info Build platform: $(TARGET_PLATFORM))
$(info Build type: $(TARGET_BUILD))

#------------------------------------------------------------------------------
# Folders & target.
#------------------------------------------------------------------------------ 
LIBS_DIR := $(CURDIR)/libs
BUILD_DIR := $(CURDIR)/build/$(TARGET_PLATFORM)/$(TARGET_BUILD)
BIN_DIR := $(BUILD_DIR)/bin
OBJS_DIR := $(BUILD_DIR)/objs
DEPS_DIR := $(BUILD_DIR)/deps

TARGET := $(BIN_DIR)/$(TARGET_NAME)

#------------------------------------------------------------------------------
# Sub-directories and files listing.
#------------------------------------------------------------------------------
# Warning: files *MUST* have unique filename across the whole codebase.

SOURCES := \
	src/atom.c \
	src/camera.c \
	src/display.c \
	src/main.c \
	src/player.c \
	src/render.c \

INCLUDE_DIRS := \
	include

LIBS :=

include third-party/third_party.mk

#------------------------------------------------------------------------------
# Tools selections
#------------------------------------------------------------------------------
ifeq ($(TARGET_PLATFORM),linux)
    CC := cc
endif

ifeq ($(TARGET_PLATFORM),windows)
    CC := x86_64-w64-mingw32-gcc
endif

#------------------------------------------------------------------------------
# Compiler options.
#------------------------------------------------------------------------------
CFLAGS := -std=c11 -c -Wall -Wextra -Wpedantic
CPPFLAGS := $(addprefix -I,$(INCLUDE_DIRS))
DEPFLAGS = -MMD -MT $@ -MF $(DEPS_DIR)/$(@F:.o=.d) 

EXTRA_FLAGS_debug := -O0 -g
EXTRA_FLAGS_release := -O2 -DNDEBUG
EXTRA_FLAGS := $(EXTRA_FLAGS_$(TARGET_BUILD))

#------------------------------------------------------------------------------
# Linker options.
#------------------------------------------------------------------------------
LDFLAGS := -L$(LIBS_DIR)/$(TARGET_PLATFORM)
LDLIBS := $(LIBS)

#------------------------------------------------------------------------------
# Create object and dependency files lists.
#------------------------------------------------------------------------------
ALL_FILES := $(notdir $(SOURCES))
ALL_FOLDERS := $(dir $(SOURCES))

OBJECTS := $(ALL_FILES:%.c=$(OBJS_DIR)/%.o)
DEPS := $(ALL_FILES:%.c=$(DEPS_DIR)/%.d)

# Because filenames *MUST* be unique we can add source files directories to vpath as well as the
# objects directory where all the objects are gathered.
VPATH := $(OBJS_DIR) $(ALL_FOLDERS)

#------------------------------------------------------------------------------
# Rules.
#------------------------------------------------------------------------------
.PHONY: all
all: copy

.PHONY: clean
clean:
	rm $(OBJECTS)

.PHONY: copy
copy: $(TARGET)
ifeq ($(TARGET_PLATFORM),windows)
	cp third-party/sdl2/bin/SDL2.dll $(BIN_DIR)
	cp third-party/sdl2/bin/README-SDL.txt $(BIN_DIR)
endif

$(OBJS_DIR) $(DEPS_DIR) $(BIN_DIR):
	@mkdir -p $@

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(addprefix $(OBJS_DIR)/,$(notdir $^)) $(LDFLAGS) $(LDLIBS) -o $@

$(OBJS_DIR)/%.o: %.c | $(OBJS_DIR) $(DEPS_DIR)
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(CPPFLAGS) $(DEPFLAGS) $< -o $@

-include $(DEPS)
