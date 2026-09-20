# mingw32-make -j $(nproc)
# mingw32-make -j$(nproc) CONFIG=console RUN=1
# mingw32-make clean
# options CONFIG=release|console RUN=1 CLEAR=1

CONFIG ?= release

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
CXXFLAGS := -O3 -DNDEBUG -Wall -std=c++23 -Isrc/common -I$(ASLOV)
SRCS     := src/main.cpp $(wildcard src/*/*.cpp) $(ASLOV)/aslov.cpp

ifeq ($(CONFIG), release)
    CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0)
    LIBS     := $(shell pkg-config --libs gtk+-3.0) -lcurl
    SRCS     += $(ASLOV)/CheckNewVersion.cpp src/Frame.cpp
    EXTRA_OBJS := $(TARGET)/icon/app.o
else ifeq ($(CONFIG), console)
    CGI      := ../../cgi/cgi/src/cgi
    CXXFLAGS += -DNOGTK=0 -I$(CGI)
    SRCS     += $(CGI)/cgi.cpp
    LDFLAGS  := -liconv
else
    $(error Invalid CONFIG="$(CONFIG)". Valid options are 'release' or 'console')
endif

OBJS     := $(SRCS:.cpp=.o)

.PHONY: all clean wipe run_app

# Главная цель теперь зависит от 'wipe', затем выводит текст сборки, затем собирает проект и запускает
all: wipe
	@echo "Starting compilation in $${MSYSTEM:-$(OS)}..."
	@$(MAKE) $(TARGET) --no-print-directory
	@$(MAKE) run_app --no-print-directory

wipe:
ifeq ($(CLEAR),1)
	@$(WIPE_CMD)
endif

# Отдельная цель для запуска приложения, которая срабатывает только при RUN=1
run_app:
ifneq ($(RUN),)
	@echo "--- Running application ---"
	./$(TARGET).exe
endif

# Сборка исполняемого файла
$(TARGET): $(OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(EXTRA_OBJS) $(LDFLAGS) $(LIBS)

# Компиляция каждого отдельного файла .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET).exe
