# shlab

- [shlab](#shlab)
  - [准备](#准备)
  - [概念](#概念)
  - [分析](#分析)
  - [核心代码](#核心代码)
    - [eval](#eval)
    - [builtin\_cmd](#builtin_cmd)
    - [do\_bgfg](#do_bgfg)
    - [waitfg](#waitfg)
    - [signal handle](#signal-handle)
    - [测试](#测试)
  - [Bug 遗留](#bug-遗留)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

## 概念

在开始之前我们需要明确几个概念：

* `tsh` 用任务 `job` 来管理不同的程序，每当执行一个程序时，都会创建一个 `job`，`job` 的定义如下：

```c
struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
```

`pid` 表示当前进程的 `id`，`jid` 表示当前 `job` 的 `id`，`state` 用于表示当前任务的状态，有前台 `foreground`、后台 `background` 和 暂停 `stop`

`cmdline` 表示执行该任务输入的命令行。具体地，如果输入 `/bin/ls -alh`，那么 `cmdline` 就是 `/bin/ls -alh`

> 需要说明的是，任务 `job` 和进程 `process` 可以理解成一种东西，因为任务其实是对进程的一次封装

* `tsh` 会打印一个命令提示符 `prompt` 等待用户的输入，用户输入的指令有两种：内置指令 `built-in command` 和可执行程序
  * 对于 `built-in` 指令而言，**由 `tsh` 立即执行**
  * 对于可执行程序而言，`fork` 一个子进程然后调用 `execve` 执行，这种情况下，**任务既可以前台执行也可以后台执行**

对于 `fork` 得到的子进程而言，无论是前台执行还是后台执行，其结束后内核会向父进程发生 `SIGCHLD` 信号，父进程需要对子进程进行回收（利用 `waitpid` 函数）

如果该任务是前台执行，那么 `tsh` 需要等待其执行完毕，也就是**父进程需要等待子进程执行完毕**

* `built-in` 指令有四个，分别为：
  * `quit`：停止 `tsh` 的执行
  * `bg <job>`：将一个暂停的 `stop` 后台 `background` 任务转为执行的后台任务
  * `fg <job>`：将一个后台任务转为前台任务
  * `jobs`：列出所有的任务

`bg` 和 `fg` 指令均是向子进程发送一个 `SIGCONT` 信号，收到该信号的进程会自动从暂停 `stop` 状态转为运行 `running` 状态

为了保证进程在执行某些指令具有原子性，我们需要在这个过程中阻塞掉某些信号，对于被阻塞的信号而言，会被暂时挂起，等待进程阻塞结束后进而转到信号处理函数处执行

如果父进程需要等待子进程，那么：

```c
sigset_t mask;
sigemptyset(&mask);
while(child is running)
    sigsuspend(&mask)
```

`sigsuspend` 函数会将当前进程的阻塞集合用 `mask` 替换，然后挂起该进程，等待收到信号后再恢复阻塞集合

因此当子进程在执行时，`while` 循环处始终为 `true`，父进程会一直处于挂起的状态。而子进程结束后内核会向父进程发生一个 `SIGCHLD` 信号，这正好可以激活父进程

在 `fork` 时，父进程需要阻塞 `SIGCHLD` 信号，这是为了避免还未将子进程加入任务数组中就将其删除掉

> 前台执行与后台执行的区别在于，**前台执行父进程需要等待子进程执行完毕，而后台执行则不需要等待其执行完毕**

## 分析

我们先看作者给我们的函数：

```c
/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}
/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}
```

清理 `job` 和初始化函数，前者会在后面的 `deletejob` 中调用

```c
/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}
```

返回最大的 `job id`，这个函数主要是用来求出当前数组中 `job` 的个数

```c
/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}
```

增加和删除一个 `job`，需要的时候直接调用即可

```c
/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

```

该函数返回处于前台进程的 `pid`，我们可以将其作为父进程等待子进程的判断条件

```c
/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}
```

这两个函数是一起的，一个是用 `pid` 找 `job`，另一个是用 `jid` 找 `job`

```c
/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

```

将 `pid` 转为 `jid`，用于求作业的 `jid`

```c
/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
```

列举出所有的 `job`

我们先写出这些封装函数

```c
//function wrappers
pid_t Fork(void);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *iptr, int options);
void Kill(pid_t pid, int signum);
unsigned int Sleep(unsigned int secs);
void Pause(void);
unsigned int Alarm(unsigned int seconds);
void Setpgid(pid_t pid, pid_t pgid);
pid_t Getpgrp();

pid_t Fork(void)
{
    pid_t pid;
    if((pid = fork()) < 0)
        unix_error("Fork error");
    return pid;
}

void Execve(const char *filename, char *const argv[], char *const envp[])
{
    if(execve(filename, argv, envp) < 0)
    {
        printf("%s: command not found\n", argv[0]);
        exit(0);
    }
}

pid_t Wait(int *status)
{
    pid_t pid;
    if((pid = wait(status)) < 0)
        unix_error("Wait error");
    return pid;
}

pid_t Waitpid(pid_t pid, int *iptr, int options)
{
    pid_t rpid;
    if((rpid = waitpid(pid, iptr, options)) < 0)
        unix_error("Waitpid error");
    return rpid;
}

void Kill(pid_t pid, int signum)
{
    if(kill(pid, signum) < 0)
        unix_error("Kill error");
}

unsigned int Sleep(unsigned int secs)
{
    int ret;
    if((ret = sleep(secs)) < 0)
        unix_error("Sleep error");
    return ret;
}

void Pause(void)
{
    pause();
    return;
}

unsigned int Alarm(unsigned int seconds)
{
    return alarm(seconds);
}

void Setpgid(pid_t pid, pid_t pgid)
{
    if(setpgid(pid, pgid) < 0)
        unix_error("Setgpid error");
}

pid_t Getpgrp()
{
    return getpgrp();
}
```

## 核心代码

我们需要实现的函数为：

* `eval`：用于处理可执行程序以及前台执行和后台执行
* `builtin_cmd`：检查是否为内置 `built-in` 指令，并执行 `quit` 和 `jobs`
* `do_bgfg`：执行内置指令 `bg` 和 `fg`
* `waitfg`：父进程等待子进程的执行
* `sigchld`：子进程结束时内核发生 `SIGCHLD` 信号给父进程，父进程的信号处理函数
* `sigint`：键盘键入 `ctrl + c`，对应的信号处理程序
* `sigtstp`：键盘键入 `ctrl + z`，对应的信号处理程序



### eval

```c
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char buf[MAXLINE], *argv[MAXARGS];//buf存放输入的命令行，argv存放具体指令的参数，格式与main中的argv一样
    strcpy(buf, cmdline);
    int background = parseline(buf, argv);
    int status = background ? BG : FG;
    if(argv[0] == NULL) return;

    sigset_t mask_one, prev, mask_all;
    pid_t pid;

    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);
    sigfillset(&mask_all);
    if(!builtin_cmd(argv))//非内置指令
    {
        sigprocmask(SIG_BLOCK, &mask_one, &prev);
        //子进程自己执行，当执行完毕后会发送SIF_CHLD信号给父进程
        if((pid = Fork()) == 0)
        {
            sigprocmask(SIG_SETMASK, &prev, NULL);
            Setpgid(0, 0);//设置子进程的进程组
            Execve(argv[0], argv, environ);//environ定义于lib中
            exit(0);
        }
        //父进程执行以下代码
        if(status == BG)//后台执行
        {
            sigprocmask(SIG_BLOCK, &mask_all, &prev);//阻塞所有信号，保证以下函数原子执行
            addjob(jobs, pid, status, cmdline);
            sigprocmask(SIG_SETMASK, &prev, NULL);
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
        }
        if(status == FG)
        {
            sigprocmask(SIG_BLOCK, &mask_all, &prev);
            addjob(jobs, pid, status, cmdline);
            sigprocmask(SIG_SETMASK, &prev, NULL);
            waitfg(pid);            
        }
    }
    return;
}
```

可以看到，前台进程与后台进程的唯一区别在于，**仅仅是父进程会等待子进程执行完毕**

### builtin_cmd

```c
/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    if(!strcmp(*argv, "quit")) 
        exit(1);
    else if(!strcmp(*argv, "jobs"))
    {
        listjobs(jobs);
        return 1;
    }
    else if(!strcmp(*argv, "bg") || !strcmp(*argv, "fg"))
    {
        do_bgfg(argv);
        return 1;
    }
    return 0;     /* not a builtin command */
}
```

### do_bgfg

```c
void do_bgfg(char **argv) 
{
    int status, id;
    struct job_t* ptr = NULL;
    if(!strcmp(*argv, "bg")) status = BG;
    else status = FG;
    if(argv[1] == NULL || ( argv[1][0] != '%' && !isdigit(argv[1][0])))
    {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }
    if(argv[1][0] == '%') 
    {
        id = atoi(argv[1] + 1);
        if((ptr = getjobjid(jobs, id)) == NULL)
        {
            printf("%%%d: No such job\n", id);
            return;
        }
    }
    else 
    {
        id = atoi(argv[1]);
        if((ptr = getjobpid(jobs, id)) == NULL)
        {
            printf("(%d): No such job\n", id);
            return;
        }
    }
    ptr->state = status;
    Kill(-ptr->pid, SIGCONT);
    if(status == FG)
        waitfg(ptr->pid);
    else    
        printf("[%d] (%d) %s", ptr->jid, ptr->pid, ptr->cmdline);
    return;
}
```

这个函数的主义功能都集中在判断是 `pid` 还是 `jid` 上，实际上发送信号的代码只有一行

发送信号时，我们是对整个进程组发信号，这样子可以唤醒一个子进程及该子进程所有的子进程

### waitfg

```c
/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    sigset_t mask;
    sigemptyset(&mask);
    while(fgpid(jobs) != 0)
        sigsuspend(&mask);
    return;
}
```

### signal handle

```c
/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    int olderror = errno;
    int status, pid;
    sigset_t mask, prev;
    sigfillset(&mask);
    if((pid = Waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        sigprocmask(SIG_BLOCK, &mask, &prev);
        if(WIFEXITED(status))
        {
            deletejob(jobs, pid);
        }
        if(WIFSIGNALED(status))
        {
            printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(status));
            deletejob(jobs, pid);
        }   
        if(WIFSTOPPED(status))
        {
            printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(pid), pid, WSTOPSIG(status));
            struct job_t* ptr = getjobpid(jobs, pid);
            ptr->state = ST;
        }
        sigprocmask(SIG_SETMASK, &prev, NULL);        
    }
    errno = olderror;
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    int olderror = errno;
    sigset_t mask, prev;
    int pid;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &prev);
    if((pid = fgpid(jobs)) > 0)
    {
        sigprocmask(SIG_SETMASK, &prev, NULL);
        Kill(-pid, SIGINT);
    }
    errno = olderror;
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    int olderror = errno;
    sigset_t mask, prev;
    int pid;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &prev);
    if((pid = fgpid(jobs)) > 0)
    {
        sigprocmask(SIG_SETMASK, &prev, NULL);
        Kill(-pid, SIGSTOP);
    }
    errno = olderror;
    return;
}
```

后两个信号处理函数的逻辑是一样的，我们主要看第一个

在 `waitpid` 中带参数 `WNOHANG | WUNTRACED`，那么 `waitpid` 会直接返回，如果等待集合中有子进程暂停或终止，那么返回子进程的 `pid`，如果没有则返回零

因此我们用该函数得到处于结束但未被回收的子进程的 `pid`，随后我们根据该子进程终止的原因打印不同的信息（利用 `status` 变量）

如果子进程正常执行完毕（通过 `return` 或者 `exit` 退出），那么宏 `WIFEXITED` 将返回 `true`

如果子进程被信号终止，那么宏 `WIFSIGNALED` 将返回 `true`，具体信号可以由 `WTERMSIG` 得到

如果子进程当前的状态是暂停的，那么宏 `WIFSTOPPED` 返回 `true`，暂停的信号由 `WSTOPSIG` 得到



### 测试

这个实验没有自动评分程序，我们只需要保证在结果上与参考程序相同即可，下面给出几个有代表性的测试程序

```bash
$ make test07
./sdriver.pl -t trace07.txt -s ./tsh -a "-p"
#
# trace07.txt - Forward SIGINT only to foreground job.
#
tsh> ./myspin 4 &
[1] (17299) ./myspin 4 &
tsh> ./myspin 5
Job [2] (17301) terminated by signal 2
tsh> jobs
[1] (17299) Running ./myspin 4 &


$ make rtest07
./sdriver.pl -t trace07.txt -s ./tshref -a "-p"
#
# trace07.txt - Forward SIGINT only to foreground job.
#
tsh> ./myspin 4 &
[1] (17337) ./myspin 4 &
tsh> ./myspin 5
Job [2] (17339) terminated by signal 2
tsh> jobs
[1] (17337) Running ./myspin 4 &

----------------------------------------------------------

$ make test08
./sdriver.pl -t trace08.txt -s ./tsh -a "-p"
#
# trace08.txt - Forward SIGTSTP only to foreground job.
#
tsh> ./myspin 4 &
[1] (17498) ./myspin 4 &
tsh> ./myspin 5
Job [2] (17500) stopped by signal 19
tsh> jobs
[1] (17498) Running ./myspin 4 &
[2] (17500) Stopped ./myspin 5 


$ make rtest08
./sdriver.pl -t trace08.txt -s ./tshref -a "-p"
#
# trace08.txt - Forward SIGTSTP only to foreground job.
#
tsh> ./myspin 4 &
[1] (17471) ./myspin 4 &
tsh> ./myspin 5
Job [2] (17473) stopped by signal 20
tsh> jobs
[1] (17471) Running ./myspin 4 &
[2] (17473) Stopped ./myspin 5 

----------------------------------------------------------

$ make test14
./sdriver.pl -t trace14.txt -s ./tsh -a "-p"
#
# trace14.txt - Simple error handling
#
tsh> ./bogus
./bogus: command not found
tsh> ./myspin 4 &
[1] (17766) ./myspin 4 &
tsh> fg
fg command requires PID or %jobid argument
tsh> bg
bg command requires PID or %jobid argument
tsh> fg a
fg command requires PID or %jobid argument
tsh> bg a
bg command requires PID or %jobid argument
tsh> fg 9999999
(9999999): No such job
tsh> bg 9999999
(9999999): No such job
tsh> fg %2
%2: No such job
tsh> fg %1
Job [1] (17766) stopped by signal 19
tsh> bg %2
%2: No such job
tsh> bg %1
[1] (17766) ./myspin 4 &
tsh> jobs
[1] (17766) Running ./myspin 4 &


$ make rtest14
./sdriver.pl -t trace14.txt -s ./tshref -a "-p"
#
# trace14.txt - Simple error handling
#
tsh> ./bogus
./bogus: Command not found
tsh> ./myspin 4 &
[1] (17712) ./myspin 4 &
tsh> fg
fg command requires PID or %jobid argument
tsh> bg
bg command requires PID or %jobid argument
tsh> fg a
fg: argument must be a PID or %jobid
tsh> bg a
bg: argument must be a PID or %jobid
tsh> fg 9999999
(9999999): No such process
tsh> bg 9999999
(9999999): No such process
tsh> fg %2
%2: No such job
tsh> fg %1
Job [1] (17712) stopped by signal 20
tsh> bg %2
%2: No such job
tsh> bg %1
[1] (17712) ./myspin 4 &
tsh> jobs
[1] (17712) Running ./myspin 4 &

```

## Bug 遗留

我的这个 `tsh` 虽然通过测试程序没有说明问题，但有 `bug`

如果运行 `./myspin 5 &`，那么 `5` 秒后运行 `jobs` 依旧会打印对应的任务，表面该进程并没有被回收

但执行 `fg %jid` 后该进程则立刻被回收