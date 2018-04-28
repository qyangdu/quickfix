/****************************************************************************
** Copyright (c) quickfixengine.org  All rights reserved.
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifdef _MSC_VER
#include "stdafx.h"
#else
#include "config.h"
#endif

#include "FieldConvertors.h"

namespace FIX {

ALIGN_DECL_DEFAULT const double DoubleConvertor::m_mul1[8] = { 1E1, 1E2, 1E3, 1E4, 1E5, 1E6, 1E7, 1E8 };
ALIGN_DECL_DEFAULT const double DoubleConvertor::m_mul8[8] = { 1E8, 1E16, 1E24, 1E32, 1E40, 1E48, 1E56, 1E64 };

#define NUMOF(a) (sizeof(a) / sizeof(a[0]))

int HEAVYUSE DoubleConvertor::Proxy::generate(char* buf) const
{
 /* 
  * Fixed format encoding only. Based in part on modp_dtoa():
  *
  * Copyright &copy; 2007, Nick Galbreath -- nickg [at] modp [dot] com
  * All rights reserved.
  * http://code.google.com/p/stringencoders/
  * Released under the bsd license.
  */

  static const double threshold = 1000000000000000LL; // 10^(MaxPrecision)
  ALIGN_DECL_DEFAULT static const double pwr10[] =
  {
    1, 10, 100, 1000, 10000, 100000, 1000000,
    10000000, 100000000, 1000000000, 10000000000LL,
    100000000000LL, 1000000000000LL, 10000000000000LL,
    100000000000000LL, 1000000000000000LL, 10000000000000000LL
  };

  PREFETCH((const char*)pwr10, 0, 0);

  char* wstr = buf;
  double value = m_value;

  /* Hacky test for NaN
   * under -fast-math this won't work, but then you also won't
   * have correct nan values anyways.  The alternative is
   * to link with libmath (bad) or hack IEEE double bits (bad)
   */
  if ( (value == value))
  {
    int64_t whole;
    uint64_t frac;
    double tmp, diff = 0.0;
    std::size_t count, not_negative = 1;
    std::size_t precision = m_padded;

    /* we'll work in positive values and deal with the
       negative sign issue later */
    if (value < 0) {
      not_negative = 0;
      value = -value;
    }

    if (value < threshold) {

      /* may have fractional part */
      whole = (int64_t) value;
      count = Util::ULong::numDigits(whole) - (whole == 0);

      if (precision) {
        if (!m_round && value != 0)
        {
          frac = static_cast<uint64_t>((value - whole) * pwr10[MaxPrecision + 1 - count]);
          if (frac) {
            int64_t rem = frac % 10;
            precision =
              Util::ULong::numFractionDigits( frac + (rem >= 5) * 10 - rem )
              - count;
          }
        }
      } else {
        precision = MaxPrecision - count;
      }

      tmp = (value - whole) * pwr10[precision];
      frac = (uint64_t)(tmp);
      diff = tmp - frac;

      if (diff > 0.5) {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= pwr10[precision]) {
            frac = 0;
            ++whole;
        }
      } else if (diff == 0.5 && ((frac == 0) || (frac & 1))) {
        /* if halfway, round up if odd, OR
           if last digit is 0.  That last part is strange */
        ++frac;
      }
      if (precision == 0) {
        diff = value - whole;
        if (diff > 0.5) {
          /* greater than 0.5, round up, e.g. 1.6 -> 2 */
          ++whole;
        } else if (diff == 0.5 && (whole & 1)) {
          /* exactly 0.5 and ODD, then round up */
          /* 1.5 -> 2, but 2.5 -> 2 */
          ++whole;
        }

        if (m_padded) {
          precision = m_padded;
          do {
            *wstr++ = '0';
          } while (--precision);
          *wstr++ = '.';
        }
      } else if (frac || m_padded) {
        count = precision;
        // now do fractional part, as an unsigned number
        // we know it is not 0 but we can have trailing zeros, these
        if (!m_padded) {
          // should be removed
          while (!(frac % 10)) {
            --count;
            frac /= 10;
          }
        } else {
          // or added
          for (precision = m_padded; precision > count; precision--)
            *wstr++ = '0';
        }

        // now do fractional part, as an unsigned number
        do {
          --count;
          *wstr++ = (char)(48 + (frac % 10));
        } while (frac /= 10);
        // add extra 0s
        while (count-- > 0) *wstr++ = '0';
        // add decimal
        *wstr++ = '.';
      }
    }
    else // above threshold for fractional processing
    { 
      const double ceiling = threshold * 10;
      if (value > ceiling) {
        int level = 0;
        count = NUMOF(pwr10) - 1;
        tmp = value;
        do
        {
          diff = tmp;
          tmp = diff / pwr10[count];
          level++;
        }
        while (tmp > ceiling);
  
        count = 0;
        do
        {
          tmp = diff / pwr10[++count];
        }
        while (tmp > ceiling);
  
        count += (level - 1) * (NUMOF(pwr10) - 1);
        whole = (int64_t)tmp;
      } else {
        count = 0;
        whole = (int64_t)value;
      }

      if (precision) {
        do {
          *wstr++ = '0';
        } while (--precision);
        *wstr++ = '.';
      }

      while (count-- > 0) *wstr++ = '0';
    }
    // do whole part
    // Take care of sign
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);
    *wstr = '-';
    wstr += (1 - not_negative);
  }
  else
  {
    *wstr++ = 'n';
    *wstr++ = 'a';
    *wstr++ = 'n';
  }
  return wstr - buf;
}

}

