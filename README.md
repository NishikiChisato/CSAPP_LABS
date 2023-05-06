# CSAPP_LABS

本仓库用于记录我 `CSAPP LAB` 的源代码以及相关问题

## 目前进度

* [x] *Data Lab*
* [x] *Domb Lab*
* [ ] *Attack Lab*
* [ ] *Architecture Lab*
* [ ] *Cache Lab*
* [ ] *Performance Lab*
* [ ] *Shell Lab*
* [ ] *Malloc Lab*
* [ ] *Proxy Lab*

> * *`Architecture Lab（Y86）`* 为 `CSAPP:2e` 的版本
> * *`Buffer Lab(IA32)`* 为 *`Attack Lab`* 的 `32` 位版本，也属于 `CSAPP:2e`

## 收获

1. 对计算机补码表示和运算、二进制数据操作有更深刻的认识，对 IEEE 浮点数表示及其精度上的局限有清晰的认识
2. 具有理解汇编代码的基本能力，能对过程调用时进行的操作有清晰的了解，熟悉 GDB 的基本调试操作




---

## Bug解决

建议在`Ubuntu22.04`平台上运行本项目

如出现：

    /usr/include/stdio.h:27:10: fatal error: bits/libc-header-start.h: 没有那个文件或目录
      | #include <bits/libc-header-start.h>
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~
    compilation terminated.

说明缺少`32`位的库文件，运行以下命令进行安装

```bash
sudo apt-get install gcc-multilib
```