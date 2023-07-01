# arclab

- [arclab](#arclab)
	- [准备](#准备)
	- [环境配置](#环境配置)
	- [实验开始](#实验开始)
		- [Part A](#part-a)
		- [Part B](#part-b)
		- [Part C](#part-c)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

## 环境配置

进行这个实验，我们首先需要安装 `flex` 和 `bison`：

```bash
sudo apt-get install flex
sudo apt-get install bison
```

之后运行 `make clean ; make`，得到报错：

```bash
(cd misc; make all)
make[1]: 进入目录“/home/chisato/CSAPPLABS/ArchitectureLab/archlab-handout/sim/misc”
gcc -Wall -O1 -g yas-grammar.o yas.o isa.o -lfl -o yas
/usr/bin/ld: yas.o:/home/chisato/CSAPPLABS/ArchitectureLab/archlab-handout/sim/misc/yas.h:13: multiple definition of `lineno'; yas-grammar.o:(.bss+0x0): first defined here
collect2: error: ld returned 1 exit status
make[1]: *** [Makefile:32：yas] 错误 1
make[1]: 离开目录“/home/chisato/CSAPPLABS/ArchitectureLab/archlab-handout/sim/misc”
make: *** [Makefile:26：all] 错误 2
```

这是由于 `gcc` 版本过老导致的，解决方式如下：

将 `misc/Makefile` 中

```makefile
CFLAGS=-Wall -O1 -g
LCFLAGS=-O1
```

改为：

```makefile
CFLAGS=-Wall -O1 -g -fcommon
LCFLAGS=-O1 -fcommon
```

对于其他的 `pipe` 和 `seq` 中的 `Makefile` 文件，我们也需要同样做更改

原贴如下：[Fail to compile the Y86 simulatur (CSAPP)](https://stackoverflow.com/questions/63152352/fail-to-compile-the-y86-simulatur-csapp)

> 对于全局变量来说，如果初始化为不为零的值，那么保存在 `data` 段，初始化为零值，保存在 `BSS` 段，如果没有初始化仅仅是声明，那么保存在 `common` 段，等到链接时再放入 `BSS` 段

这个实验的模拟器有两种：`TTY` 和 `GUI`

如果需要用 `GUI` ，那么可以安装 `Tcl/Tk`：

```bash
sudo apt install tcl tcl-dev tk tk-dev
```

我们并没有安装，因此用 `TTY`

我们将 `pipe/Makefile` 和 `pipe/Makefile` 中的 `GUIMODE, TKLIBS, TKINC` 全部注释掉

将 `sim/Makefile` 从原先的：

```
all:
	(cd misc; make all)
	(cd pipe; make all GUIMODE=$(GUIMODE) TKLIBS="$(TKLIBS)" TKINC="$(TKINC)")
	(cd seq; make all GUIMODE=$(GUIMODE) TKLIBS="$(TKLIBS)" TKINC="$(TKINC)")
	(cd y86-code; make all)

clean:
	rm -f *~ core
	(cd misc; make clean)
	(cd pipe; make clean)
	(cd seq; make clean)
	(cd y86-code; make clean)
	(cd ptest; make clean)
```

修改为：

```
all:
	(cd misc; make all)
	(cd pipe; make all)
	(cd seq; make all)
	(cd y86-code; make all)

clean:
	rm -f *~ core
	(cd misc; make clean)
	(cd pipe; make clean)
	(cd seq; make clean)
	(cd y86-code; make clean)
	(cd ptest; make clean)
```

执行 `make clean ; make` 后编译成功：

```
../misc/yas abs-asum-cmov.ys
../misc/yas abs-asum-jmp.ys
../misc/yas asum.ys
../misc/yas asumr.ys
../misc/yas asumi.ys
../misc/yas cjr.ys
../misc/yas j-cc.ys
../misc/yas poptest.ys
../misc/yas pushquestion.ys
../misc/yas pushtest.ys
../misc/yas prog1.ys
../misc/yas prog2.ys
../misc/yas prog3.ys
../misc/yas prog4.ys
../misc/yas prog5.ys
../misc/yas prog6.ys
../misc/yas prog7.ys
../misc/yas prog8.ys
../misc/yas prog9.ys
../misc/yas prog10.ys
../misc/yas ret-hazard.ys
```

---

## 实验开始

> 这个实验的 `part A` 和 `part B` 很简单，重点在于 `part C`

### Part A

参照 [PDF](http://csapp.cs.cmu.edu/3e/archlab.pdf) 中 `4` 的部分

我们需要用 `Y86-64` 汇编代码写三个程序，分别为：`sum.ys, rsum.ys, copy.ys`

前两个的测试数据如下：

```assembly
# Sample linked list
.align 8
ele1:
	.quad 0x00a
	.quad ele2
ele2:
	.quad 0x0b0
	.quad ele3
ele3:
	.quad 0xc00
	.quad 0
```

第三个的测试数据如下：

```assembly
.align 8
# Source block
src:
	.quad 0x00a
	.quad 0x0b0
	.quad 0xc00
# Destination block
dest:
	.quad 0x111
	.quad 0x222
	.quad 0x333
```

我们只需要将这些数据放在汇编代码里面即可，这里面的每一个标号都表示一个地址，我们可以直接用 `irmovq` 对寄存器赋值，例如：`irmovq ele1, %rsi`

这一部分非常简单，根据书中 `254` 页的代码参考一下就好，这里我们直接给出答案

只要最后 `%rax` 的值为 `0xcba`，那么答案就是正确的

* `sum.ys`

```assembly
# author: NishikiChisato

# Execution begains at address 0
    .pos 0
    irmovq  stack, %rsp
    call    main
    halt

# Sample linked list
.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0

main:
    irmovq  ele1, %rdi
    call    sum_list
    ret

sum_list:
    xorq    %rax, %rax
    andq    %rdi, %rdi
    jmp    test
loop:
    mrmovq  (%rdi), %rbx
    addq    %rbx, %rax
    mrmovq  8(%rdi), %rbx
    rrmovq  %rbx, %rdi
    andq    %rdi, %rdi
test:
    jne     loop
    ret

# stack
    .pos   0x200
stack:

```

文件的最后需要空一行，测试结果如下：

```bash
$ ./yas ./answer/sum.ys
$ ./yis ./answer/sum.yo
Stopped in 29 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200

Changes to memory:
0x01f0: 0x0000000000000000      0x000000000000005b
0x01f8: 0x0000000000000000      0x0000000000000013
```

可以看到最后 `%rax` 的值为 `0xcba`，说明答案正确

* `rsum.ys`

```assembly
# author: NishikiChisato

# Execution begains at address 0
    .pos 0
    irmovq  stack, %rsp
    call    main
    halt

# Sample linked list
.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0

main:
    irmovq  ele1, %rdi
    call    rsum_list
    ret

rsum_list:
    xorq    %rax, %rax
    andq    %rdi, %rdi
    je      return
    pushq    %rbx
    mrmovq  (%rdi), %rbx
    mrmovq  8(%rdi), %rdi
    call    rsum_list
    addq    %rbx, %rax
    popq    %rbx
return:
    ret

# stack
    .pos   0x200
stack:

```

测试结果：

```bash
$ ./yas ./answer/rsum.ys
$ ./yis ./answer/rsum.yo
Stopped in 40 steps at PC = 0x13.  Status 'HLT', CC Z=0 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200

Changes to memory:
0x01c0: 0x0000000000000000      0x0000000000000088
0x01c8: 0x0000000000000000      0x00000000000000b0
0x01d0: 0x0000000000000000      0x0000000000000088
0x01d8: 0x0000000000000000      0x000000000000000a
0x01e0: 0x0000000000000000      0x0000000000000088
0x01f0: 0x0000000000000000      0x000000000000005b
0x01f8: 0x0000000000000000      0x0000000000000013
```

最后 `%rax` 的值依旧为 `0xcba`，结果正确

* `copy.ys`

```assembly
# author: NishikiChisato

# Execution begains at address 0
    .pos 0
    irmovq  stack, %rsp
    call    main
    halt

# Sample linked list
.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0

main:
    irmovq  ele1, %rdi
    call    sum_list
    ret

sum_list:
    xorq    %rax, %rax
    andq    %rdi, %rdi
    jmp    test
loop:
    mrmovq  (%rdi), %rbx
    addq    %rbx, %rax
    mrmovq  8(%rdi), %rbx
    rrmovq  %rbx, %rdi
    andq    %rdi, %rdi
test:
    jne     loop
    ret

# stack
    .pos   0x200
stack:

```

测试结果：

```bash
$ ./yas ./answer/copy.ys
$ ./yis ./answer/copy.yo
Stopped in 38 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rcx:   0x0000000000000000      0x0000000000000c00
%rbx:   0x0000000000000000      0x0000000000000008
%rsp:   0x0000000000000000      0x0000000000000200
%rsi:   0x0000000000000000      0x0000000000000048
%rdi:   0x0000000000000000      0x0000000000000030
%r8:    0x0000000000000000      0x0000000000000001

Changes to memory:
0x0030: 0x0000000000000111      0x000000000000000a
0x0038: 0x0000000000000222      0x00000000000000b0
0x0040: 0x0000000000000333      0x0000000000000c00
0x01f0: 0x0000000000000000      0x000000000000006f
0x01f8: 0x0000000000000000      0x0000000000000013
```

`%rax` 的值依然为 `0xcba`，结果正确

### Part B

参照 [PDF](http://csapp.cs.cmu.edu/3e/archlab.pdf) 中 `5` 的部分

我们需要修改 `seq/seq-full.hcl` 文件，使得其能够执行 `iaddq` 指令

指令的字节表示在书中练习题 `4.3`

本质上，`iaddq` 指令是 `irmovq` 和 `OPq` 指令的结合，我们按照书中的五个阶段分别写出其表示即可：

* `Fetch` 阶段：
  * `icode:ifun <- M[PC]`
  * `rA:rB <- M[PC + 1]`
  * `valC <- M[PC + 2]`
  * `valP <- PC + 10`

* `Decode` 阶段：
  * `valB <- R[rB]`

* `Execute` 阶段：
  * `valE <- valB + valC`
  * `set CC`

* `Memory` 阶段：
  * `NULL`

* `Write back` 阶段：
  * `R[rB] <- valE`

* `PC update` 阶段：
  * `PC <- valP`

在 `seq-ful.hcl` 文件中，`iaddq` 指令用 `IIADDQ` 表示，我们依次修改即可：

* `Fetch` 阶段：

```hcl
bool instr_valid = icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	       IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ, IIADDQ };

# Does fetched instruction require a regid byte?
bool need_regids =
	icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ };

# Does fetched instruction require a constant word?
bool need_valC =
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL, IIADDQ };
```

* `Decode & Write back` 阶段：

```hcl
## What register should be used as the B source?
word srcB = [
	icode in { IOPQ, IRMMOVQ, IMRMOVQ, IIADDQ } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];

## What register should be used as the E destination?
word dstE = [
	icode in { IRRMOVQ } && Cnd : rB;
	icode in { IIRMOVQ, IOPQ, IIADDQ } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];
```

* `Execute` 阶段：

```hcl
## Select input A to ALU
word aluA = [
	icode in { IRRMOVQ, IOPQ } : valA;
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ } : valC;
	icode in { ICALL, IPUSHQ } : -8;
	icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];

## Select input B to ALU
word aluB = [
	icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		      IPUSHQ, IRET, IPOPQ, IIADDQ } : valB;
	icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];

## Set the ALU function
word alufun = [
	icode == IOPQ : ifun;
	icode == IIADDQ : ifun;
	1 : ALUADD;
];

## Should the condition codes be updated?
bool set_cc = icode in { IOPQ, IIADDQ };
```

* `PC update & Memory` 阶段无需更改

如果要测试，我们需要先创建一个新的 `SEQ simulator`：

```bash
make VERSION=full
```

随后先测试一个简单的小程序：

```bash
$ ./ssim -t ../y86-code/asumi.yo
Y86-64 Processor: seq-full.hcl
137 bytes of code read
IF: Fetched irmovq at 0x0.  ra=----, rb=%rsp, valC = 0x100
IF: Fetched call at 0xa.  ra=----, rb=----, valC = 0x38
Wrote 0x13 to address 0xf8
IF: Fetched irmovq at 0x38.  ra=----, rb=%rdi, valC = 0x18
IF: Fetched irmovq at 0x42.  ra=----, rb=%rsi, valC = 0x4
IF: Fetched call at 0x4c.  ra=----, rb=----, valC = 0x56
Wrote 0x55 to address 0xf0
IF: Fetched xorq at 0x56.  ra=%rax, rb=%rax, valC = 0x0
IF: Fetched andq at 0x58.  ra=%rsi, rb=%rsi, valC = 0x0
IF: Fetched jmp at 0x5a.  ra=----, rb=----, valC = 0x83
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched ret at 0x8c.  ra=----, rb=----, valC = 0x0
IF: Fetched ret at 0x55.  ra=----, rb=----, valC = 0x0
IF: Fetched halt at 0x13.  ra=----, rb=----, valC = 0x0
32 instructions executed
Status = HLT
Condition Codes: Z=1 S=0 O=0
Changed Register State:
%rax:   0x0000000000000000      0x0000abcdabcdabcd
%rsp:   0x0000000000000000      0x0000000000000100
%rdi:   0x0000000000000000      0x0000000000000038
%r10:   0x0000000000000000      0x0000a000a000a000
Changed Memory State:
0x00f0: 0x0000000000000000      0x0000000000000055
0x00f8: 0x0000000000000000      0x0000000000000013
ISA Check Succeeds
```

运行成功

随后我们去测试标准程序：

```bash
$ make testssim
Makefile:42: 警告：忽略后缀规则定义的先决条件
Makefile:45: 警告：忽略后缀规则定义的先决条件
Makefile:48: 警告：忽略后缀规则定义的先决条件
Makefile:51: 警告：忽略后缀规则定义的先决条件
../seq/ssim -t asum.yo > asum.seq
../seq/ssim -t asumr.yo > asumr.seq
../seq/ssim -t cjr.yo > cjr.seq
../seq/ssim -t j-cc.yo > j-cc.seq
../seq/ssim -t poptest.yo > poptest.seq
../seq/ssim -t pushquestion.yo > pushquestion.seq
../seq/ssim -t pushtest.yo > pushtest.seq
../seq/ssim -t prog1.yo > prog1.seq
../seq/ssim -t prog2.yo > prog2.seq
../seq/ssim -t prog3.yo > prog3.seq
../seq/ssim -t prog4.yo > prog4.seq
../seq/ssim -t prog5.yo > prog5.seq
../seq/ssim -t prog6.yo > prog6.seq
../seq/ssim -t prog7.yo > prog7.seq
../seq/ssim -t prog8.yo > prog8.seq
../seq/ssim -t ret-hazard.yo > ret-hazard.seq
grep "ISA Check" *.seq
asum.seq:ISA Check Succeeds
asumr.seq:ISA Check Succeeds
cjr.seq:ISA Check Succeeds
j-cc.seq:ISA Check Succeeds
poptest.seq:ISA Check Succeeds
prog1.seq:ISA Check Succeeds
prog2.seq:ISA Check Succeeds
prog3.seq:ISA Check Succeeds
prog4.seq:ISA Check Succeeds
prog5.seq:ISA Check Succeeds
prog6.seq:ISA Check Succeeds
prog7.seq:ISA Check Succeeds
prog8.seq:ISA Check Succeeds
pushquestion.seq:ISA Check Succeeds
pushtest.seq:ISA Check Succeeds
ret-hazard.seq:ISA Check Succeeds
rm asum.seq asumr.seq cjr.seq j-cc.seq poptest.seq pushquestion.seq pushtest.seq prog1.seq prog2.seq prog3.seq prog4.seq prog5.seq prog6.seq prog7.seq prog8.seq ret-hazard.seq
```

虽然有警告，但我们的测试还是成功了

接着我们测试除 `iaddq` 以外的其他指令：

```bash
$ make SIM=../seq/ssim
./optest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 49 ISA Checks Succeed
./jtest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 64 ISA Checks Succeed
./ctest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 22 ISA Checks Succeed
./htest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 600 ISA Checks Succeed
```

全部成功

最后我们测试 `iaddq` 指令：

```bash
$ make SIM=../seq/ssim TFLAGS=-i
./optest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 58 ISA Checks Succeed
./jtest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 96 ISA Checks Succeed
./ctest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 22 ISA Checks Succeed
./htest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 756 ISA Checks Succeed
```

同样成功

至此该实验的前两个 `part` 完成

### Part C

我们先写一个模块化的 $2\times 1$ 循环展开：

```assembly
# Loop header
    xorq 	%rax,%rax      
    xorq 	%r13, %r13	# i = 0
	rrmovq 	%rdx, %r14
	iaddq 	$-1, %r14	# limit
	jmp 	Test1
Loop1:
	addq 	%r14, %r13
	mrmovq 	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	jle 	Loop2
	iaddq 	$1, %rax
Loop2:
	mrmovq 	8(%rdi), %r9
	andq 	%r9, %r9
	rmmovq 	%r9, 8(%rsi)
	jle 	Update
	iaddq 	$1, %rax
Update:

	iaddq 	$16, %rdi
	iaddq 	$16, %rsi
	iaddq 	$2, %r13

Test1:
	subq 	%r14, %r13	# i - limit
	jl 		Loop1
	addq 	%r14, %r13
	jmp 	Test2

Last1:
	addq 	%r13, %rdx
	mrmovq 	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	jle 	Last2
	iaddq 	$1, %rax
Last2:
	iaddq 	$8, %rdi
	iaddq 	$8, %rsi
	iaddq 	$1, %r13

Test2:
	subq 	%r13, %rdx
	jg 		Last1
```

执行：

```bash
$ ./correctness.pl
$ ./benchmark.pl 
Average CPE     11.42
Score   0.0/60.0
```

这里省略 `correctness.pl` 的执行结果，我们两次展开的 `CPE` 为 `11.42`，继续增加展开次数

我们拓展到六次展开（因为这是我们当前的寄存器只能支持我们在不往堆栈内写数据的情况下的最大值）：

> 这里我是想错了,实际 `mrmovq` 的目的寄存器不同并不会提高并发性
>
> 这是因为 `rmmovq` 最多只会占用目的寄存器一个周期（哪怕后面的指令需要用到目的寄存器，我们只会往流水线中插入一个气泡）

```assembly
# Loop header
    xorq 	%rax,%rax      
    xorq 	%r13, %r13	# i = 0
	rrmovq 	%rdx, %r14
	iaddq 	$-5, %r14	# limit
	jmp 	Test1
Loop1:
	addq 	%r14, %r13

	mrmovq 	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	jle 	Loop2
	iaddq 	$1, %rax
Loop2:
	mrmovq 	8(%rdi), %r9
	andq 	%r9, %r9
	rmmovq 	%r9, 8(%rsi)
	jle 	Loop3
	iaddq 	$1, %rax
Loop3:
	mrmovq 	16(%rdi), %r10
	andq 	%r10, %r10
	rmmovq 	%r10, 16(%rsi)
	jle 	Loop4
	iaddq 	$1, %rax
Loop4:
	mrmovq 	24(%rdi), %r10
	andq 	%r10, %r10
	rmmovq 	%r10, 24(%rsi)
	jle 	Loop5
	iaddq 	$1, %rax
Loop5:
	mrmovq 	32(%rdi), %r11
	andq 	%r11, %r11
	rmmovq 	%r11, 32(%rsi)
	jle 	Loop6
	iaddq 	$1, %rax
Loop6:
	mrmovq 	40(%rdi), %r12
	andq 	%r12, %r12
	rmmovq 	%r12, 40(%rsi)
	jle 	Update
	iaddq 	$1, %rax
Update:

	iaddq 	$48, %rdi
	iaddq 	$48, %rsi
	iaddq 	$6, %r13

Test1:
	subq 	%r14, %r13	# i - limit
	jl 		Loop1
	addq 	%r14, %r13
	jmp 	Test2

Last1:
	addq 	%r13, %rdx
	mrmovq 	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	jle 	Last2
	iaddq 	$1, %rax
Last2:
	iaddq 	$8, %rdi
	iaddq 	$8, %rsi
	iaddq 	$1, %r13

Test2:
	subq 	%r13, %rdx	# len - i
	jg 		Last1
```

得到的结果为：

```bash
Average CPE     10.12
Score   7.6/60.0
```

我们考虑每次从内存当中读取两个数，以此来减小气泡和增大并发性：

```assbegly
# Loop header
    xorq 	%rax,%rax      
    xorq 	%r13, %r13	# i = 0
	rrmovq 	%rdx, %r14
	iaddq 	$-5, %r14	# limit
	jmp 	Test1
Loop1:
	addq 	%r14, %r13

	mrmovq 	(%rdi), %r8
	mrmovq 	8(%rdi), %r9	
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	rmmovq 	%r9, 8(%rsi)	
	jle 	Loop2
	iaddq 	$1, %rax
Loop2:
	andq 	%r9, %r9
	jle 	Loop3
	iaddq 	$1, %rax
Loop3:
	mrmovq 	16(%rdi), %r10
	mrmovq 	24(%rdi), %r11	
	andq 	%r10, %r10
	rmmovq 	%r10, 16(%rsi)
	rmmovq 	%r11, 24(%rsi)	
	jle 	Loop4
	iaddq 	$1, %rax
Loop4:
	andq 	%r11, %r11
	jle 	Loop5
	iaddq 	$1, %rax
Loop5:
	mrmovq 	32(%rdi), %r12
	mrmovq 	40(%rdi), %rbx	
	andq 	%r12, %r12
	rmmovq 	%r12, 32(%rsi)
	rmmovq 	%rbx, 40(%rsi)	
	jle 	Loop6
	iaddq 	$1, %rax
Loop6:
	andq 	%rbx, %rbx
	jle 	Update
	iaddq 	$1, %rax
Update:

	iaddq 	$48, %rdi
	iaddq 	$48, %rsi
	iaddq 	$6, %r13

Test1:
	subq 	%r14, %r13	# i - limit
	jl 		Loop1
	addq 	%r14, %r13
	jmp 	Test2

# Test2 block

Last1:
	addq 	%r13, %rdx

	mrmovq 	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	jle 	Last2
	iaddq 	$1, %rax
Last2:
	iaddq 	$8, %rdi
	iaddq 	$8, %rsi
	iaddq 	$1, %r13

Test2:
	subq 	%r13, %rdx	# len - i
	jg 		Last1
```

运行结果：

```bash
Average CPE     9.29
Score   24.3/60.0
```

我们发现这个提示并不明显，我们考虑将 `i` 和 `limit` 去掉，重新设计整个的循环，这样可以减少一些指令的执行

> 我们在循环展开时，每次展开只需要使用两个寄存器即可

```assembly
# Loop header
    xorq 	%rax,%rax      
	jmp 	Test1
Loop1:

	mrmovq 	(%rdi), %r8
	mrmovq 	8(%rdi), %r9	
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	rmmovq 	%r9, 8(%rsi)
	jle 	Loop2
	iaddq 	$1, %rax
Loop2:
	andq 	%r9, %r9
	jle 	Loop3
	iaddq 	$1, %rax
Loop3:
	mrmovq 	16(%rdi), %r8
	mrmovq 	24(%rdi), %r9	
	andq 	%r8, %r8
	rmmovq 	%r8, 16(%rsi)
	rmmovq 	%r9, 24(%rsi)	
	jle 	Loop4
	iaddq 	$1, %rax
Loop4:
	andq 	%r9, %r9
	jle 	Loop5
	iaddq 	$1, %rax
Loop5:
	mrmovq 	32(%rdi), %r8
	mrmovq 	40(%rdi), %r9
	andq 	%r8, %r8
	rmmovq 	%r8, 32(%rsi)
	rmmovq 	%r9, 40(%rsi)	
	jle 	Loop6
	iaddq 	$1, %rax
Loop6:
	andq 	%r9, %r9
	jle 	Update
	iaddq 	$1, %rax
Update:

	iaddq 	$48, %rdi
	iaddq 	$48, %rsi

Test1:
	iaddq 	$-6, %rdx	# len - 6
	jge		Loop1
	iaddq 	$6, %rdx
	jmp 	Test2
```

运行结果：

```bash
        ncopy
0       22
1       31      31.00
2       43      21.50
3       52      17.33
4       64      16.00
5       73      14.60
6       59      9.83
7       68      9.71
8       80      10.00
9       89      9.89
10      101     10.10
11      110     10.00
12      96      8.00
13      105     8.08
14      117     8.36
15      126     8.40
16      138     8.62
17      147     8.65
18      133     7.39
19      142     7.47
20      154     7.70
21      163     7.76
22      175     7.95
23      184     8.00
24      170     7.08
25      179     7.16
26      191     7.35
27      200     7.41
28      212     7.57
29      221     7.62
30      207     6.90
31      216     6.97
32      228     7.12
33      237     7.18
34      249     7.32
35      258     7.37
36      244     6.78
37      253     6.84
38      265     6.97
39      274     7.03
40      286     7.15
41      295     7.20
42      281     6.69
43      290     6.74
44      302     6.86
45      311     6.91
46      323     7.02
47      332     7.06
48      318     6.62
49      327     6.67
50      339     6.78
51      348     6.82
52      360     6.92
53      369     6.96
54      355     6.57
55      364     6.62
56      376     6.71
57      385     6.75
58      397     6.84
59      406     6.88
60      392     6.53
61      401     6.57
62      413     6.66
63      422     6.70
64      434     6.78
Average CPE     8.45
Score   40.9/60.0
```

