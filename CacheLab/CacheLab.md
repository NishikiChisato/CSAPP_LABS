# cachelab

- [cachelab](#cachelab)
  - [准备](#准备)
  - [实验开始](#实验开始)
    - [Part A](#part-a)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

## 实验开始

### Part A

> 我们需要首先学会 `getopt` 这个函数

直接编译会报错：

```bash
error: implicit declaration of function ‘getopt’
```

这是因为编译参数为 `-std=c99` 导致的，这将会阻止 `GNU extension`，因此我们需要自己包含 `getopt.h` 这个头文件

[原贴](https://stackoverflow.com/questions/22575940/getopt-not-included-implicit-declaration-of-function-getopt)

