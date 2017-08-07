#!/bin/sh

SETUP_FILE=$1
PACK_DIR=$2
BUILD_TIME=`date +"%s"`

if [ ! -r "$SETUP_FILE" ] || [ ! -d "$PACK_DIR" ];then
    echo "Usage `basename $0` <SETUP_FILE> <PACK_DIR>"
    exit 1;
fi

echo "PACK '$PACK_DIR' to '$SETUP_FILE'"

## pack name
PACK_FILENAME="$SETUP_FILE.tmp"

## do pack
cd "`dirname \"$PACK_DIR\"`"  \
    && tar -cJf "$PACK_FILENAME" "`basename \"$PACK_DIR\"`" \
        || exit 1

## get size
PACK_FILESIZE=$(stat -c%s "$PACK_FILENAME")

## add pack to setup
cat      "$PACK_FILENAME"            >> "$SETUP_FILE"
echo -en "\n\n"                      >> "$SETUP_FILE"
echo     "BUILD_TIME=$BUILD_TIME"    >> "$SETUP_FILE"
echo     "SIZE=$PACK_FILESIZE"       >> "$SETUP_FILE"

## remove remporary file
rm -f "$PACK_FILENAME"

## remove pack dir
rm -rf "$PACK_DIR"

