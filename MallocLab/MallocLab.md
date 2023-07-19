# malloclab

- [malloclab](#malloclab)
  - [准备](#准备)
  - [实验开始](#实验开始)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

注：本实验无测试文件，我的 `trace` 测试文件是从 [这里](https://github.com/pgoodman/csc369/tree/master/malloclab/traces) 下载的

## 实验开始

我们先将课本的代码敲出来，然后在此基础上做更改。该代码的成绩为：

```bash
$ ./mdriver -v -t ./traces/
Team Name:Atom
Member 1 :Nishiki Chisato:NishikiChisato@outlook.com
Using default tracefiles in ./traces/
Measuring performance with gettimeofday().

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.003874  1470
 1       yes   99%    5848  0.003733  1566
 2       yes   99%    6648  0.005404  1230
 3       yes  100%    5380  0.003904  1378
 4       yes   66%   14400  0.000070204836
 5       yes   93%    4800  0.005083   944
 6       yes   92%    4800  0.004984   963
 7       yes   55%   12000  0.073554   163
 8       yes   51%   24000  0.149539   160
 9       yes   27%   14401  0.033063   436
10       yes   34%   14401  0.001632  8823
Total          74%  112372  0.284840   395

Perf index = 44 (util) + 26 (thru) = 71/100
```