.text
.arch armv7a
.global mod_for_asm
.global route_jni_func
.type route_jni_func,%function
route_jni_func:
//int32_t route_jni_func(   int32_t r0,
//                          int32_t r1,
//                          int32_t r2,
//                          int32_t r3 ,
//                          int32_t argc, //0x0
//                          void* stack_params_start, //0x4
//                          void* pcall);//0x8

PUSH {r4,r5,r7, lr}
sub sp,sp,0x10
mov r7,sp
//保存参数
str r0,[r7,0x10-0x4]
str r1,[r7,0x10-0x8]
str r2,[r7,0x10-0xc]
str r3,[r7,0x10-0x10]

//栈对齐
ldr r0,[r7,0x10+0x10+0x0]
mov r1,2
bl mod_for_asm
cmp r0,0
beq nopadding
mov r0,0
push {r0}
nopadding:

//读栈参数
ldr r0,[r7,0x10+0x10+0x0]
ldr r1,[r7,0x10+0x10+0x4]
ldr r4,[r7,0x10+0x10+0x8]
//计算参数尾 r1=(r1-1)*4
sub r0,r0,1
lsl r0,r0,2

loop:
//r0 < 0
cmp r0,0
blt call
//参数重压栈
ldr r2,[r1,r0]
push {r2}
sub r0,r0,4
b loop

call:
//调用pcall
ldr r0,[r7,0x10-0x4]
ldr r1,[r7,0x10-0x8]
ldr r2,[r7,0x10-0xc]
ldr r3,[r7,0x10-0x10]
adr lr, ret
blx r4

ret:
//恢复堆栈
mov sp,r7
add sp,sp,0x10

POP {r4,r5,R7, PC}
.end