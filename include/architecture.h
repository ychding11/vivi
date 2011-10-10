/*
 * History
 *
 * 2000-07-05: George France <france@crl.dec.com>
 *    - Architecture definitions
 *
 * 2001-10-24: Janghoon Lyu <nandy@mizi.com>
 *    - Adds Exiliens, S3C2400
 *
 * 2002-01-26: Janghoon Lyu <nandy@mizi.com>
 *    - Modify a little bit
 *
 * 2002-05-22: Janghoon Lyu <nandy@mizi.com>
 *    - Add S3C2410
 */

/* There can be 255 platforms       ( 8 bit)
   Each platform can have 255 cpus  ( 8 bit)
   Each cpu can have 65355 machines (16 bit) */

#define ARM_PLATFORM   1
   #define ARM_SA110_CPU   1
   #define ARM_SA1100_CPU  2
   #define ARM_SA1110_CPU  3
   #define ARM_SA1111_CPU  4
   #define ARM_S3C2400_CPU 5
   #define ARM_S3C2410_CPU 6
#define ARM_PXA250_CPU	6
