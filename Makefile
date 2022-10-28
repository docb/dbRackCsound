# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
include $(RACK_DIR)/arch.mk
ifdef ARCH_MAC
  //FLAGS += -I lib/mac/CsoundLib64.framework/Versions/6.0/Headers
  FLAGS += -I lib/linux/csound
  LDFLAGS += lib/mac/libCsoundLib64.a lib/mac/libsndfile.a
else ifdef ARCH_WIN
  FLAGS += -I lib/linux/csound
  LDFLAGS += -Wl,--export-all-symbols lib/win/libcsound64.a lib/win/libsndfile.a -lws2_32
else
  FLAGS += -I lib/linux/csound
  LDFLAGS += lib/linux/libcsound64.a lib/linux/libsndfile.a
endif

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
