#!/bin/sh
set -eu
 
# Set the versions we will be using.
binutils_version="2.30"
gcc_version="7.3.0"
newlib_version="3.0.0"
 
# This script expects the target triplet (e.g. i786-pc-elf) as command line argument.
target=x86_64-elf
 
# The tools will be installed in ~/cross/$target.
prefix=/opt/x86-64k2
 
# First check whether the toolchain was already built on a previous run of this script.
if [ ! -d $prefix ]
then
	mkdir -p /tmp/toolchain
	cd /tmp/toolchain
 
	# Download gcc sources if they are not yet downloaded.
	if [ ! -f gcc-$gcc_version.tar.gz ]
	then
		wget -c -O gcc-$gcc_version.tar.gz ftp://ftp.gnu.org/gnu/gcc/gcc-$gcc_version/gcc-$gcc_version.tar.gz
		tar -xf gcc-$gcc_version.tar.gz
	fi
 
	# Download binutils sources if they are not yet downloaded.
	if [ ! -f binutils-$binutils_version.tar.gz ]
	then
		wget -c -O binutils-$binutils_version.tar.gz ftp://ftp.gnu.org/gnu/binutils/binutils-$binutils_version.tar.gz
		tar -xf binutils-$binutils_version.tar.gz
	fi
 
	# Optional: download newlib sources if they are not yet downloaded.
	if [ ! -f newlib-$newlib_version.tar.gz ]
	then
		wget -c -O newlib-$newlib_version.tar.gz ftp://sources.redhat.com/pub/newlib/newlib-$newlib_version.tar.gz
		tar -xf newlib-$newlib_version.tar.gz
	fi
 
	# Create build paths.
	mkdir -p /tmp/toolchain/build-binutils
	mkdir -p /tmp/toolchain/build-gcc
	mkdir -p /tmp/toolchain/build-newlib
 
	# Build binutils.
	cd /tmp/toolchain/build-binutils
	sudo rm -rf *
	/tmp/toolchain/binutils-$binutils_version/configure --target=$target --prefix=$prefix --disable-nls 2>&1
	make all 2>&1
	make install 2>&1
	sudo rm -rf *
 
	# Build gcc and libgcc.
	cd /tmp/toolchain/build-gcc
	/tmp/toolchain/gcc-$gcc_version/configure --target=$target --prefix=$prefix --disable-nls --enable-languages=c,c++ --enable-libstdcxx --without-headers 2>&1
	make all-gcc 2>&1
	make install-gcc 2>&1
	make all-target-libgcc 2>&1
	make install-target-libgcc 2>&1
 
	# Make sure that our cross compiler will be found by creating links.
	# Alternative: Add the $prefix/bin directory to your $PATH.
	sudo ln -s -f $prefix/bin/* /usr/local/bin/
 
	# Optional: Build newlib. This is necessary only for the next, also optional build step.
	cd /tmp/toolchain/build-newlib
	sudo rm -rf *
	/tmp/toolchain/newlib-$newlib_version/configure --target=$target --prefix=$prefix 2>&1
	make all 2>&1
	make install 2>&1
	sudo rm -rf *
 
	# Optional: Build libstdc++. This is done in order to install the freestanding headers for using the C++11, C++14, C++17 standards.
	cd /tmp/toolchain/build-gcc
	/tmp/toolchain/gcc-$gcc_version/configure --target=$target --prefix=$prefix --disable-nls --enable-languages=c,c++ --enable-libstdcxx --without-headers --with-newlib 2>&1
	make all-target-libstdc++-v3 2>&1
	make install-target-libstdc++-v3 2>&1
	sudo rm -rf *
fi
 
# Also if the cross compiler has not been freshly build, link it so that it will be found.
sudo ln -s -f $prefix/bin/* /usr/local/bin/
