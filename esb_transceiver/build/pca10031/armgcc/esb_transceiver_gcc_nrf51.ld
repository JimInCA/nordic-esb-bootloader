/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00005400, LENGTH = 0x3AC00
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 0x6000
  
}

SECTIONS
{
}

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }

} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  

} INSERT AFTER .text

INCLUDE "nrf5x_common.ld"
