#include "fixpoint.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////
// Helper functions
// Note that you can make these "visible" (not static)
// if you want to be able to write unit tests for them
////////////////////////////////////////////////////////////////////////

// TODO: add helper functions

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

  uint32_t fracSum = left->frac + right->frac;
  uint32_t carry = (fracSum < left->frac) ? 1 : 0;

  uint32_t wholeSum = left->whole + right->whole + carry;

  result->frac = fracSum;
  result->whole = wholeSum;

  // Oveflow threshold
  const uint32_t threshold = 2147483647; // 2^(w-1) -1

  bool overflow = false;
  if (left->negative == right->negative) {
    // same sign → check for signed overflow
    if (!left->negative) {
      // positive + positive
      if (wholeSum > threshold || wholeSum < left->whole) {
        overflow = true;
        result->negative = true; // it becomes negative (overflow)
      } else {
        result->negative = false;
      }
    } else {
      // negative + negative
      if (wholeSum > threshold || wholeSum < left->whole) {
        overflow = true;
        result->negative = false; // becomes positive (negatve overflow)
      } else {
        result->negative = true;
      }
    }
  } else {
    // opposite signs → cannot overflow
    if (left->whole > right->whole ||
        (left->whole == right->whole && left->frac >= right->frac)) {
      result->negative = left->negative;
    } else {
      result->negative = right->negative;
    }
  }
  return overflow ? RESULT_OVERFLOW : RESULT_OK;
}

result_t fixpoint_sub(fixpoint_t *result, const fixpoint_t *left,
                      const fixpoint_t *right) {
  // TODO: implement
}

result_t fixpoint_mul(fixpoint_t *result, const fixpoint_t *left,
                      const fixpoint_t *right) {
  // TODO: implement
}

int fixpoint_compare(const fixpoint_t *left, const fixpoint_t *right) {
  // TODO: implement
}

void fixpoint_format_hex(fixpoint_str_t *s, const fixpoint_t *val) {
  // TODO: implement
}

bool fixpoint_parse_hex(fixpoint_t *val, const fixpoint_str_t *s) {
  // TODO: implement
}
