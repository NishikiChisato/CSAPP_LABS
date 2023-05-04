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
    - [phase\_6](#phase_6)
    - [secret\_phase](#secret_phase)


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

```c
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

```c
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

```c
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

```c
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	call   40143a <explode_bomb>
```

我们重点关注后面的代码：

```c
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

```c
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

```c
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

```c
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

```c
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

```c
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

```c
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

```c
  40106a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401071:	00 00 
  401073:	48 89 44 24 18       	mov    %rax,0x18(%rsp)

  4010d9:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
  4010de:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
```

该函数首先分配 `32` 字节的栈帧，然后将**输入参数 `%rdi` 移到 `%rbx` 中**，并且要求输入字符串（存在 `%rdi` 中）的长度必须为 `6`

将寄存器 `%eax` 的值清零后跳到 `0x40108b`，代码如下：

```c
  40108b:	0f b6 0c 03          	movzbl (%rbx,%rax,1),%ecx
  40108f:	88 0c 24             	mov    %cl,(%rsp)
  401092:	48 8b 14 24          	mov    (%rsp),%rdx
  401096:	83 e2 0f             	and    $0xf,%edx
  401099:	0f b6 92 b0 24 40 00 	movzbl 0x4024b0(%rdx),%edx
  4010a0:	88 54 04 10          	mov    %dl,0x10(%rsp,%rax,1)
  4010a4:	48 83 c0 01          	add    $0x1,%rax
  4010a8:	48 83 f8 06          	cmp    $0x6,%rax
  4010ac:	75 dd                	jne    40108b <phase_5+0x29>
```

输入字符串存储在 `%rdx` 中，往后用 `s` 表示

第一行先将 `s` 的第一个字符取出，将低八位移到 `%rdx` 后与 `0xf` 执行按位与。然后将**地址为 `0x4024b0` 的字符串**对应下标的值取出，放到 `0xedx`。接着取出 `%edx` 的低八位字节，放在 `0x10 + %rsp + %rax` 的位置

重复六次后，执行以下代码：

```c
  4010ae:	c6 44 24 16 00       	movb   $0x0,0x16(%rsp)
  4010b3:	be 5e 24 40 00       	mov    $0x40245e,%esi
  4010b8:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi
  4010bd:	e8 76 02 00 00       	call   401338 <strings_not_equal>
  4010c2:	85 c0                	test   %eax,%eax
  4010c4:	74 13                	je     4010d9 <phase_5+0x77>
```

这一段代码将**地址为 `0x40245e` 的字符串**与从 `0x10 + %rsp` 开始的字符串相比较，二者相等的话即可跳转，往后此函数便结束了

我们打印出地址为 `0x4024b0` 和 `0x40245e` 的字符串，如下：

```c
(gdb) x /s 0x4024b0
0x4024b0 <array.3449>:  "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
(gdb) x /s 0x40245e
0x40245e:       "flyers"
```

我们将 `%rdx` 与 `0xf` 按位与之后的结果最大值为 `15` ，正好对应 `maduiersnfotvbyl`。字符串 `flyers` 的下标分别为：`9`、`15`、`14`、`5`、`6`、`7`

因此此函数将输入字符串的每个字符按位与 `0xf` 之后得到下标：`9`、`15`、`14`、`5`、`6`、`7`

通过查询 `ASCII` 表，只要后四位满足这几个数的字符，都符合条件

有多个答案，我们输入 `yonuvw` 即可

### phase_6

汇编代码如下：

```c
00000000004010f4 <phase_6>:
  4010f4:	41 56                	push   %r14
  4010f6:	41 55                	push   %r13
  4010f8:	41 54                	push   %r12
  4010fa:	55                   	push   %rbp
  4010fb:	53                   	push   %rbx
  4010fc:	48 83 ec 50          	sub    $0x50,%rsp
  401100:	49 89 e5             	mov    %rsp,%r13
  401103:	48 89 e6             	mov    %rsp,%rsi
  401106:	e8 51 03 00 00       	call   40145c <read_six_numbers>
  40110b:	49 89 e6             	mov    %rsp,%r14
  40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  401114:	4c 89 ed             	mov    %r13,%rbp
  401117:	41 8b 45 00          	mov    0x0(%r13),%eax
  40111b:	83 e8 01             	sub    $0x1,%eax
  40111e:	83 f8 05             	cmp    $0x5,%eax
  401121:	76 05                	jbe    401128 <phase_6+0x34>
  401123:	e8 12 03 00 00       	call   40143a <explode_bomb>
  401128:	41 83 c4 01          	add    $0x1,%r12d
  40112c:	41 83 fc 06          	cmp    $0x6,%r12d
  401130:	74 21                	je     401153 <phase_6+0x5f>
  401132:	44 89 e3             	mov    %r12d,%ebx
  401135:	48 63 c3             	movslq %ebx,%rax
  401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax
  40113b:	39 45 00             	cmp    %eax,0x0(%rbp)
  40113e:	75 05                	jne    401145 <phase_6+0x51>
  401140:	e8 f5 02 00 00       	call   40143a <explode_bomb>
  401145:	83 c3 01             	add    $0x1,%ebx
  401148:	83 fb 05             	cmp    $0x5,%ebx
  40114b:	7e e8                	jle    401135 <phase_6+0x41>
  40114d:	49 83 c5 04          	add    $0x4,%r13
  401151:	eb c1                	jmp    401114 <phase_6+0x20>
  401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi
  401158:	4c 89 f0             	mov    %r14,%rax
  40115b:	b9 07 00 00 00       	mov    $0x7,%ecx
  401160:	89 ca                	mov    %ecx,%edx
  401162:	2b 10                	sub    (%rax),%edx
  401164:	89 10                	mov    %edx,(%rax)
  401166:	48 83 c0 04          	add    $0x4,%rax
  40116a:	48 39 f0             	cmp    %rsi,%rax
  40116d:	75 f1                	jne    401160 <phase_6+0x6c>
  40116f:	be 00 00 00 00       	mov    $0x0,%esi
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
  401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx
  40117a:	83 c0 01             	add    $0x1,%eax
  40117d:	39 c8                	cmp    %ecx,%eax
  40117f:	75 f5                	jne    401176 <phase_6+0x82>
  401181:	eb 05                	jmp    401188 <phase_6+0x94>
  401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2)
  40118d:	48 83 c6 04          	add    $0x4,%rsi
  401191:	48 83 fe 18          	cmp    $0x18,%rsi
  401195:	74 14                	je     4011ab <phase_6+0xb7>
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx
  40119a:	83 f9 01             	cmp    $0x1,%ecx
  40119d:	7e e4                	jle    401183 <phase_6+0x8f>
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
  4011ab:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx
  4011b0:	48 8d 44 24 28       	lea    0x28(%rsp),%rax
  4011b5:	48 8d 74 24 50       	lea    0x50(%rsp),%rsi
  4011ba:	48 89 d9             	mov    %rbx,%rcx
  4011bd:	48 8b 10             	mov    (%rax),%rdx
  4011c0:	48 89 51 08          	mov    %rdx,0x8(%rcx)
  4011c4:	48 83 c0 08          	add    $0x8,%rax
  4011c8:	48 39 f0             	cmp    %rsi,%rax
  4011cb:	74 05                	je     4011d2 <phase_6+0xde>
  4011cd:	48 89 d1             	mov    %rdx,%rcx
  4011d0:	eb eb                	jmp    4011bd <phase_6+0xc9>
  4011d2:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx)
  4011d9:	00 
  4011da:	bd 05 00 00 00       	mov    $0x5,%ebp
  4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax
  4011e3:	8b 00                	mov    (%rax),%eax
  4011e5:	39 03                	cmp    %eax,(%rbx)
  4011e7:	7d 05                	jge    4011ee <phase_6+0xfa>
  4011e9:	e8 4c 02 00 00       	call   40143a <explode_bomb>
  4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx
  4011f2:	83 ed 01             	sub    $0x1,%ebp
  4011f5:	75 e8                	jne    4011df <phase_6+0xeb>
  4011f7:	48 83 c4 50          	add    $0x50,%rsp
  4011fb:	5b                   	pop    %rbx
  4011fc:	5d                   	pop    %rbp
  4011fd:	41 5c                	pop    %r12
  4011ff:	41 5d                	pop    %r13
  401201:	41 5e                	pop    %r14
  401203:	c3                   	ret    
```

前面我们知道，`read_six_number` 会读取六个**无符号数**，分别存放到内存为 `(%rsp)`、`%0x4(%rsp)`、`0x14(%rsp)`、`0x10(%rsp)`、`0xc(%rsp)`、`0x8(%rsp)`。相关代码如下：

```c
  4010fc:	48 83 ec 50          	sub    $0x50,%rsp
  401100:	49 89 e5             	mov    %rsp,%r13
  401103:	48 89 e6             	mov    %rsp,%rsi
  401106:	e8 51 03 00 00       	call   40145c <read_six_numbers>
  40110b:	49 89 e6             	mov    %rsp,%r14
  40110e:	41 bc 00 00 00 00    	mov    $0x0,%r12d
  401114:	4c 89 ed             	mov    %r13,%rbp
```
这部分的代码在读取了六个数之后，还设置了几个寄存器的值：

```c
%r13 = %rsi = %r14 = %rbp = %rsp
%r12 = 0
```

后面紧跟着的代码如下：

```c
  401117:	41 8b 45 00          	mov    0x0(%r13),%eax
  40111b:	83 e8 01             	sub    $0x1,%eax
  40111e:	83 f8 05             	cmp    $0x5,%eax
  401121:	76 05                	jbe    401128 <phase_6+0x34>
  401123:	e8 12 03 00 00       	call   40143a <explode_bomb>
  401128:	41 83 c4 01          	add    $0x1,%r12d
  40112c:	41 83 fc 06          	cmp    $0x6,%r12d
  401130:	74 21                	je     401153 <phase_6+0x5f>
  401132:	44 89 e3             	mov    %r12d,%ebx
  401135:	48 63 c3             	movslq %ebx,%rax
  401138:	8b 04 84             	mov    (%rsp,%rax,4),%eax
  40113b:	39 45 00             	cmp    %eax,0x0(%rbp)
  40113e:	75 05                	jne    401145 <phase_6+0x51>
  401140:	e8 f5 02 00 00       	call   40143a <explode_bomb>
  401145:	83 c3 01             	add    $0x1,%ebx
  401148:	83 fb 05             	cmp    $0x5,%ebx
  40114b:	7e e8                	jle    401135 <phase_6+0x41>
  40114d:	49 83 c5 04          	add    $0x4,%r13
  401151:	eb c1                	jmp    401114 <phase_6+0x20>
```

这一段实质上是二重循环，我们设之前 `6` 个数分别存储在数组 `a[0~5]` 中，那么有：

```c
for(int i = 0; i <= 5; i ++)
{
    //这里的a[i]-1是一个无符号数
    if((a[i] - 1) > 5) explode_bomb();
    for(int j = i + 1; j <= 5; j ++)
    {
        if(a[i] == a[j]) explode_bomb();
    }
}
```

接着跳到 `0x401153`，后续代码如下：

```c
  401153:	48 8d 74 24 18       	lea    0x18(%rsp),%rsi
  401158:	4c 89 f0             	mov    %r14,%rax
  40115b:	b9 07 00 00 00       	mov    $0x7,%ecx
  401160:	89 ca                	mov    %ecx,%edx
  401162:	2b 10                	sub    (%rax),%edx
  401164:	89 10                	mov    %edx,(%rax)
  401166:	48 83 c0 04          	add    $0x4,%rax
  40116a:	48 39 f0             	cmp    %rsi,%rax
  40116d:	75 f1                	jne    401160 <phase_6+0x6c>
  40116f:	be 00 00 00 00       	mov    $0x0,%esi
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
```

核心在于：

```c
  401162:	2b 10                	sub    (%rax),%edx
  401164:	89 10                	mov    %edx,(%rax)
```

首先将每个数的**地址**加载到 `%rax` 中，然后将每个数自身减七，得到的结果在放回原数，翻译后的代码为：

```c
for(int i = 0; i < 6; i ++)
{
    a[i] = 7 - a[i];
}
```

循环结束后将 `%esi` 设为 `0` ，之后跳转到 `0x401197`，后续代码如下：

```c
  40116f:	be 00 00 00 00       	mov    $0x0,%esi
  401174:	eb 21                	jmp    401197 <phase_6+0xa3>
  401176:	48 8b 52 08          	mov    0x8(%rdx),%rdx
  40117a:	83 c0 01             	add    $0x1,%eax
  40117d:	39 c8                	cmp    %ecx,%eax
  40117f:	75 f5                	jne    401176 <phase_6+0x82>
  401181:	eb 05                	jmp    401188 <phase_6+0x94>
  401183:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  401188:	48 89 54 74 20       	mov    %rdx,0x20(%rsp,%rsi,2)
  40118d:	48 83 c6 04          	add    $0x4,%rsi
  401191:	48 83 fe 18          	cmp    $0x18,%rsi
  401195:	74 14                	je     4011ab <phase_6+0xb7>
  401197:	8b 0c 34             	mov    (%rsp,%rsi,1),%ecx
  40119a:	83 f9 01             	cmp    $0x1,%ecx
  40119d:	7e e4                	jle    401183 <phase_6+0x8f>
  40119f:	b8 01 00 00 00       	mov    $0x1,%eax
  4011a4:	ba d0 32 60 00       	mov    $0x6032d0,%edx
  4011a9:	eb cb                	jmp    401176 <phase_6+0x82>
```

看到代码中有 `0x6032d0` ，我们查看该地址处的内容：

```c
(gdb) x 0x6032d0
0x6032d0 <node1>:       0x0000014c
```

该首地址对应的对象被命名为 `node1` ，我们尝试从改地址处连续打印 `30` 个内存单元，每个单元 `4` 字节：

```c
(gdb) x /30xw  0x6032d0
0x6032d0 <node1>:       0x0000014c      0x00000001      0x006032e0      0x00000000
0x6032e0 <node2>:       0x000000a8      0x00000002      0x006032f0      0x00000000
0x6032f0 <node3>:       0x0000039c      0x00000003      0x00603300      0x00000000
0x603300 <node4>:       0x000002b3      0x00000004      0x00603310      0x00000000
0x603310 <node5>:       0x000001dd      0x00000005      0x00603320      0x00000000
0x603320 <node6>:       0x000001bb      0x00000006      0x00000000      0x00000000
0x603330:       0x00000000      0x00000000      0x00000000      0x00000000
0x603340 <host_table>:  0x00402629      0x00000000
```

对于每一个 `node` 而言，有用的数据只有前三个，

每个 `node` 的第二个数据很明显表示编号，而第三个数据刚好是下一个 `node` 的首地址，因此这是一个链表

我们尝试给出链表中节点的信息：

```c
struct node
{
    int val;
    int idx;//序号
    node* next;//下一个节点
}
```

按顺序翻译代码如下：

```c
int i = 0;//i in %rsi
while(i != 6)
{
    //a 等于 %rsp
  	int v = *(a + i);   //mov (%rsp,%rsi,1),%ecx, v in %ecx
    node* ptr;          //ptr in %edx
    if(v <= 1)
    {
        ptr = 0x6032d0; //ptr in %edx
    }
    else
    {
        int j = 1;      //j in %eax
        ptr = 0x6032d0; //ptr in %edx
        do
        {
            ptr = ptr + 0x8;    //ptr = ptr->next, mov 0x8(%rdx),%rdx
            j++;
        }while(j != v);
    }
    a[2 * i + 8] = ptr;         //mov %rdx, 0x20(%rsp,%rsi,2)    
  	i ++;                       //add $0x4,%rsi
}
```

观察代码，我们可以得到以下结论：

* 从地址 `%rsp + 0x20` 处开始存储链表节点，存储方式类似于栈
* 由于链表只有 `6` 个节点，且**节点内容固定**，因此输入的 `6` 个数只能为 `1 ~ 6`
* 对于每个数 `a[i]` ，找到与其对应的链表节点编号，然后**将该链表节点插入到栈顶**

也就是将六个节点按输入数字的顺序进行排序

注意：此时仅仅是将值存储在对应地址处，每个节点的 `next` 指针并没有赋值

代码结束后跳到地址 `0x4011ab`，后续代码如下：

```c
  4011ab:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx
  4011b0:	48 8d 44 24 28       	lea    0x28(%rsp),%rax
  4011b5:	48 8d 74 24 50       	lea    0x50(%rsp),%rsi
  4011ba:	48 89 d9             	mov    %rbx,%rcx
  4011bd:	48 8b 10             	mov    (%rax),%rdx
  4011c0:	48 89 51 08          	mov    %rdx,0x8(%rcx)
  4011c4:	48 83 c0 08          	add    $0x8,%rax
  4011c8:	48 39 f0             	cmp    %rsi,%rax
  4011cb:	74 05                	je     4011d2 <phase_6+0xde>
  4011cd:	48 89 d1             	mov    %rdx,%rcx
  4011d0:	eb eb                	jmp    4011bd <phase_6+0xc9>
  4011d2:	48 c7 42 08 00 00 00 	movq   $0x0,0x8(%rdx)
  4011d9:	00
```

这里的时序如下：

* 将第一个节点的地址赋给 `%rbx`，第一个节点的 `next` 指针赋值给 `%rax`，设定边界 `%rsi`
* `%rcx` 存储第一个节点的地址，`%rdx` 存储第二个节点的地址
* 将第二个节点的地址赋值给第一个节点的 `next` 指针
* 第一个节点的指针后移，判断是否到达边界

然后循环五次，最终会将最后一个节点的 `next` 指针置空，翻译成 `C` 代码如下：

```c
node* p[6];
for(int i = 0; i < 5; i ++)
{
    p[i]->next = p[i + 1]->next;
}
p[5]->next = NULL;
```

相当于是**将六个节点的 `next` 指针相互串起来**，后续代码跳到 `0x4011da`，相关代码如下：

```c
  4011da:	bd 05 00 00 00       	mov    $0x5,%ebp
  4011df:	48 8b 43 08          	mov    0x8(%rbx),%rax
  4011e3:	8b 00                	mov    (%rax),%eax
  4011e5:	39 03                	cmp    %eax,(%rbx)
  4011e7:	7d 05                	jge    4011ee <phase_6+0xfa>
  4011e9:	e8 4c 02 00 00       	call   40143a <explode_bomb>
  4011ee:	48 8b 5b 08          	mov    0x8(%rbx),%rbx
  4011f2:	83 ed 01             	sub    $0x1,%ebp
  4011f5:	75 e8                	jne    4011df <phase_6+0xeb>
```

这里需要注意的是，**`%rbp` 始终保存着第一个链表节点的地址**。这里的时序如下：

* 先将第二个节点的地址赋给 `%rax`
* 将第二个节点的 `val` 赋值给 `eax`
* 比较第二个节点的 `val` 与第一个节点的 `val` ，**前者必须大于等于后者**，也就是按**降序排序**

对应 `C` 语言代码如下：

```c
node* p[6];
for(int i = 0; i < 5; i ++)
{
    if(p[i + 1]->val < p[i]->val) explode_bomb();
}
```

我们考虑六个节点中 `val` 部分的值：

|`idx`|`val`|
|:-:|:-:|
|`0x14c = 332`|`1`|
|`0x0a8 = 168`|`2`|
|`0x39c = 924`|`3`|
|`0x2b3 = 691`|`4`|
|`0x1dd = 477`|`5`|
|`0x1bb = 443`|`6`|

将 `val` 按降序排序后的编号为：`3 4 5 6 1 2`，由于之前将每个输入值 `a[i]` 都变为 `7 - a[i]`，因此最终结果为：`4 3 2 1 6 5`

此时将以下内容写入文件 `ans` 中便可通关（结尾需要有空行）

```c
Border relations with Canada have never been better.
1 2 4 8 16 32
0 207
7 0
yonuvw
4 3 2 1 6 5

```

运行结果：

```c
$ ./bomb ans
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Phase 1 defused. How about the next one?
That's number 2.  Keep going!
Halfway there!
So you got that one.  Try this one.
Good work!  On to the next...
Congratulations! You've defused the bomb!
```

---

### secret_phase

汇编代码如下：

```c
0000000000401242 <secret_phase>:
  401242:	53                   	push   %rbx
  401243:	e8 56 02 00 00       	call   40149e <read_line>
  401248:	ba 0a 00 00 00       	mov    $0xa,%edx
  40124d:	be 00 00 00 00       	mov    $0x0,%esi
  401252:	48 89 c7             	mov    %rax,%rdi
  401255:	e8 76 f9 ff ff       	call   400bd0 <strtol@plt>
  40125a:	48 89 c3             	mov    %rax,%rbx
  40125d:	8d 40 ff             	lea    -0x1(%rax),%eax
  401260:	3d e8 03 00 00       	cmp    $0x3e8,%eax
  401265:	76 05                	jbe    40126c <secret_phase+0x2a>
  401267:	e8 ce 01 00 00       	call   40143a <explode_bomb>
  40126c:	89 de                	mov    %ebx,%esi
  40126e:	bf f0 30 60 00       	mov    $0x6030f0,%edi
  401273:	e8 8c ff ff ff       	call   401204 <fun7>
  401278:	83 f8 02             	cmp    $0x2,%eax
  40127b:	74 05                	je     401282 <secret_phase+0x40>
  40127d:	e8 b8 01 00 00       	call   40143a <explode_bomb>
  401282:	bf 38 24 40 00       	mov    $0x402438,%edi
  401287:	e8 84 f8 ff ff       	call   400b10 <puts@plt>
  40128c:	e8 33 03 00 00       	call   4015c4 <phase_defused>
  401291:	5b                   	pop    %rbx
  401292:	c3                   	ret    
  401293:	90                   	nop
  401294:	90                   	nop
  401295:	90                   	nop
  401296:	90                   	nop
  401297:	90                   	nop
  401298:	90                   	nop
  401299:	90                   	nop
  40129a:	90                   	nop
  40129b:	90                   	nop
  40129c:	90                   	nop
  40129d:	90                   	nop
  40129e:	90                   	nop
  40129f:	90                   	nop
```

这里面会使用到 `fun7` ，我们先去将 `fun7` 的表达式写出来，对应汇编如下：

```c
0000000000401204 <fun7>:
  401204:	48 83 ec 08          	sub    $0x8,%rsp
  401208:	48 85 ff             	test   %rdi,%rdi
  40120b:	74 2b                	je     401238 <fun7+0x34>
  40120d:	8b 17                	mov    (%rdi),%edx
  40120f:	39 f2                	cmp    %esi,%edx
  401211:	7e 0d                	jle    401220 <fun7+0x1c>
  401213:	48 8b 7f 08          	mov    0x8(%rdi),%rdi
  401217:	e8 e8 ff ff ff       	call   401204 <fun7>
  40121c:	01 c0                	add    %eax,%eax
  40121e:	eb 1d                	jmp    40123d <fun7+0x39>
  401220:	b8 00 00 00 00       	mov    $0x0,%eax
  401225:	39 f2                	cmp    %esi,%edx
  401227:	74 14                	je     40123d <fun7+0x39>
  401229:	48 8b 7f 10          	mov    0x10(%rdi),%rdi
  40122d:	e8 d2 ff ff ff       	call   401204 <fun7>
  401232:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
  401236:	eb 05                	jmp    40123d <fun7+0x39>
  401238:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
  40123d:	48 83 c4 08          	add    $0x8,%rsp
  401241:	c3                   	ret
```

这个函数不难，需要注意的是： **`fun7` 的第一个参数为指针，第二个参数为整数**。翻译为 `C` 代码如下：

```c
//x in %rdi, y in %rsi
int func7(int& x, int y)
{
    if(x == NULL) return 0xffffffff;
    if(*x <= y) 
    {
        if(*x == y) return 0;
        else 
        {
            int res = fun7(x + 16, y);
            return 2 * res + 1;
        }
    }
    else
    {
        int res =  fun7(x + 8, y);
        return 2 * res;
    }
}
```

回到 `secret_phase` 中，先看前面的部分：

```c
  401243:	e8 56 02 00 00       	call   40149e <read_line>
  401248:	ba 0a 00 00 00       	mov    $0xa,%edx
  40124d:	be 00 00 00 00       	mov    $0x0,%esi
  401252:	48 89 c7             	mov    %rax,%rdi
  401255:	e8 76 f9 ff ff       	call   400bd0 <strtol@plt>
  40125a:	48 89 c3             	mov    %rax,%rbx
```

首先将 `%edx` 与 `%esi` 的值分别设为 `0xa` 和 `0x0` ，然后将 `read_line` 的返回值赋值给 `%rdi`

将 `strtol` 的返回值赋给 `%rbx`

>`strtol` 是一个库函数，其定义为：`long int strtol(const char *str, char **endptr, int base)`
>
> 可以将 `str` 给定的字符串按照所给定的 `base` 转换成一个 `long int`
>
> * `str` 要转换为长整数的字符串
> * `endptr` 表示第一个非法字符的地址
> * `base` 表示基数，数值在 `2` 到 `36` 之间或者 `0`
> * 函数执行成功则返回对应数，失败则返回 `0`
>
> `base` 用于指定合法字符的范围以及权值，比如：
> 
> * 当 `base` 为 `2` 时，只有字符 `0 ~ 1` 是合法的，此时每个位的权重为 `2` 的幂次
> * 当 `base` 为 `8` 时，只有字符 `0 ~ 7` 是合法的，此时每个位的权重为 `8` 的幂次
> * 当 `base` 为 `36` 时，只有字符 `0 ~ 9, a ~ z` 是合法的，此时每个位的权重为 `36` 的幂次

因此，`read_line` 得到的字符串作为 `strtol` 的第一个参数，一个初始为空的指针作为第二个参数，数字 `10` 作为第三个参数，而从 `read_line` 中读到的整数则放到 `strtol` 的返回值当中（也就是 `%rax` 和 `%rbx`）

> 判断：舍去得到结果的最低位字节（`lea -0x1(%rax),%eax`，最低位字节是因为采用小端存储），将其放在 `%eax` 中，此时要求 `%eax` 必须大于等于 `1000`

往后将地址 `0x6030f0` 作为 `fun7` 的第一参数，将 `strtol` 得到的长整数作为第二参数，**要求 `fun7` 的返回值为 `2`**

我们打印地址为 `0x6030f0` 处的内容：

```c
(gdb) x /100xw 0x6030f0
0x6030f0 <n1>:  0x00000024      0x00000000      0x00603110      0x00000000
0x603100 <n1+16>:       0x00603130      0x00000000      0x00000000      0x00000000
0x603110 <n21>: 0x00000008      0x00000000      0x00603190      0x00000000
0x603120 <n21+16>:      0x00603150      0x00000000      0x00000000      0x00000000
0x603130 <n22>: 0x00000032      0x00000000      0x00603170      0x00000000
0x603140 <n22+16>:      0x006031b0      0x00000000      0x00000000      0x00000000
0x603150 <n32>: 0x00000016      0x00000000      0x00603270      0x00000000
0x603160 <n32+16>:      0x00603230      0x00000000      0x00000000      0x00000000
0x603170 <n33>: 0x0000002d      0x00000000      0x006031d0      0x00000000
0x603180 <n33+16>:      0x00603290      0x00000000      0x00000000      0x00000000
0x603190 <n31>: 0x00000006      0x00000000      0x006031f0      0x00000000
0x6031a0 <n31+16>:      0x00603250      0x00000000      0x00000000      0x00000000
0x6031b0 <n34>: 0x0000006b      0x00000000      0x00603210      0x00000000
0x6031c0 <n34+16>:      0x006032b0      0x00000000      0x00000000      0x00000000
0x6031d0 <n45>: 0x00000028      0x00000000      0x00000000      0x00000000
0x6031e0 <n45+16>:      0x00000000      0x00000000      0x00000000      0x00000000
0x6031f0 <n41>: 0x00000001      0x00000000      0x00000000      0x00000000
0x603200 <n41+16>:      0x00000000      0x00000000      0x00000000      0x00000000
0x603210 <n47>: 0x00000063      0x00000000      0x00000000      0x00000000
0x603220 <n47+16>:      0x00000000      0x00000000      0x00000000      0x00000000
0x603230 <n44>: 0x00000023      0x00000000      0x00000000      0x00000000
0x603240 <n44+16>:      0x00000000      0x00000000      0x00000000      0x00000000
0x603250 <n42>: 0x00000007      0x00000000      0x00000000      0x00000000
0x603260 <n42+16>:      0x00000000      0x00000000      0x00000000      0x00000000
0x603270 <n43>: 0x00000014      0x00000000      0x00000000      0x00000000
```

这个结构是一棵**二叉树**，其节点声明如下：

```c
struct node
{
    int val;
    node* left;
    node* right;
}
```

因此对于节点 `n` 而言，`n + 8` 为左儿子，`n + 16` 为右儿子，最后一个字节留空

这里的二叉树最好在本子上画出来，也就只有 `4` 层，方便理解

这本质上其实是一棵二叉搜索树，大概为：

```
                     36
            8                 30
       16       22         45   107
     1   7   20    35   45 
```

由于最终的返回值必须为 `2` ，因此第一次必须走左子树（返回值直接乘二的那条），第二次可以必须走右子树（返回值乘二再加一的那条），第三次可走可不走，走的话必须走左子树

因此结果为 `20` 或 `22`，往后函数结束

但有一个问题是，该如何进入 `secret_phase` 呢？

在 `secret_phase` 中最后会调用 `phase_defused`，我们去看看这里的代码：

```c
00000000004015c4 <phase_defused>:
  4015c4:	48 83 ec 78          	sub    $0x78,%rsp
  4015c8:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  4015cf:	00 00 
  4015d1:	48 89 44 24 68       	mov    %rax,0x68(%rsp)
  4015d6:	31 c0                	xor    %eax,%eax
  4015d8:	83 3d 81 21 20 00 06 	cmpl   $0x6,0x202181(%rip)        # 603760 <num_input_strings>
  4015df:	75 5e                	jne    40163f <phase_defused+0x7b>
  4015e1:	4c 8d 44 24 10       	lea    0x10(%rsp),%r8
  4015e6:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  4015eb:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  4015f0:	be 19 26 40 00       	mov    $0x402619,%esi
  4015f5:	bf 70 38 60 00       	mov    $0x603870,%edi
  4015fa:	e8 f1 f5 ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  4015ff:	83 f8 03             	cmp    $0x3,%eax
  401602:	75 31                	jne    401635 <phase_defused+0x71>
  401604:	be 22 26 40 00       	mov    $0x402622,%esi
  401609:	48 8d 7c 24 10       	lea    0x10(%rsp),%rdi
  40160e:	e8 25 fd ff ff       	call   401338 <strings_not_equal>
  401613:	85 c0                	test   %eax,%eax
  401615:	75 1e                	jne    401635 <phase_defused+0x71>
  401617:	bf f8 24 40 00       	mov    $0x4024f8,%edi
  40161c:	e8 ef f4 ff ff       	call   400b10 <puts@plt>
  401621:	bf 20 25 40 00       	mov    $0x402520,%edi
  401626:	e8 e5 f4 ff ff       	call   400b10 <puts@plt>
  40162b:	b8 00 00 00 00       	mov    $0x0,%eax
  401630:	e8 0d fc ff ff       	call   401242 <secret_phase>
  401635:	bf 58 25 40 00       	mov    $0x402558,%edi
  40163a:	e8 d1 f4 ff ff       	call   400b10 <puts@plt>
  40163f:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
  401644:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax
  40164b:	00 00 
  40164d:	74 05                	je     401654 <phase_defused+0x90>
  40164f:	e8 dc f4 ff ff       	call   400b30 <__stack_chk_fail@plt>
  401654:	48 83 c4 78          	add    $0x78,%rsp
  401658:	c3                   	ret    
  401659:	90                   	nop
  40165a:	90                   	nop
  40165b:	90                   	nop
  40165c:	90                   	nop
  40165d:	90                   	nop
  40165e:	90                   	nop
  40165f:	90                   	nop
```

此函数会先将 `num_input_strings` 与 `6` 进行比较，不等则直接返回。我们猜测 `num_input_strings` 表示输入字符串的大小

此后我们打印地址 `0x402619` 和 `0x603870` 的字符串，有：

```c
(gdb) x /s 0x402619
0x402619:       "%d %d %s"
(gdb) x /s 0x603870
0x603870 <input_strings+240>:   ""
```

我们发现此时的字符串为空，我们在每个 `phase` 函数前面打断点，看在哪个位置输出地址 `0x602870` 有值：

```c
Breakpoint 4, 0x000000000040100c in phase_4 ()
(gdb) x /s 0x603870
0x603870 <input_strings+240>:   "7 0"
```

也就是说，**当运行到 `phase_4` 时此字符串有值**

之后调用 `sscanf`，从输入字符串中读取数据。如果最终读取的数据个数不为 `3` 则直接结束函数（也就是说此时输入的数为 `2`）

这三个输入分别对应地址为：`0x8(%rsp)`、`0xc(%rsp)`、`0x10(%rsp)`（最后一个为字符串）

之后会比较输入字符串与地址 `0x402622` 处的字符串是否相同，我们打印此处的字符串：

```c
(gdb) x /s 0x402622
0x402622:       "DrEvil"
```

如果相同，后面会输出两个提示信息：

```c
(gdb) x /s 0x4024f8
0x4024f8:       "Curses, you've found the secret phase!"
(gdb) x /s 0x402520
0x402520:       "But finding it and solving it are quite different..."
```

综上，我们需要在 `phase_4` 输入的后面加上 `DrEvil` 便可以进入 `secret_phase`

总的答案为（保存在 `ans` 文件中）：

```c
Border relations with Canada have never been better.
1 2 4 8 16 32
0 207
7 0 DrEvil
yonuvw
4 3 2 1 6 5
20

```

运行结果：

```c
$ ./bomb ans 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Phase 1 defused. How about the next one?
That's number 2.  Keep going!
Halfway there!
So you got that one.  Try this one.
Good work!  On to the next...
Curses, you've found the secret phase!
But finding it and solving it are quite different...
Wow! You've defused the secret stage!
Congratulations! You've defused the bomb!
```