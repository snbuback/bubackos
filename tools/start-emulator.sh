#!/bin/bash
set -e
set -x

if [ "$#" -eq 0 ]; then
    echo "Missing iso file"
    exit 1
fi
ISO=$1

KERNEL_IMAGE=$2
echo "args=$#"

QEMU_ARGS="-m 128 -cpu Skylake-Server -boot order=d -cdrom $ISO -no-reboot \
-no-shutdown -usb -device usb-tablet -icount auto,sleep=on \
-show-cursor -d guest_errors,unimp,page,in_asm,int,pcall"

# if there is the kernel image, means we are running in debug mode
if [ ! -z "$KERNEL_IMAGE" ]; then
    QEMU_ARGS="$QEMU_ARGS -S"
fi

TMUX_CONF=`mktemp`

# script that waits for the qemu dies, and doesn't consume the standard input.
# This allows qemu monitor have stdin in another terminal
NO_INPUT='bash -c "sleep 1; tail --pid=`pgrep qemu-system` -f /dev/null"'

cat > $TMUX_CONF <<EOF
# pts/1
new-session -n run -x "80" -y "25" \
	qemu-system-x86_64 $QEMU_EXTRA_ARGS $QEMU_ARGS -serial file:/dev/pts/3 -s -display curses -monitor /dev/pts/4 -D /dev/pts/4
# pts/2
split-window -v -d \
	./tools/gdb-startup.sh $KERNEL_IMAGE localhost:1234
# pts/3
split-window -h -d \
	$NO_INPUT
# pts/4
new-window -d -n tools \
	$NO_INPUT
# pts/5
split-window -h -d -t tools \
	bash -c 'make dump-asm | less'

# ensure the qemu have 25 lines. Doesn't work every time, why?
resize-pane -t 0 -y 25 -x 80

# select gdb
select-pane -t 2

set mouse on

display-panes

EOF

tmux -2 start-server \; source-file $TMUX_CONF
exit 0
