cd emsdk
git pull
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..
emcmake cmake -S llvm/llvm -B llvm-build -G "Ninja Multi-Config" -DLLVM_TARGETS_TO_BUILD=
cmake --build llvm-build --target LLVMDemangle --config Release --parallel 8

