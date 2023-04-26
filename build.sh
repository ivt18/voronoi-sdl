if [ ! -d "build" ]; then
    mkdir build
fi
g++ ./src/main.cpp -g -pedantic -o ./build/main -lSDL2
./build/main
