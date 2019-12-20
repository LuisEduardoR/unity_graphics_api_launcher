INPUT=./src/launcher.c
OUTPUT=SlenderBrody_Launcher.x86_64

make all:

	gcc `pkg-config --cflags gtk+-3.0` -o $(OUTPUT) $(INPUT) `pkg-config --libs gtk+-3.0`