#!/bin/bash


readonly SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )
source "$SCRIPT_DIR/_init.sh"


readonly INPUT_DIR="$BASE_DIR/sketch_WMaster/dump/"

INPUT_HOST="http://esp8266"
INPUT_USER=""
INPUT_PASSWORD=""


while getopts h:p:u: flag
do
    case "${flag}" in
        h) INPUT_HOST=${OPTARG};;
        p) INPUT_PASSWORD=${OPTARG};;
        u) INPUT_USER=${OPTARG};;
    esac
done

function upload_config {
filename=$(basename -- "$1")
extension="${filename##*.}"
filename="${filename%.*}"

echo "POST ${INPUT_HOST}/cfg/${filename}"

curl \
    --request POST \
    --url "${INPUT_HOST}/cfg/${filename}" \
    --insecure \
    --header 'Content-Type: application/json' \
    --data @"${SKETCH_DUMP_DIR}/$1" \
    --silent \

}


echo ls -lh $SKETCH_DUMP_DIR
ls -lh $SKETCH_DUMP_DIR


cat <<EOT


upload global.json
######
EOT
upload_config g.json


cat <<EOT


upload wifi.json
######
EOT
upload_config w.json


cat <<EOT


upload device.json
######
EOT
upload_config d.json


cat <<EOT


upload rule.json
######
EOT
upload_config r.json


cat <<EOT


upload transport.json
######
EOT
upload_config t.json


cat <<EOT


reboot
######
DELETE ${INPUT_HOST}/cfg/reboot
EOT
curl \
    --request DELETE \
    --url "${INPUT_HOST}/cfg/reboot" \
    --insecure \
    --basic \
    --user "${INPUT_USER}:${INPUT_PASSWORD}" \
    --silent \
    
