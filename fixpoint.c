#include "fixpoint.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////
// Helper functions
// Note that you can make these "visible" (not static)
// if you want to be able to write unit tests for them
////////////////////////////////////////////////////////////////////////

// fixpoint add and sub  helper functions
// step 1 of add
uint32_t addition_Frac(uint32_t left, uint32_t right, uint32_t *carry) {
  uint32_t sum = left + right;
  *carry = (sum < left) ? 1 : 0; // step 2 of add
  return sum;
}

uint32_t addition_Whole(uint32_t left, uint32_t right, uint32_t *carry) {
  return left + right + *carry; // step 3 of add
}

bool positive_Overflow(fixpoint_t *result, uint32_t whole, uint32_t leftWhole) {
  // Overflow threshold for 32 bit integer
  const uint32_t threshold = 2147483647; // 2^(w-1) -1

  if (whole > threshold || whole < leftWhole) {
    result->negative = true; // it becomes negative (overflow)
    return true;             // sum exceeds threshold
  } else {
    result->negative = false;
    return false;
  }
}
bool negative_Overflow(fixpoint_t *result, uint32_t whole, uint32_t leftWhole) {
  const uint32_t threshold = 2147483647;

  if (whole == 0) { // -min - max
    result->negative = false;
    return true;
  }
  if (whole > threshold || whole < leftWhole) {
    result->negative = false; // becomes positive (negatve overflow)
    return true;
  }
  result->negative = true;
  return false;
}
void magnitude(fixpoint_t *result, const fixpoint_t *left,
               const fixpoint_t *right) {
  // detemrine if result is negative with diff operaters
  if (left->whole > right->whole || // greater operand determines if negative
      (left->whole == right->whole && left->frac >= right->frac)) {
    result->negative = left->negative;
  } else {
    result->negative = right->negative;
  }
}
// fixpoint_mul helper
uint64_t multiplyHighLow(const fixpoint_t *left, const fixpoint_t *right) {
  // high and low 32-bit parts factors
  uint64_t highLeftPart = (uint64_t)left->whole;
  uint64_t lowLeftPart = (uint64_t)left->frac;
  uint64_t highRightPart = (uint64_t)right->whole;
  uint64_t lowRightPart = (uint64_t)right->frac;

  uint64_t PRS =
      (lowLeftPart * lowRightPart) >> 32; // fraction product down to 32 bits
  uint64_t TUV = highLeftPart * lowRightPart +
                 lowLeftPart * highRightPart;           // whole and fractional
  uint64_t productWhole = highLeftPart * highRightPart; // whole product
  return (productWhole << 32) + TUV + PRS;
}
// fixpoint_format_hex Helper
static int formatFracHex(char *buffer, size_t size, uint32_t frac) {
  int fracLength = 0; // num of chars

  if (frac == 0) { // if fraction is zero: defualt sequence (.0)
    buffer[fracLength++] = '.';
    buffer[fracLength++] = '0';
    buffer[fracLength] = '\0';
  } else {              // non-zero
    char fracBuffer[9]; // 8 hex digits + null
    snprintf(fracBuffer, sizeof(fracBuffer), "%08x", frac);

    // Trim trailing zeros
    int trim = 8;
    while (trim > 1 && fracBuffer[trim - 1] == '0')
      trim--;

    buffer[fracLength++] = '.'; // append decimal
    for (int i = 0; i < trim; i++) {
      buffer[fracLength++] = fracBuffer[i]; // append frac digits
    }
    buffer[fracLength] = '\0'; // null terminator
  }

  return fracLength; // number of characters written
}
// parse hex Helper
static bool parseFracHex(uint32_t *frac, const char *s, int *matchedFrac) {
  if (sscanf(s, "%8x%n", frac, matchedFrac) != 1) // read up to 8 hex digits
    return false;
 
  while (*matchedFrac < 8) { 
    *frac <<= 4; // shift left by one hex digit
    (*matchedFrac)++;
  }
  return true;
}
////////////////////////////////////////////////////////////////////////
// Public API functions
////////////////////////////////////////////////////////////////////////

void fixpoint_init(fixpoint_t *val, uint32_t whole, uint32_t frac,
                   bool negative) {
  // initialize each field of val with correct field vallues

  val->whole = whole;
  val->frac = frac;
  val->negative = negative;

  // if whole and frac are zero it is not negative
  if (whole == 0 && frac == 0) {
    val->negative = false;
  } else {
    val->negative = negative;
  }
}

uint32_t fixpoint_get_whole(const fixpoint_t *val) {
  // returns the field value of val after initialization
  return (*val).whole;
}

uint32_t fixpoint_get_frac(const fixpoint_t *val) {
  // returns the frac value of val after initialization
  return (*val).frac;
}

bool fixpoint_is_negative(const fixpoint_t *val) {
  // returns neg value of val after initalizaiton, where we already checked for
  // 0 in the whole and 0 in the fraction
  return (*val).negative;
}

void fixpoint_negate(fixpoint_t *val) {
  // if whole and frac are 0 it is not negative
  if (val->whole == 0 && val->frac == 0) {
    val->negative = false;
  }

  // switch the negative sign
  else {
    val->negative = !val->negative;
  }
}

// stop here for milestone 1

static inline bool is_zero_mag(const fixpoint_t *x) {
  return x->whole == 0 && x->frac == 0;
}

result_t fixpoint_add(fixpoint_t *result, const fixpoint_t *left,
                      const fixpoint_t *right) {
  // Normalize input signs for logic, treating −0  as +0 numerically
  bool lneg = left->negative  && !is_zero_mag(left);
  bool rneg = right->negative && !is_zero_mag(right);

  if (lneg == rneg) {
    // Same signm add magnitudes 
    uint64_t frac_sum  = (uint64_t)left->frac + (uint64_t)right->frac;
    uint64_t carry     = frac_sum >> 32; // 1 if fractional overflow
    uint64_t whole_sum = (uint64_t)left->whole + (uint64_t)right->whole + carry;

    result->frac     = (uint32_t)(frac_sum  & 0xFFFFFFFFu);
    result->whole    = (uint32_t)(whole_sum & 0xFFFFFFFFu);
    // both signs same
    result->negative = lneg; 

    result_t flags = RESULT_OK;
    if (whole_sum >> 32) flags |= RESULT_OVERFLOW; // true 32-bit overflow

    // Zero should not be negative, except for the negative overflow case
    if (result->whole == 0 && result->frac == 0) {
      if (!(lneg && rneg && (flags & RESULT_OVERFLOW))) {
        // +0 in all other cases
        result->negative = false; 
      }
    }
    return flags;
  } else {
    // Opposite, signs subtract smaller magnitude from larger 
    // Compare magnitudes ignoring sign
    int cmp;
    if (left->whole != right->whole) {
      cmp = (left->whole < right->whole) ? -1 : 1;
    } else if (left->frac != right->frac) {
      cmp = (left->frac  < right->frac ) ? -1 : 1;
    } else {
      cmp = 0;
    }

    if (cmp == 0) {
      // x + (−x) => +0
      result->whole = 0;
      result->frac  = 0;
      result->negative = false;
      return RESULT_OK;
    }

    // Identify larger and smaller magnitudes
    const fixpoint_t *larger  = (cmp > 0) ? left  : right;
    const fixpoint_t *smaller = (cmp > 0) ? right : left;

    uint64_t Lw = (uint64_t)larger->whole,  Lf = (uint64_t)larger->frac;
    uint64_t Sw = (uint64_t)smaller->whole, Sf = (uint64_t)smaller->frac;

    uint32_t out_frac, out_whole;
    if (Lf >= Sf) {
      // no borrow from whole
      out_frac  = (uint32_t)(Lf - Sf);
      out_whole = (uint32_t)(Lw - Sw);
    } else {
      // borrow 1 whole (adds 2^32 to fractional part)
      out_frac  = (uint32_t)((Lf + (1ULL << 32)) - Sf);
      out_whole = (uint32_t)((Lw - 1) - Sw);
    }

    result->whole    = out_whole;
    result->frac     = out_frac;
    result->negative = larger->negative && !is_zero_mag(larger); // sign of larger magnitude

    // Normalizes if exact zero,  its never negative here
    if (result->whole == 0 && result->frac == 0) result->negative = false;

    return RESULT_OK; 
    // no overflow when signs differ
  }
}


result_t fixpoint_sub(fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right) {
  // invert second operand
  fixpoint_t negRight = *right;                 // copy
  fixpoint_negate(&negRight);                   // negate right
  return fixpoint_add(result, left, &negRight); // call add lft + -right
}

result_t fixpoint_mul(fixpoint_t *result, const fixpoint_t *left,
                      const fixpoint_t *right) {
  uint64_t product = multiplyHighLow(left, right); // 64 bit integr product

  result->whole = (uint32_t)(product >> 32);
  result->frac = (uint32_t)product;
  result->negative = left->negative ^ right->negative; // xor

  if (result->whole == 0 && result->frac == 0) { // if 0 case
    result->negative = false;
  }
  return RESULT_OK;
}

int fixpoint_compare(const fixpoint_t *left, const fixpoint_t *right) {

  if (left->whole != right->whole) { // not equal then comapre
    return (left->whole < right->whole) ? -1 : 1;
  } // if whole is equal check frac
  return (left->frac < right->frac)   ? -1
         : (left->frac > right->frac) ? 1
                                      : 0; // 0 when both are false
}

void fixpoint_format_hex(fixpoint_str_t *s, const fixpoint_t *val) {
  char buffer[FIXPOINT_STR_MAX_SIZE]; // buffer
  int hexString = 0;

  if (val->negative)
    buffer[hexString++] = '-';
  // write whole part in hex
  hexString += snprintf(buffer + hexString, sizeof(buffer) - hexString, "%x",
                        val->whole);
  // write fractional partin hex
  hexString +=
      formatFracHex(buffer + hexString, sizeof(buffer) - hexString, val->frac);
  // copy full hexidecimal into stuct
  snprintf(s->str, FIXPOINT_STR_MAX_SIZE, "%s", buffer);
}

bool fixpoint_parse_hex(fixpoint_t *val, const fixpoint_str_t *s) {
  int matchedWhole = 0; // hex digits matched
  int matchedFrac = 0;
  int skip = 0;

  if (s->str[0] == '-') { //check if negative
    val->negative = true;
    skip = 1; // skip for parsing if negative
  } else {
    val->negative = false;
  }

  // parse whole part
  if (sscanf(s->str + skip, "%8x%n", &val->whole, &matchedWhole) != 1)
    return false;
  skip += matchedWhole; // move skip past whole part

  // Parse fractional
  if (s->str[skip] != '.') // decimal for fraction part
    return false;
  skip++; // skip dot
  if (!parseFracHex(&val->frac, s->str + skip, &matchedFrac))
    return false;

  if (val->whole == 0 && val->frac == 0) // edge case
    val->negative = false;

  return true;
}
