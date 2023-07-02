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

Average CPE     8.45
Score   40.9/60.0
```

```assembly
# Loop header
    xorq 	%rax,%rax
	iaddq 	$-8, %rdx
	jl 		Root		# len < 8
	
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
	jle 	Loop7
	iaddq 	$1, %rax
Loop7:
	mrmovq 	48(%rdi), %r8
	mrmovq 	56(%rdi), %r9
	andq 	%r8, %r8
	rmmovq 	%r8, 48(%rsi)
	rmmovq 	%r9, 56(%rsi)	
	jle 	Loop8
	iaddq 	$1, %rax
Loop8:
	andq 	%r9, %r9
	jle 	Update
	iaddq 	$1, %rax

Update:
	iaddq 	$64, %rdi
	iaddq 	$64, %rsi

Test1:
	iaddq 	$-8, %rdx	# len - 8
	jge		Loop1		


# len in [0, 1, ..., 7]
Root:
	iaddq	$4, %rdx
	jl		Left		# len < 4
	jg 		Right		# len > 4	
	je 		R4			# len = 4

# len in [0, 1, 2, 3]
Left:
	iaddq 	$1, %rdx
	je		R3			# len = 3
	iaddq	$1, %rdx	
	je		R2			# len = 2
	iaddq	$1, %rdx
	je 		R1			# len = 1
	iaddq	$1, %rdx
	je		Done


# len in [5, 6, 7]
Right:
	iaddq 	$-1, %rdx
	je		R5			# len = 5
	iaddq	$-1, %rdx
	je		R6			# len = 6
	iaddq	$-1, %rdx
	je 		R7			# len = 7

R7:
	mrmovq	48(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 48(%rsi)
	jle		R6
	iaddq 	$1, %rax
R6:
	mrmovq	40(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 40(%rsi)
	jle		R5
	iaddq 	$1, %rax
R5:
	mrmovq	32(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 32(%rsi)
	jle		R4
	iaddq 	$1, %rax
R4:
	mrmovq	24(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 24(%rsi)
	jle		R3
	iaddq 	$1, %rax
R3:
	mrmovq	16(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 16(%rsi)
	jle		R2
	iaddq 	$1, %rax
R2:
	mrmovq	8(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 8(%rsi)
	jle		R1
	iaddq 	$1, %rax
R1:
	mrmovq	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, (%rsi)
	jle		Done
	iaddq 	$1, %rax
```

测试结果：

```bash
Average CPE     7.87
Score   52.5/60.0
```

```assembly
# Loop header
    #xorq 	%rax,%rax
	iaddq 	$-10, %rdx
	jl 		Root		# len < 8
	
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
	jle 	Loop7
	iaddq 	$1, %rax
Loop7:
	mrmovq 	48(%rdi), %r8
	mrmovq 	56(%rdi), %r9
	andq 	%r8, %r8
	rmmovq 	%r8, 48(%rsi)
	rmmovq 	%r9, 56(%rsi)	
	jle 	Loop8
	iaddq 	$1, %rax
Loop8:
	andq 	%r9, %r9
	jle 	Loop9
	iaddq 	$1, %rax

Loop9:
	mrmovq 	64(%rdi), %r8
	mrmovq 	72(%rdi), %r9
	andq 	%r8, %r8
	rmmovq 	%r8, 64(%rsi)
	rmmovq	%r9, 72(%rsi)
	jle 	Loop10
	iaddq 	$1, %rax
Loop10:
	andq	%r9, %r9
	jle 	Update
	iaddq	$1, %rax

Update:
	iaddq 	$80, %rdi
	iaddq 	$80, %rsi

Test1:
	iaddq 	$-10, %rdx	# len - 8
	jge		Loop1		


# len in [0, 1, ..., 9]
Root:
	iaddq	$6, %rdx
	jl		Left		# len < 4
	jg 		Right		# len > 4	
	je 		R4			# len = 4

# len in [0, 1, 2, 3]
Left:
	iaddq 	$2, %rdx
	jg		R3			# len = 3
	je		R2			# len = 2
	iaddq	$1, %rdx
	je 		R1			# len = 1
	jl		Done		# len = 0


# len in [5, 6, 7, 8, 9]
Right:
	iaddq 	$-2, %rdx
	jl		R5			# len = 5
	je		R6			# len = 6
	iaddq	$-1, %rdx
	je 		R7			# len = 7
	iaddq 	$-1, %rdx
	je 		R8			# len = 8

R9:
	mrmovq	64(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 64(%rsi)
	jle		R8
	iaddq 	$1, %rax
R8:
	mrmovq	56(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 56(%rsi)
	jle		R7
	iaddq 	$1, %rax
R7:
	mrmovq	48(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 48(%rsi)
	jle		R6
	iaddq 	$1, %rax
R6:
	mrmovq	40(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 40(%rsi)
	jle		R5
	iaddq 	$1, %rax
R5:
	mrmovq	32(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 32(%rsi)
	jle		R4
	iaddq 	$1, %rax
R4:
	mrmovq	24(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 24(%rsi)
	jle		R3
	iaddq 	$1, %rax
R3:
	mrmovq	16(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 16(%rsi)
	jle		R2
	iaddq 	$1, %rax
R2:
	mrmovq	8(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 8(%rsi)
	jle		R1
	iaddq 	$1, %rax
R1:
	mrmovq	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, (%rsi)
	jle		Done
	iaddq 	$1, %rax
```

测试结果：

```bash
        ncopy
0       26
1       28      28.00
2       32      16.00
3       34      11.33
4       46      11.50
5       50      10.00
6       61      10.17
7       70      10.00
8       82      10.25
9       89      9.89
10      89      8.90
11      91      8.27
12      95      7.92
13      97      7.46
14      109     7.79
15      113     7.53
16      124     7.75
17      133     7.82
18      145     8.06
19      152     8.00
20      148     7.40
21      150     7.14
22      154     7.00
23      156     6.78
24      168     7.00
25      172     6.88
26      183     7.04
27      192     7.11
28      204     7.29
29      211     7.28
30      207     6.90
31      209     6.74
32      213     6.66
33      215     6.52
34      227     6.68
35      231     6.60
36      242     6.72
37      251     6.78
38      263     6.92
39      270     6.92
40      266     6.65
41      268     6.54
42      272     6.48
43      274     6.37
44      286     6.50
45      290     6.44
46      301     6.54
47      310     6.60
48      322     6.71
49      329     6.71
50      325     6.50
51      327     6.41
52      331     6.37
53      333     6.28
54      345     6.39
55      349     6.35
56      360     6.43
57      369     6.47
58      381     6.57
59      388     6.58
60      384     6.40
61      386     6.33
62      390     6.29
63      392     6.22
64      404     6.31
Average CPE     7.76
Score   54.9/60.0
```

上面的代码中，在

```assembly
R9:
	mrmovq	64(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 64(%rsi)
	jle		R8
	iaddq 	$1, %rax
```

当中还是插入了一个气泡，这是因为第一条语句对 `%r8` 进行写入后，第二条语句马上就要使用

**我们必须在 `mrmovq` 到达访存阶段时，才能执行 `andq` 的译码阶段**

我们考虑去消除这个气泡，由于每一个循环必须要执行 `5` 条指令，因此如果我们当前这个循环执行的是上一条语句的跳转指令的话，也就可以避免当前循环中跳转指令与访存指令的冲突

这样有一个问题是，当前周期会执行一次上一周期的跳转，为了避免执行该额外的跳转，我们的控制逻辑需要**保证每次跳转的状态码都是小于等于零**

```assembly
# Loop header
    #xorq 	%rax,%rax
	iaddq 	$-10, %rdx
	jl 		Root		# len < 8
	
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
	jle 	Loop7
	iaddq 	$1, %rax
Loop7:
	mrmovq 	48(%rdi), %r8
	mrmovq 	56(%rdi), %r9
	andq 	%r8, %r8
	rmmovq 	%r8, 48(%rsi)
	rmmovq 	%r9, 56(%rsi)	
	jle 	Loop8
	iaddq 	$1, %rax
Loop8:
	andq 	%r9, %r9
	jle 	Loop9
	iaddq 	$1, %rax

Loop9:
	mrmovq 	64(%rdi), %r8
	mrmovq 	72(%rdi), %r9
	andq 	%r8, %r8
	rmmovq 	%r8, 64(%rsi)
	rmmovq	%r9, 72(%rsi)
	jle 	Loop10
	iaddq 	$1, %rax
Loop10:
	andq	%r9, %r9
	jle 	Update
	iaddq	$1, %rax

Update:
	iaddq 	$80, %rdi
	iaddq 	$80, %rsi

Test1:
	iaddq 	$-10, %rdx	# len - 8
	jge		Loop1		


# len in [0, 1, ..., 9]
Root:
	iaddq	$6, %rdx	# len - 4
	jl		Left			# len < 4
	jg 		Right			# len > 4	
	je 		R4				# len = 4

# len in [0, 1, 2, 3]
Left:
	iaddq 	$1, %rdx	# len - 3
	je		R3				# len = 3
	iaddq	$1, %rdx	# len - 2
	je		R2				# len = 2
	iaddq	$1, %rdx	# len - 1
	je 		R1				# len = 1
	ret


# len in [5, 6, 7, 8, 9]
Right:
	iaddq 	$-2, %rdx	# len - 6
	jl		R5				# len = 5
	je		R6				# len = 6
	iaddq	$-1, %rdx	# len - 7
	je 		R7				# len = 7
	iaddq 	$-1, %rdx	# len - 8
	je 		R8				# len = 8



R9:
	mrmovq	64(%rdi), %r8
	andq 	%r8, %r8
	rmmovq	%r8, 64(%rsi)
R8:
	mrmovq	56(%rdi), %r8
	jle		R81
	iaddq 	$1, %rax
R81:
	rmmovq	%r8, 56(%rsi)
	andq 	%r8, %r8
R7:
	mrmovq	48(%rdi), %r8
	jle		R71
	iaddq 	$1, %rax
R71:
	rmmovq	%r8, 48(%rsi)
	andq 	%r8, %r8
R6:
	mrmovq	40(%rdi), %r8
	jle		R61
	iaddq 	$1, %rax
R61:
	rmmovq	%r8, 40(%rsi)	
	andq 	%r8, %r8
R5:
	mrmovq	32(%rdi), %r8
	jle		R51
	iaddq 	$1, %rax
R51:
	rmmovq	%r8, 32(%rsi)
	andq 	%r8, %r8
R4:
	mrmovq	24(%rdi), %r8
	jle		R41
	iaddq 	$1, %rax
R41:
	rmmovq	%r8, 24(%rsi)
	andq 	%r8, %r8
R3:
	mrmovq	16(%rdi), %r8
	jle		R31
	iaddq 	$1, %rax
R31:
	rmmovq	%r8, 16(%rsi)
	andq 	%r8, %r8
R2:
	mrmovq	8(%rdi), %r8
	jle		R21
	iaddq 	$1, %rax
R21:
	rmmovq	%r8, 8(%rsi)
	andq 	%r8, %r8
R1:
	mrmovq	(%rdi), %r8
	jle		R11
	iaddq 	$1, %rax
R11:
	andq 	%r8, %r8
	rmmovq	%r8, (%rsi)
	jle		Done
	iaddq 	$1, %rax
```

测试结果：

```bash
        ncopy
0       26
1       29      29.00
2       32      16.00
3       32      10.67
4       43      10.75
5       46      9.20
6       56      9.33
7       64      9.14
8       75      9.38
9       81      9.00
10      89      8.90
11      92      8.36
12      95      7.92
13      95      7.31
14      106     7.57
15      109     7.27
16      119     7.44
17      127     7.47
18      138     7.67
19      144     7.58
20      148     7.40
21      151     7.19
22      154     7.00
23      154     6.70
24      165     6.88
25      168     6.72
26      178     6.85
27      186     6.89
28      197     7.04
29      203     7.00
30      207     6.90
31      210     6.77
32      213     6.66
33      213     6.45
34      224     6.59
35      227     6.49
36      237     6.58
37      245     6.62
38      256     6.74
39      262     6.72
40      266     6.65
41      269     6.56
42      272     6.48
43      272     6.33
44      283     6.43
45      286     6.36
46      296     6.43
47      304     6.47
48      315     6.56
49      321     6.55
50      325     6.50
51      328     6.43
52      331     6.37
53      331     6.25
54      342     6.33
55      345     6.27
56      355     6.34
57      363     6.37
58      374     6.45
59      380     6.44
60      384     6.40
61      387     6.34
62      390     6.29
63      390     6.19
64      401     6.27
Average CPE     7.60
Score   58.1/60.0
```

这个代码的长度达到了 `999` 个字节，已经是达到极限了

如果需要继续优化的话，可以从控制逻辑上下手