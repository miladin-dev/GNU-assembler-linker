
.section ivt   
ldr r1, %myStart
ldr r1, myStart
jeq *r4
jeq *[r2]
    .word 2 
 jmp *14
 jgt *0x32
  jne *myStart
  

  jgt *[r7 + 9]
  jgt *[r7 + 0xFE]
  jgt *[r7 + myStart]

.section zzez
    .word 0xb
    .word 0
    .word myStart
myStart:

.end