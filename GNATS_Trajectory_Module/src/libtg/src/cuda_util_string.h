/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.
Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Drs. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.
---------------------------------------------------------------------------------------------------------------------
*/

#ifndef __CUDA_UTIL_STRING_H__
#define __CUDA_UTIL_STRING_H__

#include "cuda_compat.h"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strcasecmp.c        5.5 (Berkeley) 11/24/87";
#endif /* LIBC_SCCS and not lint */

#include "ansidecl.h"
#include <stddef.h>

#ifndef ABORT_INSTRUCTION
/* No such instruction is available.  */
# define ABORT_INSTRUCTION
#endif

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
__device__ const unsigned char charmap[] = {
        '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
        '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
        '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
        '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
        '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
        '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
        '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
        '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
        '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
        '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
        '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
        '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
        '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
        '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
        '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
        '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
        '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
        '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
        '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
        '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

//void
//abort (void)
//{
//  struct sigaction act;
//  sigset_t sigs;
//  /* First acquire the lock.  */
//  __libc_lock_lock_recursive (lock);
//  /* Now it's for sure we are alone.  But recursive calls are possible.  */
//  /* Unblock SIGABRT.  */
//  if (stage == 0)
//    {
//      ++stage;
//      __sigemptyset (&sigs);
//      __sigaddset (&sigs, SIGABRT);
//      __sigprocmask (SIG_UNBLOCK, &sigs, 0);
//    }
//  /* Send signal which possibly calls a user handler.  */
//  if (stage == 1)
//    {
//      /* This stage is special: we must allow repeated calls of
//         `abort' when a user defined handler for SIGABRT is installed.
//         This is risky since the `raise' implementation might also
//         fail but I don't see another possibility.  */
//      int save_stage = stage;
//      stage = 0;
//      __libc_lock_unlock_recursive (lock);
//      raise (SIGABRT);
//      __libc_lock_lock_recursive (lock);
//      stage = save_stage + 1;
//    }
//  /* There was a handler installed.  Now remove it.  */
//  if (stage == 2)
//    {
//      ++stage;
//      memset (&act, '\0', sizeof (struct sigaction));
//      act.sa_handler = SIG_DFL;
//      __sigfillset (&act.sa_mask);
//      act.sa_flags = 0;
//      __sigaction (SIGABRT, &act, NULL);
//    }
//  /* Try again.  */
//  if (stage == 3)
//    {
//      ++stage;
//      raise (SIGABRT);
//    }
//  /* Now try to abort using the system specific command.  */
//  if (stage == 4)
//    {
//      ++stage;
//      ABORT_INSTRUCTION;
//    }
//  /* If we can't signal ourselves and the abort instruction failed, exit.  */
//  if (stage == 5)
//    {
//      ++stage;
//      _exit (127);
//    }
//  /* If even this fails try to use the provided instruction to crash
//     or otherwise make sure we never return.  */
//  while (1)
//    /* Try for ever and ever.  */
//    ABORT_INSTRUCTION;
//}

__device__ int cuda_util_strcmp(const char *str_a, const char *str_b, unsigned len = 256) {
  int match = 0;
  unsigned i = 0;
  unsigned done = 0;

  while ((i < len) && (match == 0) && !done) {
      if ((str_a[i] == 0) || (str_b[i] == 0))
    	  done = 1;
      else if (str_a[i] != str_b[i]) {
        match = i+1;

        if (((int)str_a[i] - (int)str_b[i]) < 0)
			match = 0 - (i + 1);
      }

      i++;
  }

  return match;
}

__device__ size_t cuda_util_strlen (const char *str)
{
  const char *char_ptr;
  const unsigned long int *longword_ptr;
  unsigned long int longword, himagic, lomagic;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = str;
		  ((unsigned long int) char_ptr & (sizeof (longword) - 1)) != 0;
       	    ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - str;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *) char_ptr;

  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:
     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD
     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof (longword) > 4)
    {
      /* 64-bit version of the magic.  */
      /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
      himagic = ((himagic << 16) << 16) | himagic;
      lomagic = ((lomagic << 16) << 16) | lomagic;
    }

  if (sizeof (longword) > 8)
      //abort ();
	  return -1;

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  for (;;)
    {
      longword = *longword_ptr++;

      if (((longword - lomagic) & ~longword & himagic) != 0)
	{
	  /* Which of the bytes was the zero?  If none of them were, it was
	     a misfire; continue the search.  */

	  const char *cp = (const char *) (longword_ptr - 1);

	  if (cp[0] == 0)
	    return cp - str;
	  if (cp[1] == 0)
	    return cp - str + 1;
	  if (cp[2] == 0)
	    return cp - str + 2;
	  if (cp[3] == 0)
	    return cp - str + 3;
	  if (sizeof (longword) > 4)
	    {
	      if (cp[4] == 0)
		return cp - str + 4;
	      if (cp[5] == 0)
		return cp - str + 5;
	      if (cp[6] == 0)
		return cp - str + 6;
	      if (cp[7] == 0)
		return cp - str + 7;
	    }
	}
    }
}

__device__ char* cuda_util_strchr (register const char *s, int c)
{
  do {
    if (*s == c)
      {
	return (char*)s;
      }
  } while (*s++);

  return (0);
}

__device__ int cuda_util_strncmp(const char *s1, const char *s2, register size_t n)
{
  register unsigned char u1, u2;

  while (n-- > 0)
    {
      u1 = (unsigned char) *s1++;
      u2 = (unsigned char) *s2++;
      if (u1 != u2)
	return u1 - u2;
      if (u1 == '\0')
	return 0;
    }
  return 0;
}

__device__ char* cuda_util_strstr (const char *s1, const char *s2)
{
  const char *p = s1;
  const size_t len = cuda_util_strlen (s2);

  for (; (p = cuda_util_strchr (p, *s2)) != 0; p++)
    {
      if (cuda_util_strncmp (p, s2, len) == 0)
	return (char *)p;
    }
  return (0);
}

__device__ int cuda_util_strncasecmp(const char *s1, const char *s2, register size_t n)
{
    register unsigned char u1, u2;
    for (; n != 0; --n) {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (charmap[u1] != charmap[u2]) {
            return charmap[u1] - charmap[u2];
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}

__device__ int cuda_util_indexOf_shift (char* base, char* str, int startIndex) {
    int result;
    unsigned int baselen = cuda_util_strlen(base);
    // str should not longer than base
    if (cuda_util_strlen(str) > baselen || startIndex > baselen) {
        result = -1;
    } else {
        if (startIndex < 0 ) {
            startIndex = 0;
        }
        char* pos = cuda_util_strstr(base+startIndex, str);
        if (pos == NULL) {
            result = -1;
        } else {
            result = pos - base;
        }
    }
    return result;
}

__device__ int cuda_util_indexOf (char* base, char* str) {
    return cuda_util_indexOf_shift(base, str, 0);
}

#endif
