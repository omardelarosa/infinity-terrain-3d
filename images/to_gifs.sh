#!/bin/bash
FILES=*.mov

for f in $FILES
do
 gifify --colors 128 --compress 100 -o "GIFs/$f.gif" --resize 320:-1 $f
 # do something on $f
done