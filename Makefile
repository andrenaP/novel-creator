# Компилятор
CXX := g++

# Пути
DIR_SRC := tab/src
DIR_INC := tab/include
DIR_BUILD := build

# Raylib (Windows)
RAY_INC := D:/libs/raylib-5.5_win64_mingw-w64/include
RAY_LIB := D:/libs/raylib-5.5_win64_mingw-w64/lib

# Поиск всех .cpp файлов рекурсивно
SRC := $(wildcard $(DIR_SRC)/**/*.cpp) $(wildcard $(DIR_SRC)/*.cpp)

# Преобразуем пути: tab/src/xxx.cpp → build/xxx.o
OBJ := $(patsubst $(DIR_SRC)/%.cpp, $(DIR_BUILD)/%.o, $(SRC))

# Целевой исполняемый файл

# Detect platform
ifeq ($(OS),Windows_NT)
    # Windows settings
    TARGET := $(DIR_BUILD)/my_app.exe
    LIBS = -lraylib -lgdi32 -lwinmm
else
    # Linux/Unix settings
    TARGET := $(DIR_BUILD)/my_app.out
    LIBS = -lraylib
endif

# Главная цель
all: $(TARGET)

# Сборка .exe из object-файлов
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ -L$(RAY_LIB) $(LIBS)

# Компиляция .cpp → .o
$(DIR_BUILD)/%.o: $(DIR_SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -I$(DIR_INC) -I$(RAY_INC) -c $< -o $@

# Очистка
clean:
	rm -rf $(DIR_BUILD)
