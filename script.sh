rm -r build
mkdir build
cd build
../init --plat pc99 --tut hello-camkes-1 --solution
#/usr/local/bin/cmake -DCMAKE_TOOLCHAIN_FILE=../kernel/gcc.cmake -G Ninja -DTUT_BOARD=pc -DTUT_ARCH=x86_64 -DTUTORIAL=hello-camkes-1 -DBUILD_SOLUTIONS=TRUE ..
#ninja -j 2
