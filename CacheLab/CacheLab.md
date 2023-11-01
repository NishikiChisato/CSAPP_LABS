# cachelab

- [cachelab](#cachelab)
  - [准备](#准备)
  - [实验开始](#实验开始)
    - [Part A](#part-a)
      - [重要概念](#重要概念)
      - [核心代码](#核心代码)
    - [Part B](#part-b)
  - [后记](#后记)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

## 实验开始

### Part A

> 我们需要首先学会 `getopt` 这个函数

直接编译会报错：

```bash
error: implicit declaration of function ‘getopt’
```

这是因为编译参数为 `-std=c99` 导致的，这将会阻止 `GNU extension`，因此我们需要自己包含 `getopt.h` 这个头文件。见这里[原贴](https://stackoverflow.com/questions/22575940/getopt-not-included-implicit-declaration-of-function-getopt)

#### 重要概念

该实验要求我们模拟一个高速缓存 `cache` ，我们需要先明确几个概念并且一定要理解书中对于高速缓存的过程模拟（`430` 页），不然这里很难进行

* 基本概念：

我们取最一般的情况：组相联的高速缓存。假设组 `set` 的个数为 $S=2^s$，每个组中含有 $E$ 个行 `line`。每个行可以划分为**三个部分**：有效位 `valid` 、标记位 `tag` 和高速缓存块 `block`

高速缓存块 `block` 的大小为 $B=2^b$，高速缓存块实际上是一系列对象的集合。当我们需要查找某对象的值时，首先给出该对象的地址，如果在当前**层**无法利用该地址找到对应的对象，我们需要从下一层将高速缓存块 `block` 替换上来。这里有一点很关键是：一行的实际内容就是高速缓存块，因此我们**实际上执行的是行替换**

当我们利用组索引和标记确定确定哪一行之后，我们便确定了高速缓存块，这个块中存储了很多的对象，我们利用 `b` 位的块偏移来索引这些对象

* 关于读取

我给定一个地址，我需要读该地址对应的内存字（某个对象），此时与两种情况：

* 如果命中 `hit`，那么高速缓存将直接返回该内存字
* 如果不命中 `miss`，如上面所说，我们会执行行替换，再将该内存字返回（我们在此不考虑替换策略）

* 关于写入

我们给定一个地址，我需要向该地址写入某个值

* 如果命中 `hit`，意思是指在当前层的高速缓存中，**存储了该地址对应的字的值**，那么我们直接对该地址所对应的值修改即可（我们不考虑写命中的修改方法）
* 如果不命中 `miss`，意思是指当前层的高速缓存中，**没有存储该地址对应的字的值**，我们需要执行行替换，将这个字写入（这被称为 `write-allocate`，还有一种被称为 `not-write-allocate` 的方法，直接将这个字写入到低一级的高速缓存中）

在这里，由于我们假设地址是对齐的，因此可以不考虑 `size` 这个变量，那么我们已知一个地址，只需要：

* 找出其组索引 `group` 和标记位 `tag`。前者用于**寻找该地址属于哪个组**，后者用于**确定该组中是否存在包含该地址的行**。一个地址**一定**对应一行和一个标记位
* 我们的 `LRU` 算法替换的对象是**同组当中的某一行**，当我们将某个组替换上来后，该组的有效位要置一
* 不像书中模拟那样，我们需要依靠块索引来得到地址包含的内容。在这里我们只是确定是否会发生 `hit, miss, evict`，并不需要实际的读取出相应的值。因此，只要我们能够确定该地址对应的组的某一行中，那么就是 `hit`，如果该组的所有行已经满了，那么就是 `evict`，如果该地址对应的组中没有行包含该地址，那么就是 `miss`
* 无论是 `Load` 指令还是 `Store` 指令，都遵循上面这一点，因此我们只需要实现一个公共的函数供二者调用即可。我们仅仅只是模拟，并不需要将数值取出来
* 对于 `Modify`，本质上是先执行 `Load` 后执行 `Store`，我们调用两次公共函数即可

#### 核心代码

因此，我们构建一个二维数组，以组索引 `group` 为横坐标，行的个数 `E` 为纵坐标，定义如下：

```c
typedef struct
{
    int tag, valid, timestamp;
}line;

typedef struct
{
    int S, E, B;
    line** ptr;
}cache;
```

`timestamp` 表示时间戳，用于记录该地址进入缓存 `cache` 的时间

根据上面的分析可知，我们不需要得到块索引，只需要得到组索引和标记位：

```c
//addr is address
int group = (addr >> b) & ((1 << s) - 1);
int tag = addr >> (b + s);
```

我们首先考察在 `group` 组中是否存在该地址，也就是 `group` 中所存储的行的 `tag` 是否与当前地址的 `tag` 相同

```c
int get_idx(int tag, int group)
{
    for(int i = 0; i < set.E; i ++)
    {
        if(set.ptr[group][i].valid && set.ptr[group][i].tag == tag)
            return i;
    }
    return -1;
}
```

我们公共函数 `update` 的定义如下：

当我们需要的行不存在时，进一步考察该组是否已经放满

* 如果放满则需要用 `LRU` 找出应该被踢出的行，将其下标标记为 `idx`

* 如果没有放满则找到一个可用的行，也就是找到 `valid` 为零的行

如果我们需要的行存在，则不做任何处理

在每次循环的结尾，我们需要更新将当前地址对应行的时间戳设置为当前值 `time`

```c
void update(int tag, int group, int time)
{
    //idx表示某一行，即当前地址所对应的行是否存在
    int idx = get_idx(tag, group);
    if(idx == -1)//miss
    {
        misses++;
        if(verbose) printf("miss\n");
        int full = 1;
        //该组中是否存在行没有满
        for(int i = 0; i < set.E; i ++)
        {
            if(set.ptr[group][i].valid == 0)
                full = 0;
        }
        if(full)
        {
            evictions++;
            if(verbose) printf("eviction\n");
            int mx = -1;
            for(int i = 0; i < set.E; i ++)
            {
                if(time - set.ptr[group][i].timestamp > mx)
                {
                    mx = time - set.ptr[group][i].timestamp;
                    idx = i;
                }
            }
        }
        else
        {
            for(int i = 0; i < set.E; i ++)
            {
                if(set.ptr[group][i].valid == 0)
                {
                    idx = i;
                    break;
                }
            }
        }
    }
    else
    {
        hits++;
        if(verbose) printf("hit\n");
    }
    set.ptr[group][idx].timestamp = time;
    set.ptr[group][idx].tag = tag;
    set.ptr[group][idx].valid = 1;
}
```

完整代码如下：

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include "cachelab.h"


void usage();
int parse_arg(int argc, char* argv[]);
void init();
void calculate();
void update(int, int, int);
void free_cache();

typedef struct
{
    int tag, valid, timestamp;
}line;

typedef struct
{
    int S, E, B;
    line** ptr;
}cache;

cache set;
int op = -1, verbose = 0, s = -1, E = -1, b = -1;
char* file_input = NULL;
int hits, misses, evictions;


int main(int argc, char* argv[])
{

    int res = parse_arg(argc, argv);
    if(res == -1) return -1;

    init();
    calculate();
    //free_cache();
    printSummary(hits, misses, evictions);    
    return 0;
}

void free_cache()
{
    for(int i = 0; i < set.S; i ++)
        free(set.ptr[i]);
    free(set.ptr);
}

void calculate()
{
    FILE* fp = fopen(file_input, "r");
    char file_line[20], op;
    int addr, size, time = 0;
    while(fgets(file_line, 20, fp) != NULL)
    {
        if(file_line[0] == 'I') continue;
        //我们假设内存自动对齐，因此可以忽略size
        sscanf(file_line, " %c %x,%d", &op, &addr, &size);
        int group = (addr >> b) & ((1 << s) - 1);
        int tag = addr >> (b + s);

        switch(op)
        {
            case 'M': update(tag, group, ++time);
            case 'L':
            case 'S': update(tag, group, ++time);
        }
    }
    fclose(fp);
}

int get_idx(int tag, int group)
{
    for(int i = 0; i < set.E; i ++)
    {
        if(set.ptr[group][i].valid && set.ptr[group][i].tag == tag)
            return i;
    }
    return -1;
}

void update(int tag, int group, int time)
{
    //idx表示某一行，即当前地址所对应的行是否存在
    int idx = get_idx(tag, group);
    if(idx == -1)//miss
    {
        misses++;
        if(verbose) printf("miss\n");
        int full = 1;
        //该组中是否存在行没有满
        for(int i = 0; i < set.E; i ++)
        {
            if(set.ptr[group][i].valid == 0)
                full = 0;
        }
        if(full)
        {
            evictions++;
            if(verbose) printf("eviction\n");
            int mx = -1;
            for(int i = 0; i < set.E; i ++)
            {
                if(time - set.ptr[group][i].timestamp > mx)
                {
                    mx = time - set.ptr[group][i].timestamp;
                    idx = i;
                }
            }
        }
        else
        {
            for(int i = 0; i < set.E; i ++)
            {
                if(set.ptr[group][i].valid == 0)
                {
                    idx = i;
                    break;
                }
            }
        }
    }
    else
    {
        hits++;
        if(verbose) printf("hit\n");
    }
    set.ptr[group][idx].timestamp = time;
    set.ptr[group][idx].tag = tag;
    set.ptr[group][idx].valid = 1;
}

void init()
{
    set.S = 1 << s;
    set.B = 1 << b;
    set.E = E;      //E表示标记个数
    set.ptr = (line**)malloc(set.S * sizeof (line*));
    for(int i = 0; i < set.S; i ++)
    {
        set.ptr[i] = (line*)malloc(set.E * sizeof (line));
        for(int j = 0; j < set.E; j ++)
        {
            set.ptr[i][j].tag = -1;
            set.ptr[i][j].valid = 0;
            set.ptr[i][j].timestamp = 0;
        }
    }
}

void usage()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("-h: Optional help flag that prints usage info\n");
    printf("-v: Optional verbose flag that displays trace info\n");
    printf("-s <s>: Number of set index bits (S = 2s is the number of sets)\n");
    printf("-E <E>: Associativity (number of lines per set)\n");
    printf("-b <b>: Number of block bits (B = 2b is the block size)\n");
    printf("-t <tracefile>: Name of the valgrind trace to replay\n");
}

int parse_arg(int argc, char* argv[])
{
    while((op = getopt(argc, argv, "hvs:E:b:t:")) != -1)
    {
        switch(op)
        {
            case 'h':
                usage();
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                file_input = optarg;
                break;
            default:
                usage();
                return -1;
        }
    }
    if(file_input == NULL) 
    {
        usage();
        return -1;
    }
    return 0;
}
```

测试结果：

```bash
$ ./test-csim 
                        Your simulator     Reference simulator
Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts
     3 (1,1,1)       9       8       6       9       8       6  traces/yi2.trace
     3 (4,2,4)       4       5       2       4       5       2  traces/yi.trace
     3 (2,1,4)       2       3       1       2       3       1  traces/dave.trace
     3 (2,1,3)     167      71      67     167      71      67  traces/trans.trace
     3 (2,2,3)     201      37      29     201      37      29  traces/trans.trace
     3 (2,4,3)     212      26      10     212      26      10  traces/trans.trace
     3 (5,1,5)     231       7       0     231       7       0  traces/trans.trace
     6 (5,1,5)  265189   21775   21743  265189   21775   21743  traces/long.trace
    27

TEST_CSIM_RESULTS=27
```

### Part B

`Part B` 要求我们优化矩阵转置函数，使得其 `miss` 数最小

开始这个实验之前，一定要去把书后面的家庭作业做完，不然无法进行下去

我们给定的高速缓存的参数为 $s = 5,\ E = 1,\ b = 5$，因此高速缓存有 `32` 行，每行可以存储 $2^{5}/4=8$ 个 `int`。也就是说，我们缓存可以存储的 `int` 个数为 $32\times 8 = 256$ 个

但，这当中会发生地址的碰撞，我们详细地考虑一下高速缓存的碰撞问题

高速缓存需要用地址来进行索引，对于单个 $32\times 32$ 的 `int` 矩阵而言，地址的范围为：$32\times 32\times 4 =4096=2^{12}$，也就是说，我们需要 `12` 位地址来进行索引

`12` 位地址的后五位表示 `b`，往前五位表示 `s`，最高的三位表示标记，即：

```
00   00000   00000
标记  组索引  块索引
```

由于我们的相联度只有 `1`，即每组只有一行，每一行能存储 `8` 个 `int`，我们必须要考虑什么时候会发生碰撞的问题：

当组索引相同是，无论块索引的值如何，**只要标记不同，就会发生碰撞**

也就是说，以下的两个地址会发生碰撞：

```
01   10010    01000
10   10010    10101  
```

这是因为标记的作用是在同一组中寻找需要的行的，而我们这里每一组只有一行，因此只要两个地址的组索引相同，那么就会映射到同一行

因此，最大不冲突的行索引为 `11111`，对应的地址为 `00 11111 11111`。也就是说，所有高于此地址的映射都将发生冲突

因此**这个高速缓存在不发生冲突的情况下，最多只能存储 $2^{10}/4=256$ 个 `int`**，也就是 `A[8]` 行会与 `A[0]` 行重合，以此类推

这是单个矩阵的情况，我们转置需要两个矩阵，**因此`A[0]` 行、`A[8]` 行、`B[0]` 行和 `B[8]` 会共享同一行**

由于我们每一行只能存储 `8` 个 `int`，并且矩阵的转置要求小矩阵是正方形，因此适合的分块大小为 $8\times 8$

我们在 $32\times 32$ 矩阵中进行分析：

将 $32\times 32$ 矩阵分成 $16$ 个 $8\times 8$ 的小矩阵，分别用坐标 $(0,0)$ 到 $(3,3)$ 来表示

我们用 `h` 表示 `hit`，`m` 表示 `miss`，对于 $(0,0)$ 处的小矩阵有：

对于矩阵 $A$ 而言，其碰撞结果如下：

||`c0`|`c1`|`c2`|`c3`|`c4`|`c5`|`c6`|`c7`|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|`r0`|m|m|h|h|h|h|h|h|
|`r1`|m|h|m|h|h|h|h|h|
|`r2`|m|h|h|m|h|h|h|h|
|`r3`|m|h|h|h|m|h|h|h|
|`r4`|m|h|h|h|h|m|h|h|
|`r5`|m|h|h|h|h|h|m|h|
|`r6`|m|h|h|h|h|h|h|m|
|`r7`|m|h|h|h|h|h|h|h|

对于转置矩阵 $B$ 而言，其碰撞结果如下：

||`c0`|`c1`|`c2`|`c3`|`c4`|`c5`|`c6`|`c7`|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|`r0`|m|m|h|h|h|h|h|h|
|`r1`|m|m|m|h|h|h|h|h|
|`r2`|m|h|m|m|h|h|h|h|
|`r3`|m|h|h|m|m|h|h|h|
|`r4`|m|h|h|h|m|m|h|h|
|`r5`|m|h|h|h|h|m|m|h|
|`r6`|m|h|h|h|h|h|m|m|
|`r7`|m|h|h|h|h|h|h|m|

我们求出上面一个发生了 $37$ 次 `miss`

$(0,0)$ 位置的矩阵二者存在相互碰撞的情况，事实上所有处于对角线的小矩阵都是如此

对于两个不出在对角线的矩阵，二者之间的映射不会发生碰撞，因此一共会有 `16` 次 `miss`

那么我们可以求出整个 $32\times 32$ 矩阵的总 `miss` 的值：$37\times 4+16\times 6=340$

我们按上述思想写出如下程序：

```c
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < M; i += 8)
        for(int j = 0; j < N; j += 8)
            for(int n = 0; n < 8; n ++)
                for(int m = 0; m < 8; m ++)
                    B[j + m][i + n] = A[i + n][j + m];
}
```

运行结果：

```bash
$ ./test-trans -N 32 -M 32

Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:1709, misses:344, evictions:312

Summary for official submission (func 0): correctness=1 misses=344

TEST_TRANS_RESULTS=1:344
```

可以看到与我们的计算结果十分的接近，我们现在知道出现 `miss` 的原因主要是二者映射到同一块区域造成的

我们考虑在一个分块内，利用八个变量先将 $A$ 的值记录下来，然后再对 $B$ 赋值

我们用 `(i, j)` 表示每个分块左上角的坐标，用 `k` 来对分块内部进行枚举，之后枚举八次，因此有：

```c
void solve32(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < M; i += 8)
    {
        for(int j = 0; j < N; j += 8)
        {
            for(int k = 0; k < 8; k ++)
            {
                int a1 = A[k + i][j];
                int a2 = A[k + i][j + 1];
                int a3 = A[k + i][j + 2];
                int a4 = A[k + i][j + 3];
                int a5 = A[k + i][j + 4];
                int a6 = A[k + i][j + 5];
                int a7 = A[k + i][j + 6];
                int a8 = A[k + i][j + 7];
                B[j][k + i] = a1;
                B[j + 1][k + i] = a2;
                B[j + 2][k + i] = a3;
                B[j + 3][k + i] = a4;
                B[j + 4][k + i] = a5;
                B[j + 5][k + i] = a6;
                B[j + 6][k + i] = a7;
                B[j + 7][k + i] = a8;
            }
        }
    }
}
```

测试结果：

```bash
$ ./test-trans -N 32 -M 32

Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:1765, misses:288, evictions:256

Summary for official submission (func 0): correctness=1 misses=288

TEST_TRANS_RESULTS=1:288
```

当我们将这个程序运行在 $64\times 64$ 的矩阵上，我们得到如下结果：

```bash
func 0 (Transpose submission): hits:3585, misses:4612, evictions:4580
```

这跟没优化的结果基本一致，我们需要知道这是为什么

这个高速缓存在不发生碰撞的情况下最多只能存储 `256` 个 `int`，因此对于 $64\times 64$ 的矩阵而言，只能保证前四行是不发生碰撞的

换句话说，`A[0]`、`A[4]`、`B[0]`、`B[4]` 的映射都会发生碰撞

因此我们直接将 $64\times 64$ 的矩阵划分为 $4\times 4$ 的矩阵，先测试一下效率：

```c
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < M; i += 4)
    {
        for(int j = 0; j < N; j += 4)
        {
            for(int k = 0; k < 4; k ++)
            {
                int a1 = A[k + i][j];
                int a2 = A[k + i][j + 1];
                int a3 = A[k + i][j + 2];
                int a4 = A[k + i][j + 3];
                B[j][k + i] = a1;
                B[j + 1][k + i] = a2;
                B[j + 2][k + i] = a3;
                B[j + 3][k + i] = a4;
            }
        }
    }
}
```

测试结果：

```bash
TEST_TRANS_RESULTS=1:1700
```

可以发现，我们的思路其实是没有说明问题的，我们需要找到一种更大的分块方法并保证发生的碰撞尽可能小

将 $8\times 8$ 的矩阵分成 `4` 个小块，然后分别对每个小块进行处理，得到如下代码：

```c
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < M; i += 8)
    {
        for(int j = 0; j < N; j += 8)
        {
            //A左上角 - B左上角
            for(int k = i; k < i + 4; k ++)
            {
                int a1 = A[k][j];
                int a2 = A[k][j + 1];
                int a3 = A[k][j + 2];
                int a4 = A[k][j + 3];
                B[j][k] = a1;
                B[j + 1][k] = a2;
                B[j + 2][k] = a3;
                B[j + 3][k] = a4;
            }
            //A左下角 - B右上角
            for(int k = i; k < i + 4; k ++)
            {
                int a1 = A[k + 4][j];
                int a2 = A[k + 4][j + 1];
                int a3 = A[k + 4][j + 2];
                int a4 = A[k + 4][j + 3];
                B[j][k + 4] = a1;
                B[j + 1][k + 4] = a2;
                B[j + 2][k + 4] = a3;
                B[j + 3][k + 4] = a4;
            }
            //A右下角 - B右下角
            for(int k = i; k < i + 4; k ++)
            {
                int a1 = A[k + 4][j + 4];
                int a2 = A[k + 4][j + 5];
                int a3 = A[k + 4][j + 6];
                int a4 = A[k + 4][j + 7];
                B[j + 4][k + 4] = a1;
                B[j + 5][k + 4] = a2;
                B[j + 6][k + 4] = a3;
                B[j + 7][k + 4] = a4;
            }
            //A右上角 - B左下角
            for(int k = i; k < i + 4; k ++)
            {
                int a1 = A[k][j + 4];
                int a2 = A[k][j + 5];
                int a3 = A[k][j + 6];
                int a4 = A[k][j + 7];
                B[j + 4][k] = a1;
                B[j + 5][k] = a2;
                B[j + 6][k] = a3;
                B[j + 7][k] = a4;                
            }
        }
    }
}
```

测试结果：

```bash
TEST_TRANS_RESULTS=1:1428
```

我们尽可能让它们之间不要起冲突，具体地，可以只要对应块的映射不重叠，则可以保证冲突的最小值，例如：

* `A` 的左上角可以与 `B` 的右上或右下角（`A` 的左下角同理）
* `A` 的左上角和右上角可以与 `B` 的左上角和右上角（利用前面的临时变量展开的原理）

我们将 $8\times 8$ 的矩阵分割成四个 $4\times 4$ 的矩阵，从左上到右下分别用坐标 `(0, 0)` 到 `(1, 1)` 表示

在一开始我们对 $A$ 中矩阵 `(0, 0)` 和 `(0, 1)` 操作时，为了尽可能的利用高速缓存，我们将 $A$ 的一行记录在局部变量中，这一行本应放在 $B$ 的 `(0, 0)` 和 `(1, 0)`，但由于这两块会引起冲突，因此我们将 $A$ 的 `(0, 1)` 放在 $B$ 的 `(0, 1)`，我们之后再将 $B$ 的 `(0, 1)` 移动到 $B$ 的左下角即可

重新排列之后的结果为：

```c
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int a1, a2, a3, a4, a5, a6, a7, a8;
    for(int i = 0; i < M; i += 8)
    {
        for(int j = 0; j < N; j += 8)
        {
            //A的左上 -> B的左上
            //A的右上 -> B的右上
            for(int k = i; k < i + 4; k ++)
            {
                a1 = A[k][j];
                a2 = A[k][j + 1];
                a3 = A[k][j + 2];
                a4 = A[k][j + 3];
                a5 = A[k][j + 4];
                a6 = A[k][j + 5];
                a7 = A[k][j + 6];
                a8 = A[k][j + 7];
                B[j][k] = a1;
                B[j + 1][k] = a2;
                B[j + 2][k] = a3;
                B[j + 3][k] = a4;
                B[j][k + 4] = a5;
                B[j + 1][k + 4] = a6;
                B[j + 2][k + 4] = a7;
                B[j + 3][k + 4] = a8;
            }

            //B的右上 -> B的左下
            //A的左下 -> B的右上
            for(int k = i; k < i + 4; k ++)
            {
                //B的右上
                a5 = B[j][k + 4];
                a6 = B[j + 1][k + 4];
                a7 = B[j + 2][k + 4];
                a8 = B[j + 3][k + 4];

                //A的左下
                a1 = A[k + 4][j];
                a2 = A[k + 4][j + 1];
                a3 = A[k + 4][j + 2];
                a4 = A[k + 4][j + 3];

                //B的右上
                B[j][k + 4] = a1;
                B[j + 1][k + 4] = a2;
                B[j + 2][k + 4] = a3;
                B[j + 3][k + 4] = a4;

                //B的左下
                B[j + 4][k] = a5;
                B[j + 5][k] = a6;
                B[j + 6][k] = a7;
                B[j + 7][k] = a8;
            }
            //A的右下 -> B的右下
            for(int k = i; k < i + 4; k ++)
            {
                a1 = A[k + 4][j + 4];
                a2 = A[k + 4][j + 5];
                a3 = A[k + 4][j + 6];
                a4 = A[k + 4][j + 7];    
                B[j + 4][k + 4] = a1;
                B[j + 5][k + 4] = a2;
                B[j + 6][k + 4] = a3;
                B[j + 7][k + 4] = a4;
 
            }
        }
    }
}
```

测试结果：

```bash
TEST_TRANS_RESULTS=1:2684
```

我们看到速度反而下降了，我们在将 $B$ 的 `(0, 1)` 矩阵移动到 `(1, 0)` 时，采取的是列优先的策略，我们考虑将其改为行优先，这样对于每一次循环，发生 `miss` 的次数将从 `4` 次降为 `1` 次

由于 $B$ 为 $A$ 的转置矩阵，因此对于 $A$ 中坐标 $(i,j)$ 而言，其在 $B$ 中的坐标为 $(j,i)$。换句话说，**如果我们用 $K$ 枚举 $A$ 中的行，那么对于 $B$ 就是在枚举列**

在上面的代码中，由于我们的 $k$ 枚举的是 $A$ 中的行，这会对应于去枚举 $B$ 中的列，换句话说，在我们对 $B$ 的 `(0, 1)` 矩阵进行移动时，**只能做到一列一列移动**

为了将 $B$ 的移动转为一行一行移动，我们需要让 $k$ 枚举 $A$ 中的列。我们写出如下代码：

```c
void solve64(int M, int N, int A[N][M], int B[M][N])
{
    int a1, a2, a3, a4, a5, a6, a7, a8;
    for(int i = 0; i < M; i += 8)
    {
        for(int j = 0; j < N; j += 8)
        {
            //A的左上 -> B的左上
            //A的右上 -> B的右上
            for(int k = i; k < i + 4; k ++)//k枚举A中的行
            {
                a1 = A[k][j];
                a2 = A[k][j + 1];
                a3 = A[k][j + 2];
                a4 = A[k][j + 3];
                a5 = A[k][j + 4];
                a6 = A[k][j + 5];
                a7 = A[k][j + 6];
                a8 = A[k][j + 7];
                B[j][k] = a1;
                B[j + 1][k] = a2;
                B[j + 2][k] = a3;
                B[j + 3][k] = a4;
                B[j][k + 4] = a5;
                B[j + 1][k + 4] = a6;
                B[j + 2][k + 4] = a7;
                B[j + 3][k + 4] = a8;
            }

            //B的右上 -> B的左下
            //A的左下 -> B的右上
            for(int k = j; k < j + 4; k ++)//k枚举A中的列
            {
                //若A中枚举坐标为(i, k)，则B中对应的坐标为(k, i)
                //B的右上
                a1 = B[k][i + 4];
                a2 = B[k][i + 5];
                a3 = B[k][i + 6];
                a4 = B[k][i + 7];
                //A的左下
                a5 = A[i + 4][k];
                a6 = A[i + 5][k];
                a7 = A[i + 6][k];
                a8 = A[i + 7][k];
                //B的右上
                B[k][i + 4] = a5;
                B[k][i + 5] = a6;
                B[k][i + 6] = a7;
                B[k][i + 7] = a8;
                //B的左下
                B[k + 4][i] = a1;
                B[k + 4][i + 1] = a2;
                B[k + 4][i + 2] = a3;
                B[k + 4][i + 3] = a4;

            }
            //A的右下 -> B的右下
            for(int k = i; k < i + 4; k ++)//k枚举A中的行
            {
                a1 = A[k + 4][j + 4];
                a2 = A[k + 4][j + 5];
                a3 = A[k + 4][j + 6];
                a4 = A[k + 4][j + 7];    
                B[j + 4][k + 4] = a1;
                B[j + 5][k + 4] = a2;
                B[j + 6][k + 4] = a3;
                B[j + 7][k + 4] = a4;
            }
        }
    }
}
```

测试结果：

```bash
$ ./test-trans -N 64 -M 64

Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:9017, misses:1228, evictions:1196

Summary for official submission (func 0): correctness=1 misses=1228

TEST_TRANS_RESULTS=1:1228
```

对于第三个 $67\times 61$ 的矩阵，这个矩阵我们无法将其划分为完整的小分块，我们随意的尝试几个数，得出结果即可，这个很难优化

我们将分块的大小设置为 `18`，这个结果是我试出来的，这里的限制很松，很容易过

```c
void solveMisc(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < N; i += 18)
    {
        for(int j = 0; j < M; j += 18)
        {
            for(int n = i; n < i + 18 && n < N; n ++)
            {
                for(int m = j; m < j + 18 && m < M; m ++)
                {
                    B[m][n] = A[n][m];
                }
            }
        }
    }
}
```

测试结果：

```bash
$ ./test-trans -N 67 -M 61

Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:6217, misses:1962, evictions:1930

Summary for official submission (func 0): correctness=1 misses=1962
```

全部测试结果如下：

```bash
$ ./driver.py 
Part A: Testing cache simulator
Running ./test-csim
                        Your simulator     Reference simulator
Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts
     3 (1,1,1)       9       8       6       9       8       6  traces/yi2.trace
     3 (4,2,4)       4       5       2       4       5       2  traces/yi.trace
     3 (2,1,4)       2       3       1       2       3       1  traces/dave.trace
     3 (2,1,3)     167      71      67     167      71      67  traces/trans.trace
     3 (2,2,3)     201      37      29     201      37      29  traces/trans.trace
     3 (2,4,3)     212      26      10     212      26      10  traces/trans.trace
     3 (5,1,5)     231       7       0     231       7       0  traces/trans.trace
     6 (5,1,5)  265189   21775   21743  265189   21775   21743  traces/long.trace
    27


Part B: Testing transpose function
Running ./test-trans -M 32 -N 32
Running ./test-trans -M 64 -N 64
Running ./test-trans -M 61 -N 67

Cache Lab summary:
                        Points   Max pts      Misses
Csim correctness          27.0        27
Trans perf 32x32           8.0         8         288
Trans perf 64x64           8.0         8        1228
Trans perf 61x67          10.0        10        1962
          Total points    53.0        53
```

---

## 后记

这个实验我花了两天的时间，大概肝了 `20+` 个小时。通过这个实验，我这才把 `cache` 的寻址以及碰撞给搞清楚了。我一把书看完就来做这个实验，一开始这些概念真的很模糊，但这个实验神奇的地方就在于它会强迫我去通过做书后面的家庭作业来让我能够有一个更加深入的理解。总的来说，感觉还不错