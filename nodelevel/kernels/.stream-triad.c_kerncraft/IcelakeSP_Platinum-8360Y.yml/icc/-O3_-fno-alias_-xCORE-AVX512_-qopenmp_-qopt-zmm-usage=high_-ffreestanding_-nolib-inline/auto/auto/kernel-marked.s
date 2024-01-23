# mark_description "Intel(R) C Intel(R) 64 Compiler Classic for applications running on Intel(R) 64, Version 2021.10.0 Build 202";
# mark_description "30609_000000";
# mark_description "-I../.local/lib/python3.9/site-packages/kerncraft/headers -Wl,-rpath,/apps/SPACK/0.19.1/opt/linux-almalinux8";
# mark_description "-icelake/gcc-8.5.0/intel-oneapi-compilers-2023.2.1-axze7ocbc5zpklxb6kr5h2d5hbpf2lxk/compiler/2023.2.1/linux/";
# mark_description "lib -Wl,-rpath,/apps/SPACK/0.19.1/opt/linux-almalinux8-icelake/gcc-8.5.0/intel-oneapi-compilers-2023.2.1-axz";
# mark_description "e7ocbc5zpklxb6kr5h2d5hbpf2lxk/compiler/2023.2.1/linux/lib/x64 -Wl,-rpath,/apps/SPACK/0.19.1/opt/linux-almali";
# mark_description "nux8-icelake/gcc-8.5.0/intel-oneapi-compilers-2023.2.1-axze7ocbc5zpklxb6kr5h2d5hbpf2lxk/compiler/2023.2.1/li";
# mark_description "nux/lib/oclfpga/host/linux64/lib -Wl,-rpath,/apps/SPACK/0.19.1/opt/linux-almalinux8-icelake/gcc-8.5.0/intel-";
# mark_description "oneapi-compilers-2023.2.1-axze7ocbc5zpklxb6kr5h2d5hbpf2lxk/compiler/2023.2.1/linux/lib/oclfpga/linux64/lib -";
# mark_description "Wl,-rpath,/apps/SPACK/0.19.1/opt/linux-almalinux8-icelake/gcc-8.5.0/intel-oneapi-compilers-2023.2.1-axze7ocb";
# mark_description "c5zpklxb6kr5h2d5hbpf2lxk/compiler/2023.2.1/linux/compiler/lib/intel64_lin -gcc-name=/usr/bin/gcc -gxx-name=/";
# mark_description "usr/bin/g++ -c -o nodelevel/kernels/.stream-triad.c_kerncraft/IcelakeSP_Platinum-8360Y.yml/icc/-O3_-fno-alia";
# mark_description "s_-xCORE-AVX512_-qopenmp_-qopt-zmm-usage=high_-ffreestanding_-nolib-inline/kernel.s -O3 -fno-alias -xCORE-AV";
# mark_description "X512 -qopenmp -qopt-zmm-usage=high -ffreestanding -nolib-inline -S -std=c99";
	.file "kernel.c"
	.text
..TXTST0:
.L_2__routine_start_kernel_0:
# -- Begin  kernel
	.text
# mark_begin;
       .align    16,0x90
	.globl kernel
# --- kernel(double *, double *, double *, double *, const int)
kernel:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %rdx
# parameter 4: %rcx
# parameter 5: %r8d
..B1.1:                         # Preds ..B1.0
                                # Execution count [1.00e+00]
	.cfi_startproc
..___tag_value_kernel.1:
..L2:
                                                          #2.1
        movq      %rdx, %r9                                     #2.1
        vmovsd    (%rcx), %xmm4                                 #4.8
        vmovsd    %xmm4, s.2.0.1(%rip)                          #4.3
        testl     %r8d, %r8d                                    #5.25
        jle       ..B1.18       # Prob 50%                      #5.25
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 r8d xmm4
..B1.2:                         # Preds ..B1.1
                                # Execution count [9.00e-01]
        cmpl      $8, %r8d                                      #5.5
        jl        ..B1.20       # Prob 10%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 r8d xmm4
..B1.3:                         # Preds ..B1.2
                                # Execution count [9.00e-01]
        cmpl      $62, %r8d                                     #5.5
        jl        ..B1.19       # Prob 10%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 r8d xmm4
..B1.4:                         # Preds ..B1.3
                                # Execution count [9.00e-01]
        movq      %rdi, %rax                                    #5.5
        andq      $63, %rax                                     #5.5
        testb     $7, %al                                       #5.5
        je        ..B1.6        # Prob 50%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax r8d xmm4
..B1.5:                         # Preds ..B1.4
                                # Execution count [4.50e-01]
        xorl      %eax, %eax                                    #5.5
        jmp       ..B1.8        # Prob 100%                     #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax r8d xmm4
..B1.6:                         # Preds ..B1.4
                                # Execution count [4.50e-01]
        testl     %eax, %eax                                    #5.5
        je        ..B1.8        # Prob 50%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax r8d xmm4
..B1.7:                         # Preds ..B1.6
                                # Execution count [5.00e+00]
        negl      %eax                                          #5.5
        addl      $64, %eax                                     #5.5
        shrl      $3, %eax                                      #5.5
        cmpl      %eax, %r8d                                    #5.5
        cmovl     %r8d, %eax                                    #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax r8d xmm4
..B1.8:                         # Preds ..B1.5 ..B1.7 ..B1.6
                                # Execution count [1.00e+00]
        movl      %r8d, %edx                                    #5.5
        subl      %eax, %edx                                    #5.5
        andl      $7, %edx                                      #5.5
        negl      %edx                                          #5.5
        addl      %r8d, %edx                                    #5.5
        cmpl      $1, %eax                                      #5.5
        jb        ..B1.12       # Prob 50%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax edx r8d xmm4
..B1.9:                         # Preds ..B1.8
                                # Execution count [9.00e-01]
        vmovdqu   .L_2il0floatpacket.1(%rip), %ymm3             #5.5
        xorl      %r11d, %r11d                                  #5.5
        vmovdqu   .L_2il0floatpacket.0(%rip), %ymm2             #5.5
        vpbroadcastd %eax, %ymm1                                #5.5
        vbroadcastsd %xmm4, %zmm0                               #10.4
        movslq    %eax, %r10                                    #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r10 r11 r12 r13 r14 r15 eax edx r8d xmm4 ymm1 ymm2 ymm3 zmm0
movl      $111, %ebx # OSACA START MARKER
.byte     100        # OSACA START MARKER
.byte     103        # OSACA START MARKER
.byte     144        # OSACA START MARKER
# pointer_increment=64 3274527bde828b80a8dbd0d7b114c570
..B1.10:                        # Preds ..B1.10 ..B1.9
                                # Execution count [5.00e+00]
        vpcmpgtd  %ymm2, %ymm1, %k1                             #5.5
        vpaddd    %ymm3, %ymm2, %ymm2                           #5.5
        vmovupd   (%r9,%r11,8), %zmm5{%k1}{z}                   #7.24
        vfmadd213pd (%rsi,%r11,8), %zmm0, %zmm5{%k1}{z}         #7.24
        vmovupd   %zmm5, (%rdi,%r11,8){%k1}                     #7.5
        addq      $8, %r11                                      #5.5
        cmpq      %r10, %r11                                    #5.5
        jb        ..B1.10       # Prob 82%                      #5.5
movl      $222, %ebx # OSACA END MARKER
.byte     100        # OSACA END MARKER
.byte     103        # OSACA END MARKER
.byte     144        # OSACA END MARKER
                                # LOE rcx rbx rbp rsi rdi r9 r10 r11 r12 r13 r14 r15 eax edx r8d xmm4 ymm1 ymm2 ymm3 zmm0
..B1.11:                        # Preds ..B1.10
                                # Execution count [9.00e-01]
        cmpl      %eax, %r8d                                    #5.5
        je        ..B1.18       # Prob 10%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax edx r8d xmm4
..B1.12:                        # Preds ..B1.11 ..B1.8 ..B1.19
                                # Execution count [5.00e+00]
        lea       8(%rax), %r10d                                #5.5
        cmpl      %r10d, %edx                                   #5.5
        jl        ..B1.16       # Prob 50%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax edx r8d xmm4
..B1.13:                        # Preds ..B1.12
                                # Execution count [9.00e-01]
        movslq    %eax, %rax                                    #5.5
        vbroadcastsd %xmm4, %zmm0                               #10.4
        movslq    %edx, %r10                                    #5.5
                                # LOE rax rcx rbx rbp rsi rdi r9 r10 r12 r13 r14 r15 edx r8d xmm4 zmm0
..B1.14:                        # Preds ..B1.14 ..B1.13
                                # Execution count [5.00e+00]
        vmovups   (%r9,%rax,8), %zmm1                           #7.24
        vfmadd213pd (%rsi,%rax,8), %zmm0, %zmm1                 #7.24
        vmovupd   %zmm1, (%rdi,%rax,8)                          #7.5
        addq      $8, %rax                                      #5.5
        cmpq      %r10, %rax                                    #5.5
        jb        ..B1.14       # Prob 82%                      #5.5
                                # LOE rax rcx rbx rbp rsi rdi r9 r10 r12 r13 r14 r15 edx r8d xmm4 zmm0
..B1.16:                        # Preds ..B1.14 ..B1.12 ..B1.20
                                # Execution count [1.00e+00]
        lea       1(%rdx), %eax                                 #5.5
        cmpl      %r8d, %eax                                    #5.5
        ja        ..B1.18       # Prob 50%                      #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 edx r8d xmm4
..B1.17:                        # Preds ..B1.16
                                # Execution count [5.00e+00]
        subl      %edx, %r8d                                    #5.5
        vpbroadcastd %r8d, %ymm0                                #5.5
        vbroadcastsd %xmm4, %zmm2                               #10.4
        vpcmpgtd  .L_2il0floatpacket.0(%rip), %ymm0, %k1        #5.5
        movslq    %edx, %rdx                                    #5.5
        vmovupd   (%r9,%rdx,8), %zmm1{%k1}{z}                   #7.24
        vfmadd213pd (%rsi,%rdx,8), %zmm1, %zmm2{%k1}{z}         #7.24
        vmovupd   %zmm2, (%rdi,%rdx,8){%k1}                     #7.5
                                # LOE rcx rbx rbp r12 r13 r14 r15 xmm4
..B1.18:                        # Preds ..B1.1 ..B1.16 ..B1.11 ..B1.17
                                # Execution count [1.00e+00]
        vmovsd    %xmm4, (%rcx)                                 #10.4
        vzeroupper                                              #11.1
        ret                                                     #11.1
                                # LOE
..B1.19:                        # Preds ..B1.3
                                # Execution count [9.00e-02]: Infreq
        movl      %r8d, %edx                                    #5.5
        xorl      %eax, %eax                                    #5.5
        andl      $-8, %edx                                     #5.5
        jmp       ..B1.12       # Prob 100%                     #5.5
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 eax edx r8d xmm4
..B1.20:                        # Preds ..B1.2
                                # Execution count [9.00e-02]: Infreq
        xorl      %edx, %edx                                    #5.5
        jmp       ..B1.16       # Prob 100%                     #5.5
        .align    16,0x90
                                # LOE rcx rbx rbp rsi rdi r9 r12 r13 r14 r15 edx r8d xmm4
	.cfi_endproc
# mark_end;
	.type	kernel,@function
	.size	kernel,.-kernel
..LNkernel.0:
	.bss
	.align 8
	.align 8
s.2.0.1:
	.type	s.2.0.1,@object
	.size	s.2.0.1,8
	.space 8	# pad
	.data
# -- End  kernel
	.section .rodata, "a"
	.align 32
	.align 32
.L_2il0floatpacket.0:
	.long	0x00000000,0x00000001,0x00000002,0x00000003,0x00000004,0x00000005,0x00000006,0x00000007
	.type	.L_2il0floatpacket.0,@object
	.size	.L_2il0floatpacket.0,32
	.align 32
.L_2il0floatpacket.1:
	.long	0x00000008,0x00000008,0x00000008,0x00000008,0x00000008,0x00000008,0x00000008,0x00000008
	.type	.L_2il0floatpacket.1,@object
	.size	.L_2il0floatpacket.1,32
	.data
	.section .note.GNU-stack, ""
# End
