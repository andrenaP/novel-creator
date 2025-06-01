# Компилятор
CXX := g++

# Пути
DIR_SRC := src
DIR_INC := src/include
DIR_BUILD := build

# Raylib (Windows)
RAY_INC := D:/libs/raylib-5.5_win64_mingw-w64/include
RAY_LIB := D:/libs/raylib-5.5_win64_mingw-w64/lib

# Поиск всех .cpp файлов рекурсивно, исключая main.cpp и mainrender.cpp
SRC := $(filter-out $(DIR_SRC)/main.cpp $(DIR_SRC)/mainrender.cpp, $(wildcard $(DIR_SRC)/**/*.cpp) $(wildcard $(DIR_SRC)/*.cpp))

# Объектные файлы для общих исходников
OBJ := $(patsubst $(DIR_SRC)/%.cpp, $(DIR_BUILD)/%.o, $(SRC))

# Объектные файлы для main и mainrender
OBJ_MAIN := $(DIR_BUILD)/main.o
OBJ_MAINRENDER := $(DIR_BUILD)/mainrender.o

# Целевые исполняемые файлы
ifeq ($(OS),Windows_NT)
    # Windows settings
    TARGET_EDITOR := $(DIR_BUILD)/editor.exe
    TARGET_RENDERER := $(DIR_BUILD)/renderer.exe
    LIBS = -lraylib -lgdi32 -lwinmm
else
    # Linux/Unix settings
    TARGET_EDITOR := $(DIR_BUILD)/editor.out
    TARGET_RENDERER := $(DIR_BUILD)/renderer.out
    LIBS = -lraylib
endif

# Главная цель: сборка обоих приложений
all: $(TARGET_EDITOR) $(TARGET_RENDERER)

# Сборка editor.exe/out
$(TARGET_EDITOR): $(OBJ) $(OBJ_MAIN)
	$(CXX) $(OBJ) $(OBJ_MAIN) -o $@ -L$(RAY_LIB) $(LIBS)

# Сборка renderer.exe/out
$(TARGET_RENDERER): $(OBJ) $(OBJ_MAINRENDER)
	$(CXX) $(OBJ) $(OBJ_MAINRENDER) -o $@ -L$(RAY_LIB) $(LIBS)

# Компиляция .cpp → .o для всех исходников
$(DIR_BUILD)/%.o: $(DIR_SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -I$(DIR_INC) -I$(RAY_INC) -c $< -o $@

# Очистка
clean:
	rm -rf $(DIR_BUILD)

# Фиктивные цели
.PHONY: all clean
