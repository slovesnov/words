# mingw32-make -j%NUMBER_OF_PROCESSORS%

# 1. Настройки компилятора и флагов
ASLOV:=../../aslov/aslov/src/aslov
CXX      := g++
CXXFLAGS := -O2 -I$(ASLOV) -Isrc/common $(shell pkg-config --cflags gtk+-3.0)
LIBS     := $(shell pkg-config --libs gtk+-3.0)
TARGET   := a

# 2. Автоматический поиск файлов
# Находит все файлы .cpp в текущей директории
SRCS     := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard $(ASLOV)/*.cpp)
# Меняет расширение .cpp на .o для создания списка объектных файлов
OBJS     := $(SRCS:.cpp=.o)

# 3. Главный таргет (выполняется по команде `make`)
all: $(TARGET)

# Сборка исполняемого файла из объектных
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Правило для компиляции каждого файла .cpp в .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Вспомогательные команды
# Очистка проекта от временных файлов (запуск: `make clean`)
clean:
	rm -f $(OBJS) $(TARGET)

# Указываем, что эти цели не являются реальными файлами
.PHONY: all clean
