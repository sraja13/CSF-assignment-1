#include "fixpoint.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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


// helper function returns if magnitude is zero or not
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
    uint64_t carry = frac_sum >> 32; // 1 if fractional overflow
    uint64_t whole_sum = (uint64_t)left->whole + (uint64_t)right->whole + carry;

    result->frac = (uint32_t)(frac_sum  & 0xFFFFFFFFu);
    result->whole= (uint32_t)(whole_sum & 0xFFFFFFFFu);
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
    int compare;
    if (left->whole != right->whole) {
      compare = (left->whole < right->whole) ? -1 : 1;
    } else if (left->frac != right->frac) {
      compare = (left->frac  < right->frac ) ? -1 : 1;
    } else {
      compare = 0;
    }

    if (compare == 0) {
      // x + (−x) => +0
      result->whole = 0;
      result->frac  = 0;
      result->negative = false;
      return RESULT_OK;
    }

    // finding larger and smaller magnitudes
    const fixpoint_t *larger  = (compare > 0) ? left  : right;
    const fixpoint_t *smaller = (compare > 0) ? right : left;

    uint64_t largerwhole = (uint64_t)larger->whole;
    uint64_t largerfrac = (uint64_t)larger->frac;
    uint64_t smallerwhole = (uint64_t)smaller->whole;
    uint64_t smallerfrac = (uint64_t)smaller->frac;

    uint32_t out_frac, out_whole;
    if (largerfrac >= smallerfrac) {
      // no borrow from whole
      out_frac  = (uint32_t)(largerfrac - smallerfrac);
      out_whole = (uint32_t)(largerwhole - smallerwhole);
    } else {
      // borrow 1 whole (adds 2^32 to fractional part)
      out_frac  = (uint32_t)((largerfrac + (1ULL << 32)) - smallerfrac);
      out_whole = (uint32_t)((largerwhole - 1) - smallerwhole);
    }

    result->whole = out_whole;
    result->frac = out_frac;
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
  /*uint64_t product = multiplyHighLow(left, right); // 64 bit integr product

  result->whole = (uint32_t)(product >> 32);
  result->frac = (uint32_t)product;
  result->negative = left->negative ^ right->negative; // xor

  if (result->whole == 0 && result->frac == 0) { // if 0 case
    result->negative = false;
  }
  return RESULT_OK;*/

   // dealing with 0 edgecase for negatives

  bool left_neg = left->negative  && !is_zero_mag(left);
  bool right_neg = right->negative && !is_zero_mag(right);
  bool out_sign = (left_neg ^ right_neg);

   // Split 64-bit magnitudes into whole (A,B) and frac (a,b)

  uint64_t A = (uint64_t)left->whole;
  uint64_t a = (uint64_t)left->frac;
  uint64_t B = (uint64_t)right->whole;
  uint64_t b = (uint64_t)right->frac;
  
  uint64_t p0 = a * b; 
  uint64_t p1 = a * B; 
  uint64_t p2 = A * b;
  uint64_t p3 = A * B;

  uint32_t x0_low = (uint32_t)(p0 & 0xFFFFFFFFu); // bits lost by >>32
  bool underflow = (x0_low != 0);

  uint64_t x1 = (p0 >> 32)+ (p1 & 0xFFFFFFFFu)+ (p2 & 0xFFFFFFFFu);
  uint64_t x2 = (p1 >> 32)+ (p2 >> 32)+ p3;

  uint32_t out_frac = (uint32_t)(x1 & 0xFFFFFFFFu);
  uint64_t carry1= (x1 >> 32);

  uint64_t tmp = x2 + carry1; // <= 0xFFFFFFFFFFFFFFFF (no 64-bit overflow)
  uint32_t out_whole = (uint32_t)(tmp & 0xFFFFFFFFu);
  bool overflow  = ((tmp >> 32) != 0);

  result->whole = out_whole;
  result->frac  = out_frac;
  result->negative = out_sign;
  

if (result->whole == 0 && result->frac == 0) {
    if (!(underflow || overflow)) {
      result->negative = false;
    } else {
      result->negative = out_sign; // keep the "would-have-been" sign
    }
  }

  result_t flags = RESULT_OK;
  if (overflow)  flags |= RESULT_OVERFLOW;
  if (underflow) flags |= RESULT_UNDERFLOW;
  return flags;
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


  if (s == NULL) return false;

  const char *p = s->str;
  if (*p == '\0') return false;// empty string
  if (isspace((unsigned char)*p)) return false; //no whitespace leading

  // '-', '+' is not allowed 
  bool neg = false;
  if (*p == '-') { 
    neg = true; ++p; }
  else if (*p == '+') { 
    return false; }
  //Parse whole, 1..8 hex digits, no 0x/0X prefix allowed
  uint32_t whole = 0;
  int whole_digits = 0;

  while (whole_digits < 8 && isxdigit((unsigned char)*p)) {
    int d;
    if (*p >= '0' && *p <= '9') d = *p - '0';
    else {
      char c = (char)tolower((unsigned char)*p);
      if (c < 'a' || c > 'f') break;
      d = 10 + (c - 'a');
    }
    whole = (whole << 4) | (uint32_t)d;
    ++p; ++whole_digits;
  }
  if (whole_digits < 1) return false;// need at least one digit
  if (whole_digits == 8 && isxdigit((unsigned char)*p)) return false; // >8 digits

  // Next must be a dot
  if (*p != '.') return false;
  ++p;
  

  // Parse frac, 1..8 hex digits, then end of the string
  uint32_t frac = 0;
  int frac_digits = 0;

  while (frac_digits < 8 && isxdigit((unsigned char)*p)) {
    int d;
    if (*p >= '0' && *p <= '9') d = *p - '0';
    else {
      char c = (char)tolower((unsigned char)*p);
      if (c < 'a' || c > 'f') break;
      d = 10 + (c - 'a');
    }
    frac = (frac << 4) | (uint32_t)d;
    ++p; ++frac_digits;
  }
  if (frac_digits < 1) return false;// need at least one frac digit
  if (frac_digits == 8 && isxdigit((unsigned char)*p)) return false; // >8 digits
  if (*p != '\0') return false; // no trailing junk

  // Left align fractional hex to 32 bits
  if (frac_digits < 8) {
    frac <<= 4 * (8 - frac_digits);
  }

  // Normalize negative zero to +0
  if (whole == 0 && frac == 0) {
    neg = false;
  }

  val->whole = whole;
  val->frac = frac;
  val->negative = neg;
  return true;
}
