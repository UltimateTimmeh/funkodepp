# FunKodePP

A repository in which I have fun with C++ code!

## olcPixelGameEngine applications

### Setup

```bash
sudo apt update
sudo apt install build-essential libglu1-mesa-dev libpng-dev
```

### Building

#### For desktop use

```bash
g++ -o ExampleProgram ExampleProgram.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17 -O3
```

#### For browser use

Compiling applications for the browser is done using Emscripten. These instructions assume
you have it installed.

```bash
cd <Emscripten root directory>
source ./emsdk_env.sh
cd <ExampleProgram directory>
em++ -std=c++17 -O3 -s ALLOW_MEMORY_GROWTH=1 -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2 -s USE_LIBPNG=1 ExampleProgram.cpp -o example.html --preload-file ./assets
```

You can then test in your browser with:

```bash
emrun example.html
```

### Executing

```bash
./ExampleProgram
```
