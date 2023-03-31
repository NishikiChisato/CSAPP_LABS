/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return ~(~(x & ~y) & ~(~x & y));
}


/**
 * bitXor思路：
 * 首先 a ^ b = (a & ~b) | (~a & b)，考虑如何用 ~, & 表示 |
 * 下面开始讨论如何表示a|b
 * |的二进制表示为
 * 0 0 0
 * 0 1 1
 * 1 0 1
 * 1 1 1
 * 
 * 注意到，如果可以表示出
 * 0 0 1
 * 0 1 0
 * 1 0 0
 * 1 1 0
 * 那么|也就可以表示了（取反即可）
 * 这个位模式跟&非常像，只是对变量先取反再按位与即可，因此a|b可以表示为
 * ~(~a & ~b)
 * 用 (a &~b) 替换 a，(~a & b) 替换 b 即可
 * 
 */


/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  return !(~(x + 1) ^ x) & !!(x + 1);
}

/**
 * isTmax思路：
 * 以4位举例，TMax为 0 1 1 1
 * 这个数加一为 1 0 0 0
 * 我们对 1 0 0 0 每一位均取反便可得到原数，因此只需要判断~(x + 1) == x是否成立即可
 * 由于不能用等号，因此用异或代替，即!(~(x + 1) ^ x)
 * 有一个例外情况是 -1，位表示为 1 1 1 1
 * 显然，-1也是满足上述条件的，因此需要将x + 1 = 0这种情况排除掉
 * 我们让所有x + 1 = 0的数变为0，让x + 1 != 0的数变为1，然后跟刚才的结果按位与即可
 * 
 */

/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int op = 0xAA;
  op |= (op << 8);
  op |= (op << 16);
  return !((x & op) ^ op); 
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int ahead = !((x >> 4) ^ 0x03);
  int lower4Bits = x & 0xF;
  int carry = lower4Bits + 0x06;
  return ahead & !(carry >> 4);
}

/**
 * 这里重点在于对后4位的判断
 * 后四位的范围要在0000到1001之间才符合题意
 * 注意到，1001加上0110就可以得到4位的最大值
 * 因此如果后四位大于1001，那么加上0110一定会造成进位，我们将进位位取出即可判断后四位是否在范围内
 * 
 */



/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */

//x is 0, res is false; x is not 0, res is true
int conditional(int x, int y, int z) {
  int isZero = ~!x + 1;//x == 0 => [1...1], x == 1 => [0...0]
  return (~isZero & y) + (isZero & z);

}


/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int isEqual = !(x ^ y);
  int sub = y - x;
  int isBiggerZero = !(sub >> 31) & !!sub;
  return isEqual | isBiggerZero;
}



//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  return ((x | (~x + 1)) >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  int sign = x >> 31;//positive will set to 0, and negative is 11...1
  /*
  If x <  0, its bit representation is 11...10...
  If x >= 0, its bit representation is 00...01...
  So, for negative num, we should find the first 0 on the far left and the res must plus 1
  And for positive num, we shouble find the first 1 on the far left, and the res also plus 1
  We consider turn negative num form into positive num form
  */
  x = x ^ sign;
  //The follow code attempt to find the fitst 1 on far left， this process is like Binary search
  int bit16, bit8, bit4, bit2, bit1, bit;
  bit16 = !!(x >> 16) << 4;//at least 16 bits
  x >>= bit16;
  bit8 = !!(x >> 8) << 3;//at least 8 bits
  x >>= bit8;
  bit4 = !!(x >> 4) << 2;//at least 4 bits
  x >>= bit4;
  bit2 = !!(x >> 2) << 1;//at least 2 bits
  x >>= bit2;
  bit1 = !!(x >> 1);//at least 1 bits
  x >>= bit1;
  bit = x;
  return bit16 + bit8 + bit4 + bit2 + bit1 + bit + 1;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  unsigned ans = uf;
  //normalizal
  if((uf & 0x7f800000) != 0 && (uf & 0x7f800000) != 0x7f800000) ans += 0x00800000;
  //denormalizal, move directly one bit to the left, notes the sign bits
  //can not move directly to the left
  else if((uf & 0x7f800000) == 0) ans = ((ans & 0x007fffff) << 1) | (ans & 0x80000000);
  return ans;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  if((uf & 0x7fffffff) == 0) return 0;
  //go to get sign bits
  int sign = uf >> 31;
  int E, M;
  
  E = ((uf & 0x7f800000) >> 23) - 127, M = (uf & 0x007fffff) | 0x00800000;//get 1 + M (notes the position of 1)

  if(E >= 31) return 0x80000000;
  else if(E < 0) return 0;

  //the number of the digit of the E is 23, so if E >= 32, M shoule muliple 2^(E - 23), M itself at least has 23 bits
  if(E > 23) M <<= (E - 23);
  else M >>= (23 - E);

  //if the sign of uf is equal to the sign of M, return M, else return -M
  if(sign ^ (M >> 31)) return ~M + 1;
  else return M;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {

  return 2;
    

}
