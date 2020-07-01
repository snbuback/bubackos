#!/bin/bash
set -e

[ "$NO_GDB" == "1" ] && exit 0

if [ ! $# -eq 2 ]; then
    # When runs in running mode (debug fails to initialize)
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
EOF

# Add all modules
for mod in `cat src/modules/modules.list`; do
  module="build/bootloader/boot/${mod}.mod"
  addr=`readelf -S ${module} | grep .text | egrep -o '([0-9]{16})'`
  echo "add-symbol-file ${module} 0x${addr}" >> $GDB_SCRIPT_1
done

BREAKPOINTS=$(objdump -d --section=.text build/bootloader/boot/{kernel.elf,*.mod} | egrep -i 'xchg.*%bx,%bx' | cut -d: -f1 | sed -e 's/^[[:space:]]*//' -e "s/^/break *0x/")

cat >> $GDB_SCRIPT_1 <<EOF
target remote $ENDPOINT
break k_64bits
$BREAKPOINTS
continue
# After change to long, gdb will disconnect. So, debugging continue on next script
EOF

cat > $GDB_SCRIPT_2 <<EOF
set architecture i386:x86-64
disconnect
target remote $ENDPOINT
$BREAKPOINTS
# continue
EOF

echo "####### GDB_SCRIPT_1 #########"
cat $GDB_SCRIPT_1

echo "####### GDB_SCRIPT_2 #########"
cat $GDB_SCRIPT_2

echo "##############################"
gdb --init-command=$GDB_SCRIPT_1 --command=$GDB_SCRIPT_2

