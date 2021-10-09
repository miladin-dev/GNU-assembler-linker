# file test.s
.section  miladin
    jmp *[r5 - 5]
    .equ a, 0x10    
    .extern a, b , c, mile, tina, lemilica, abc
    .word mile, lemi
    .skip 53

    myLabela:           
    .word 05
    lemi:

 add r5,r6 # rewrewrwerwerf
 sub r2, r3 
    # sub r2, r4
    xchg r0, r0

    int r5
         int r5

     jmp *r5
     jeq *[r7]                  

    jmp *r5


 jne *[r5 +5] # ne postoji zbog r8
 jne *[r2 - 5] # radi
jne *[r2 ]
jne %lemilica
    jeq 0x55
    jeq abc
    jeq 55
    jeq *52
    jeq *0x52
    jeq *abc
    jeq *[r2+a]
    jeq *[ r2 + abc]


ldr r0,  r5
    ldr r2, [r5]
ldr r1,r2 # sa komentarom
    str    r3   , [   r5 ]

str r0, $10
str r1, 10
str r2, $mile
str r3, tina
ldr r4, %lemi
    ldr r0, [r5 + 5]
    ldr r0, [r4 + a]
    ldr r0, [r4 + 0x54]
    
    ldr r0,[r2-2]

.end