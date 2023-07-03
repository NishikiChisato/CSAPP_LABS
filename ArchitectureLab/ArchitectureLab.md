# arclab

- [arclab](#arclab)
	- [准备](#准备)
	- [环境配置](#环境配置)
	- [实验开始](#实验开始)
		- [Part A](#part-a)
		- [Part B](#part-b)
		- [Part C](#part-c)
		- [尾声](#尾声)


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

参照 [PDF](http://csapp.cs.cmu.edu/3e/archlab.pdf) 中 `6` 的部分

本阶段需要我们修改 `ncopy.ys` 和 `pipe-full.hcl` ，使得该代码能够尽可能运行的更快

我们首先需要修改 `pipe-full.hcl`，使得其能够支持 `iaddq` 指令，这样我们可以少写一条汇编指令

修改部分如下：

```HCL
# Is instruction valid?
bool instr_valid = f_icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	  IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ, IIADDQ };

# Does fetched instruction require a regid byte?
bool need_regids =
	f_icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ };

# Does fetched instruction require a constant word?
bool need_valC =
	f_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL, IIADDQ };

## What register should be used as the B source?
word d_srcB = [
	D_icode in { IOPQ, IRMMOVQ, IMRMOVQ, IIADDQ  } : D_rB;
	D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];

## What register should be used as the E destination?
word d_dstE = [
	D_icode in { IRRMOVQ, IIRMOVQ, IOPQ, IIADDQ } : D_rB;
	D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];

## Select input A to ALU
word aluA = [
	E_icode in { IRRMOVQ, IOPQ } : E_valA;
	E_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ } : E_valC;
	E_icode in { ICALL, IPUSHQ } : -8;
	E_icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];

## Select input B to ALU
word aluB = [
	E_icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		     IPUSHQ, IRET, IPOPQ, IIADDQ } : E_valB;
	E_icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];

## Set the ALU function
word alufun = [
	E_icode == IOPQ : E_ifun;
	E_icode == IIADDQ : E_ifun;
	1 : ALUADD;
];

## Should the condition codes be updated?
bool set_cc = E_icode == IOPQ || E_icode == IIADDQ &&
	# State changes only during normal operation
	!m_stat in { SADR, SINS, SHLT } && !W_stat in { SADR, SINS, SHLT };
```

这里与 `Part B` 基本一致，只要那个能做，这里也没问题

我们首先给出如下代码：


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

# Test2 block

Last1:

	mrmovq 	(%rdi), %r8
	andq 	%r8, %r8
	rmmovq 	%r8, (%rsi)
	jle 	Last2
	iaddq 	$1, %rax
Last2:
	iaddq 	$8, %rdi
	iaddq 	$8, %rsi

Test2:
	iaddq 	$-1, %rdx	
	jge 		Last1
```

运行结果：

```bash
./benchmark.pl
Average CPE     8.45
Score   40.9/60.0
```

需要说明的几点：

* 这里我们循环展开的此时为 `6` 次，可以继续增大展开次数以此提升 `CPE`
  * 循环展开**并不会降低总的操作次数**，只会**降低循环的次数**，循环的索引计算和分支计算都会减少
* 在单个 `Loop` 中，我们连续对两个内存进行读取，随后再执行判断语句。这么做是为了消除加载/使用冒险
  * 如果前一条语句从内存当中读取某个值到寄存器，下一条马上使用该寄存器，那么流水线控制逻辑会在其中插入一个气泡。使得当前一条指令进入写回阶段时，后一条指令进入译码阶段
* 我们直接使用 `len` 是否小于零来进行循环次数的判断，这样可以执行更少的指令

我们从循环展开部分的代码可以得知，**对单个元素进行操作时，必须要执行五条汇编指令**

上面代码的问题是，我们六路循环展开后会得到一些剩余元素，元素个数为从 `0` 到 `5` ，我们对这些元素采取的是循环的方式

我们知道，循环是会产生额外的判断和索引计算，因此对于最后的元素，根据其执行次数不同，我们可以将其**倒序**进行排列，然后依据剩余元素个数跳转到对应的位置即可

我们将循环展开的次数提升到八次，写出如下代码：

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

需要说明的是：
 
* 数据的输入个数为 `0` 到 `64` ，并且我们是求每次算出来的 `CPE` 的平均值，因此所有数量的权重相同，我们应当尽可能减小余数为 `0, 1, 2, 4` 的 `CPE`，也就是让这些情况提前跳转

* 在控制逻辑的跳转部分，我们先让 `2` 和 `3` 进行跳转，随后才是 `1`，这样子总的 `CPE` 会更小

可以看到这次的 `CPE` 有了很大的提升，我们将循环展开增加到 `10` 次

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

对应的 `CPE` 也达到了 `7.6` ，距离满分的 `7.5` 只差 `0.1`，如果需要继续优化的话，只能从控制逻辑上下手

去寻找一个更优的控制逻辑，或者尝试去实现对 `pipe-ful.hcl` 进行修改，实现条件传送指令

### 尾声

在我们对 `psim` 执行测试时，有如下输出：

```bash
$ make SIM=../pipe/psim
./optest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
  All 49 ISA Checks Succeed
./jtest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
Test jf-je-32-64 failed
Test jf-jge-32-64 failed
Test jf-jg-32-64 failed
Test jb-jmp-32-32 failed
Test jb-jmp-32-64 failed
Test jb-jmp-64-32 failed
Test jb-jmp-64-64 failed
Test jb-jle-32-32 failed
Test jb-jle-32-64 failed
Test jb-jle-64-64 failed
Test jb-jl-32-64 failed
Test jb-je-32-32 failed
Test jb-je-64-64 failed
Test jb-jne-32-64 failed
Test jb-jne-64-32 failed
Test jb-jge-32-32 failed
Test jb-jge-64-32 failed
Test jb-jge-64-64 failed
Test jb-jg-64-32 failed
Test jb-call-32-32 failed
Test jb-call-32-64 failed
Test jb-call-64-32 failed
Test jb-call-64-64 failed
  23/64 ISA Checks Failed
./ctest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
  All 22 ISA Checks Succeed
./htest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
  All 600 ISA Checks Succeed
```

但我们最后显示 `600` 条 `ISA` 全部测试成功

在 `Part B` 中我们知道，这里只有 `600` 条指令，全部测试成功表示通过，并且我们在 `ncopy.ys` 中使用 `je, jl, jg` 并没有报错

也就是这三条指令是被正确执行的，这里显示错误我确实十分的疑惑，但我确实不知道为什么，所以只能留在这里了