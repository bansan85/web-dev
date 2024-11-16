npm install -g npm@latest
npm install -g @angular/cli

git submodule update --init --recursive --depth=1

# Build generator with native llvm to prepare wrapper for clang-format
cmake -S utils -B build_utils_release -G Ninja -DCMAKE_BUILD_TYPE=Release
cd build_utils_release
npm install typescript
export PATH=$PATH:$(pwd)/node_modules/.bin
cmake --build . --parallel 8
cd ..

cmake -S utils -B build_utils_debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cd build_utils_debug
npm install typescript
export PATH=$PATH:$(pwd)/node_modules/.bin
cmake --build . --parallel 8
cd ..

# Build wasm-assembly wrapper
cd emsdk
git pull
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..

emcmake cmake -S . -B build_release -G "Ninja" -DCMAKE_BUILD_TYPE="Release"
cmake --build build_release --parallel $(nproc --all)
mkdir -p angular/src/assets
rm -f angular/src/assets/*
cp build_release/web* angular/src/assets/

emcmake cmake -S . -B build_debug -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" -DWITH_SANITIZE_ADDRESS=ON -DWITH_SANITIZE_UNDEFINED=ON
cmake --build build_debug --parallel $(nproc --all)
mkdir -p angular/src/assets
rm -f angular/src/assets/*
cp build_debug/web* angular/src/assets/

# Build Angular project
cd angular
npm install
ng build
ng serve --open --host 0.0.0.0
ng test --browsers=ChromeHeadless --watch=false

