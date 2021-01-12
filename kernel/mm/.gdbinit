set startup-with-shell off
set environment LD_PRELOAD /media/tjz/macHDD/Programming/OS2/kernel/mm/linux-malloc-build/malloc.so
#set environment LD_PRELOAD /home/tjz/mtest/linux-build/malloc.so
set follow-fork-mode child
set detach-on-fork off
file ls
#file gcc 
#set args -O0 -g -W -D LINUX_MALLOC -Wall -Wextra -shared -fPIC -I../../include kmalloc.c -o linux-build/malloc.so
