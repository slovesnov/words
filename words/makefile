# mingw32-make -j $(nproc)
# mingw32-make -j$(nproc) CONFIG=console RUN=1
# mingw32-make clean
# cd e:/slovesno/words/words
CONFIG ?= release

# 1. Compiler and flags settings
TARGET   := words
ASLOV    := ../../aslov/aslov/src/aslov
CXX      := g++
#-O2 -Wall -Wextra -Wpedantic
CXXFLAGS := -O3 -DNDEBUG -Wall -std=c++23 -Isrc/common -I$(ASLOV)
SRCS     := src/main.cpp $(wildcard src/*/*.cpp) $(ASLOV)/aslov.cpp

ifeq ($(CONFIG), release)
	CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0)
	LIBS     := $(shell pkg-config --libs gtk+-3.0) -lcurl
	SRCS     += $(ASLOV)/CheckNewVersion.cpp src/Frame.cpp
	ifeq ($(OS),Windows_NT)
	#	LDFLAGS += -mwindows
	endif
	EXTRA_OBJS := $(TARGET)/icon/app.o
else ifeq ($(CONFIG), console)
	CGI    	 := ../../cgi/cgi/src/cgi
	CXXFLAGS += -DNOGTK=0 -I$(CGI)
	SRCS     += $(CGI)/cgi.cpp
	LDFLAGS = -liconv
else
	$(error Invalid CONFIG="$(CONFIG)". Valid options are 'release' or 'console')
endif

# Replaces .cpp extensions with .o to generate the object files list
OBJS     := $(SRCS:.cpp=.o)

# 3. Main target (executed by default via `make`)
all: $(TARGET)

ifneq ($(RUN),)
	@echo "--- Running application ---"
	./$(TARGET).exe
endif

# Links the executable from the object files
$(TARGET): $(OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(EXTRA_OBJS) $(LDFLAGS) $(LIBS)

# Rule to compile each .cpp file into a .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Auxiliary commands
# Cleans the project by removing temporary build files (run: `make clean`)
clean:
	rm -f $(OBJS) $(TARGET).exe

# Declares targets that do not represent actual files on disk
.PHONY: all clean
