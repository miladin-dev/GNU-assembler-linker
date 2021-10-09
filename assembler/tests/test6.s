.section ivt

    ldr r1  , $myStart
    ldr r1, $10
    ldr r1, $0x10

     ldr r1, myStartt # abs
                    ldr r1, %myStart # pcrel
    ldr r1, 10
    ldr r1, 0x10

    ldr r1, [r4 +   myStart]
    ldr r1, [r4 + 5   ]
    ldr r1, [r4 + 0x5      ]

.word 9
myStart:

.end

