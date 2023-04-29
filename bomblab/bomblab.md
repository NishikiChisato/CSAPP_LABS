# bomblab

- [bomblab](#bomblab)
  - [准备](#准备)
    - [文件下载](#文件下载)
    - [文件处理](#文件处理)
    - [提交](#提交)
  - [炸弹拆除](#炸弹拆除)
    - [phase\_1](#phase_1)
    - [phase\_2](#phase_2)
    - [phase\_3](#phase_3)
    - [phase\_4](#phase_4)
    - [phase\_5](#phase_5)


## 准备

### 文件下载

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

### 文件处理

将文件解包后会得到三个文件：`README`、`bomb`、`bomb.c`，我们首先利用 `bojdump` 生成 `bomb` 的汇编文件，并将其保存在 `disassemble.asm` 当中

```bash
objdump -d bomb > disassemble.asm
```

这里推荐使用 `vscode` 进行后续实验，下载拓展 `x86 and x86_64 Assembly` 可以对汇编文件进行高亮显示

### 提交

本实验要求我们**按顺序输入每个炸弹的密码**，也可以将所有的密码**全部保存在一个文件里面**，具体在 `bomb.c` 当中的代码如下：

```c
    char *input;
    if (argc == 1) {  
	infile = stdin;
    } 
    else if (argc == 2) {
	if (!(infile = fopen(argv[1], "r"))) {
	    printf("%s: Error: Couldn't open %s\n", argv[0], argv[1]);
	    exit(8);
	}
    }
    else {
	printf("Usage: %s [<input_file>]\n", argv[0]);
	exit(8);
    }
```

例如，如果将所有答案保存在文件 `ans` 当中，可以输入 `./bomb ans` 进行提交（`ans` 中最后需要空一行）

---

## 炸弹拆除

### phase_1

汇编代码如下：

```asm
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi
  400ee9:	e8 4a 04 00 00       	call   401338 <strings_not_equal>
  400eee:	85 c0                	test   %eax,%eax
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>
  400ef2:	e8 43 05 00 00       	call   40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	ret    
```

首先进入 `GDB` 调试：`GDB bomb`，在函数 `pahse_1` 出打断点：`break phase_1`

观察到有条件跳转指令，我们首先查看跳转目的的地址：`x phase_1 + 0x17`，得到：`0x400ef7 <phase_1+23>:  0x08c48348`

也就是说，寄存器 `%eax` 的值必须等于零，否则将会执行 `explode_bomb` 函数（阶段一失败）

`%esi` 用于保存函数的第一个调用参数，`%eax` 用于保存函数的返回值，因此这两个寄存器的值正好对应了 `strings_not_equal` 函数的参数与返回值

注意到这个函数用于比较字符串，因此我们以字符串的形式打印 `0x402400`，输入：`x /s 0x402400`，得到：`Border relations with Canada have never been better.`

这便是 `phase_1` 的答案

### phase_2

汇编代码如下：

`phase_2` 代码：

```asm
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp
  400f02:	48 89 e6             	mov    %rsp,%rsi
  400f05:	e8 52 05 00 00       	call   40145c <read_six_numbers>
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	call   40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
  400f1a:	01 c0                	add    %eax,%eax
  400f1c:	39 03                	cmp    %eax,(%rbx)
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	call   40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	ret    
```

`read_six_numbers` 代码：

```asm
000000000040145c <read_six_numbers>:
  40145c:	48 83 ec 18          	sub    $0x18,%rsp
  401460:	48 89 f2             	mov    %rsi,%rdx
  401463:	48 8d 4e 04          	lea    0x4(%rsi),%rcx
  401467:	48 8d 46 14          	lea    0x14(%rsi),%rax
  40146b:	48 89 44 24 08       	mov    %rax,0x8(%rsp)
  401470:	48 8d 46 10          	lea    0x10(%rsi),%rax
  401474:	48 89 04 24          	mov    %rax,(%rsp)
  401478:	4c 8d 4e 0c          	lea    0xc(%rsi),%r9
  40147c:	4c 8d 46 08          	lea    0x8(%rsi),%r8
  401480:	be c3 25 40 00       	mov    $0x4025c3,%esi
  401485:	b8 00 00 00 00       	mov    $0x0,%eax
  40148a:	e8 61 f7 ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  40148f:	83 f8 05             	cmp    $0x5,%eax
  401492:	7f 05                	jg     401499 <read_six_numbers+0x3d>
  401494:	e8 a1 ff ff ff       	call   40143a <explode_bomb>
  401499:	48 83 c4 18          	add    $0x18,%rsp
  40149d:	c3                   	ret 
```

在 `phase_2` 函数中，首先分配了 `40` 字节的栈帧，并**将当前栈指针 `%rsp` 作为第一个参数 `$rsi` 传递到 `read_six_numbers` 当中**

在 `read_six_numbers` 当中，首先分配了 `24` 字节的栈帧，分别将寄存器或内存 `%rsi`、`%0x4(%rsi)`、`0x14(%rsi)`、`0x10(%rsi)`、`0xc(%rsi)`、`0x8(%rsi)` 对应到 `$rdx`、`%rcx`、`0x8(%rsp)`、`(%rsp)`、`%r8`、`%r9`

注意到，前面 `6` 个对应于 `pahse_2` 函数栈帧当中，而后面的 `6` 个则存储前面 `6` 个的内存地址，也就是说 `read_six_numbers` 函数的作用是为 `phase_2` 函数读取 `6` 个数

我们可以检查 `0x4025c3` ：`x /s 0x4025c3`，得到：`"%d %d %d %d %d %d"`，这可以验证我们的结论

我们从下往上用 `1` 到 `6` 分半对这几个数进行标号，需要说明的是，这六个数，**每两个数之间均是差 `4` 个字节**

在 `phase_2` 中，`read_six_numbers` 要求第一个数必须是 `1`：

```asm
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	call   40143a <explode_bomb>
```

我们重点关注后面的代码：

```asm
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
  400f1a:	01 c0                	add    %eax,%eax
  400f1c:	39 03                	cmp    %eax,(%rbx)
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	call   40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	ret    
```

当跳到 `0x400f30` 后，将第二个数的地址与 `0x18(%rsp)` 加载到 `%rbx` 和 `%rbp` 中，随后跳到 `0x400f17`

然后将第一个数加载到 `%eax` 中，将其乘二与 `%rbx` 进行对比，如果相等的话则跳到 `0x400f25`

此时 `%rbx` 变为第三个数，将其与 `rbp` 比较，如果不等的话则重复刚刚的过程

我们由此可以得出：此函数会**从下到上**依次检查**上面的数是否为下面的数的两倍**，因此最终答案为：`1 2 4 8 16 32`

### phase_3

汇编代码如下：

```asm
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	call   40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
  400f75:	ff 24 c5 70 24 40 00 	jmp    *0x402470(,%rax,8)
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	call   40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	call   40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	ret    
```

有了前面的经验，我们首先检查 `0x0x4025cf`，得到：`0x4025cf:  "%d %d"`，也就是说 `phase_3` 会读入两个数，我们设第一个为 `x` ，第二个为 `y`（分别存放在 `0x8(%rsp)` 和 `0xc(%esp)`），检查完输入后跳转到 `0x400f6a`

此时检查 `x` 的大小，如果大于 `7` 则直接失败（我们还可以得到，`x` 为无符号整数，因此其大小最小为 `0`，最大为 `7`）

之后对输入的 `x` 进行跳转，分别有八种情况，对应 `x` 的八种取值，将不同的值复制到 `%eax` 后，最后都会跳到 `0x400fbe`

此时会比较 `y` 与 `%eax` 的值，如果不相同则失败

我们依次检查 `x` 的不同取值所对应的跳转地址（`x` 每次会自增 `8`）：

```gdb
(gdb) x 0x402470
0x402470:       0x00400f7c
(gdb) x 0x402470 + 8
0x402478:       0x00400fb9
(gdb) x 0x402470 + 16
0x402480:       0x00400f83
(gdb) x 0x402470 + 24
0x402488:       0x00400f8a
(gdb) x 0x402470 + 32
0x402490:       0x00400f91
(gdb) x 0x402470 + 40
0x402498:       0x00400f98
(gdb) x 0x402470 + 48
0x4024a0:       0x00400f9f
(gdb) x 0x402470 + 56
0x4024a8:       0x00400fa6
```

取每个跳转目的的值，结果如下表：

|`x`|`y`|
|:-:|:-:|
|`0`|`207`|
|`1`|`311`|
|`2`|`707`|
|`3`|`256`|
|`4`|`389`|
|`5`|`206`|
|`6`|`682`|
|`7`|`327`|

### phase_4

汇编代码如下：

`phase_4` 代码：

```asm
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
  401033:	76 05                	jbe    40103a <phase_4+0x2e>
  401035:	e8 00 04 00 00       	call   40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	call   400fce <func4>
  40104d:	85 c0                	test   %eax,%eax
  40104f:	75 07                	jne    401058 <phase_4+0x4c>
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
  401056:	74 05                	je     40105d <phase_4+0x51>
  401058:	e8 dd 03 00 00       	call   40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	ret
```

前面的几行与 `phase_3` 一致，都是读取两个数，分别放在 `0x8(%rsp)` 和 `%0xc(rsp)`，我们分别设为 `x` 和 `y`

我们看后面的代码：

```asm
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
  401033:	76 05                	jbe    40103a <phase_4+0x2e>
  401035:	e8 00 04 00 00       	call   40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	call   400fce <func4>
  40104d:	85 c0                	test   %eax,%eax
  40104f:	75 07                	jne    401058 <phase_4+0x4c>
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
  401056:	74 05                	je     40105d <phase_4+0x51>
  401058:	e8 dd 03 00 00       	call   40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	ret
```

可以得知，`x` 必须小于 `14`（也就是 `0xe`），此时将 `x`、`0`、`12` **按顺序**传入 `func4`

并且我们得知，`func4` 的返回值必须为 `0`，`y` 的值必须为 `0`

要确定 `x` 的值，我们只能去分析 `func4` ，代码如下：

```asm
0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp
  400fd2:	89 d0                	mov    %edx,%eax
  400fd4:	29 f0                	sub    %esi,%eax
  400fd6:	89 c1                	mov    %eax,%ecx
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx
  400fdb:	01 c8                	add    %ecx,%eax
  400fdd:	d1 f8                	sar    %eax
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx
  400fe2:	39 f9                	cmp    %edi,%ecx
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24>
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx
  400fe9:	e8 e0 ff ff ff       	call   400fce <func4>
  400fee:	01 c0                	add    %eax,%eax
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax
  400ff7:	39 f9                	cmp    %edi,%ecx
  400ff9:	7d 0c                	jge    401007 <func4+0x39>
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
  400ffe:	e8 cb ff ff ff       	call   400fce <func4>
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
  401007:	48 83 c4 08          	add    $0x8,%rsp
  40100b:	c3
```

对于参数，我们需要明确：

* `%rdi` 保存第一个参数，初始值为 `x` 
* `%rsi` 保存第二个参数，初始值为 `0`
* `%rdx` 保存第三个参数，初始值为 `12`

我们可以得到 `func4` 的声明：`int func4(int x, int y, int z)`

我们设 `%eax` 当中存储的变量为 `val`，那么：

```asm
  400fce:	48 83 ec 08          	sub    $0x8,%rsp
  400fd2:	89 d0                	mov    %edx,%eax
  400fd4:	29 f0                	sub    %esi,%eax
  400fd6:	89 c1                	mov    %eax,%ecx
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx
  400fdb:	01 c8                	add    %ecx,%eax
  400fdd:	d1 f8                	sar    %eax
```

这些可以等价为：

```c
val = z - y;
//%ecx 当中的值设为 k 
k = val >> 31;
val = (val + k) >> 1;
```

当 `y = 0`，`z = 12` 时，`val = 7`

后面将 `val + y` 加载到 `%ecx` 中，并将其与 `x` 进行比较

如果 `val + y` **大于** `x` ，那么调用 `func4(x, y, val + y - 1)`

如果 `val + y` **等于** `x` ，则返回 `0` ，这是我们希望的

如果 `val + y` **小于** `x` ，则调用 `func4(val + y + 1, y, z)`

我们整理一下 `func4` ，如下：

```c
int func4(int x, int y, iny z)
{
    int val = z - y;
    val = (val + (val >> 31)) >> 1;
    if(val + y > x) return func4(x, y, val + y - 1);
    else if(val + y < x) return func4(val + y + 1, y, z);
    else return 0;
}
```

此时我们发现，只要走 `val + y < x` 那条分支，最终一定会导致死循环，最终会一直走到 `func4(8, 0, 12)` 和 `func4(4, 0, 6)` 这两个状态

因此我们枚举 `val + y > x` 的分支，得到 `x` 可能的取值为 `7`、`3`、`1`、`0`

总的取值如下表：

|`x`|`y`|
|:-:|:-:|
|`0`|`0`|
|`1`|`0`|
|`3`|`0`|
|`7`|`0`|

### phase_5

汇编代码如下：

```asm
0000000000401062 <phase_5>:
  401062:	53                   	push   %rbx
  401063:	48 83 ec 20          	sub    $0x20,%rsp
  401067:	48 89 fb             	mov    %rdi,%rbx
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
  401078:	31 c0                	xor    %eax,%eax
  40107a:	e8 9c 02 00 00       	call   40131b <string_length>
  40107f:	83 f8 06             	cmp    $0x6,%eax
  401082:	74 4e                	je     4010d2 <phase_5+0x70>
  401084:	e8 b1 03 00 00       	call   40143a <explode_bomb>
  401089:	eb 47                	jmp    4010d2 <phase_5+0x70>
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx
  40108f:	88 0c 24             	mov    %cl,(%rsp)
  401092:	48 8b 14 24          	mov    (%rsp),%rdx
  401096:	83 e2 0f             	and    $0xf,%edx
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)
  4010a4:	48 83 c0 01          	add    $0x1,%rax
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi
  4010bd:	e8 76 02 00 00       	call   401338 <strings_not_equal>
  4010c2:	85 c0                	test   %eax,%eax
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>
  4010c6:	e8 6f 03 00 00       	call   40143a <explode_bomb>
  4010cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
  4010d0:	eb 07                	jmp    4010d9 <phase_5+0x77>
  4010d2:	b8 00 00 00 00       	mov    $0x0,%eax
  4010d7:	eb b2                	jmp    40108b <phase_5+0x29>
  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
  4010e5:	00 00 
  4010e7:	74 05                	je     4010ee <phase_5+0x8c>
  4010e9:	e8 42 fa ff ff       	call   400b30 <__stack_chk_fail@plt>
  4010ee:	48 83 c4 20          	add    $0x20,%rsp
  4010f2:	5b                   	pop    %rbx
  4010f3:	c3
```

事先说明一点，以下几行为金丝雀，直接忽略就行

```asm
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)

  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
```

