#!/bin/bash


readonly BIN_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )
readonly BASE_DIR=$( dirname $BIN_DIR)

readonly HTML_DIR="$BASE_DIR/web/html/"
readonly SKETCH_DIR="$BASE_DIR/sketch_wiscan/"
readonly SKETCH_DATA_DIR="$SKETCH_DIR/data/"
readonly SKETCH_DUMP_DIR="$SKETCH_DIR/dump/"

readonly DEFAULT_DNAME="wiscan"
readonly DEFAULT_AP_IP="192.168.4.1"
