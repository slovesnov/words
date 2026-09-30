SHELL := /bin/bash
# make -j $(nproc)
# make -j $(nproc) RUN=1
# make -j$(nproc) CONFIG=console RUN=1
# make clean
# options CONFIG=release|debug|console RUN=1 CLEAR=1

# ==============================================================================
# CASE-INSENSITIVE ARGUMENTS NORMALIZATION (CONFIG, RUN, and CLEAR)
# ==============================================================================
lowercase = a b c d e f g h i j k l m n o p q r s t u v w x y z
uppercase = A B C D E F G H I J K L M N O P Q R S T U V W X Y Z

define to_lower
$(strip $(eval __val := $1)\
  $(foreach c,$(join $(uppercase),$(addprefix _,$(lowercase))),\
    $(eval __val := $(subst $(word 1,$(subst _, ,$c)),$(word 2,$(subst _, ,$c)),$(__val))))\
  $(__val))
endef

# 1. Normalize CONFIG / config
ifdef config
    USER_CONFIG := $(config)
else
    USER_CONFIG := $(CONFIG)
endif

# Safe fallback definition check
USER_CONFIG ?= release
ifeq ($(strip $(USER_CONFIG)),)
    USER_CONFIG := release
endif

CONFIG := $(call to_lower,$(USER_CONFIG))

# Direct validation patch: force release if string evaluation ends up blank
ifeq ($(CONFIG),)
    CONFIG := release
endif

# 2. Normalize RUN / run
ifdef run
    USER_RUN := $(run)
else
    USER_RUN := $(RUN)
endif
RUN := $(call to_lower,$(USER_RUN))

# 3. Normalize CLEAR / clear
ifdef clear
    USER_CLEAR := $(clear)
else
    USER_CLEAR := $(CLEAR)
endif
CLEAR := $(call to_lower,$(USER_CLEAR))
# ==============================================================================
# END OF ARGUMENTS NORMALIZATION
# ==============================================================================

# 1. Screen clearing setup
ifdef MSYSTEM
    WIPE_CMD := clear
else ifdef OS
    WIPE_CMD := cmd /c cls
else
    WIPE_CMD := clear
endif

# 2. Compiler and flags settings
TARGET   := words
ASLOV    := ../../aslov/aslov/src/aslov
CXX      := g++
CXXFLAGS := -Wall -std=c++23 -Isrc/common -I$(ASLOV)
SRCS     := src/main.cpp $(wildcard src/*/*.cpp) $(ASLOV)/aslov.cpp

# GTK dependencies are included for both release and debug configurations
ifneq ($(filter release debug,$(CONFIG)),)
    CXXFLAGS   += $(shell pkg-config --cflags gtk+-3.0)
    LIBS       := $(shell pkg-config --libs gtk+-3.0) -lcurl -lpcre2-8
    SRCS       += $(ASLOV)/CheckNewVersion.cpp src/Frame.cpp
    EXTRA_OBJS := $(TARGET)/icon/app.o
endif

# Only validate CONFIG if we are NOT running 'make clean'
ifneq ($(filter clean,$(MAKECMDGOALS)),clean)
    ifeq ($(CONFIG), release)
        CXXFLAGS   += -O3 -DNDEBUG
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
        $(error Invalid CONFIG="$(USER_CONFIG)". Valid options are 'release', 'debug' or 'console')
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
