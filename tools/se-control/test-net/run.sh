#!/bin/sh
set -eu
cd "$(dirname "$0")/../../.."
mkdir -p work/se-control-recovery
cc -std=c99 -Wno-unknown-escape-sequence -fsanitize=address,undefined -g \
 -I tools/se-control/test-net tools/se-control/test-net/transport.c \
 tools/se-control/vendor/nettcp.c -o work/se-control-recovery/test-net
work/se-control-recovery/test-net
