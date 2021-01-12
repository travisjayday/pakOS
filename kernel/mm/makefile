pwd=$(shell pwd)
build=linux-malloc-build

linux-malloc-test: clean
	mkdir $(build)
	gcc -O3 -g -W -D LINUX_MALLOC -Wall -Werror -Wextra -shared -fPIC -I../../include kmalloc.c -o $(build)/malloc.so
	@echo ""
	export LD_PRELOAD=$(pwd)/$(build)/malloc.so
	@echo ""
	@echo "Run the above export command to use this mallocator"

basic: 
	gcc -O0 -g -W -D LINUX_MALLOC -Wall -Wextra -shared -fPIC basic.c -o $(build)/malloc.so

clean: 
	rm -rf $(build)
	
