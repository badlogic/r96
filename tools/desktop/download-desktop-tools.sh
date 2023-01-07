#!/bin/bash
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
pushd $dir > /dev/null

os=$OSTYPE
echo "[Installing desktop tools]"

if [[ "$os" == "linux-gnu"* ]]; then
    ninja_url="https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip"
    apt --version &> /dev/null
    if [ ! $? -eq 0 ]; then
        echo "Please install the following packages using your Linux distribution's package manager,"
        echo "then re-run this script."
        echo
        echo "build-essential git gdb python3.11 cmake curl libx11-dev libxkbcommon-dev libgl1-mesa-dev"
        exit;
    else
        echo "sudo apt install build-essential git gdb python3.11 cmake curl libx11-dev libxkbcommon-dev libgl1-mesa-dev"
        sudo apt install build-essential git gdb python3.11 cmake curl libx11-dev libxkbcommon-dev libgl1-mesa-dev
    fi
elif [[ "$os" == "darwin"* ]]; then
    ninja_url="https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-mac.zip"
    
    clang --version &> /dev/null
    if [ ! $? -eq 0 ]; then
        xcode-select --install &> /dev/null
    fi

    brew --version &> /dev/null
    if [ ! $? -eq 0 ]; then
        echo "--- Homebrew"
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi

    cmake --version &> /dev/null
    if [ ! $? -eq 0 ]; then
        echo "--- CMake"
        brew install cmake >> ../log.txt 2>&1
    fi

elif [[ "$os" == "cygwin" ]] || [[ "$os" == "msys" ]] || [[ $(uname -r) =~ WSL ]]; then
    clang_url="https://github.com/mstorsjo/llvm-mingw/releases/download/20220906/llvm-mingw-20220906-msvcrt-x86_64.zip"
    cmake_url="https://github.com/Kitware/CMake/releases/download/v3.25.1/cmake-3.25.1-windows-x86_64.msi"
    ninja_url="https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip"

    rm -rf ninja
    rm -rf clang

    cmake --version &> /dev/null
    if [ ! $? -eq 0 ]; then
        echo "--- CMake"
        curl -L $cmake_url --output cmake.msi
        cmd "/C install-cmake.bat" &> ../log.txt 2>&1
        rm -rf cmake.exe
    fi

    echo "--- Clang"
    curl -L $clang_url --output clang.zip
    unzip -o clang.zip >> ../log.txt 2>&1
    rm clang.zip
    mv llvm-mingw-20220906-msvcrt-x86_64 clang
else
    echo "Sorry, OS $os not supported"
    exit
fi

echo "--- Ninja"
mkdir -p ninja
pushd ninja > /dev/null
curl -L $ninja_url --output ninja.zip
unzip -o ninja.zip >> ../log.txt 2>&1
rm ninja.zip    
popd > /dev/null

echo "--- VS Code C/C++ extensions"
if [[ $(code --version) ]]; then
    code --install-extension llvm-vs-code-extensions.vscode-clangd --install-extension ms-vscode.cmake-tools --install-extension ms-vscode.cpptools --install-extension webfreak.debug &> ../log.txt 2>&1
else
    echo "WARNING: could not find 'code' on path. Could not install VS Code extensions!"
    echo "         1. Open VS Code, CTRL+SHIFT+P (or CMD+SHIFT+P on macOS)"
    echo "         2. Enter 'install code command path' and press enter"
    echo "         3. Run the following command on the command line:"
    echo
    echo "         code --install-extension llvm-vs-code-extensions.vscode-clangd --install-extension ms-vscode.cmake-tools --install-extension ms-vscode.cpptools --install-extension webfreak.debug"
fi

popd > /dev/null