#!/bin/bash
set -e
set -x

if [ ! $# -eq 2 ]; then
    echo "Missing gdb endpoint"
    exit 1
fi


KERNEL_IMAGE=$1
ENDPOINT=$2
GDB_SCRIPT_1=`mktemp`
GDB_SCRIPT_2=`mktemp`

# The disconnect and reconnect trick is necessary. More info: https://wiki.osdev.org/QEMU_and_GDB_in_long_mode

cat > $GDB_SCRIPT_1 <<EOF
set pagination off
set non-stop off
define exit
  kill
  set confirm off
  quit
end

set architecture i386:x86-64:intel
file $KERNEL_IMAGE
target remote $ENDPOINT
break native_boot
continue
# After change to long, gdb will disconnect. So, debugging continue on next script
EOF

BREAKPOINTS=$(objdump -t --section=.text $KERNEL_IMAGE | grep kernel_debug | egrep -io '([a-z0-9_.]*kernel_debug[a-z0-9_.]*)' | sed "s/^/break /")

cat > $GDB_SCRIPT_2 <<EOF
set architecture i386:x86-64
disconnect
target remote $ENDPOINT
$BREAKPOINTS
continue
EOF

gdb --init-command=$GDB_SCRIPT_1 --command=$GDB_SCRIPT_2

