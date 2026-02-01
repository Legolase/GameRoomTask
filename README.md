# GameRootTask

## Dependencies
Dependencies:
* cmake
* clang-format-14 (optional for developing)

## Instruction for Linux

### Installation
```
git clone --recurse-submodules https://github.com/Legolase/GameRoomTask
```
After the current repository is built, the necessary submodules are also loaded.

Don't forget about the `--recurse-submodules` parameter. If you forgot, run the command:
```
git submodule update --init --recursive
```

### Building
* Executable file:
```
./build-executable.sh
```
* Tests:
```
./build-tests.sh
```
> All test cases are contained in [./test/test.cpp](https://github.com/Legolase/GameRoomTask/blob/master/test/test.cpp)

### Run
```
./run.sh [file.txt]
```

When compiling tests, you do not need to specify the input file.

### Formatting
```
./format.sh
```

Applies a style to files in `./include/*`, `./src/*` and `./test/*` 