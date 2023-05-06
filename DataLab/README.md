## Some Tricks record

- [Some Tricks record](#some-tricks-record)
  - [When i want to judge two number whether is it equal ?](#when-i-want-to-judge-two-number-whether-is-it-equal-)
  - [When i want to change not-zero number into Zero or zero number into One ?](#when-i-want-to-change-not-zero-number-into-zero-or-zero-number-into-one-)
  - [Try to add some operators between x and -x](#try-to-add-some-operators-between-x-and--x)
  - [Pay attention to the left shift](#pay-attention-to-the-left-shift)


### When i want to judge two number whether is it equal ?

> Use **XOR**
> Judging `x` is equal to `y`, which is equivalent to judge `!(x ^ y)`

### When i want to change not-zero number into Zero or zero number into One ?

> Use LogicalNeg `!`
> if `x` is **not-zero**, `!x` will transfrom it into **Zero**, Frutherly, `!!x` will get **One**
> if `x` is **zero**, `!x` will transfrom it into **One**, `!!x` will get **Zero**

### Try to add some operators between x and -x

> `-x` is equivalent to `~x + 1`. 
> From a bit-level perspective, `~x + 1` will find **the first 1 on far right**, and **negate the left bit-by-bit**
> like `x & (~x + 1)`, `x | (~x + 1)`, `x ^ (~x + 1)`

### Pay attention to the left shift

> For signed number, like `int x`
> If x is negative num, `x >> 31` will get the vector $[111\cdots 1]$
> If x is non-negateve num, `x >> 31` will get the vector $[000\cdots 0]$
> So, when i go to get the **sign bits**, the following operation is correct
> `sign = x & 0x80000000`
