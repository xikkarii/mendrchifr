cmake_minimum_required(VERSION 3.16)

project(MeandrCipher VERSION 1.0 LANGUAGES CXX)

# ── Стандарт языка C++17 (требование ТЗ, раздел 9) ──
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Автоматическая обработка мета-объектов Qt ──
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Поиск Qt 6 (Widgets) ──
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ── Исходные файлы ──
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/meanderwidget.cpp
    src/meander.cpp
    src/crypto.cpp
)

set(HEADERS
    src/mainwindow.h
    src/meanderwidget.h
    src/meander.h
    src/crypto.h
)

# ── Исполняемый файл ──
qt_add_executable(MeandrCipher ${SOURCES} ${HEADERS})

target_link_libraries(MeandrCipher PRIVATE Qt6::Widgets)

# На macOS — оформить как .app bundle; на Windows — оконное приложение.
set_target_properties(MeandrCipher PROPERTIES
    MACOSX_BUNDLE ON
    WIN32_EXECUTABLE ON
)
