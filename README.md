# CSAPP_LABS

本仓库用于记录我 `CSAPP LAB` 的源代码以及相关问题

## 目前进度

* [x] *Data Lab*
* [x] *Domb Lab*
* [x] *Attack Lab*
* [x] *Architecture Lab*
* [x] *Cache Lab*
* [ ] *Performance Lab*
* [ ] *Shell Lab*
* [ ] *Malloc Lab*
* [ ] *Proxy Lab*

> * *`Architecture Lab（Y86）`* 为 `CSAPP:2e` 的版本
> * *`Buffer Lab(IA32)`* 为 *`Attack Lab`* 的 `32` 位版本，也属于 `CSAPP:2e`
> * *`Performance Lab`* 为 *`Cache Lab`* 的弱化版本，由于此实验设计较为粗糙，因此可以只做 *`Cache Lab`*

## 收获

1. 对计算机补码表示和运算、二进制数据操作有更深刻的认识，对 IEEE 浮点数表示及其精度上的局限有清晰的认识
2. 具有理解汇编和反汇编代码的能力，能对过程调用时所进行的操作有清晰的理解，熟悉 CGDB 调试操作
3. 了解 `CPU` 的运行方式，对 `CPU` 识别指令的过程以及缓冲区溢出攻击有所认识
4. 加深对 `CPU` 流水线的工作原理的理解，学习了对底层指令进行优化的方法，提高了编码能力
5. 学习了 `LRU` 在 `cache` 当中的实现，了解了 `cache` 的寻址以及碰撞等工作原理，能够有能力编写出降低 `cache` 碰撞的代码




---

## Bug解决

建议在 `Ubuntu22.04` 平台上运行本项目

如出现：

    /usr/include/stdio.h:27:10: fatal error: bits/libc-header-start.h: 没有那个文件或目录
      | #include <bits/libc-header-start.h>
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~
    compilation terminated.

说明缺少`32`位的库文件，运行以下命令进行安装

```bash
sudo apt-get install gcc-multilib
```