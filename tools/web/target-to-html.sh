#!/bin/bash
html=$1
html=${html#r96_}
html=${html%.js}
echo $html.html