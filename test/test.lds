
SECTIONS { 
  .text 0x30008000 : { *(.text) }
  .data ALIGN(4): { *(.data) }
  .bss ALIGN(4):  { *(.bss)  *(COMMON) }
} 
