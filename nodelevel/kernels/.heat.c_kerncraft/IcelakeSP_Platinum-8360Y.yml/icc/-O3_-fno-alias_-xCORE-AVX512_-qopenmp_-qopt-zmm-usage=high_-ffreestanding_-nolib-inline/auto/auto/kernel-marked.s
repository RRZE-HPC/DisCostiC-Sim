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
# mark_description "usr/bin/g++ -c -o nodelevel/kernels/.heat.c_kerncraft/IcelakeSP_Platinum-8360Y.yml/icc/-O3_-fno-alias_-xCORE";
# mark_description "-AVX512_-qopenmp_-qopt-zmm-usage=high_-ffreestanding_-nolib-inline/kernel.s -O3 -fno-alias -xCORE-AVX512 -qo";
# mark_description "penmp -qopt-zmm-usage=high -ffreestanding -nolib-inline -S -std=c99";
	.file "kernel.c"
	.text
..TXTST0:
.L_2__routine_start_kernel_0:
# -- Begin  kernel
	.text
# mark_begin;
       .align    16,0x90
	.globl kernel
# --- kernel(double *, double *, const int, const int)
kernel:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %edx
# parameter 4: %ecx
..B1.1:                         # Preds ..B1.0
                                # Execution count [1.00e+00]
	.cfi_startproc
..___tag_value_kernel.1:
..L2:
                                                          #2.1
        pushq     %r12                                          #2.1
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
        movq      %rdi, %r12                                    #2.1
        movslq    %ecx, %rax                                    #2.1
        xorl      %r9d, %r9d                                    #3.5
        movslq    %edx, %r10                                    #2.1
        testq     %rax, %rax                                    #3.25
        jle       ..B1.23       # Prob 9%                       #3.25
                                # LOE rax rbx rbp rsi r9 r10 r12 r13 r14 r15 edx
..B1.2:                         # Preds ..B1.1
                                # Execution count [9.00e-01]
        vmovups   .L_2il0floatpacket.0(%rip), %zmm1             #5.28
        lea       (,%r10,8), %r11                               #5.38
        vmovdqu   .L_2il0floatpacket.1(%rip), %ymm2             #4.5
        vmovdqu   .L_2il0floatpacket.2(%rip), %ymm0             #4.5
        negq      %r11                                          #5.97
        lea       (%rsi,%r10,8), %rdi                           #5.127
        addq      %rsi, %r11                                    #5.97
        xorl      %r8d, %r8d                                    #3.5
        movq      %r11, -40(%rsp)                               #5.97[spill]
        movq      %rsi, %rcx                                    #3.5
        movq      %rdi, -48(%rsp)                               #5.127[spill]
        movq      %rax, -8(%rsp)                                #3.5[spill]
        movq      %r12, -24(%rsp)                               #3.5[spill]
        movq      %rsi, -16(%rsp)                               #3.5[spill]
        movq      %r13, -56(%rsp)                               #3.5[spill]
        movq      %r14, -64(%rsp)                               #3.5[spill]
        movq      %r15, -72(%rsp)                               #3.5[spill]
        movq      %rbx, -80(%rsp)                               #3.5[spill]
	.cfi_offset 3, -96
	.cfi_offset 13, -72
	.cfi_offset 14, -80
	.cfi_offset 15, -88
                                # LOE rcx rbp rdi r8 r9 r10 r11 edx ymm0 ymm2 zmm1
..B1.3:                         # Preds ..B1.21 ..B1.2
                                # Execution count [5.00e+00]
        testq     %r10, %r10                                    #4.25
        jle       ..B1.21       # Prob 50%                      #4.25
                                # LOE rcx rbp rdi r8 r9 r10 r11 edx ymm0 ymm2 zmm1
..B1.4:                         # Preds ..B1.3
                                # Execution count [4.50e+00]
        cmpl      $16, %edx                                     #4.5
        jl        ..B1.24       # Prob 10%                      #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 edx ymm0 ymm2 zmm1
..B1.5:                         # Preds ..B1.4
                                # Execution count [4.50e+00]
        movq      -24(%rsp), %r15                               #5.5[spill]
        addq      %r8, %r15                                     #5.5
        movq      %r15, %r12                                    #4.5
        andq      $63, %r12                                     #4.5
        testl     $7, %r12d                                     #4.5
        je        ..B1.7        # Prob 50%                      #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 r15 edx r12d ymm0 ymm2 zmm1
..B1.6:                         # Preds ..B1.5
                                # Execution count [2.25e+00]
        xorl      %r12d, %r12d                                  #4.5
        jmp       ..B1.9        # Prob 100%                     #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 r15 edx r12d ymm0 ymm2 zmm1
..B1.7:                         # Preds ..B1.5
                                # Execution count [2.25e+00]
        testl     %r12d, %r12d                                  #4.5
        je        ..B1.9        # Prob 50%                      #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 r15 edx r12d ymm0 ymm2 zmm1
..B1.8:                         # Preds ..B1.7
                                # Execution count [2.50e+01]
        negl      %r12d                                         #4.5
        addl      $64, %r12d                                    #4.5
        shrl      $3, %r12d                                     #4.5
        cmpl      %r12d, %edx                                   #4.5
        cmovl     %edx, %r12d                                   #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 r15 edx r12d ymm0 ymm2 zmm1
..B1.9:                         # Preds ..B1.6 ..B1.8 ..B1.7
                                # Execution count [5.00e+00]
        movl      %edx, %eax                                    #4.5
        subl      %r12d, %eax                                   #4.5
        andl      $15, %eax                                     #4.5
        negl      %eax                                          #4.5
        addl      %edx, %eax                                    #4.5
        cmpl      $1, %r12d                                     #4.5
        jb        ..B1.13       # Prob 50%                      #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 r15 eax edx r12d ymm0 ymm2 zmm1
..B1.10:                        # Preds ..B1.9
                                # Execution count [4.50e+00]
        vmovdqa   %ymm0, %ymm4                                  #4.5
        xorl      %esi, %esi                                    #4.5
        vpbroadcastd %r12d, %ymm3                               #4.5
        movslq    %r12d, %rbx                                   #4.5
                                # LOE rcx rbx rbp rsi rdi r8 r9 r10 r11 r15 eax edx r12d ymm0 ymm2 ymm3 ymm4 zmm1
..B1.11:                        # Preds ..B1.11 ..B1.10
                                # Execution count [2.50e+01]
        vpcmpgtd  %ymm4, %ymm3, %k1                             #4.5
        vpaddd    %ymm2, %ymm4, %ymm4                           #4.5
        vmovupd   -8(%rcx,%rsi,8), %zmm5{%k1}{z}                #5.38
        vmovupd   8(%rcx,%rsi,8), %zmm6{%k1}{z}                 #5.67
        vmovupd   (%r11,%rsi,8), %zmm8{%k1}{z}                  #5.97
        vmovupd   (%rdi,%rsi,8), %zmm10{%k1}{z}                 #5.127
        vaddpd    %zmm6, %zmm5, %zmm7                           #5.67
        vaddpd    %zmm8, %zmm7, %zmm9                           #5.97
        vaddpd    %zmm10, %zmm9, %zmm11                         #5.127
        vmulpd    %zmm11, %zmm1, %zmm12                         #5.127
        vmovupd   %zmm12, (%r15,%rsi,8){%k1}                    #5.5
        addq      $8, %rsi                                      #4.5
        cmpq      %rbx, %rsi                                    #4.5
        jb        ..B1.11       # Prob 82%                      #4.5
                                # LOE rcx rbx rbp rsi rdi r8 r9 r10 r11 r15 eax edx r12d ymm0 ymm2 ymm3 ymm4 zmm1
..B1.12:                        # Preds ..B1.11
                                # Execution count [4.50e+00]
        cmpl      %r12d, %edx                                   #4.5
        je        ..B1.21       # Prob 10%                      #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 r15 eax edx r12d ymm0 ymm2 zmm1
..B1.13:                        # Preds ..B1.9 ..B1.12
                                # Execution count [2.50e+01]
        lea       16(%r12), %ebx                                #4.5
        cmpl      %ebx, %eax                                    #4.5
        jl        ..B1.17       # Prob 50%                      #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 r15 eax edx r12d ymm0 ymm2 zmm1
..B1.14:                        # Preds ..B1.13
                                # Execution count [4.50e+00]
        movslq    %r12d, %r12                                   #4.5
        movl      %edx, -32(%rsp)                               #4.5[spill]
        movslq    %eax, %rbx                                    #4.5
        movq      -16(%rsp), %rdx                               #4.5[spill]
        lea       (%r8,%r12,8), %r14                            #4.5
        lea       (%rdi,%r12,8), %r13                           #5.127
        lea       (%r11,%r12,8), %rsi                           #5.97
                                # LOE rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 r15 eax ymm0 ymm2 zmm1
movl      $111, %ebx # OSACA START MARKER
.byte     100        # OSACA START MARKER
.byte     103        # OSACA START MARKER
.byte     144        # OSACA START MARKER
# pointer_increment=128 a3ceb447b87890bff773412cc6500237
..B1.15:                        # Preds ..B1.15 ..B1.14
                                # Execution count [2.50e+01]
        vmovupd   -8(%r14,%rdx), %zmm3                          #5.38
        vaddpd    8(%r14,%rdx), %zmm3, %zmm4                    #5.67
        vaddpd    (%rsi), %zmm4, %zmm5                          #5.97
        vaddpd    (%r13), %zmm5, %zmm6                          #5.127
        vmulpd    %zmm6, %zmm1, %zmm7                           #5.127
        vmovupd   %zmm7, (%r15,%r12,8)                          #5.5
        vmovupd   56(%r14,%rdx), %zmm8                          #5.38
        vaddpd    72(%r14,%rdx), %zmm8, %zmm9                   #5.67
        addq      $128, %r14                                    #4.5
        vaddpd    64(%rsi), %zmm9, %zmm10                       #5.97
        addq      $128, %rsi                                    #4.5
        vaddpd    64(%r13), %zmm10, %zmm11                      #5.127
        addq      $128, %r13                                    #4.5
        vmulpd    %zmm11, %zmm1, %zmm12                         #5.127
        vmovupd   %zmm12, 64(%r15,%r12,8)                       #5.5
        addq      $16, %r12                                     #4.5
        cmpq      %rbx, %r12                                    #4.5
        jb        ..B1.15       # Prob 82%                      #4.5
movl      $222, %ebx # OSACA END MARKER
.byte     100        # OSACA END MARKER
.byte     103        # OSACA END MARKER
.byte     144        # OSACA END MARKER
                                # LOE rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 r15 eax ymm0 ymm2 zmm1
..B1.16:                        # Preds ..B1.15
                                # Execution count [4.50e+00]
        movl      -32(%rsp), %edx                               #[spill]
                                # LOE rcx rbp rdi r8 r9 r10 r11 eax edx ymm0 ymm2 zmm1
..B1.17:                        # Preds ..B1.16 ..B1.13 ..B1.24
                                # Execution count [5.00e+00]
        lea       1(%rax), %ebx                                 #4.5
        cmpl      %edx, %ebx                                    #4.5
        ja        ..B1.21       # Prob 50%                      #4.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 eax edx ymm0 ymm2 zmm1
..B1.18:                        # Preds ..B1.17
                                # Execution count [4.50e+00]
        movslq    %eax, %r12                                    #5.57
        negl      %eax                                          #4.5
        addl      %edx, %eax                                    #4.5
        xorl      %r13d, %r13d                                  #4.5
        vpbroadcastd %eax, %ymm3                                #4.5
        movq      -24(%rsp), %rsi                               #5.5[spill]
        movq      -48(%rsp), %rax                               #5.127[spill]
        movq      -40(%rsp), %r15                               #5.97[spill]
        vmovdqa   %ymm0, %ymm4                                  #4.5
        lea       (%rsi,%r8), %rbx                              #5.5
        lea       (%rax,%r8), %r14                              #5.127
        lea       (%rbx,%r12,8), %rsi                           #5.5
        lea       (%r14,%r12,8), %rbx                           #5.127
        movq      -16(%rsp), %r14                               #5.38[spill]
        lea       (%r15,%r8), %rax                              #5.97
        lea       (%rax,%r12,8), %rax                           #5.97
        lea       (%r14,%r8), %r15                              #5.38
        lea       (%r15,%r12,8), %r14                           #5.38
        negq      %r12                                          #4.5
        addq      %r10, %r12                                    #4.5
                                # LOE rax rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 edx ymm0 ymm2 ymm3 ymm4 zmm1
..B1.19:                        # Preds ..B1.19 ..B1.18
                                # Execution count [2.50e+01]
        vpcmpgtd  %ymm4, %ymm3, %k1                             #4.5
        vpaddd    %ymm2, %ymm4, %ymm4                           #4.5
        vmovupd   -8(%r14,%r13,8), %zmm5{%k1}{z}                #5.38
        vmovupd   8(%r14,%r13,8), %zmm6{%k1}{z}                 #5.67
        vmovupd   (%rax,%r13,8), %zmm8{%k1}{z}                  #5.97
        vmovupd   (%rbx,%r13,8), %zmm10{%k1}{z}                 #5.127
        vaddpd    %zmm6, %zmm5, %zmm7                           #5.67
        vaddpd    %zmm8, %zmm7, %zmm9                           #5.97
        vaddpd    %zmm10, %zmm9, %zmm11                         #5.127
        vmulpd    %zmm11, %zmm1, %zmm12                         #5.127
        vmovupd   %zmm12, (%rsi,%r13,8){%k1}                    #5.5
        addq      $8, %r13                                      #4.5
        cmpq      %r12, %r13                                    #4.5
        jb        ..B1.19       # Prob 82%                      #4.5
                                # LOE rax rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 edx ymm0 ymm2 ymm3 ymm4 zmm1
..B1.21:                        # Preds ..B1.19 ..B1.3 ..B1.17 ..B1.12
                                # Execution count [5.00e+00]
        incq      %r9                                           #3.5
        lea       (%rcx,%r10,8), %rcx                           #3.5
        lea       (%r11,%r10,8), %r11                           #3.5
        lea       (%rdi,%r10,8), %rdi                           #3.5
        lea       (%r8,%r10,8), %r8                             #3.5
        cmpq      -8(%rsp), %r9                                 #3.5[spill]
        jb        ..B1.3        # Prob 82%                      #3.5
                                # LOE rcx rbp rdi r8 r9 r10 r11 edx ymm0 ymm2 zmm1
..B1.22:                        # Preds ..B1.21
                                # Execution count [9.00e-01]
        movq      -56(%rsp), %r13                               #[spill]
	.cfi_restore 13
        movq      -64(%rsp), %r14                               #[spill]
	.cfi_restore 14
        movq      -72(%rsp), %r15                               #[spill]
	.cfi_restore 15
        movq      -80(%rsp), %rbx                               #[spill]
	.cfi_restore 3
                                # LOE rbx rbp r13 r14 r15
..B1.23:                        # Preds ..B1.1 ..B1.22
                                # Execution count [1.00e+00]
        vzeroupper                                              #8.1
	.cfi_restore 12
        popq      %r12                                          #8.1
	.cfi_def_cfa_offset 8
        ret                                                     #8.1
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -96
	.cfi_offset 12, -16
	.cfi_offset 13, -72
	.cfi_offset 14, -80
	.cfi_offset 15, -88
                                # LOE
..B1.24:                        # Preds ..B1.4
                                # Execution count [4.50e-01]: Infreq
        xorl      %eax, %eax                                    #4.5
        jmp       ..B1.17       # Prob 100%                     #4.5
        .align    16,0x90
                                # LOE rcx rbp rdi r8 r9 r10 r11 eax edx ymm0 ymm2 zmm1
	.cfi_endproc
# mark_end;
	.type	kernel,@function
	.size	kernel,.-kernel
..LNkernel.0:
	.data
# -- End  kernel
	.section .rodata, "a"
	.align 64
	.align 64
.L_2il0floatpacket.0:
	.long	0x00000000,0x3fd00000,0x00000000,0x3fd00000,0x00000000,0x3fd00000,0x00000000,0x3fd00000,0x00000000,0x3fd00000,0x00000000,0x3fd00000,0x00000000,0x3fd00000,0x00000000,0x3fd00000
	.type	.L_2il0floatpacket.0,@object
	.size	.L_2il0floatpacket.0,64
	.align 32
.L_2il0floatpacket.1:
	.long	0x00000008,0x00000008,0x00000008,0x00000008,0x00000008,0x00000008,0x00000008,0x00000008
	.type	.L_2il0floatpacket.1,@object
	.size	.L_2il0floatpacket.1,32
	.align 32
.L_2il0floatpacket.2:
	.long	0x00000000,0x00000001,0x00000002,0x00000003,0x00000004,0x00000005,0x00000006,0x00000007
	.type	.L_2il0floatpacket.2,@object
	.size	.L_2il0floatpacket.2,32
	.data
	.section .note.GNU-stack, ""
# End
