# Makefile to rebuild SM64 split image

### Default target ###

default: all

### Build Options ###

# These options can either be changed by modifying the makefile, or
# by building with 'make SETTING=value'. 'make clean' may be required.

# Version of the game to build
VERSION ?= us
# Graphics microcode used
GRUCODE ?= f3d_old
# If COMPARE is 1, check the output sha1sum when building 'all'
COMPARE ?= 0
# If NON_MATCHING is 1, define the NON_MATCHING and AVOID_UB macros when building (recommended)
NON_MATCHING ?= 1
# Build for the N64 (turn this off for ports)
TARGET_N64 ?= 0
# Build for Emscripten/WebGL
TARGET_WEB ?= 0
# Build for DOS
TARGET_DOS ?= 0

# NSP
TARGET_NSP ?= 1

# Compiler to use (ido or gcc)
COMPILER ?= gcc

# DirectX11
ENABLE_DX11 ?= 0
# DirectX12
ENABLE_DX12 ?= 0
# GL2.1 / GLES2
ENABLE_OPENGL ?= 0
# Legacy OGL
ENABLE_OPENGL_LEGACY ?= 0
# Software rasterizer
ENABLE_SOFTRAST ?= 1
# Pick GL backend for DOS: osmesa, dmesa
DOS_GL := osmesa

# Automatic settings only for ports
ifeq ($(TARGET_N64),0)

  NON_MATCHING := 1
  GRUCODE := f3dex2e
  TARGET_WINDOWS := 0
  ifeq ($(TARGET_WEB),0)
	ifeq ($(TARGET_DOS),0)
	  ifeq ($(OS),Windows_NT)
		TARGET_WINDOWS := 1
	  else
		# TODO: Detect Mac OS X, BSD, etc. For now, assume Linux
		TARGET_LINUX := 1
	  endif
	endif
  endif

  ifeq ($(TARGET_WINDOWS),1)
	# On Windows, default to DirectX 11
	ifeq ($(ENABLE_OPENGL)$(ENABLE_OPENGL_LEGACY)$(ENABLE_DX12)$(ENABLE_SOFTRAST),0000)
	  ENABLE_DX11 ?= 1
	endif
  else ifeq ($(ENABLE_OPENGL_LEGACY)$(ENABLE_SOFTRAST),00)
	# On others, default to OpenGL
	ENABLE_OPENGL ?= 1
  endif

  # Sanity checks
  ifeq ($(ENABLE_DX11),1)
	ifneq ($(TARGET_WINDOWS),1)
	  $(error The DirectX 11 backend is only supported on Windows)
	endif
	ifeq ($(ENABLE_OPENGL),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_OPENGL_LEGACY),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX12),1)
	  $(error Cannot specify multiple graphics backends)
	endif
  endif
  ifeq ($(ENABLE_DX12),1)
	ifneq ($(TARGET_WINDOWS),1)
	  $(error The DirectX 12 backend is only supported on Windows)
	endif
	ifeq ($(ENABLE_OPENGL),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_OPENGL_LEGACY),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX11),1)
	  $(error Cannot specify multiple graphics backends)
	endif
  endif
  ifeq ($(ENABLE_SOFTRAST),1)
	ifeq ($(ENABLE_OPENGL),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_OPENGL_LEGACY),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX11),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX12),1)
	  $(error Cannot specify multiple graphics backends)
	endif
  endif
endif

ifeq ($(COMPILER),gcc)
  NON_MATCHING := 1
endif

# Release

ifeq ($(VERSION),jp)
  VERSION_DEF := VERSION_JP
  GRUCODE_DEF := F3D_OLD
else
ifeq ($(VERSION),us)
  VERSION_DEF := VERSION_US
  GRUCODE_DEF := F3D_OLD
else
ifeq ($(VERSION),eu)
  VERSION_DEF := VERSION_EU
  GRUCODE_DEF := F3D_NEW
else
ifeq ($(VERSION),sh)
  $(warning Building SH is experimental and is prone to breaking. Try at your own risk.)
  VERSION_DEF := VERSION_SH
  GRUCODE_DEF := F3D_NEW
# TODO: GET RID OF THIS!!! We should mandate assets for Shindou like EU but we dont have the addresses extracted yet so we'll just pretend you have everything extracted for now.
  NOEXTRACT := 1
else
  $(error unknown version "$(VERSION)")
endif
endif
endif
endif

TARGET := sm64.$(VERSION)
VERSION_CFLAGS := -D$(VERSION_DEF)
VERSION_ASFLAGS := #-D$(VERSION_DEF)=1

# Microcode

ifeq ($(GRUCODE),f3dex) # Fast3DEX
  GRUCODE_DEF := F3DEX_GBI
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1
  TARGET := $(TARGET).f3dex
  COMPARE := 0
else
ifeq ($(GRUCODE), f3dex2) # Fast3DEX2
  GRUCODE_DEF := F3DEX_GBI_2
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1
  TARGET := $(TARGET).f3dex2
  COMPARE := 0
else
ifeq ($(GRUCODE), f3dex2e) # Fast3DEX2 Extended (for PC)
  GRUCODE_DEF := F3DEX_GBI_2E
  TARGET := $(TARGET).f3dex2e
  COMPARE := 0
else
ifeq ($(GRUCODE),f3d_new) # Fast3D 2.0H (Shindou)
  GRUCODE_DEF := F3D_NEW
  TARGET := $(TARGET).f3d_new
  COMPARE := 0
else
ifeq ($(GRUCODE),f3dzex) # Fast3DZEX (2.0J / Animal Forest - Dōbutsu no Mori)
  $(warning Fast3DZEX is experimental. Try at your own risk.)
  GRUCODE_DEF := F3DEX_GBI_2
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1
  TARGET := $(TARGET).f3dzex
  COMPARE := 0
endif
endif
endif
endif
endif

GRUCODE_CFLAGS := -D$(GRUCODE_DEF)
GRUCODE_ASFLAGS := $(GRUCODE_ASFLAGS) --defsym $(GRUCODE_DEF)=1

ifeq ($(NON_MATCHING),1)
  MATCH_CFLAGS := -DNON_MATCHING -DAVOID_UB
  MATCH_ASFLAGS := --defsym AVOID_UB=1
  COMPARE := 0
endif

################### Universal Dependencies ###################

# (This is a bit hacky, but a lot of rules implicitly depend
# on tools and assets, and we use directory globs further down
# in the makefile that we want should cover assets.)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)

# Make sure assets exist
NOEXTRACT ?= 0
ifeq ($(NOEXTRACT),0)
DUMMY != ./extract_assets.py $(VERSION) >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Failed to extract assets)
endif
endif

# Make tools if out of date
DUMMY != $(MAKE) -s -C tools >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Failed to build tools)
endif

endif
endif

################ Target Executable and Sources ###############

# BUILD_DIR is location where all build artifacts are placed
BUILD_DIR_BASE := build
ifeq ($(TARGET_N64),1)
  BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)
else
ifeq ($(TARGET_WEB),1)
  BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_web
else
  ifeq ($(TARGET_NSP),1)
	BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_nsp
  else
	BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_pc
  endif
endif
endif

LIBULTRA := $(BUILD_DIR)/libultra.a
ifeq ($(TARGET_WEB),1)
EXE := $(BUILD_DIR)/$(TARGET).html
else
  ifeq ($(TARGET_WINDOWS),1)
	EXE := $(BUILD_DIR)/$(TARGET).exe
  else
	ifeq ($(TARGET_DOS),1)
	  EXE := $(BUILD_DIR)/$(TARGET).exe
	else
	  EXE := $(BUILD_DIR)/$(TARGET)
	endif
  endif
endif
ROM := $(BUILD_DIR)/$(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
LD_SCRIPT := sm64.ld
MIO0_DIR := $(BUILD_DIR)/bin
SOUND_BIN_DIR := $(BUILD_DIR)/sound
TEXTURE_DIR := textures
ACTOR_DIR := actors
LEVEL_DIRS := $(patsubst levels/%,%,$(dir $(wildcard levels/*/header.h)))

# Directories containing source files
SRC_DIRS := src src/engine src/game src/audio src/menu src/buffers actors levels bin bin/$(VERSION) data assets
ASM_DIRS := lib
ifeq ($(TARGET_N64),1)
  ASM_DIRS := asm $(ASM_DIRS)
else
  SRC_DIRS := $(SRC_DIRS) src/pc src/pc/gfx src/pc/audio src/pc/controller
  ASM_DIRS :=
endif
ifeq ($(TARGET_DOS),1)
  SRC_DIRS += lib/glide3
endif
BIN_DIRS := bin bin/$(VERSION)

ULTRA_SRC_DIRS := lib/src lib/src/math
ULTRA_ASM_DIRS := lib/asm lib/data
ULTRA_BIN_DIRS := lib/bin

GODDARD_SRC_DIRS := src/goddard src/goddard/dynlists

MIPSISET := -mips2
MIPSBIT := -32

ifeq ($(COMPILER),gcc)
  MIPSISET := -mips3
endif

ifeq ($(TARGET_N64),1)

ifeq ($(VERSION),eu)
  OPT_FLAGS := -O2
else
ifeq ($(VERSION),sh)
  OPT_FLAGS := -O2
else
  OPT_FLAGS := -O2 -g3
endif
endif

  # Use a default opt flag for gcc
  ifeq ($(COMPILER),gcc)
	OPT_FLAGS := -O2
  endif

else
ifeq ($(TARGET_WEB),1)
  OPT_FLAGS := -O2 -g4 --source-map-base http://localhost:8080/
else ifeq ($(DEBUG),1)
  OPT_FLAGS := -g
else
  OPT_FLAGS := -ffast-math -Og
endif
endif

# File dependencies and variables for specific files
include Makefile.split

# Source code files
LEVEL_C_FILES := $(wildcard levels/*/leveldata.c) $(wildcard levels/*/script.c) $(wildcard levels/*/geo.c)
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(LEVEL_C_FILES)
CXX_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
S_FILES := $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))
ULTRA_C_FILES := $(foreach dir,$(ULTRA_SRC_DIRS),$(wildcard $(dir)/*.c))
GODDARD_C_FILES := $(foreach dir,$(GODDARD_SRC_DIRS),$(wildcard $(dir)/*.c))
ifeq ($(TARGET_N64),1)
  ULTRA_S_FILES := $(foreach dir,$(ULTRA_ASM_DIRS),$(wildcard $(dir)/*.s))
endif
GENERATED_C_FILES := $(BUILD_DIR)/assets/mario_anim_data.c $(BUILD_DIR)/assets/demo_data.c \
  $(addprefix $(BUILD_DIR)/bin/,$(addsuffix _skybox.c,$(notdir $(basename $(wildcard textures/skyboxes/*.png)))))

ifeq ($(TARGET_WINDOWS),0)
  CXX_FILES :=
endif

ifneq ($(TARGET_N64),1)
  ULTRA_C_FILES := \
	alBnkfNew.c \
	guLookAtRef.c \
	guMtxF2L.c \
	guNormalize.c \
	guOrthoF.c \
	guPerspectiveF.c \
	guRotateF.c \
	guScaleF.c \
	guTranslateF.c

  C_FILES := $(filter-out src/game/main.c,$(C_FILES))
  ULTRA_C_FILES := $(addprefix lib/src/,$(ULTRA_C_FILES))
endif

ifeq ($(VERSION),sh)
SOUND_BANK_FILES := $(wildcard sound/sound_banks/*.json)
SOUND_SEQUENCE_FILES := $(wildcard sound/sequences/jp/*.m64) \
	$(wildcard sound/sequences/*.m64) \
	$(foreach file,$(wildcard sound/sequences/jp/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
	$(foreach file,$(wildcard sound/sequences/*.s),$(BUILD_DIR)/$(file:.s=.m64))
else
SOUND_BANK_FILES := $(wildcard sound/sound_banks/*.json)
SOUND_SEQUENCE_FILES := $(wildcard sound/sequences/$(VERSION)/*.m64) \
	$(wildcard sound/sequences/*.m64) \
	$(foreach file,$(wildcard sound/sequences/$(VERSION)/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
	$(foreach file,$(wildcard sound/sequences/*.s),$(BUILD_DIR)/$(file:.s=.m64))
endif

SOUND_SAMPLE_DIRS := $(wildcard sound/samples/*)
SOUND_SAMPLE_AIFFS := $(foreach dir,$(SOUND_SAMPLE_DIRS),$(wildcard $(dir)/*.aiff))
SOUND_SAMPLE_TABLES := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.table))
SOUND_SAMPLE_AIFCS := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.aifc))
SOUND_OBJ_FILES := $(SOUND_BIN_DIR)/sound_data.o


# Object files
O_FILES := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
		   $(foreach file,$(CXX_FILES),$(BUILD_DIR)/$(file:.cpp=.o)) \
		   $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
		   $(foreach file,$(GENERATED_C_FILES),$(file:.c=.o))

ULTRA_O_FILES := $(foreach file,$(ULTRA_S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
				 $(foreach file,$(ULTRA_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

GODDARD_O_FILES := $(foreach file,$(GODDARD_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d) $(ULTRA_O_FILES:.o=.d) $(GODDARD_O_FILES:.o=.d) $(BUILD_DIR)/$(LD_SCRIPT).d

# Files with GLOBAL_ASM blocks
ifeq ($(NON_MATCHING),0)
GLOBAL_ASM_C_FILES != grep -rl 'GLOBAL_ASM(' $(wildcard src/**/*.c)
GLOBAL_ASM_O_FILES = $(foreach file,$(GLOBAL_ASM_C_FILES),$(BUILD_DIR)/$(file:.c=.o))
GLOBAL_ASM_DEP = $(BUILD_DIR)/src/audio/non_matching_dep
endif

# Segment elf files
SEG_FILES := $(SEGMENT_ELF_FILES) $(ACTOR_ELF_FILES) $(LEVEL_ELF_FILES)

##################### Compiler Options #######################
INCLUDE_CFLAGS := -I include -I $(BUILD_DIR) -I $(BUILD_DIR)/include -I src -I .
ENDIAN_BITWIDTH := $(BUILD_DIR)/endian-and-bitwidth

ifeq ($(TARGET_N64),1)
IRIX_ROOT := tools/ido5.3_compiler

ifeq ($(shell type mips-linux-gnu-ld >/dev/null 2>/dev/null; echo $$?), 0)
  CROSS := mips-linux-gnu-
else ifeq ($(shell type mips64-linux-gnu-ld >/dev/null 2>/dev/null; echo $$?), 0)
  CROSS := mips64-linux-gnu-
else
  CROSS := mips64-elf-
endif

# check that either QEMU_IRIX is set or qemu-irix package installed
ifeq ($(COMPILER),ido)
  ifndef QEMU_IRIX
	QEMU_IRIX := $(shell which qemu-irix 2>/dev/null)
	ifeq (, $(QEMU_IRIX))
	  $(error Please install qemu-irix package or set QEMU_IRIX env var to the full qemu-irix binary path)
	endif
  endif
endif

AS        := $(CROSS)as
CC        := $(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/bin/cc
CPP       := cpp -P -Wno-trigraphs
LD        := $(CROSS)ld
AR        := $(CROSS)ar
OBJDUMP   := $(CROSS)objdump
OBJCOPY   := $(CROSS)objcopy
PYTHON    := python3

# change the compiler to gcc, to use the default, install the gcc-mips-linux-gnu package
ifeq ($(COMPILER),gcc)
  CC        := $(CROSS)gcc
endif

ifeq ($(TARGET_N64),1)
  TARGET_CFLAGS := -nostdinc -I include/libc -DTARGET_N64 -D_LANGUAGE_C
  CC_CFLAGS := -fno-builtin
endif

INCLUDE_CFLAGS := -I include -I $(BUILD_DIR) -I $(BUILD_DIR)/include -I src -I .

# Check code syntax with host compiler
CC_CHECK := gcc
CC_CHECK_CFLAGS := -fsyntax-only -fsigned-char $(CC_CFLAGS) $(TARGET_CFLAGS) $(INCLUDE_CFLAGS) -std=gnu90 -Wall -Wextra -Wno-format-security -Wno-main -DNON_MATCHING -DAVOID_UB $(VERSION_CFLAGS) $(GRUCODE_CFLAGS)

COMMON_CFLAGS = $(OPT_FLAGS) $(TARGET_CFLAGS) $(INCLUDE_CFLAGS) $(VERSION_CFLAGS) $(MATCH_CFLAGS) $(MIPSISET) $(GRUCODE_CFLAGS)

ASFLAGS := -march=vr4300 -mabi=32 -I include -I $(BUILD_DIR) $(VERSION_ASFLAGS) $(MATCH_ASFLAGS) $(GRUCODE_ASFLAGS)
CFLAGS = -Wab,-r4300_mul -non_shared -G 0 -Xcpluscomm -Xfullwarn -signed $(COMMON_CFLAGS) $(MIPSBIT)
OBJCOPYFLAGS := --pad-to=0x800000 --gap-fill=0xFF
SYMBOL_LINKING_FLAGS := $(addprefix -R ,$(SEG_FILES))
LDFLAGS := -T undefined_syms.txt -T $(BUILD_DIR)/$(LD_SCRIPT) -Map $(BUILD_DIR)/sm64.$(VERSION).map --no-check-sections $(SYMBOL_LINKING_FLAGS)
ENDIAN_BITWIDTH := $(BUILD_DIR)/endian-and-bitwidth

ifeq ($(COMPILER),gcc)
  CFLAGS := -march=vr4300 -mfix4300 -mabi=32 -mno-shared -G 0 -mhard-float -fno-stack-protector -fno-common -fno-zero-initialized-in-bss -I include -I src/ -I $(BUILD_DIR)/include -fno-PIC -mno-abicalls -fno-strict-aliasing -fno-inline-functions -ffreestanding -fwrapv -Wall -Wextra $(COMMON_CFLAGS)
endif

ifeq ($(shell getconf LONG_BIT), 32)
  # Work around memory allocation bug in QEMU
  export QEMU_GUEST_BASE := 1
else
  # Ensure that gcc treats the code as 32-bit
  CC_CHECK_CFLAGS += -m32
endif

# Prevent a crash with -sopt
export LANG := C

else # TARGET_N64

AS := as
ifneq ($(TARGET_WEB),1)
  CC := gcc
  CXX := g++
else
  CC := emcc
endif
ifeq ($(TARGET_WINDOWS),1)
  LD := $(CXX)
else
  LD := $(CC)
endif
CPP := cpp -P
OBJDUMP := objdump
OBJCOPY := objcopy
# OBJCOPY RUNNING INTO ERRORS
ifeq ($(TARGET_DOS),1)
  CPP := i586-pc-msdosdjgpp-cpp -P
  OBJDUMP := i586-pc-msdosdjgpp-objdump
  OBJCOPY := i586-pc-msdosdjgpp-objcopy
  AS := i586-pc-msdosdjgpp-as
  CC := i586-pc-msdosdjgpp-gcc
  CXX := i586-pc-msdosdjgpp-g++
  LD := $(CXX)
endif

ifeq ($(TARGET_NSP), 1)
  AS := arm-none-eabi-as
  CC := nspire-gcc
  LD := nspire-gcc
  CXX := nspire-g++
  OBJDUMP := arm-none-eabi-objdump
  OBJCOPY := arm-none-eabi-objcopy
endif

PYTHON := python3

# Platform-specific compiler and linker flags
ifeq ($(TARGET_WINDOWS),1)
  PLATFORM_CFLAGS  := -DTARGET_WINDOWS
  PLATFORM_LDFLAGS := -lm -lxinput9_1_0 -lole32 -no-pie -mwindows
endif
ifeq ($(TARGET_LINUX),1)
  PLATFORM_CFLAGS  := -DTARGET_LINUX `pkg-config --cflags libusb-1.0`
  PLATFORM_LDFLAGS := -lm -lpthread `pkg-config --libs libusb-1.0` -lasound -lpulse -no-pie
endif
ifeq ($(TARGET_WEB),1)
  PLATFORM_CFLAGS  := -DTARGET_WEB
  PLATFORM_LDFLAGS := -lm -no-pie -s TOTAL_MEMORY=20MB -g4 --source-map-base http://localhost:8080/ -s "EXTRA_EXPORTED_RUNTIME_METHODS=['callMain']"
endif
ifeq ($(TARGET_DOS),1)
  PLATFORM_CFLAGS  := -DTARGET_DOS -std=gnu99 -nostdlib -m32 -march=i586 -mtune=pentium-mmx -mmmx -ffreestanding -fgnu89-inline
  PLATFORM_LDFLAGS := -lm -no-pie -Llib/allegro -lalleg
endif

ifeq ($(TARGET_NSP),1)
	PLATFORM_CFLAGS := -DTARGET_NSP -mthumb -mthumb-interwork
	
endif

PLATFORM_CFLAGS += -Wfatal-errors -DNO_SEGMENTED_MEMORY 
PLATFORM_LDFLAGS :=

# Compiler and linker flags for graphics backend
ifeq ($(ENABLE_OPENGL),1)
  GFX_CFLAGS  := -DENABLE_OPENGL
  GFX_LDFLAGS :=
endif
ifeq ($(ENABLE_DX11),1)
  GFX_CFLAGS := -DENABLE_DX11
  PLATFORM_LDFLAGS += -lgdi32 -static
endif
ifeq ($(ENABLE_DX12),1)
  GFX_CFLAGS := -DENABLE_DX12
  PLATFORM_LDFLAGS += -lgdi32 -static
endif
ifeq ($(ENABLE_SOFTRAST),1)
  GFX_CFLAGS := -DENABLE_SOFTRAST
  GFX_LDFLAGS :=
endif
ifeq ($(ENABLE_OPENGL_LEGACY),1)
  GFX_CFLAGS  := -DENABLE_OPENGL_LEGACY
  GFX_LDFLAGS :=
endif

ifneq ($(ENABLE_OPENGL)$(ENABLE_OPENGL_LEGACY),00)
  ifeq ($(TARGET_WINDOWS),1)
	GFX_CFLAGS  += $(shell sdl2-config --cflags) -DGLEW_STATIC
	GFX_LDFLAGS += $(shell sdl2-config --libs) -lglew32 -lopengl32 -lwinmm -limm32 -lversion -loleaut32 -lsetupapi
  endif
  ifeq ($(TARGET_LINUX),1)
	GFX_CFLAGS  += $(shell sdl2-config --cflags)
	GFX_LDFLAGS += -lGL $(shell sdl2-config --libs) -lX11 -lXrandr
  endif
  ifeq ($(TARGET_WEB),1)
	GFX_CFLAGS  += -s USE_SDL=2
	GFX_LDFLAGS += -lGL -lSDL2
  endif
  ifeq ($(TARGET_DOS),1)
	ifeq ($(DOS_GL),dmesa)
	  GFX_CFLAGS += -Iinclude/glide3 -Iinclude/dmesa -DENABLE_DMESA
	  GFX_LDFLAGS += -Llib/dmesa -lgl -Llib/glide3 -lglide3i
	else ifeq ($(DOS_GL),osmesa)
	  GFX_CFLAGS += -Iinclude/osmesa -DENABLE_OSMESA
	  GFX_LDFLAGS += -Llib/osmesa -lgl
	else
	  $(error OpenGL enabled, but no correct DOS_GL value specified (osmesa, dmesa))
	endif
  endif
else ifeq ($(ENABLE_SOFTRAST),1)
  ifeq ($(TARGET_WINDOWS),1)
	GFX_CFLAGS  += $(shell sdl2-config --cflags)
	GFX_LDFLAGS += $(shell sdl2-config --libs)
  endif
  ifeq ($(TARGET_LINUX),1)
  endif
  ifeq ($(TARGET_WEB),1)
	GFX_CFLAGS  += -s USE_SDL=2
	GFX_LDFLAGS += -lSDL2
  endif
endif

GFX_CFLAGS += -DWIDESCREEN

ifeq ($(TARGET_DOS),0)
  MARCH := 
endif

CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security -D_LANGUAGE_C $(VERSION_CFLAGS) $(MATCH_CFLAGS) $(PLATFORM_CFLAGS) $(GFX_CFLAGS) $(GRUCODE_CFLAGS)
CFLAGS := $(OPT_FLAGS) $(INCLUDE_CFLAGS) -D_LANGUAGE_C $(VERSION_CFLAGS) $(MATCH_CFLAGS) $(PLATFORM_CFLAGS) $(GFX_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv $(MARCH)

ASFLAGS := -I include -I $(BUILD_DIR) $(VERSION_ASFLAGS)

LDFLAGS := $(PLATFORM_LDFLAGS) #$(GFX_LDFLAGS)

endif

####################### Other Tools #########################

# N64 tools
TOOLS_DIR = tools
MIO0TOOL = $(TOOLS_DIR)/mio0
N64CKSUM = $(TOOLS_DIR)/n64cksum
N64GRAPHICS = $(TOOLS_DIR)/n64graphics
N64GRAPHICS_CI = $(TOOLS_DIR)/n64graphics_ci
TEXTCONV = $(TOOLS_DIR)/textconv
AIFF_EXTRACT_CODEBOOK = $(TOOLS_DIR)/aiff_extract_codebook
VADPCM_ENC = $(TOOLS_DIR)/vadpcm_enc
EXTRACT_DATA_FOR_MIO = $(TOOLS_DIR)/extract_data_for_mio
SKYCONV = $(TOOLS_DIR)/skyconv
EMULATOR = mupen64plus
EMU_FLAGS = --noosd
LOADER = loader64
LOADER_FLAGS = -vwf
SHA1SUM = sha1sum

ifeq (, $(shell which armips 2>/dev/null))
  RSPASM := $(TOOLS_DIR)/armips
else
  RSPASM = armips
endif

# Use Objcopy instead of extract_data_for_mio
ifeq ($(COMPILER),gcc)
EXTRACT_DATA_FOR_MIO := $(OBJCOPY) -O binary --only-section=.data
endif

######################## Targets #############################

ifeq ($(TARGET_N64),1)
all: $(ROM)
ifeq ($(COMPARE),1)
	@$(SHA1SUM) -c $(TARGET).sha1 || (echo 'The build succeeded, but did not match the official ROM. This is expected if you are making changes to the game.\nTo silence this message, use "make COMPARE=0"'. && false)
endif
else
all: $(EXE).tns
endif

clean:
	$(RM) -r $(BUILD_DIR_BASE)

distclean:
	$(RM) -r $(BUILD_DIR_BASE)
	./extract_assets.py --clean

test: $(ROM)
	$(EMULATOR) $(EMU_FLAGS) $<

load: $(ROM)
	$(LOADER) $(LOADER_FLAGS) $<

libultra: $(BUILD_DIR)/libultra.a

$(BUILD_DIR)/asm/boot.o: $(IPL3_RAW_FILES)
$(BUILD_DIR)/src/game/crash_screen.o: $(CRASH_TEXTURE_C_FILES)

$(BUILD_DIR)/lib/rsp.o: $(BUILD_DIR)/rsp/rspboot.bin $(BUILD_DIR)/rsp/fast3d.bin $(BUILD_DIR)/rsp/audio.bin

$(BUILD_DIR)/include/text_strings.h: include/text_strings.h.in
	$(TEXTCONV) charmap.txt $< $@

$(BUILD_DIR)/include/text_menu_strings.h: include/text_menu_strings.h.in
	$(TEXTCONV) charmap_menu.txt $< $@

ifeq ($(COMPILER),gcc)
$(BUILD_DIR)/lib/src/math/%.o: CFLAGS += -fno-builtin
endif

ifeq ($(VERSION),eu)
TEXT_DIRS := text/de text/us text/fr

# EU encoded text inserted into individual segment 0x19 files,
# and course data also duplicated in leveldata.c
$(BUILD_DIR)/bin/eu/translation_en.o: $(BUILD_DIR)/text/us/define_text.inc.c
$(BUILD_DIR)/bin/eu/translation_de.o: $(BUILD_DIR)/text/de/define_text.inc.c
$(BUILD_DIR)/bin/eu/translation_fr.o: $(BUILD_DIR)/text/fr/define_text.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/us/define_courses.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/de/define_courses.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/fr/define_courses.inc.c

else
ifeq ($(VERSION),sh)
TEXT_DIRS := text/jp
$(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/jp/define_text.inc.c

else
TEXT_DIRS := text/$(VERSION)

# non-EU encoded text inserted into segment 0x02
$(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/$(VERSION)/define_text.inc.c
endif
endif

$(BUILD_DIR)/text/%/define_courses.inc.c: text/define_courses.inc.c text/%/courses.h
	$(CPP) $(VERSION_CFLAGS) $< -o - -I text/$*/ | $(TEXTCONV) charmap.txt - $@

$(BUILD_DIR)/text/%/define_text.inc.c: text/define_text.inc.c text/%/courses.h text/%/dialogs.h
	$(CPP) $(VERSION_CFLAGS) $< -o - -I text/$*/ | $(TEXTCONV) charmap.txt - $@

RSP_DIRS := $(BUILD_DIR)/rsp
ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(ASM_DIRS) $(GODDARD_SRC_DIRS) $(ULTRA_SRC_DIRS) $(ULTRA_ASM_DIRS) $(ULTRA_BIN_DIRS) $(BIN_DIRS) $(TEXTURE_DIRS) $(TEXT_DIRS) $(SOUND_SAMPLE_DIRS) $(addprefix levels/,$(LEVEL_DIRS)) include) $(MIO0_DIR) $(addprefix $(MIO0_DIR)/,$(VERSION)) $(SOUND_BIN_DIR) $(SOUND_BIN_DIR)/sequences/$(VERSION) $(RSP_DIRS)

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

$(BUILD_DIR)/include/text_strings.h: $(BUILD_DIR)/include/text_menu_strings.h
$(BUILD_DIR)/src/menu/file_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/menu/star_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/game/ingame_menu.o: $(BUILD_DIR)/include/text_strings.h

################################################################
# TEXTURE GENERATION                                           #
################################################################

# RGBA32, RGBA16, IA16, IA8, IA4, IA1, I8, I4
$(BUILD_DIR)/%: %.png
	$(N64GRAPHICS) -i $@ -g $< -f $(lastword $(subst ., ,$@))

$(BUILD_DIR)/%.inc.c: $(BUILD_DIR)/% %.png
	hexdump -v -e '1/1 "0x%X,"' $< > $@
	echo >> $@

# Color Index CI8
$(BUILD_DIR)/%.ci8: %.ci8.png
	$(N64GRAPHICS_CI) -i $@ -g $< -f ci8

# Color Index CI4
$(BUILD_DIR)/%.ci4: %.ci4.png
	$(N64GRAPHICS_CI) -i $@ -g $< -f ci4

################################################################

# compressed segment generation

ifeq ($(TARGET_N64),1)
# TODO: ideally this would be `-Trodata-segment=0x07000000` but that doesn't set the address

$(BUILD_DIR)/bin/%.elf: $(BUILD_DIR)/bin/%.o
	$(LD) -e 0 -Ttext=$(SEGMENT_ADDRESS) -Map $@.map -o $@ $<
$(BUILD_DIR)/actors/%.elf: $(BUILD_DIR)/actors/%.o
	$(LD) -e 0 -Ttext=$(SEGMENT_ADDRESS) -Map $@.map -o $@ $<

# Override for level.elf, which otherwise matches the above pattern
.SECONDEXPANSION:
$(BUILD_DIR)/levels/%/leveldata.elf: $(BUILD_DIR)/levels/%/leveldata.o $(BUILD_DIR)/bin/$$(TEXTURE_BIN).elf
	$(LD) -e 0 -Ttext=$(SEGMENT_ADDRESS) -Map $@.map --just-symbols=$(BUILD_DIR)/bin/$(TEXTURE_BIN).elf -o $@ $<

$(BUILD_DIR)/bin/%.bin: $(BUILD_DIR)/bin/%.elf
	$(EXTRACT_DATA_FOR_MIO) $< $@

$(BUILD_DIR)/actors/%.bin: $(BUILD_DIR)/actors/%.elf
	$(EXTRACT_DATA_FOR_MIO) $< $@

$(BUILD_DIR)/levels/%/leveldata.bin: $(BUILD_DIR)/levels/%/leveldata.elf
	$(EXTRACT_DATA_FOR_MIO) $< $@

$(BUILD_DIR)/%.mio0: $(BUILD_DIR)/%.bin
	$(MIO0TOOL) $< $@

$(BUILD_DIR)/%.mio0.o: $(BUILD_DIR)/%.mio0.s
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.mio0.s: $(BUILD_DIR)/%.mio0
	printf ".section .data\n\n.incbin \"$<\"\n" > $@
endif

$(BUILD_DIR)/%.table: %.aiff
	$(AIFF_EXTRACT_CODEBOOK) $< >$@

$(BUILD_DIR)/%.aifc: $(BUILD_DIR)/%.table %.aiff
	$(VADPCM_ENC) -c $^ $@

$(BUILD_DIR)/rsp/%.bin $(BUILD_DIR)/rsp/%_data.bin: rsp/%.s
	$(RSPASM) -sym $@.sym -definelabel $(VERSION_DEF) 1 -definelabel $(GRUCODE_DEF) 1 -strequ CODE_FILE $(BUILD_DIR)/rsp/$*.bin -strequ DATA_FILE $(BUILD_DIR)/rsp/$*_data.bin $<

$(ENDIAN_BITWIDTH): tools/determine-endian-bitwidth.c
	$(CC) -c $(CFLAGS) -o $@.dummy2 $< 2>$@.dummy1; true
	grep -o 'msgbegin --endian .* --bitwidth .* msgend' $@.dummy1 > $@.dummy2
	head -n1 <$@.dummy2 | cut -d' ' -f2-5 > $@
	@rm $@.dummy1
	@rm $@.dummy2

#SOUND TARGETS REMOVED HERE

$(BUILD_DIR)/levels/scripts.o: $(BUILD_DIR)/include/level_headers.h

$(BUILD_DIR)/include/level_headers.h: levels/level_headers.h.in
	$(CPP) -I . levels/level_headers.h.in | $(PYTHON) tools/output_level_headers.py > $(BUILD_DIR)/include/level_headers.h

$(BUILD_DIR)/assets/mario_anim_data.c: $(wildcard assets/anims/*.inc.c)
	$(PYTHON) tools/mario_anims_converter.py > $@

$(BUILD_DIR)/assets/demo_data.c: assets/demo_data.json $(wildcard assets/demos/*.bin)
	$(PYTHON) tools/demo_data_converter.py assets/demo_data.json $(VERSION_CFLAGS) > $@

ifeq ($(COMPILER),ido)
# Source code
$(BUILD_DIR)/levels/%/leveldata.o: OPT_FLAGS := -g
$(BUILD_DIR)/actors/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/bin/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/src/goddard/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/src/goddard/%.o: MIPSISET := -mips1
$(BUILD_DIR)/src/audio/%.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/src/audio/load.o: OPT_FLAGS := -O2 -framepointer -Wo,-loopunroll,0
$(BUILD_DIR)/lib/src/%.o: OPT_FLAGS :=
$(BUILD_DIR)/lib/src/math/ll%.o: MIPSISET := -mips3 -32
$(BUILD_DIR)/lib/src/math/%.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/math/ll%.o: OPT_FLAGS :=
$(BUILD_DIR)/lib/src/ldiv.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/string.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/gu%.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/al%.o: OPT_FLAGS := -O3

ifeq ($(VERSION),eu)
$(BUILD_DIR)/lib/src/_Litob.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/_Ldtob.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/_Printf.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/sprintf.o: OPT_FLAGS := -O3

# enable loop unrolling except for external.c (external.c might also have used
# unrolling, but it makes one loop harder to match)
$(BUILD_DIR)/src/audio/%.o: OPT_FLAGS := -O2
$(BUILD_DIR)/src/audio/load.o: OPT_FLAGS := -O2
$(BUILD_DIR)/src/audio/external.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
else

# The source-to-source optimizer copt is enabled for audio. This makes it use
# acpp, which needs -Wp,-+ to handle C++-style comments.
$(BUILD_DIR)/src/audio/effects.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0 -sopt,-inline=sequence_channel_process_sound,-scalaroptimize=1 -Wp,-+
$(BUILD_DIR)/src/audio/synthesis.o: OPT_FLAGS := -O2 -sopt,-scalaroptimize=1 -Wp,-+
#$(BUILD_DIR)/src/audio/seqplayer.o: OPT_FLAGS := -O2 -sopt,-inline_manual,-scalaroptimize=1 -Wp,-+ #-Wo,-v,-bb,-l,seqplayer_list.txt

# Add a target for build/eu/src/audio/*.copt to make it easier to see debug
$(BUILD_DIR)/src/audio/%.acpp: src/audio/%.c
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/acpp $(TARGET_CFLAGS) $(INCLUDE_CFLAGS) $(VERSION_CFLAGS) $(MATCH_CFLAGS) $(GRUCODE_CFLAGS) -D__sgi -+ $< > $@

$(BUILD_DIR)/src/audio/seqplayer.copt: $(BUILD_DIR)/src/audio/seqplayer.acpp
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/copt -signed -I=$< -CMP=$@ -cp=i -scalaroptimize=1 -inline_manual

$(BUILD_DIR)/src/audio/%.copt: $(BUILD_DIR)/src/audio/%.acpp
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/copt -signed -I=$< -CMP=$@ -cp=i -scalaroptimize=1

endif
endif

ifeq ($(NON_MATCHING),0)
$(GLOBAL_ASM_O_FILES): CC := $(PYTHON) tools/asm_processor/build.py $(CC) -- $(AS) $(ASFLAGS) --
endif

# Rebuild files with 'GLOBAL_ASM' if the NON_MATCHING flag changes.
$(GLOBAL_ASM_O_FILES): $(GLOBAL_ASM_DEP).$(NON_MATCHING)
$(GLOBAL_ASM_DEP).$(NON_MATCHING):
	@rm -f $(GLOBAL_ASM_DEP).*
	touch $@

$(BUILD_DIR)/%.o: %.cpp
	@$(CXX) -fsyntax-only $(CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CXX) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<


$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	@$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -MD $(BUILD_DIR)/$*.d -o $@ $<

ifeq ($(TARGET_N64),1)
$(BUILD_DIR)/$(LD_SCRIPT): $(LD_SCRIPT)
	$(CPP) $(VERSION_CFLAGS) -MMD -MP -MT $@ -MF $@.d -I include/ -I . -DBUILD_DIR=$(BUILD_DIR) -o $@ $<

$(BUILD_DIR)/libultra.a: $(ULTRA_O_FILES)
	$(AR) rcs -o $@ $(ULTRA_O_FILES)
	tools/patch_libultra_math $@

$(BUILD_DIR)/libgoddard.a: $(GODDARD_O_FILES)
	$(AR) rcs -o $@ $(GODDARD_O_FILES)

$(ELF): $(O_FILES) $(MIO0_OBJ_FILES) $(SOUND_OBJ_FILES) $(SEG_FILES) $(BUILD_DIR)/$(LD_SCRIPT) undefined_syms.txt $(BUILD_DIR)/libultra.a $(BUILD_DIR)/libgoddard.a
	$(LD) -L $(BUILD_DIR) $(LDFLAGS) -o $@ $(O_FILES)$(LIBS) -lultra -lgoddard

$(ROM): $(ELF)
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $(@:.z64=.bin) -O binary
	$(N64CKSUM) $(@:.z64=.bin) $@

$(BUILD_DIR)/$(TARGET).objdump: $(ELF)
	$(OBJDUMP) -D $< > $@

else
$(EXE).elf: $(O_FILES) $(MIO0_FILES:.mio0=.o)  $(ULTRA_O_FILES) $(GODDARD_O_FILES)
	$(LD) -L $(BUILD_DIR) -o $@ $(O_FILES) $(ULTRA_O_FILES) $(GODDARD_O_FILES) $(LDFLAGS) 

ZEHNFLAGS := --name "sm64" --uses-lcd-blit 1 --240x320-support 1 --color-support 1 --32MB-support 0 --compress

$(EXE).tns: $(EXE).elf
	#arm-none-eabi-strip -s $^
	genzehn --input $^ --output $@.zehn $(ZEHNFLAGS)
	make-prg $@.zehn $@
	rm $@.zehn
endif



.PHONY: all clean distclean default diff test load libultra
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
