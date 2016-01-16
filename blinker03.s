//-------------------------------------------------------------------------
// VideoCore IV implementation of dwelch67's blinker01 sample.
// See: https://github.com/dwelch67/raspberrypi/tree/master/blinker01)
//-------------------------------------------------------------------------
  

  START("blinker03.bin");

  equ(GPFSEL2, 0x7E200008);
  equ(GPSET0, 0x7e20001c);
  equ(GPCLR0, 0x7e200028);

  push_r6_rn(r9);

  movi(r1, GPFSEL2);
  ld(r0, r1);
  // out for Port 20 and 21
  andi(r0, ~(7<<0) | ~(7<<3) );
  ori(r0, (1<<0) | (1<< 3));
  st(r0, r1);

  movi(r0, GPSET0);
  movi(r1, GPCLR0);
  movi(r2, (1<<20) | (0<<21) ); // clock change, data no change
  movi(r3, (1<<20) | (1<<21) ); // clock change, data,no change
  movi(r4, (0<<20) | (1<<21) ); // change only data
  movi(r5, 32);
  movi(r6, 0xFF0055AA);
#define MAGIC(x) \
  declare(one##x);\
  declare(done##x);\
  btsti(r6, x);  \
  bne(one##x); \
  st(r2, r0); \
  st(r4, r1);  \
  st(r2,r1);\
  b(done##x);  \
  label(one##x); \
  st(r3,r0); \
  st(r2,r1); \
  st(r2,r1); \
  label(done##x); \
  

    MAGIC(0)
    MAGIC(1)
    MAGIC(2)
    MAGIC(3)
    MAGIC(4)
    MAGIC(5)
    MAGIC(6)
    MAGIC(7)
    MAGIC(8)
    MAGIC(9)
    MAGIC(10)
    MAGIC(11)
    MAGIC(12)
    MAGIC(13)
    MAGIC(14)
    MAGIC(15)
    MAGIC(16)
    MAGIC(17)
    MAGIC(18)
    MAGIC(19)
    MAGIC(20)
    MAGIC(21)
    MAGIC(22)
    MAGIC(23)
    MAGIC(24)
    MAGIC(25)
    MAGIC(26)
    MAGIC(27)
    MAGIC(28)
    MAGIC(29)
    MAGIC(30)
    MAGIC(31)

  st(r3,r0); \
  pop_r6_rn(r9);
  rts();

  END
