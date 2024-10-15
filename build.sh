#!/bin/bash
mkdir -p bin
eval cc src/main.c -framework IOKit -framework Cocoa -framework OpenGL $(pkg-config --libs --cflags raylib) -o bin/TowerDefense

./bin/TowerDefense
