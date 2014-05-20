#!/bin/bash

TOLUA=tolua++
SRC_DIR="../../src/test/bind"

echo "Bind all scripts API ..."
cd ${SRC_DIR}
${TOLUA} -o bind.cpp -H bind.hpp bind.pkg
cd - >/dev/null
echo "Bind all scripts API ... DONE"
