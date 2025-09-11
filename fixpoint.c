#include "fixpoint.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////
// Helper functions
// Note that you can make these "visible" (not static)
// if you want to be able to write unit tests for them
////////////////////////////////////////////////////////////////////////

// fixpoint add and sub  helper functions
// Add magnitudes when same sign
static inline bool is_zero_mag(const fixpoint_t *x) {
  return x->whole == 0 && x->frac == 0;
}
// Compare magnitudes of two fixpoint numbers
static int compare_magnitudes(const fixpoint_t *left, const fixpoint_t *right) {
  if (left->whole != right->whole) {
    return (left->whole < right->whole) ? -1 : 1;
  } else if (left->frac != right->frac) {
    return (left->frac < right->frac) ? -1 : 1;
  } else {
    return 0;
  }
}

static result_t add_magnitudes_same_sign(fixpoint_t *result,
                                         const fixpoint_t *left,
                                         const fixpoint_t *right,
                                         bool negative) {
  uint64_t frac_sum = (uint64_t)left->frac + (uint64_t)right->frac;
  uint64_t carry = frac_sum >> 32;
  uint64_t whole_sum = (uint64_t)left->whole + (uint64_t)right->whole + carry;

  result->frac = (uint32_t)(frac_sum & 0xFFFFFFFFu);
  result->whole = (uint32_t)(whole_sum & 0xFFFFFFFFu);
  result->negative = negative;

  result_t flags = RESULT_OK;
  if (whole_sum >> 32)
    flags |= RESULT_OVERFLOW; // overflow on 33rd bit

  // Normalize zero except negative overflow case
  if (result->whole == 0 && result->frac == 0) {
    if (!(negative && (flags & RESULT_OVERFLOW))) {
      result->negative = false;
    }
  }
  return flags;
}

// Subtract magnitudes when opposite sign
static void subtract_magnitudes_opposite_sign(fixpoint_t *result,
                                              const fixpoint_t *larger,
                                              const fixpoint_t *smaller) {
  uint64_t largerwhole = (uint64_t)larger->whole;
  uint64_t largerfrac = (uint64_t)larger->frac;
  uint64_t smallerwhole = (uint64_t)smaller->whole;
  uint64_t smallerfrac = (uint64_t)smaller->frac;

  uint32_t out_frac, out_whole;
  if (largerfrac >= smallerfrac) {
    // no borrow
    out_frac = (uint32_t)(largerfrac - smallerfrac);
    out_whole = (uint32_t)(largerwhole - smallerwhole);
  } else { // borrow 1 whole
    out_frac = (uint32_t)((largerfrac + (1ULL << 32)) - smallerfrac);
    out_whole = (uint32_t)((largerwhole - 1) - smallerwhole);
  }

  result->whole = out_whole;
  result->frac = out_frac;
  result->negative = larger->negative && !is_zero_mag(larger);
  if (result->whole == 0 && result->frac == 0) {
    result->negative = false; // normalize zero
  }
}
// Compute final fraction, whole, and flags
static void finalize_mul(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3,
                         uint32_t *out_frac, uint32_t *out_whole,
                         bool *overflow, bool *underflow) {
  uint32_t x0_low = (uint32_t)(p0 & 0xFFFFFFFFu);
  *underflow = (x0_low != 0);

  uint64_t x1 = (p0 >> 32) + (p1 & 0xFFFFFFFFu) + (p2 & 0xFFFFFFFFu);
  uint64_t x2 = (p1 >> 32) + (p2 >> 32) + p3;

  *out_frac = (uint32_t)(x1 & 0xFFFFFFFFu);
  uint64_t carry1 = (x1 >> 32);

  uint64_t tmp = x2 + carry1;
  *out_whole = (uint32_t)(tmp & 0xFFFFFFFFu);
  *overflow = ((tmp >> 32) != 0);
}
static void normalize_zero_mul(fixpoint_t *result, bool underflow,
                               bool overflow, bool out_sign) {
  if (result->whole == 0 && result->frac == 0) {
    if (!(underflow || overflow)) {
      result->negative = false;
    } else {
      result->negative = out_sign;
    }
  }
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
static bool is_hex_digit(char c, int *value) {
    if (c >= '0' && c <= '9') { 
        *value = c - '0'; 
        return true; 
    }
    char lc = (char)tolower((unsigned char)c);
    if (lc >= 'a' && lc <= 'f') {
        *value = 10 + (lc - 'a'); 
        return true; 
    }
    return false;
}

static bool parse_hex_part(const char **p, uint32_t *out, int min_digits, int max_digits) {
    uint32_t result = 0;
    int digits = 0;
    int d;
    while (digits < max_digits && is_hex_digit(**p, &d)) {
        result = (result << 4) | (uint32_t)d;
        ++digits;
        ++(*p);
    }
    if (digits < min_digits) return false;
    if (digits == max_digits && is_hex_digit(**p, &d)) return false; // too many digits
    *out = result;
    return true;
}

static bool parse_whole(const char **p, uint32_t *whole) {
    return parse_hex_part(p, whole, 1, 8);
}

static bool parse_frac(const char **p, uint32_t *frac) {
    const char *tmp = *p;
    if (!parse_hex_part(p, frac, 1, 8)) return false;
    int digits_parsed = (int)(*p - tmp);
    // Left-align to 32 bits
    if (digits_parsed < 8) {
        *frac <<= 4 * (8 - digits_parsed);
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

result_t fixpoint_add(fixpoint_t *result, const fixpoint_t *left,
                      const fixpoint_t *right) {
  // Normalize signs
  bool lneg = left->negative && !is_zero_mag(left);
  bool rneg = right->negative && !is_zero_mag(right);

  if (lneg == rneg) {
    // Same sign: add magnitudes
    return add_magnitudes_same_sign(result, left, right, lneg);
  } else {
    // Opposite signs: subtract magnitudes
    int cmp = compare_magnitudes(left, right);

    if (cmp == 0) {
      // x + (−x) = 0
      result->whole = 0;
      result->frac = 0;
      result->negative = false;
      return RESULT_OK;
    }

    const fixpoint_t *larger = (cmp > 0) ? left : right;
    const fixpoint_t *smaller = (cmp > 0) ? right : left;

    subtract_magnitudes_opposite_sign(result, larger, smaller);
    return RESULT_OK; // no overflow for opposite signs
  }
}

result_t fixpoint_sub(fixpoint_t *result, const fixpoint_t *left,
                      const fixpoint_t *right) {
  // invert second operand
  fixpoint_t negRight = *right;                 // copy
  fixpoint_negate(&negRight);                   // negate right
  return fixpoint_add(result, left, &negRight); // call add lft + -right
}

result_t fixpoint_mul(fixpoint_t *result, const fixpoint_t *left,
                      const fixpoint_t *right) {
  // Determine output sign
  bool left_neg = left->negative && !is_zero_mag(left);
  bool right_neg = right->negative && !is_zero_mag(right);
  bool out_sign = (left_neg ^ right_neg);

  // Split magnitudes
  uint64_t A = left->whole;
  uint64_t a = left->frac;
  uint64_t B = right->whole;
  uint64_t b = right->frac;

  // Compute partial products
  uint64_t p0 = a * b;
  uint64_t p1 = a * B;
  uint64_t p2 = A * b;
  uint64_t p3 = A * B;

  // Finalize fraction, whole, and flags
  uint32_t out_frac, out_whole;
  bool overflow, underflow;
  finalize_mul(p0, p1, p2, p3, &out_frac, &out_whole, &overflow, &underflow);

  result->frac = out_frac;
  result->whole = out_whole;
  result->negative = out_sign;

  normalize_zero_mul(result, underflow, overflow, out_sign);

  result_t flags = RESULT_OK;
  if (overflow)
    flags = flags + RESULT_OVERFLOW;
  if (underflow)
    flags = flags + RESULT_UNDERFLOW;

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

if (!s || s->str[0] == '\0') return false;

    const char *p = s->str;
    if (isspace((unsigned char)*p)) return false; // no leading space

    bool neg = false;
    if (*p == '-') { neg = true; ++p; }
    else if (*p == '+') { return false; }

    uint32_t whole = 0;
    if (!parse_whole(&p, &whole)) return false;

    if (*p != '.') return false;
    ++p;

    uint32_t frac = 0;
    if (!parse_frac(&p, &frac)) return false;

    if (*p != '\0') return false; // no trailing junk

    if (whole == 0 && frac == 0) neg = false; // normalize zero

    val->whole = whole;
    val->frac = frac;
    val->negative = neg;

    return true;
}
