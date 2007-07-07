SECTIONS
{
  /* This is the location that bootmenu will be copied to.
     Set it to a safe place that will not squash any PalmOS
     data.
   */
  . = 0xa0100000;

  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  _end = .;
  _length = _end - _start;
}
