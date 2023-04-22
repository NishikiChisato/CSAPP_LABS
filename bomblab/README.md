## Some Tricks record

- [Some Tricks record](#some-tricks-record)
  - [The difference of (%rax) and %rax](#the-difference-of-rax-and-rax)
  - [The difference of lea and normal operators](#the-difference-of-lea-and-normal-operators)


### The difference of (%rax) and %rax

if the value of `%rax` is `0x100`, and the value at the `0x100` address is `0xFF`

Therefore use `%rax` will get `0x100` and use `(%rax)` will get `0xFF`, which means that the latter will **access the memory**

The expression $Imm(r_b,r_i,s)$ just to calculate a value, **regardless of whether memory is accessed** 

### The difference of lea and normal operators

`leq` means `load effect address`. But its role is calculate a value and put in a register, **it doesn't access memory**

for example, `lea 0x4(%rsp), %rbx` just set `%rbx` into `%rsp + 0x4`(if the value of `%rsp` is `0x102`, `%rbx` is `0x106`)

Since the value in the registers are **address**, they are **generally not a specific value**, therefore `load effect address` **can be ignored**

