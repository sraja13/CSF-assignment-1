#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fixpoint.h"

////////////////////////////////////////////////////////////////////////
// Helper functions
// Note that you can make these "visible" (not static)
// if you want to be able to write unit tests for them
////////////////////////////////////////////////////////////////////////

// TODO: add helper functions

////////////////////////////////////////////////////////////////////////
// Public API functions
////////////////////////////////////////////////////////////////////////

void
fixpoint_init( fixpoint_t *val, uint32_t whole, uint32_t frac, bool negative ) {

  val->whole = whole;
  val-> frac= frac;
  val -> negative = negative;
  
   if(whole==0 && frac==0){
    val->negative=false;
   }
   else{
    val -> negative = negative;
   }

}

uint32_t
fixpoint_get_whole( const fixpoint_t *val ) {

  return (*val).whole;
}

uint32_t
fixpoint_get_frac( const fixpoint_t *val ) {
  return (*val).frac;
}

bool
fixpoint_is_negative( const fixpoint_t *val ) {
  return (*val).negative;
  
}

void
fixpoint_negate( fixpoint_t *val ) {
  if(val->whole == 0 && val->frac ==0){
    val->negative= false;
  }

  else{
    val->negative= !val->negative;
  }
}
  



//stop here for milestone 1


result_t
fixpoint_add( fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right ) {
  // TODO: implement
}

result_t
fixpoint_sub( fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right ) {
  // TODO: implement
}

result_t
fixpoint_mul( fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right ) {
  // TODO: implement
}

int
fixpoint_compare( const fixpoint_t *left, const fixpoint_t *right ) {
  // TODO: implement
}

void
fixpoint_format_hex( fixpoint_str_t *s, const fixpoint_t *val ) {
  // TODO: implement
}

bool
fixpoint_parse_hex( fixpoint_t *val, const fixpoint_str_t *s ) {
  // TODO: implement
}
