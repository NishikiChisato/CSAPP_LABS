# CSAPP_LABS

本仓库用于记录我 `CSAPP LAB` 的源代码以及相关问题

## In process

* [x] *Data Lab*
* [x] *Domb Lab*
* [x] *Attack Lab*
  * [ ] *Buffer Lab(IA32)*
* [x] *Architecture Lab*
  * [ ] *Architecture Lab(Y86)*
* [x] *Cache Lab*
  * [ ] *Performance Lab*
* [x] *Shell Lab*
* [ ] *Malloc Lab*
* [ ] *Proxy Lab*

> * *`Architecture Lab(Y86)`* 为 `CSAPP:2e` 的版本，可以只做 *`Architecture Lab`*
> * *`Buffer Lab(IA32)`* 为 *`Attack Lab`* 的 `32` 位版本，也属于 `CSAPP:2e`，可以只做 *`Attack Lab`*
> * *`Performance Lab`* 为 *`Cache Lab`* 的弱化版本，由于此实验设计较为粗糙，因此可以只做 *`Cache Lab`*

## What I learn

1. 对计算机补码表示和运算、二进制数据操作有更深刻的认识，对 IEEE 浮点数表示及其精度上的局限有清晰的认识
2. 具有理解汇编和反汇编代码的能力，能对过程调用时所进行的操作有清晰的理解，熟悉 CGDB 调试操作
3. 了解 `CPU` 的运行方式，对 `CPU` 识别指令的过程以及缓冲区溢出攻击有所认识
4. 加深对 `CPU` 流水线的工作原理的理解，学习了对底层指令进行优化的方法，提高了编码能力
5. 学习了 `LRU` 在 `cache` 当中的实现，了解了 `cache` 的寻址以及碰撞等工作原理，能够有能力编写出降低 `cache` 碰撞的代码
6. 学习了信号处理函数的使用以及多进程程序避免竞争的办法

## File structure

```
.
├── ArchitectureLab
│   └── archlab-handout
│       └── sim
│           ├── misc
│           │   └── answer
│           ├── pipe
│           ├── ptest
│           ├── seq
│           └── y86-code
├── Attacklab
│   └── target1
│       └── answer
│           ├── phase_1
│           ├── phase_2
│           ├── phase_3
│           ├── phase_4
│           └── phase_5
├── BombLab
│   └── bomb
├── CacheLab
│   └── cachelab-handout
│       └── traces
├── DataLab
│   └── datalab-handout
├── ShellLab
│   └── shlab-handout
└── Tar

```

---

## Debug

在 `Ubuntu22.04` 平台上运行

如出现：

    /usr/include/stdio.h:27:10: fatal error: bits/libc-header-start.h: 没有那个文件或目录
      | #include <bits/libc-header-start.h>
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~
    compilation terminated.

说明缺少`32`位的库文件，运行以下命令进行安装

```bash
sudo apt-get install gcc-multilib
```