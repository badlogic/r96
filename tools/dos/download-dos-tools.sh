#!/bin/bash
apt --version &> /dev/null
apt_missing=$?

set -e

dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
pushd $dir > /dev/null

rm -rf gdb
rm -rf djgpp
rm -rf dosbox-x

gdb_url=""
djgpp_url=""
dosbox_url=""
os=$OSTYPE

if [[ "$os" == "linux-gnu"* ]]; then
    gdb_url="https://github.com/badlogic/gdb-7.1a-djgpp/releases/download/gdb-7.1a-djgpp/gdb-7.1a-djgpp-linux.zip";
    djgpp_url="https://github.com/andrewwutw/build-djgpp/releases/download/v3.3/djgpp-linux64-gcc1210.tar.bz2";
    dosbox_url="https://github.com/badlogic/dosbox-x/releases/download/dosbox-x-gdb-v0.84.5/dosbox-x-0.84.5-linux.zip";
elif [[ "$os" == "darwin"* ]]; then
    gdb_url="https://github.com/badlogic/gdb-7.1a-djgpp/releases/download/gdb-7.1a-djgpp/gdb-7.1a-djgpp-macos-x86_64.zip";
    djgpp_url="https://github.com/andrewwutw/build-djgpp/releases/download/v3.3/djgpp-osx-gcc1210.tar.bz2";
    dosbox_url="https://github.com/badlogic/dosbox-x/releases/download/dosbox-x-gdb-v0.84.5/dosbox-x-macosx-x86_64-20221223232510.zip";
elif [[ "$os" == "cygwin" ]] || [[ "$os" == "msys" ]] || [[ $(uname -r) =~ WSL ]]; then
    gdb_url="https://github.com/badlogic/gdb-7.1a-djgpp/releases/download/gdb-7.1a-djgpp/gdb-7.1a-djgpp-windows.zip";
    djgpp_url="https://github.com/andrewwutw/build-djgpp/releases/download/v3.3/djgpp-mingw-gcc1210-standalone.zip";
    dosbox_url="https://github.com/badlogic/dosbox-x/releases/download/dosbox-x-gdb-v0.84.5/dosbox-x-mingw-win64-20221223232734.zip";
else
    echo "Sorry, this template doesn't support $os"
    exit
fi

echo "[Installing DOS tools]"
echo "--- GDB"
mkdir -p gdb
pushd gdb > /dev/null
curl -L $gdb_url --output gdb.zip
unzip -o gdb.zip >> ../../log.txt 2>&1
rm gdb.zip >> ../../log.txt 2>&1
popd > /dev/null

echo "--- DJGPP"
if [[ "$os" == "cygwin" ]] || [[ "$os" == "msys" ]] || [[ $(uname -r) =~ WSL ]]; then
    curl -L $djgpp_url --output djgpp.zip
    unzip djgpp.zip >> ../log.txt 2>&1
    rm djgpp.zip
else
    curl -L $djgpp_url --output djgpp.tar.bz2
    tar xf djgpp.tar.bz2
    rm djgpp.tar.bz2
fi

echo "--- DOSBox-x"
curl -L $dosbox_url --output dosbox.zip
unzip -o dosbox.zip >> ../log.txt 2>&1
rm dosbox.zip

if [[ "$os" == "linux-gnu"* ]]; then
    chmod a+x gdb/gdb
    chmod a+x djgpp/bin/*
    chmod a+x djgpp/i586-pc-msdosdjgpp/bin/*
    chmod a+x dosbox-x/dosbox-x-sdl1
    ln -s $(pwd)/dosbox-x/dosbox-x-sdl1 dosbox-x/dosbox-x
    apt --version &> /dev/null
	if [ ! $apt_missing -eq 0 ]; then
        echo " Please install the following packages using your Linux distribution's "
        echo " package manager:"
        echo
        echo " libncurses5 libfl-dev libslirp-dev libfluidsynth-dev"
    else
        sudo apt install libncurses5 libfl-dev libslirp-dev libfluidsynth-dev &> /dev/null
    fi
elif [[ "$os" == "darwin"* ]]; then
    chmod a+x gdb/gdb
    chmod a+x djgpp/bin/*
    chmod a+x djgpp/i586-pc-msdosdjgpp/bin/*
    chmod a+x dosbox-x/dosbox-x.app/Contents/MacOS/dosbox-x
    ln -s $(pwd)/dosbox-x/dosbox-x.app/Contents/MacOS/dosbox-x dosbox-x/dosbox-x
elif [[ "$os" == "cygwin" ]] || [[ "$os" == "msys" ]] || [[ $(uname -r) =~ WSL ]]; then
    rm -rf "COPYING"
    mv mingw-build/mingw dosbox-x
    rm -rf mingw-build
fi

popd > /dev/null