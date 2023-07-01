#include <stdio.h>

typedef word_t word_t;

word_t benchmark(word_t *src, word_t *dst, word_t len)
{
    word_t count = 0;
    word_t val;

    while (len > 0) {
	val = *src++;
	*dst++ = val;
	if (val > 0)
	    count++;
	len--;
    }
    return count;
}

word_t test1(word_t *src, word_t *dst, word_t len)
{
    word_t count = 0, limit = len - 1;
    word_t val1, val2;
    word_t even = 0, odd = 0;
    word_t i = 0;
    for(i = 0; i < limit; i += 2)
    {
        val1 = src[i];
        val2 = src[i + 1];
        dst[i] = val1;
        dst[i + 1] = val2;
        even = val1 > 0 ? even + 1 : even;
        odd = val2 > 0 ? odd + 1 : odd;
    }
    for(; i < len; i ++)
    {
        val1 = src[i];
        dst[i] = val1;
        count = val1 > 0 ? count + 1 : count;
    }
    count += (even + odd);
    return count;
}