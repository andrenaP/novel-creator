cmake_minimum_required(VERSION 3.10)

project(NovelCreator)

set(CMAKE_CXX_STANDARD 17)

# Получаем путь к Raylib из переменной среды
if(NOT DEFINED ENV{RAYLIB_PATH})
    # Билд для линукса
else()
    set(RAYLIB_PATH "$ENV{RAYLIB_PATH}")
endif()

# Подключаем заголовки
include_directories(
    "${RAYLIB_PATH}/include"
    "libs"
)

# Ищем библиотеку Raylib
find_library(RAYLIB_LIB
    NAMES raylib libraylib.a
    PATHS "${RAYLIB_PATH}/lib"
    REQUIRED
)

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")

add_executable(editor ${SOURCES} ${HEADERS})

# Линкуем библиотеки
target_link_libraries(editor PRIVATE ${RAYLIB_LIB})

# Только на Windows линкуем winmm
if(WIN32)
    target_link_libraries(editor PRIVATE winmm)
endif()

