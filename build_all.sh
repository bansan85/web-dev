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
ng test --browsers=ChromeHeadless --watch=false
ng serve --open --host 0.0.0.0

cmake -S utils -B build_utils_iwyu -G "Ninja" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
/usr/lib/llvm/18/bin/iwyu_tool.py -p build_utils_iwyu utils/*.cpp > iwyu_tool.log
run-clang-tidy -p build_utils_iwyu > clang-tidy-utils.log

emcmake cmake -S . -B build_iwyu -G "Ninja" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build_iwyu --parallel 8
/usr/lib/llvm/18/bin/iwyu_tool.py -p build_iwyu src/*.cpp > iwyu_tool.log
run-clang-tidy -p build_iwyu src/*.cpp > clang-tidy-utils.log
