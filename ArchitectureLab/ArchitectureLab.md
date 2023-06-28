# arclab

## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

## 环境配置

进行这个实验，我们首先需要安装 `flex` 和 `bison`：

```bash
sudo apt-get install flex
sudo apt-get install bison
```

之后运行 `make clean ; make`，得到报错：

```bash
(cd misc; make all)
make[1]: 进入目录“/home/chisato/CSAPPLABS/ArchitectureLab/archlab-handout/sim/misc”
gcc -Wall -O1 -g yas-grammar.o yas.o isa.o -lfl -o yas
/usr/bin/ld: yas.o:/home/chisato/CSAPPLABS/ArchitectureLab/archlab-handout/sim/misc/yas.h:13: multiple definition of `lineno'; yas-grammar.o:(.bss+0x0): first defined here
collect2: error: ld returned 1 exit status
make[1]: *** [Makefile:32：yas] 错误 1
make[1]: 离开目录“/home/chisato/CSAPPLABS/ArchitectureLab/archlab-handout/sim/misc”
make: *** [Makefile:26：all] 错误 2
```

这是由于 `gcc` 版本过老导致的，解决方式如下：

将 `misc/Makefile` 中

```makefile
CFLAGS=-Wall -O1 -g
LCFLAGS=-O1
```

改为：

```makefile
CFLAGS=-Wall -O1 -g -fcommon
LCFLAGS=-O1 -fcommon
```

对于其他的 `pipe` 和 `seq` 中的 `Makefile` 文件，我们也需要同样做更改

原贴如下：[Fail to compile the Y86 simulatur (CSAPP)](https://stackoverflow.com/questions/63152352/fail-to-compile-the-y86-simulatur-csapp)

> 对于全局变量来说，如果初始化为不为零的值，那么保存在 `data` 段，初始化为零值，保存在 `BSS` 段，如果没有初始化仅仅是声明，那么保存在 `common` 段，等到链接时再放入 `BSS` 段

这个实验的模拟器有两种：`TTY` 和 `GUI`

如果需要用 `GUI` ，那么可以安装 `Tcl/Tk`：

```bash
sudo apt install tcl tcl-dev tk tk-dev
```

我们并没有安装，因此用 `TTY`

我们将 `pipe/Makefile` 和 `pipe/Makefile` 中的 `GUIMODE, TKLIBS, TKINC` 全部注释掉

将 `sim/Makefile` 从原先的：

```
all:
	(cd misc; make all)
	(cd pipe; make all GUIMODE=$(GUIMODE) TKLIBS="$(TKLIBS)" TKINC="$(TKINC)")
	(cd seq; make all GUIMODE=$(GUIMODE) TKLIBS="$(TKLIBS)" TKINC="$(TKINC)")
	(cd y86-code; make all)

clean:
	rm -f *~ core
	(cd misc; make clean)
	(cd pipe; make clean)
	(cd seq; make clean)
	(cd y86-code; make clean)
	(cd ptest; make clean)
```

修改为：

```
all:
	(cd misc; make all)
	(cd pipe; make all)
	(cd seq; make all)
	(cd y86-code; make all)

clean:
	rm -f *~ core
	(cd misc; make clean)
	(cd pipe; make clean)
	(cd seq; make clean)
	(cd y86-code; make clean)
	(cd ptest; make clean)
```

执行 `make clean ; make` 后编译成功：

```
../misc/yas abs-asum-cmov.ys
../misc/yas abs-asum-jmp.ys
../misc/yas asum.ys
../misc/yas asumr.ys
../misc/yas asumi.ys
../misc/yas cjr.ys
../misc/yas j-cc.ys
../misc/yas poptest.ys
../misc/yas pushquestion.ys
../misc/yas pushtest.ys
../misc/yas prog1.ys
../misc/yas prog2.ys
../misc/yas prog3.ys
../misc/yas prog4.ys
../misc/yas prog5.ys
../misc/yas prog6.ys
../misc/yas prog7.ys
../misc/yas prog8.ys
../misc/yas prog9.ys
../misc/yas prog10.ys
../misc/yas ret-hazard.ys
```

---

## 实验开始

### Part A

参照 [PDF](http://csapp.cs.cmu.edu/3e/archlab.pdf) 中 `4` 的部分

