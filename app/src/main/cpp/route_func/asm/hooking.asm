.text
.arch armv7a
.arm
.global test1
.global hooking_func
.type hooking_func,%function
hooking_func:
push {r4-r6,lr}
#0x12345678
movw r4,0x5678
movt r4,0x1234
add r5,sp,0x10
push {r4-r5}

movw r4,0x5678
movt r4,0x1234
blx r4
add sp,sp,0x8
pop {r4-r6,pc}
func_end:
.end

