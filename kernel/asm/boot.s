.set F_ALIGN,   1<<0                # flag to align loaded kernel on 4kb page boundary
.set F_MEMINFO, 1<<1                # flag to pass memory mapping info to OS from bios
.set FLAGS,     F_ALIGN | F_MEMINFO # flagfield for multiboot
.set MAGIC,     0x1badb002          # multiboot v1 magic
.set CHECKSUM,  -(MAGIC + FLAGS)    # multiboot v1 checksum
.set KOFF, 0xC0000000


# multiboot v1 compliant header
.section .multiboot.data, "ax", @progbits
.align 4                            # multiboot header needs to be 32bit aligned
.long MAGIC
.long FLAGS         
.long CHECKSUM  

# zero out space for 16kb stack 
.section .bss
.align 16
stack_bottom: 
.skip 26384
stack_top: 
.align 4096

.global pde_table
pde_table:
.skip 4096

.global bios_mmap_addr
bios_mmap_addr:
.long 0
.global bios_mmap_len
bios_mmap_len:
.long 0

.section .data
.align 4096
pte_0:
.skip 4096
.align 4096
pte_768: 
.skip 4096
.align 4
meminfo_struct:
.skip 1024

#define hw_addr $0xc0000

# default entry point specified by linker. multiboot will parse the ELF header
# and call _start for entry. 
.section .multiboot.text
.global _start
.type _start, @function
_start: 

    /* We are now in 32-bit protected mode, interrupts, paging disabled. */

    movl    (%ebx), %eax           # move FLAGS into eax
    or      %eax, 1 << 6           # check if mminfo struct is valid
    test    %eax, %eax             # if flag not set, panic
    je      panic

    # get pointer of memory info from multiboot struct 
    movl    44(%ebx), %esi          # multiboot mmap struct length
    movl    48(%ebx), %ecx          # multiboot mmap struct address

    xor     %eax, %eax
    mov     $(meminfo_struct - KOFF), %edx
copy_meminfo: 
    mov     (%ecx, %eax, 4), %ebx
    mov     %ebx, (%edx, %eax, 4) 
    inc     %eax
    cmp     %esi, %eax
    jl      copy_meminfo 

    add     $KOFF, %edx
    movl    %esi, bios_mmap_len - KOFF
    movl    %edx, bios_mmap_addr - KOFF

    # We allocate the page directory using frame allocator and a sigle page entry which should be
    # enough for our kernel (1 page entry maps 4mb of virtual addresses). 
    # The remainder of the page directory will be empty. When we request memory
    # at empty, PF will occur, we allocate another page entry. 

    # Here we map: 
    # map 0x00100000 - 0x00101000 --> 0x00100000 - 0x00101000
    # map 0x00100000 - 0x003fffff --> 0xc0100000 - 0xc03fffff
    
    # 1. Allocate PDE table. 
    # 2. Allocate one PTE which will map our kernel. 
    #   -> pde[768][0] => 0x100000
    #   -> pde[768][1] => 0x101000 
    #   -> pde[768][2] => 0x102000
    #   -> ...
    #  -> pde[768][1023] = 0x500000
    # 3. Allocate one PTE which will map our PDE. 

    # create pde table entry for 0 (temp kernel), 768 (higher kernel), 1023 (pde self-map)
    # the upper 10 bits of a virtual addres serve as index into this table. 768 --> 0xc0000000
    mov     $(pde_table - KOFF), %edi   # move pde into edi. destination is edi

    # move poitner to table 768
    mov     $(pte_768 - KOFF),   %esi   # get hw address of table
    or      $0x3, %esi                  # set preset, r/w, user bits
    mov     %esi, (768 * 4)(%edi)       # move table 768 pointer to index 768 into pde 

    # move pointer to table 0 
    mov     $(pte_0 - KOFF), %esi
    or      $0x7, %esi
    mov     %esi, (%edi)                # move table 0 pointer at index 0 into pde

    # move pointer to table 1023
    mov     $(pde_table - KOFF), %esi   # get hw address of pde for self mapping
    or      $0x3, %esi                  
    mov     %esi, (1023 * 4)(%edi)      # move table 1023 pointer at index 1023 into pde

    # edit table 0 entry to map kernel 
    mov     $256, %eax                  # start at index 256 (1MB above 0xc0000000)
    mov     $(pte_0 - KOFF), %edi       # get hw address of table
    mov     $0x00100000, %ebx           # move target address to map
    or      $0x3, %ebx                  # set rw flags
    mov     %ebx, (%edi, %eax, 4)       # move target into index 256

    # edit table 768 entry
    mov     $(pte_768 - KOFF), %edi     # get hw address of table
    mov     $0x100000, %edx             # hw address to map 
fill_loop: 
    or      $0x3, %edx
    mov     %edx, (%edi, %eax, 4)       # move target into index eax 
    add     $0x1000, %edx               # next page 
    inc     %eax                        # next page index
    cmp     $1024, %eax                 # we end at index 1023 (3MB above 0xc0000000)
    jl      fill_loop

    # map 0xb8000 to 0xc00ff000 (the page before kernel code start)
    # edit table 0 entry to map VGA buffer
    mov     $255, %eax                  # start at index 256 (1MB above 0xc0000000)
    mov     $(pte_768 - KOFF), %edi     # get hw address of table
    mov     $0x000b8000, %ebx           # move target address to map
    or      $0x3, %ebx                  # set rw flags
    mov     %ebx, (%edi, %eax, 4)       # move target into index 256

    mov     $(pde_table - KOFF), %edi   # move pde into cr3
    mov     %edi, %cr3
    mov     %cr0, %eax
    or      $0x80000000, %eax           # set paging flags
    mov     %eax, %cr0                  # enable paging

    lea     pre_init_kernel, %ecx       # long jump
    jmp     *%ecx 

.section .text
pre_init_kernel: 
    mov     $stack_top, %esp        # setup stack pointer

    mov     $0, pde_table + 0       # remove lower kernel mapping
    movl    %cr3, %ecx              # reload paging
    movl    %ecx, %cr3

    call    kernel_init

    # kernel entry
    # call kernel_main

.global ___isr
___isr:
    pushal
    cld
    popal
    iret

panic: 
    cli
1:  hlt
    jmp 1b

#.size _start, . - _start            # set size of start to current loc '.' - start. 
