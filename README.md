# CSAPP_LAB

本仓库用于记录我 `CSAPP LAB` 的源代码以及相关问题

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