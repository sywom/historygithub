#!/bin/bash

PROJECT_NAME="game"
SFML_DIR="/home/shuvi/SFML"
BUILD_DIR="Release"
COMPILER="x86_64-w64-mingw32-g++"
 
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "Building..."

$COMPILER $(find . -name "*.cpp") -o "$BUILD_DIR/$PROJECT_NAME.exe" \
-I"$SFML_DIR/include" \
-L"$SFML_DIR/lib" \
-lsfml-graphics -lsfml-window -lsfml-system -mwindows 

if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

echo "Copying assets..."
cp -r fonts images other Release/

echo "Copying SFML DLLs..."
cp "$SFML_DIR/bin"/sfml-graphics-2.dll "$BUILD_DIR/" 2>/dev/null
cp "$SFML_DIR/bin"/sfml-window-2.dll "$BUILD_DIR/" 2>/dev/null
cp "$SFML_DIR/bin"/sfml-system-2.dll "$BUILD_DIR/" 2>/dev/null

echo "Copying MinGW runtime DLLs..."
cp $(x86_64-w64-mingw32-g++ -print-file-name=libgcc_s_seh-1.dll) "$BUILD_DIR/"
cp $(x86_64-w64-mingw32-g++ -print-file-name=libstdc++-6.dll) "$BUILD_DIR/"
cp $(x86_64-w64-mingw32-g++ -print-file-name=libwinpthread-1.dll) "$BUILD_DIR/"

echo "DONE"
