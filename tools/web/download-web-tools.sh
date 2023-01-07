#!/bin/bash

dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
pushd $dir > /dev/null

python_installed=0
python --version &> /dev/null
if [ $? -eq 0 ]; then
	python_installed=1
else
	python3 --version &> /dev/null
	if [ $? -eq 0 ]; then
		python_installed=1
	fi
fi

rm -rf emsdk
rm -rf static-server*

os=$OSTYPE

echo "[Installing web tools]"

if [ $python_installed -eq 0 ]; then
	echo "--- Python"
	if [[ "$os" == "linux-gnu"* ]]; then
		apt --version &> /dev/null
		if [ ! $? -eq 0 ]; then
			echo "Please install Python using your Linux distribution's package manager,"
			echo "then re-run this script."
			exit;
		else
			set -e
			sudo apt install python &> /dev/null
		fi
		echo "-- static-server"
		curl -L "https://github.com/badlogic/static-file-server/releases/download/1.0.0/static-server-linux" --output static-server
	elif [[ "$os" == "darwin"* ]]; then
		set -e
		xcode-select --install &> /dev/null
		echo "-- static-server"
		curl -L "https://github.com/badlogic/static-file-server/releases/download/1.0.0/static-server-macos" --output static-server
	elif [[ "$os" == "cygwin" ]] || [[ "$os" == "msys" ]] || [[ $(uname -r) =~ WSL ]]; then
		set -e
		curl -L https://www.python.org/ftp/python/3.11.1/python-3.11.1-amd64.exe --output python.exe >> ../log.txt 2>&1
		cmd "/C python.exe /passive InstallAllUsers=1 PrependPath=1 Include_test=0"
		rm python.exe
		export EMSDK_PYTHON=/c/Program\ Files/Python311/python.exe		
	else
		echo "Sorry, this template doesn't support $os"
		exit
	fi
else
	if [[ "$os" == "cygwin" ]] || [[ "$os" == "msys" ]] || [[ $(uname -r) =~ WSL ]]; then
		export EMSDK_PYTHON=$(which python)
	fi
fi

echo "--- static-server"
if [[ "$os" == "linux-gnu"* ]]; then
	curl -L "https://github.com/badlogic/static-file-server/releases/download/1.0.0/static-server-linux" --output static-server
	chmod a+x static-server
elif [[ "$os" == "darwin"* ]]; then
	curl -L "https://github.com/badlogic/static-file-server/releases/download/1.0.0/static-server-macos" --output static-server
	chmod a+x static-server
elif [[ "$os" == "cygwin" ]] || [[ "$os" == "msys" ]] || [[ $(uname -r) =~ WSL ]]; then
	curl -L "https://github.com/badlogic/static-file-server/releases/download/1.0.0/static-server-windows.exe" --output static-server.exe
fi

echo "--- Emscripten"
if [[ "$os" == "msys" ]]; then
	alias python="winpty python"
fi

git clone https://github.com/emscripten-core/emsdk.git  >> ../log.txt 2>&1
pushd emsdk > /dev/null
./emsdk install latest >> ../../log.txt 2>&1
./emsdk activate latest >> ../../log.txt 2>&1
popd  > /dev/null