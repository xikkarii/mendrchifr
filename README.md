# МеандрШифр (MeandrCipher) v1.0

Система симметричного шифрования на основе меандровых последовательностей.
Проект по дисциплине «Проектная деятельность (Информационная безопасность)», МТУСИ.

## Технологии

- **C++17**
- **Qt 6** (Widgets, QPainter)
- **CMake** (сборка)
- Собственная реализация **SHA-256** (FIPS 180-4)

## Структура проекта

```
MeandrCipher/
├── CMakeLists.txt              # конфигурация сборки
├── README.md
├── src/
│   ├── main.cpp                # точка входа
│   ├── mainwindow.h/.cpp       # главное окно (GUI)
│   ├── meanderwidget.h/.cpp    # виджет визуализации меандра (QPainter)
│   ├── meander.h/.cpp          # генератор меандров
│   └── crypto.h/.cpp           # SHA-256, ключ, XOR-шифр, энтропия
└── docs/
    ├── Пояснительная_записка.md
    └── Руководство_пользователя.md
```

## Сборка

### Зависимости

- CMake ≥ 3.16
- Qt 6 (компонент Widgets)
- Компилятор с поддержкой C++17 (GCC, Clang, MSVC, MinGW)

### macOS (Homebrew)

```bash
brew install qt cmake
cmake -B build -S . -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
cmake --build build -j4
open build/MeandrCipher.app
```

### Linux

```bash
sudo apt install qt6-base-dev cmake g++   # Debian/Ubuntu
cmake -B build -S .
cmake --build build -j4
./build/MeandrCipher
```

### Windows (MinGW / MSVC)

```bat
cmake -B build -S . -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64
cmake --build build --config Release
build\Release\MeandrCipher.exe
```

## Краткое описание алгоритма

1. **Генерация меандра** порядка *n*: перебор непересекающихся паросочетаний
   с проверкой замкнутости (единственный цикл). Число меандров соответствует
   последовательности OEIS A005315.
2. **Ключ**: `K = SHA-256(последовательность обхода меандра)`.
3. **Гамма**: расширение ключа хэш-цепочкой `SHA-256(K‖счётчик)`.
4. **Шифр**: `C = P ⊕ gamma` (симметрично для шифрования и дешифрования).

Подробности — в [docs/Пояснительная_записка.md](docs/Пояснительная_записка.md).
