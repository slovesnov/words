SHELL := /bin/bash
# make -j $(nproc)
# make -j $(nproc) RUN=1
# make -j$(nproc) CONFIG=console RUN=1
# make clean
# options CONFIG=release|debug|console|finalrelease RUN=1 CLEAR=1

get_low_val = $(strip $(shell \
  match=$$(echo "$(MAKEOVERRIDES)" | grep -Ei '\b$1=' | head -n 1 | sed -E 's/.*$1=([^ ]*).*/\1/I'); \
  [ -z "$$match" ] && match=$$(env | grep -Ei '^$1=' | head -n 1 | cut -d= -f2); \
  echo "$${match,,}"))

CONFIG := $(call get_low_val,config)
ifeq ($(CONFIG),)
    CONFIG := release
endif

RUN := $(call get_low_val,run)
CLEAR := $(call get_low_val,clear)

# 1. Screen clearing setup
WIPE_CMD := clear

# 2. Compiler and flags settings
TARGET   := words
ASLOV    := ../../aslov/aslov/src/aslov
CXX      := g++
CXXFLAGS := -Wall -std=c++23 -Isrc/common -I$(ASLOV)
SRCS     := src/main.cpp $(wildcard src/*/*.cpp) $(ASLOV)/aslov.cpp

# GTK dependencies are included for release, debug, AND finalrelease configurations
ifneq ($(filter release debug finalrelease,$(CONFIG)),)
    CXXFLAGS   += $(shell pkg-config --cflags gtk+-3.0)
    LIBS       := $(shell pkg-config --libs gtk+-3.0) -lcurl -lpcre2-8
    SRCS       += $(ASLOV)/CheckNewVersion.cpp src/Frame.cpp
    EXTRA_OBJS := $(TARGET)/icon/app.o
endif

# Only validate CONFIG if we are NOT running 'make clean'
ifneq ($(filter clean,$(MAKECMDGOALS)),clean)
    ifeq ($(CONFIG), release)
        CXXFLAGS   += -O3 -DNDEBUG
    else ifeq ($(CONFIG), finalrelease)
        CXXFLAGS   += -O3 -DNDEBUG
        LDFLAGS    += -mwindows
    else ifeq ($(CONFIG), debug)
        # Debug mode with full stack trace capability and disabled optimizations
        CXXFLAGS   += -g -O0
        LDFLAGS    += -lstdc++exp
    else ifeq ($(CONFIG), console)
        CGI        := ../../cgi/cgi/src/cgi
        CXXFLAGS   += -DNOGTK=0 -I$(CGI)
        SRCS       += $(CGI)/cgi.cpp
        LDFLAGS    := -liconv
    else
        $(error Invalid CONFIG="$(CONFIG)". Valid options are 'release', 'finalrelease', 'debug' or 'console')
    endif
endif

OBJS     := $(SRCS:.cpp=.o)

.PHONY: all clean wipe run_app

# Main target: clears the screen if requested, builds the binary, and runs it
all: wipe
	@$(MAKE) $(TARGET) --no-print-directory
	@$(MAKE) run_app --no-print-directory

# Target for clearing the terminal screen, triggered if CLEAR=1, clear=true, etc.
wipe:
ifneq ($(filter 1 true yes clear,$(CLEAR)),)
	@$(WIPE_CMD)
endif

# Target for executing the application, triggered only if RUN=1, run=true, etc.
run_app:
ifneq ($(filter 1 true yes run,$(RUN)),)
	./$(TARGET).exe
endif

# Linking the final executable
$(TARGET): $(OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(EXTRA_OBJS) $(LDFLAGS) $(LIBS)

# Compiling individual source files .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET).exe
