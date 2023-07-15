# datalab

- [datalab](#datalab)
  - [准备](#准备)
    - [文件下载](#文件下载)
    - [要求](#要求)
    - [测试](#测试)
      - [操作符测试](#操作符测试)
      - [正确性测试](#正确性测试)
  - [函数编写](#函数编写)
    - [bitXor](#bitxor)
    - [tmin](#tmin)
    - [isTmax](#istmax)
    - [addOddBits](#addoddbits)
    - [negate](#negate)
    - [isAsciiDigit](#isasciidigit)
    - [conditional](#conditional)
    - [isLessOrEqual](#islessorequal)
    - [logicalNeg](#logicalneg)
    - [howManyBits](#howmanybits)
    - [floatScale2](#floatscale2)
    - [floatFloat2Int](#floatfloat2int)
    - [floatPower2](#floatpower2)

## 准备

### 文件下载

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)


### 要求

本实验要求我们补充 `bits.c` 当中的函数，使得能够通过测试

对于每个表达式，要求：

1. 整数常量 `0` 到 `255` （`0xFF`），包括 `0` 到 `255`（不允许使用大常量），例如 `0xffffffff`
2. 函数参数和局部变量（无全局变量）
3. 一元整数运算`！~`
4. 二进制整数运算 `& ^ |+ << >>`

待补充函数分为两部分：整数函数和浮点数函数

对于整数函数，要求如下：

禁止：

1. 使用任何控制结构，例如 `if`、`do while`、`for`、`switch` 等
2. 定义或使用任何宏
3. 在此文件中定义任何其他函数
4. 调用任何函数
5. 使用任何其他操作符，例如 `&&`、`||`、`-`或 `？：`
6. 使用任何形式的转型
1. 使用 `int` 以外的任何数据类型，这意味着你不能使用数组、结构或联合

假设机器：

1. 使用补码，`int` 为 `32` 位
2. 执行算术右移。
3. 如果移位量小于 `0` 或大于 `31`，有不可预测行为

对于浮点数函数，要求如下：

禁止：

1. 定义或使用任意宏
2. 在此文件中定义任何附加函数
3. 调用任意函数
4. 使用任何形式的转型
5. 使用除 `int` 或 `unsigned` 以外的任何数据类型，这意味着你不能使用数组、结构体或联合。
6. 使用任何浮点数据类型、操作或常量

### 测试

#### 操作符测试

每个函数只能使用**特定的、规定数目的操作符**

* `./dlc bits.c` 用于检查每个函数是否出现报错

* `./dlc -e bits.c` 用于检测每个函数的操作符个数

#### 正确性测试

如果需要判断函数是否正确，如果是第一次执行，首先需要生成 `btest`：

```bash
make btest
```

随后运行：

```bash
./btest
```

此后每次修改函数，都需要先清除 `btest`，然后再生成和运行：

```bash
make clean
make btest
./btest
```

直接运行 `./btest` 会检测所有函数是否通过测试，如果只想检测某个函数的话则需要带 `-f` 参数：

```bash
# 检测 foo 函数是否通过测试
./btest -f foo
```

## 函数编写

### bitXor

> 要求：使用 `~` 和 `&` 来实现异或

首先，`x ^ y` 可以表示为 `(x | ~y) & (~x | y)`

现在问题转换成用 `~` 和 `&` 来实现 `|`

我们首先写出 `&` 和 `|` 的真值表：

```
a  b  a & b   a  b  a | b
0  0    0     0  0    0
1  0    0     1  0    1
0  1    0     0  1    1
1  1    1     1  1    1
```

如果将 `~` 作用在 `(a & b)` 上，得到：
```
a b ~(a & b)
0 0     1
0 1     1
1 0     1
1 1     0
```

进一步，对 `a` 和 `b` 均取反，有：

```
a b ~(~a & ~b)
0 0      0
1 0      1
0 1      1
1 1      1
```

这个正好就是 `|` 的真值表，因此，`a | b` 等价于 `~(~a & ~b)`

答案：

```c
int bitXor(int x, int y) {
  return ~(~(x & ~y) & ~(~x & y));
}
```

### tmin

> 要求：我们需要返回 `32` 为补码的最小值

依据定义，将 `1` 左移 `31` 为即可得到

答案：

```c
int tmin(void) {
  return 1 << 31;
}
```

### isTmax

> 要求：对于 `32` 位补码数，如果 `x` 是最大值则返回 `1` ，不是则返回 `0`

我们以 `4` 位补码数举例，最大值为 `0111` ，将这个数加一得到 `1000` 

如果对 `1000` 按位取反，正好和原数相等，即：若 `x` 为最大值，有 `x == ~(x + 1)`

关于等号的判断，可以用异或加逻辑非的形式：`!(~(x + 1) ^ x)`

但有一个例外是 `-1` ，其补码表示为 `1111` ，我们发现这个数也符合我们的规定，因此需要去除 `-1` 

也就是当 `x` 为 `-1` 时返回 `0` ，当 `x` 不为 `-1` 时返回 `1`

正好可以用 `!!(x + 1)` 来进行判断

答案：

```c
int isTmax(int x) {
  return !(~(x + 1) ^ x) & !!(x + 1);
}
```

### addOddBits

> 要求：如果 `x` 的位表示（从 `0` 开始）中奇数位全为 `1` ，则返回 `1` ，否则返回 `0`

二进制中每个奇数位均为 `1` 的数为：`0xAAAAAAAA`，因此我们考虑用 `0xAA` 通过移位的形式得到

将 `0xAAAAAAAA` 按位与 `x` 的结果与 `0xAAAAAAAA` 判断是否相等即可

答案：

```c
int allOddBits(int x) {
  int op = 0xAA;
  op |= (op << 8);
  op |= (op << 16);
  return !((x & op) ^ op); 
}
```

### negate

> 要求：求出 `-x`

按照补码非的定义，$-x=\sim x + 1$

答案：

```c
int negate(int x) {
  return ~x + 1;
}
```

### isAsciiDigit

> 要求：如果 `x` 为字符 `0` 到 `9` （`ASCII` 为 `0x30` 到 `0x39` ）则返回 `1` ，否则为 `0`

我们首先判断高四位，`x` 的高四位必须是 `0x30` ，我们将其**左移四位**得到的结果与 `0x03` 按位异或，看结果是否为 `0`，即：`!((x >> 4) ^ 0x03)`

这里不能直接与 `0x30` 按位异或，因为满足条件的第四位并不全为 `0` ，因此需要将其左移四位之后再取异或

随后我们取出 `x` 的第四位的值 `lower4bits` ，往后需要判断 `lower4bits` 处于范围 `0x0` 到 `0x9` 之间

注意到 `0x9` 的二进制表述为 `1001` ，将其加上 `0110` 之后得到 `4` 为二进制的最大值 `1111` ，因此如果一个数的范围大于 `0x9` ，那么加上 `0110` 之后必然会导致**进位**，我们只需要检测是否产生进位即可（`lower4bits` 中第四位是否为 `1`）

答案：

```c
int isAsciiDigit(int x) {
  int ahead = !((x >> 4) ^ 0x03);
  int lower4Bits = x & 0xF;
  int carry = lower4Bits + 0x06;
  return ahead & !(carry >> 4);
}
```

### conditional

> 要求：实现 `x ? y : z`

如果 `x` 为 `0` ，则需要取 `z` ，反之为 `y`，因此我们考虑用**加法**的形式来实现

具体地，我们构造下式：

$$
Expr\And y + \neg Expr\And z
$$

其中 $Expr$ 为与 $x$ 有关的表达式

也就是我们需要当 `x` 为 `0` 时，$Expr$ 为 `[0000]`，当 `x` 为**非零**时，$Expr$ 为 `[1111]`

需要注意的是，$Expr$ 为一个向量，**要么全零，要么全一**

此时 $Expr$ 与 `x` 的关系为：`~!x + 1`

答案：

```c
int conditional(int x, int y, int z) {
  int isZero = ~!x + 1;//x == 0 => [1...1], x == 1 => [0...0]
  return (~isZero & y) + (isZero & z);
}
```

### isLessOrEqual

> 要求：如果 `x <= y` 则返回 `1` ，否则返回 `0`

分两种情况判断：`x < y` 与 `x = y`

后者很容易，就是：`!(x ^ y)`

对于前者的判断，我们转化为判断 `y - x` 是否大于 `0` ，**也就是最高位是否为 `0`**，设其结果为 `sub`

这里我们需要判断 `sub` 是否产生溢出，具体地，如果 `x` 为 $TMin$ 此时 `-x` 为 $TMin$ ，只要 `y` 为负数就会发生溢出

如果 `y` 为负数，此时最高位溢出为 `0` ，如果 `y` 为正数，此时最高位没有溢出，为 `1`

因此如果 `sub` 的最高位为 `0` **且** `sub` 的结果不为零，说明发生了溢出

最后取二者按位与即可

答案：

```c
int isLessOrEqual(int x, int y) {
  int isEqual = !(x ^ y);
  int sub = y + (~x + 1);
  int isBiggerZero = !(sub >> 31) & !!sub;
  return isEqual | isBiggerZero;
}
```

### logicalNeg

> 要求：不能使用 `!` 来实现 `!`

我们需要知道一个性质：`x | -x` 的值可以表示为：从右往左找到第一个 `1` ，然后左边的所有位均为 `1`

举个例子，如果 `x` 为 `[0010]` 那么 `x | -x` 为 `[1110]`

基于此，我们可以得到一个非常有用的性质，如果 `x` 为 `0` ，那么这样操作后的结果依然为 `0` ，如果 `x` 不为 `0`，那么结果必然不为 `0` 

对于后一种情况，我们可以确定的是，**其最高位一定为 `1`**

因此，我们将 `x | -x` 右移 `31` 位，**得到 `[111...111]` ，也就是 `-1`**，将其加 `1` 就是我们需要的结果

答案：

```c
int logicalNeg(int x) {
  return ((x | (~x + 1)) >> 31) + 1;
}
```

### howManyBits

> 要求：返回可以表示 `x` 的最小二进制数的个数

如果 `x` 为负数，其表示为：`[1...110...]`，也就是我们需要找到**最左边第一个 `0`** 的位置，然后将结果加一（前面要补一）

如果 `x` 为正数，其表示为：`[0...001...]`，因此我们需要找到**最左边第一个 `1`** 的位置，同时也需要将结果加上一（前面要补零）

我们考虑将这两种情况统一一下：如果 `x` 为负数的话，我们只需要将其**按位异或**一个全一的数 `[1...1]`，便可以转换成正数的情况

由于条件为 `x` 小于 `0` ，因此如果要得到 `[1...1]` ，我们将 `x` 右移 `31` 为即可得到

而如果 `x` 大于等于 `0` ，此时得到的是 `[0...0]` ，按位异或 `x` 并不影响结果

因此预处理部分为：

```c
int sign = x >> 31;
x = x ^ sign;
```

往后的做法类似于二分查找，我们先检查 `x` 低 `16` 位是否存在 `1` ，**如果存在则表明 `x` 至少需要 `16` 位来表示**

此后我们将 `x` 右移 `16` 位，检查低 `8` 位是否存在 `1` ，并重复这个过程

我们将每次得到的结果记录下来，最后将所有结果的总和**加一**就是答案

答案：

```c
int howManyBits(int x) {
  int bit16, bit8, bit4, bit2, bit1, bit;
  int sign = x >> 31;//positive will set to 0, and negative is 11...1
  /*
  If x <  0, its bit representation is 11...10...
  If x >= 0, its bit representation is 00...01...
  So, for negative num, we should find the first 0 on the far left and the res must plus 1
  And for positive num, we shouble find the first 1 on the far left, and the res also plus 1
  We consider turn negative num form into positive num form
  */
  x = x ^ sign;
  //The follow code attempt to find the fitst 1 on far left， this process is like Binary search
  bit16 = !!(x >> 16) << 4;//at least 16 bits
  x >>= bit16;
  bit8 = !!(x >> 8) << 3;//at least 8 bits
  x >>= bit8;
  bit4 = !!(x >> 4) << 2;//at least 4 bits
  x >>= bit4;
  bit2 = !!(x >> 2) << 1;//at least 2 bits
  x >>= bit2;
  bit1 = !!(x >> 1);//at least 1 bits
  x >>= bit1;
  bit = x;
  return bit16 + bit8 + bit4 + bit2 + bit1 + bit + 1;
}
```

### floatScale2

> 要求：返回 `2 * x` 的浮点数位级表示，`x` 以无符号整数给出

分规格数和非规格数进行讨论

如果 `x` 为规格数，那么**将阶码部分加一**；如果 `x` 是非规格数，在**保证符号位不变的情况下将 `x` 左移一位**

由于 `x` 是 `float` ，因此其 `23 ~ 30` 位用于表示阶码，因此如果要判断阶码是否全为 `0` 的话可以将 `x` 按位与 `0x7F800000`

如果 `x` 按位与 `0x7F800000` 不为零，说明 `x` 为规格数，否则为非规格数

对于规格数，我们将其加上 `0x00800000`；对于非规格数，我们只取其位数并左移一位，然后再按位或上 `x` 的符号

答案：

```c
unsigned floatScale2(unsigned uf) {
  unsigned ans = uf;
  //normalizal
  if((uf & 0x7f800000) != 0 && (uf & 0x7f800000) != 0x7f800000) ans += 0x00800000;
  //denormalizal, move directly one bit to the left, notes the sign bits
  //can not move directly to the left
  else if((uf & 0x7f800000) == 0) ans = ((ans & 0x007fffff) << 1) | (ans & 0x80000000);
  return ans;
}
```

### floatFloat2Int

> 要求：返回 `float` 转 `int` 得到的结果

对于 `int` 而言，可表示（正数）范围为 $0\sim 2^{31}-1$

我们首先得到 `x` 的阶码 `E` （**不是阶码的位表示 `exp`**）和 尾数 `M`，分类讨论：

如果 `E < 0` 表明 `x` 是一个小数，那么无论 `x` 的正负，我们均是向零舍入，因此直接返回 `0`

如果 `E >= 31` ，此时超出 `int` 可以表示的最大值，返回 `0x80000000`

由于尾数的位数为 `23` （如果我们直接返回位数的话，相当于本身就乘了 $2^{23}$），因此如果 `E > 23` ，我们需要额外将其左移 `E - 23` 位，如果 `E <= 23` ，我们需要将其右移 `23 - E` 位

到此为止，我们已经构造出 `M` ，我们还需要保证 `M` 的符号与 `x` 相一致

如果二者相同则返回 `M` ，否则返回 `-M`

答案：

```c
int floatFloat2Int(unsigned uf) {
  int E, M;
  //go to get sign bits
  int sign = uf >> 31;
  if((uf & 0x7fffffff) == 0) return 0;
  
  E = ((uf & 0x7f800000) >> 23) - 127, M = (uf & 0x007fffff) | 0x00800000;//get 1 + M (notes the position of 1)

  if(E >= 31) return 0x80000000;
  else if(E < 0) return 0;

  //the number of the digit of the M is 23, so if E >= 32, M shoule muliple 2^(E - 23), M itself at least has 23 bits
  if(E > 23) M <<= (E - 23);
  else M >>= (23 - E);

  //if the sign of uf is equal to the sign of M, return M, else return -M
  if(sign ^ (M >> 31)) return ~M + 1;
  else return M;
}
```

### floatPower2

> 要求：一单精度浮点数的形式返回 $2^x$ 的位表示

对于单精度浮点数而言，可表示（正数）范围为 $2^{-23}\times 2^{-126} \sim (2-\epsilon)\times 2^{127}$

因此如果 `x` 小于 `-149` ，则直接返回 `0`；相应地，如果 `x` 大于 `127` ，则返回 $INF$ `0x7F800000`

如果 `x` 处于 `-149` 到 `-127` 之间（非规格数），由于非规格化没有前置的 $1$ ，因此我们直接将 $1$ 左移 `x + 149` 位即可

如果 `x` 为规格化数，由于存在前置的 $1$ ，我们将尾数全部置零，然后凑出阶码 `exp` 即可

具体地，`exp` 为 `x + 127` （得到 `exp` 的位表示）得到的结果向左移动 `23` 位

答案：

```c
unsigned floatPower2(int x) {
  int exp = x + 127;
  //the range of float num is from 2^-149 to 2^127

  if(x < -149) return 0;
  else if(x >= 128) return 0x7f800000;

  //denormal num ranges from 2^-149 to 2^-126
  if(-149 <= x && x <= -127) return 1 << (x + 149);
  //E = e - Bias => e = E + Bias
  //frac is [00...0], resulting in M = 1, that's what we want
  return exp << 23;
}
```