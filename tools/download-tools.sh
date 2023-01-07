#!/bin/bash
set -e

dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
pushd $dir > /dev/null
rm -rf log.txt
desktop/download-desktop-tools.sh
echo
web/download-web-tools.sh
popd > /dev/null