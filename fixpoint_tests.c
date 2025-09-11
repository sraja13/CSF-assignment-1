#include <stdlib.h>
#include <string.h>
#include "tctest.h"
#include "fixpoint.h"

// Test fixture: defines some fixpoint_t instances
// that can be used by test functions
typedef struct {
  fixpoint_t zero;
  fixpoint_t one;
  fixpoint_t one_half;
  fixpoint_t max;
  fixpoint_t neg_three_eighths;
  fixpoint_t min;
  fixpoint_t one_and_one_half;
  fixpoint_t one_hundred;
  fixpoint_t neg_eleven;
} TestObjs;

// Functions to create and destroy the text fixture
// (each test function gets a "fresh" instance of the
// test fixture)
TestObjs *setup( void );
void cleanup( TestObjs *objs );

// The setup() function uses this macro to initialize
// the fixpoint_t objects in the test fixture, to avoid
// being dependent on the fixpoint_init() function.
#define TEST_FIXPOINT_INIT( val, w, f, n ) \
do { \
  (val)->frac = f; \
  (val)->whole = w; \
  (val)->negative = n; \
} while ( 0 )

// Macro to check two fixpoint_t instances for exact equality
#define TEST_EQUAL( val1, val2 ) \
do { \
  ASSERT( (val1)->whole == (val2)->whole ); \
  ASSERT( (val1)->frac == (val2)->frac ); \
  ASSERT( (val1)->negative == (val2)->negative ); \
} while ( 0 )

// Convenience macro to turn a string literal into a const pointer
// to a temporary instance of fixpoint_str_t
#define FIXPOINT_STR( strlit ) &( ( fixpoint_str_t ) { .str = (strlit) } )

// Prototypes for test functions
void test_init( TestObjs *objs );
void test_get_whole( TestObjs *objs );
void test_get_frac( TestObjs *objs );
void test_is_negative( TestObjs *objs );
void test_negate( TestObjs *objs );
void test_add( TestObjs *objs );
void test_sub( TestObjs *objs );
void test_mul( TestObjs *objs );
void test_compare( TestObjs *objs );
void test_format_hex( TestObjs *objs );
void test_parse_hex( TestObjs *objs );
// TODO: add additional test functions

int main( int argc, char **argv ) {
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST( test_init );
  TEST( test_get_whole );
  TEST( test_get_frac );
  TEST( test_is_negative );
  TEST( test_negate );
  TEST( test_add );
  TEST( test_sub );
  TEST( test_mul );
  TEST( test_compare );
  TEST( test_format_hex );
  TEST( test_parse_hex );
  // TODO: call additional test functions

  TEST_FINI();
}

TestObjs *setup( void ) {
  TestObjs *objs = (TestObjs *) malloc( sizeof( TestObjs ) );

  TEST_FIXPOINT_INIT( &objs->zero, 0, 0, false );
  TEST_FIXPOINT_INIT( &objs->one, 1, 0, false );
  TEST_FIXPOINT_INIT( &objs->one_half, 0, 0x80000000, false );
  TEST_FIXPOINT_INIT( &objs->max, 0xFFFFFFFF, 0xFFFFFFFF, false );
  TEST_FIXPOINT_INIT( &objs->neg_three_eighths, 0, 0x60000000, true );
  TEST_FIXPOINT_INIT( &objs->min, 0, 1, false );
  TEST_FIXPOINT_INIT( &objs->one_and_one_half, 1, 0x80000000, false );
  TEST_FIXPOINT_INIT( &objs->one_hundred, 100, 0, false );
  TEST_FIXPOINT_INIT( &objs->neg_eleven, 11, 0, true );

  //test_add test functions
  TEST( test_add_frac_carry );
  TEST( test_add_whole_overflow );
  TEST( test_add_large_but_fits );
  TEST( test_add_negative_overflow_to_negzero );
  TEST( test_add_cancel_to_zero );
  TEST( test_add_borrow_across_frac );
  TEST( test_add_subtract_no_borrow );
  TEST( test_add_sign_of_larger );
  TEST( test_add_negative_zero_input );
  TEST( test_add_commutativity );
  TEST( test_add_mixed_carry );
  TEST( test_add_equal_fraction_cancel );
  TEST( test_add_two_negatives );
  TEST( test_add_cancel_whole_frac_remains );
  TEST( test_add_frac_to_whole );
  TEST( test_add_large_fraction );
  TEST( test_add_associativity );
  TEST( test_add_identity_with_zero );
  TEST( test_add_min_fraction );
  TEST( test_add_max_plus_zero );


  return objs;
}

void cleanup( TestObjs *objs ) {
  free( objs );
}

void test_init( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t val;

  fixpoint_init( &val, 0, 0, false );
  ASSERT( val.whole == 0 );
  ASSERT( val.frac == 0 );
  ASSERT( val.negative == false );

  fixpoint_init( &val, 0xad2b55b1, 0xcf5f4470, true );
  ASSERT( val.whole == 0xad2b55b1 );
  ASSERT( val.frac == 0xcf5f4470 );
  ASSERT( val.negative == true );
}

void test_get_whole( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( fixpoint_get_whole( &objs->zero ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->one ) == 1 );
  ASSERT( fixpoint_get_whole( &objs->one_half ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->max ) == 0xFFFFFFFF );
  ASSERT( fixpoint_get_whole( &objs->neg_three_eighths ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->min ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->one_and_one_half ) == 1 );
  ASSERT( fixpoint_get_whole( &objs->one_hundred ) == 100 );
  ASSERT( fixpoint_get_whole( &objs->neg_eleven ) == 11 );
}

void test_get_frac( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( fixpoint_get_frac( &objs->zero ) == 0 );
  ASSERT( fixpoint_get_frac( &objs->one ) == 0 );
  ASSERT( fixpoint_get_frac( &objs->one_half ) == 0x80000000 );
  ASSERT( fixpoint_get_frac( &objs->max ) == 0xFFFFFFFF );
  ASSERT( fixpoint_get_frac( &objs->neg_three_eighths ) == 0x60000000 );
  ASSERT( fixpoint_get_frac( &objs->min ) == 1 );
  ASSERT( fixpoint_get_frac( &objs->one_and_one_half ) == 0x80000000 );
  ASSERT( fixpoint_get_frac( &objs->one_hundred ) == 0 );
  ASSERT( fixpoint_get_frac( &objs->neg_eleven ) == 0 );
}

void test_is_negative( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( fixpoint_is_negative( &objs->zero ) == false );
  ASSERT( fixpoint_is_negative( &objs->one ) == false );
  ASSERT( fixpoint_is_negative( &objs->one_half ) == false );
  ASSERT( fixpoint_is_negative( &objs->max ) == false );
  ASSERT( fixpoint_is_negative( &objs->neg_three_eighths ) == true );
  ASSERT( fixpoint_is_negative( &objs->min ) == false );
  ASSERT( fixpoint_is_negative( &objs->one_and_one_half ) == false );
  ASSERT( fixpoint_is_negative( &objs->one_hundred ) == false );
  ASSERT( fixpoint_is_negative( &objs->neg_eleven ) == true );
}

void test_negate( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;

  // Negating 0 shouldn't cause it to become negative
  result = objs->zero;
  ASSERT( false == result.negative );
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->zero.whole );
  ASSERT( result.frac == objs->zero.frac );
  ASSERT( false == result.negative );

  // Non-zero values should have their sign flip when negated,
  // but the magnitude should stay the same

  result = objs->one;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->one.whole );
  ASSERT( result.frac == objs->one.frac );
  ASSERT( true == result.negative );

  result = objs->max;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->max.whole );
  ASSERT( result.frac == objs->max.frac );
  ASSERT( true == result.negative );

  result = objs->neg_three_eighths;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->neg_three_eighths.whole );
  ASSERT( result.frac == objs->neg_three_eighths.frac );
  ASSERT( false == result.negative );
}

void test_add( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;

  ASSERT( fixpoint_add( &result, &objs->zero, &objs->zero ) == RESULT_OK );
  ASSERT( 0 == result.frac );
  ASSERT( 0 == result.whole );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->zero, &objs->one ) == RESULT_OK );
  ASSERT( 0 == result.frac );
  ASSERT( 1 == result.whole );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->max, &objs->one ) == RESULT_OVERFLOW );

  ASSERT( fixpoint_add( &result, &objs->max, &objs->min ) == RESULT_OVERFLOW );

  ASSERT( fixpoint_add( &result, &objs->zero, &objs->neg_three_eighths ) == RESULT_OK );
  ASSERT( 0x60000000 == result.frac );
  ASSERT( 0 == result.whole );
  ASSERT( true == result.negative );

  fixpoint_t neg_max = objs->max;
  neg_max.negative = true;

  fixpoint_t neg_min = objs->min;
  neg_min.negative = true;

  ASSERT( fixpoint_add( &result, &neg_max, &neg_min ) == RESULT_OVERFLOW );
}

void test_sub( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;

  ASSERT( fixpoint_sub( &result, &objs->one, &objs->zero ) == RESULT_OK );
  ASSERT( 1 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_sub( &result, &objs->zero, &objs->one ) == RESULT_OK );
  ASSERT( 1 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( true == result.negative );

  fixpoint_t neg_min = objs->min;
  fixpoint_negate( &neg_min );
  ASSERT( fixpoint_sub( &result, &neg_min, &objs->max ) == RESULT_OVERFLOW );
}

void test_mul( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;

  ASSERT( fixpoint_mul( &result, &objs->one, &objs->zero ) == RESULT_OK );
  ASSERT( 0 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_mul( &result, &objs->one_and_one_half, &objs->one_hundred ) == RESULT_OK );
  ASSERT( 150 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( false == result.negative );
}

void test_compare( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( 0 == fixpoint_compare( &objs->zero, &objs->zero ) );
  ASSERT( 1 == fixpoint_compare( &objs->one, &objs->zero ) );
  ASSERT( -1 == fixpoint_compare( &objs->zero, &objs->one ) );
  ASSERT( -1 == fixpoint_compare( &objs->neg_three_eighths, &objs->one_half ) );
  ASSERT( 1 == fixpoint_compare( &objs->one_half, &objs->neg_three_eighths ) );
}

void test_format_hex( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_str_t s;

  fixpoint_format_hex( &s, &objs->zero );
  ASSERT( 0 == strcmp( "0.0", s.str ) );

  fixpoint_format_hex( &s, &objs->one );
  ASSERT( 0 == strcmp( "1.0", s.str ) );

  fixpoint_format_hex( &s, &objs->one_half );
  ASSERT( 0 == strcmp( "0.8", s.str ) );

  fixpoint_format_hex( &s, &objs->max );
  ASSERT( 0 == strcmp( "ffffffff.ffffffff", s.str ) );

  fixpoint_format_hex( &s, &objs->neg_three_eighths );
  ASSERT( 0 == strcmp( "-0.6", s.str ) );

  fixpoint_format_hex( &s, &objs->min );
  ASSERT( 0 == strcmp( "0.00000001", s.str ) );

  fixpoint_format_hex( &s, &objs->one_and_one_half );
  ASSERT( 0 == strcmp( "1.8", s.str ) );

  fixpoint_format_hex( &s, &objs->one_hundred );
  ASSERT( 0 == strcmp( "64.0", s.str ) );

  fixpoint_format_hex( &s, &objs->neg_eleven );
  ASSERT( 0 == strcmp( "-b.0", s.str ) );
}

void test_parse_hex( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t val;

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "0.0" ) ) );
  TEST_EQUAL( &objs->zero, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "1.0" ) ) );
  TEST_EQUAL( &objs->one, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "0.8" ) ) );
  TEST_EQUAL( &objs->one_half, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "ffffffff.ffffffff" ) ) );
  TEST_EQUAL( &objs->max, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "-0.6" ) ) );
  TEST_EQUAL( &objs->neg_three_eighths, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "0.00000001" ) ) );
  TEST_EQUAL( &objs->min, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "1.8" ) ) );
  TEST_EQUAL( &objs->one_and_one_half, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "1.8" ) ) );
  TEST_EQUAL( &objs->one_and_one_half, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "64.0" ) ) );
  TEST_EQUAL( &objs->one_hundred, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "-b.0" ) ) );
  TEST_EQUAL( &objs->neg_eleven, &val );

  // Note: this test function doesn't have any tests for invalid
  // hex strings. You should add tests for invalid strings in one of
  // your own test functions.
}

// Unit tests for Assignment 1 Edge Cases:

// unit tests for fixpoint_init edge cases
void test_init_edge_cases(TestObjs *objs) {
    fixpoint_t val;

    // Only fraction is non-zero, whole is 0
    fixpoint_init(&val, 0, 1, true);
    ASSERT(val.whole == 0);
    ASSERT(val.frac == 1);
    ASSERT(val.negative == true);

    // Both whole and fraction maxed out
    fixpoint_init(&val, 0xFFFFFFFF, 0xFFFFFFFF, false);
    ASSERT(val.whole == 0xFFFFFFFF);
    ASSERT(val.frac == 0xFFFFFFFF);
    ASSERT(val.negative == false);

    // Fraction maxed, negative
    fixpoint_init(&val, 0, 0xFFFFFFFF, true);
    ASSERT(val.whole == 0);
    ASSERT(val.frac == 0xFFFFFFFF);
    ASSERT(val.negative == true);
}
//  unit test for repeated Negation
void test_repeated_negation(TestObjs *objs) {
    fixpoint_t val;

    // Negate a positive number twice should return original
    fixpoint_init(&val, 5, 12345678, false);
    fixpoint_negate(&val);
    ASSERT(val.negative == true);
    fixpoint_negate(&val);
    ASSERT(val.negative == false);

    // Negate a negative number twice should return original
    fixpoint_init(&val, 10, 987654321, true);
    fixpoint_negate(&val);
    ASSERT(val.negative == false);
    fixpoint_negate(&val);
    ASSERT(val.negative == true);

    // Negate zero multiple times should stay zero
    fixpoint_init(&val, 0, 0, false);
    fixpoint_negate(&val);
    ASSERT(val.negative == false);
    fixpoint_negate(&val);
    ASSERT(val.negative == false);
}

void test_correct_initialization(TestObjs *objs) {
    fixpoint_t val;

    // Initialize with arbitrary numbers and check getters
    fixpoint_init(&val, 1234, 0xABCDEFFF, true);
    ASSERT(fixpoint_get_whole(&val) == 1234);
    ASSERT(fixpoint_get_frac(&val) == 0xABCDEFFF);
    ASSERT(fixpoint_is_negative(&val) == true);

    // Initialize with zero whole, non-zero fraction
    fixpoint_init(&val, 0, 0x80000000, false);
    ASSERT(fixpoint_get_whole(&val) == 0);
    ASSERT(fixpoint_get_frac(&val) == 0x80000000);
    ASSERT(fixpoint_is_negative(&val) == false);
}
//unit tests for fixpoint_add:

void test_add_frac_carry(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t almost_one, tiny;
  TEST_FIXPOINT_INIT(&almost_one, 0, 0xFFFFFFFF, false);
  TEST_FIXPOINT_INIT(&tiny, 0, 0x1, false);

  ASSERT(fixpoint_add(&result, &almost_one, &tiny) == RESULT_OK);
  ASSERT(result.whole == 1);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == false);
}

void test_add_whole_overflow(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t big, tiny;
  TEST_FIXPOINT_INIT(&big, 0xFFFFFFFF, 0xFFFFFFFF, false);
  TEST_FIXPOINT_INIT(&tiny, 0, 0x1, false);

  ASSERT(fixpoint_add(&result, &big, &tiny) & RESULT_OVERFLOW);
  ASSERT(result.whole == 0); 
  ASSERT(result.frac == 0);
  ASSERT(result.negative == false);
}

void test_add_large_but_fits(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 0x80000000, 0, false);
  TEST_FIXPOINT_INIT(&b, 0x7FFFFFFF, 0, false);

  ASSERT(fixpoint_add(&result, &a, &b) == RESULT_OK);
  ASSERT(result.whole == 0xFFFFFFFF);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == false);
}

void test_add_negative_overflow_to_negzero(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 0xFFFFFFFF, 0xFFFFFFFF, true);
  TEST_FIXPOINT_INIT(&b, 0, 0x1, true);

  ASSERT(fixpoint_add(&result, &a, &b) & RESULT_OVERFLOW);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == true); // negative zero from overflow
}

void test_add_cancel_to_zero(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t pos, neg;
  TEST_FIXPOINT_INIT(&pos, 1, 0, false);
  TEST_FIXPOINT_INIT(&neg, 1, 0, true);

  ASSERT(fixpoint_add(&result, &pos, &neg) == RESULT_OK);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == false);
}

void test_add_borrow_across_frac(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t pos, negtiny;
  TEST_FIXPOINT_INIT(&pos, 1, 0, false);
  TEST_FIXPOINT_INIT(&negtiny, 0, 0x1, true);

  ASSERT(fixpoint_add(&result, &pos, &negtiny) == RESULT_OK);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 0xFFFFFFFF);
  ASSERT(result.negative == false);
}

void test_add_subtract_no_borrow(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t pos, neg;
  TEST_FIXPOINT_INIT(&pos, 1, 2, false);
  TEST_FIXPOINT_INIT(&neg, 0, 1, true);

  ASSERT(fixpoint_add(&result, &pos, &neg) == RESULT_OK);
  ASSERT(result.whole == 1);
  ASSERT(result.frac == 1);
  ASSERT(result.negative == false);
}

void test_add_sign_of_larger(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 0, 0x40000000, false); // 0.4
  TEST_FIXPOINT_INIT(&b, 0, 0x80000000, true);  // -0.8

  ASSERT(fixpoint_add(&result, &a, &b) == RESULT_OK);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 0x40000000);
  ASSERT(result.negative == true); // sign of larger magnitude
}

void test_add_negative_zero_input(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t negzero, tiny;
  TEST_FIXPOINT_INIT(&negzero, 0, 0, true);
  TEST_FIXPOINT_INIT(&tiny, 0, 1, false);

  ASSERT(fixpoint_add(&result, &negzero, &tiny) == RESULT_OK);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 1);
  ASSERT(result.negative == false); // -0 treated as +0
}

void test_add_commutativity(TestObjs *objs) {
  fixpoint_t r1, r2;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 0, 0xFFFFFFFF, false);
  TEST_FIXPOINT_INIT(&b, 1, 0, true);

  result_t res1 = fixpoint_add(&r1, &a, &b);
  result_t res2 = fixpoint_add(&r2, &b, &a);

  ASSERT(r1.whole == r2.whole);
  ASSERT(r1.frac == r2.frac);
  ASSERT(r1.negative == r2.negative);
  ASSERT(res1 == res2);
}

// tests mxed whole + fractional carry
void test_add_mixed_carry(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 0xFFFFFFFE, 0xFFFFFFFF, false);
  TEST_FIXPOINT_INIT(&b, 0, 0x1, false);

  ASSERT(fixpoint_add(&result, &a, &b) == RESULT_OK);
  ASSERT(result.whole == 0xFFFFFFFF);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == false);
}

// Opposite signs equal fractional parts (cancel to zero)
void test_add_equal_fraction_cancel(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t pos, neg;
  TEST_FIXPOINT_INIT(&pos, 1, 0x80000000, false); // +1.5
  TEST_FIXPOINT_INIT(&neg, 1, 0x80000000, true);  // -1.5

  ASSERT(fixpoint_add(&result, &pos, &neg) == RESULT_OK);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == false);
}
// Negative + negative without overflow
void test_add_two_negatives(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 2, 0x40000000, true); // -2.25
  TEST_FIXPOINT_INIT(&b, 3, 0xC0000000, true); // -3.75

  ASSERT(fixpoint_add(&result, &a, &b) == RESULT_OK);
  ASSERT(result.whole == 6);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == true);
}

// Positive + negative where whole cancels, frac remains
void test_add_cancel_whole_frac_remains(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t pos, neg;
  TEST_FIXPOINT_INIT(&pos, 2, 0x40000000, false); // 2.25
  TEST_FIXPOINT_INIT(&neg, 2, 0, true);           // -2.0

  ASSERT(fixpoint_add(&result, &pos, &neg) == RESULT_OK);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 0x40000000);
  ASSERT(result.negative == false);
}

// Fraction only values adding to whole
void test_add_frac_to_whole(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 0, 0x40000000, false); // 0.25
  TEST_FIXPOINT_INIT(&b, 0, 0xC0000000, false); // 0.75

  ASSERT(fixpoint_add(&result, &a, &b) == RESULT_OK);
  ASSERT(result.whole == 1);
  ASSERT(result.frac == 0);
  ASSERT(result.negative == false);
}

// Large fractional add that doesn’t overflow
void test_add_large_fraction(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 12345, 0, false);
  TEST_FIXPOINT_INIT(&b, 0, 0xFFFFFFFF, false);

  ASSERT(fixpoint_add(&result, &a, &b) == RESULT_OK);
  ASSERT(result.whole == 12345);
  ASSERT(result.frac == 0xFFFFFFFF);
  ASSERT(result.negative == false);
}

// Associativity check; (a+b)+c == a+(b+c)
void test_add_associativity(TestObjs *objs) {
  fixpoint_t r1, r2;
  fixpoint_t a, b, c;
  TEST_FIXPOINT_INIT(&a, 1, 0, false);
  TEST_FIXPOINT_INIT(&b, 2, 0, false);
  TEST_FIXPOINT_INIT(&c, 3, 0, false);

  fixpoint_add(&r1, &a, &b);
  fixpoint_add(&r1, &r1, &c);

  fixpoint_add(&r2, &b, &c);
  fixpoint_add(&r2, &a, &r2);

  ASSERT(r1.whole == r2.whole);
  ASSERT(r1.frac == r2.frac);
  ASSERT(r1.negative == r2.negative);
}

// Identity with zero
void test_add_identity_with_zero(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t val;
  TEST_FIXPOINT_INIT(&val, 42, 0x80000000, false); // 42.5

  ASSERT(fixpoint_add(&result, &val, &objs->zero) == RESULT_OK);
  TEST_EQUAL(&val, &result);

  ASSERT(fixpoint_add(&result, &objs->zero, &val) == RESULT_OK);
  TEST_EQUAL(&val, &result);
}

// Very small fractions
void test_add_min_fraction(TestObjs *objs) {
  fixpoint_t result;
  fixpoint_t a, b;
  TEST_FIXPOINT_INIT(&a, 0, 0x00000001, false);
  TEST_FIXPOINT_INIT(&b, 0, 0x00000001, false);

  ASSERT(fixpoint_add(&result, &a, &b) == RESULT_OK);
  ASSERT(result.whole == 0);
  ASSERT(result.frac == 0x00000002);
  ASSERT(result.negative == false);
}

// Max representable + 0 should not overflow
void test_add_max_plus_zero(TestObjs *objs) {
  fixpoint_t result;
  ASSERT(fixpoint_add(&result, &objs->max, &objs->zero) == RESULT_OK);
  TEST_EQUAL(&objs->max, &result);
}

//test_sub tests:
