#pragma once

#include <stdint.h>

typedef struct {
    uint32_t quotient;
    uint32_t remainder;
} divmod_result_t;

static inline divmod_result_t divmod_u32u32(uint32_t a, uint32_t b) {
    divmod_result_t r;
    r.quotient = a / b;
    r.remainder = a % b;
    return r;
}

static inline uint32_t to_quotient_u32(divmod_result_t r) {
    return r.quotient;
}

static inline uint32_t to_remainder_u32(divmod_result_t r) {
    return r.remainder;
}
