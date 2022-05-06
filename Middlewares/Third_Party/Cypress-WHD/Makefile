# 
# Copyright 2019 Cypress Semiconductor Corporation
# SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 

print-%  : ; @echo $* = $($*)

# Use VERBOSE=1 to get full output
ifneq ($(VERBOSE),1)
  QUIET:=@
else
  QUIET:=
endif

ifeq ($(OS),Windows_NT)
  HOST_OS := Win32
else ifeq ($(shell uname -s),Linux)
  HOST_OS := Linux
  ifeq ($(shell uname -p),x86_64)
    HOST_OS := $(HOST_OS)64
  else
    HOST_OS := $(HOST_OS)32
  endif
endif

ifeq ($(ARMCC),1)
TOOLCHAIN_NAME := ARMCC
endif

ifeq ($(IAR),1)
TOOLCHAIN_NAME := IAR
endif

#Set the default; Only win32 and arm_gnu supported for now
TOOLCHAIN_NAME ?= GCC

HAL_DIRS := External/hal
HAL_DIRS += External/rtos
INC_DIRS := . WiFi_Host_Driver/src WiFi_Host_Driver/src/include
INC_DIRS += WiFi_Host_Driver/inc
INC_DIRS += $(HAL_DIRS)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

LDFLAGS :=
ifeq ($(TOOLCHAIN_NAME),GCC)
COMPILER_FLAGS := -Wall -Wmisleading-indentation -Wnarrowing -Wsign-compare -fsigned-char -ffunction-sections -fdata-sections -fno-common -std=gnu11 -fdiagnostics-color -Werror  -DNDEBUG -ggdb -O2 -mthumb -mcpu=cortex-m4    -mlittle-endian
COMPILER_SPECIFIC_ARFLAGS_CREATE      := -rcs
LDFLAGS := -specs=nosys.specs -Wl,-Map,$(BUILD_FOLDER)/whd.map
DEP_FLAGS := -MMD
else ifeq ($(TOOLCHAIN_NAME),ARMCC)
COMPILER_FLAGS := --target=arm-arm-none-eabi -march=armv8-m.main -mfpu=fpv4-sp-d16 -Wall -Wnarrowing -Wsign-compare -fsigned-char -ffunction-sections -fdata-sections -fno-common -std=gnu11 -fdiagnostics-color -Werror  -DNDEBUG -ggdb -O2 -mthumb -mcpu=cortex-m4  -mlittle-endian  -fshort-enums
COMPILER_SPECIFIC_ARFLAGS_CREATE      := -rcs
DEP_FLAGS := -MMD
else ifeq ($(TOOLCHAIN_NAME),IAR)
COMPILER_FLAGS := --silent --char_is_signed -e --vla  --dlib_config full  -DNDEBUG --debug -Oh --cpu_mode thumb --cpu Cortex-M4  --endian little --warnings_are_errors --require_prototypes
COMPILER_SPECIFIC_ARFLAGS_CREATE := --create -o
DEP_FLAGS := --dependencies=m +
LDFLAGS := --silent
endif

ifeq ($(TOOLCHAIN_NAME),GCC)
  CC := "$(TOOLCHAIN_PATH)arm-none-eabi-gcc"
  #LD := "$(TOOLCHAIN_PATH)arm-none-eabi-g++"
  AR := "$(TOOLCHAIN_PATH)arm-none-eabi-ar"
else ifeq ($(TOOLCHAIN_NAME),ARMCC)
  CC := $(TOOLCHAIN_PATH)armclang
  #LD := $(TOOLCHAIN_PATH)armlink
  AR := $(TOOLCHAIN_PATH)armar
else ifeq ($(TOOLCHAIN_NAME),IAR)
  CC := $(TOOLCHAIN_PATH)iccarm
  #LD := $(TOOLCHAIN_PATH)ilinkarm
  AR := $(TOOLCHAIN_PATH)iarchive
endif

BUILD_FOLDER := build/whd
MKDIR = mkdir -p $1

rwildcard=$(wildcard $(addsuffix $2, $1)) $(foreach d,$(wildcard $(addsuffix *, $1)),$(call rwildcard,$d/,$2))
SRC_FOLDER := WiFi_Host_Driver/src/
SRC := $(filter %.c,$(call rwildcard,$(SRC_FOLDER),*.c))
OBJ := $(patsubst %.c,%.o, $(SRC))
OBJS = $(addprefix $(BUILD_FOLDER)/,$(OBJ))

CFLAGS := $(COMPILER_FLAGS) $(DEP_FLAGS) $(INC_FLAGS) $(EXTRA_FLAGS)

###################### Set the make variable ###########################################
$(BUILD_FOLDER)/libwhd.a : $(OBJS)
	@echo Archiving $@
	$(QUIET)$(AR) $(COMPILER_SPECIFIC_ARFLAGS_CREATE) $(BUILD_FOLDER)/libwhd.a $(OBJS)

$(BUILD_FOLDER)/%.o : %.c
	@echo Compiling $<
	$(QUIET)$(call MKDIR, $(@D))
	@echo $(CC) $(CFLAGS) -o $@ -c $<  > $@.opts
	$(QUIET)$(CC) $(CFLAGS) -o $@ -c $<

DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

clean :
	@echo Removing build/whd
	-$(QUIET)$(RM) -r build/whd

.PHONY: all clean
