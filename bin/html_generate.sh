#!/bin/bash


readonly SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )
source "$SCRIPT_DIR/_init.sh"


OUTPUT_FORMAT="br"


while getopts cf: flag
do
    case "${flag}" in
        c) rm -f $OUTPUT_DIR/*;;
        f) OUTPUT_FORMAT=${OPTARG};;
    esac
done


function seal {
filename=$(basename -- "$1")
extension="${filename##*.}"
filename="${filename%.*}"

INPUT_HTML="${HTML_DIR}/${filename}.${extension}"
TEMP_HTML="${HTML_DIR}/_${filename}.${extension}"
TEMP_GZ="${HTML_DIR}/_${filename}.gz"
TEMP_BR="${HTML_DIR}/_${filename}.br"
TEMP_EXT="${HTML_DIR}/_${filename}.${OUTPUT_FORMAT}"
OUTPUT_DATA="${SKETCH_DATA_DIR}/${filename}.${OUTPUT_FORMAT}"

SED_BACKUP_EXT=".sed"


echo "original HTML"
ls -l $INPUT_HTML
sed -E 's/^[[:space:]]*//;s/[[:space:]]*$//;s/(\$\{.\})\.json/\1/' $INPUT_HTML \
    | tr -d '\r\n' > $TEMP_HTML

# force the modification date to prevent this diff only (date embed into the zip)
touch -t 200404040200 $TEMP_HTML

echo "minify HTML"
ls -l $TEMP_HTML
gzip -c -9 $TEMP_HTML > $TEMP_GZ
brotli -c -q 11 $TEMP_HTML > $TEMP_BR

#touch -t 200404040200 $TEMP_GZ
#touch -t 200404040200 $TEMP_BR

echo "compressed GZ"
ls -l $TEMP_GZ

echo "compressed BR"
ls -l $TEMP_BR

cp $TEMP_EXT $OUTPUT_DATA

rm "$TEMP_GZ"
rm "$TEMP_BR"
#rm "$TEMP_HTML"
}


cat <<EOT

portal
######
EOT
seal portal.html


cat <<EOT

index
######
EOT
seal index.html
