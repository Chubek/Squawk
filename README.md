# Squawk: An AWK Language Interpreter

It's still under development, but it builds --- there's no parsing though yet. You can build `squawk.c` with:

```
gcc squawk.c -lunistring -lgc -lpcre2-posix
```

You will need these 3 dependences:

1- PCRE2

2- Boehm GC

3- GNU Libunistring

If you wish to generate `squawk.gperf` which is the LUT for builtin functions, you need gperf.

Pretty soon VMGEN will be added to the mix.

**UPDATE**: Most VMGEN functions written, note that GPerf and VMGEN specs have not been thoroughly inspected and debugged! But as far as I am concerned, `squawk.c` should build? Depends on your machine.

Plan on using Re2C for lexing and Yacc for parsing.

Please lemme know about any ideas that you have for Squawk:

```
chubakbidpaa@gmail.com
chubakbidpaa@riseup.net
```

Thanks!
