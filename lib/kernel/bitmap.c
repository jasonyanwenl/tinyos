#include "bitmap.h"
#include "string.h"
#include "debug.h"

void bitmap_init(struct bitmap *btmp) {
    memset(btmp->bits, 0, btmp->btmp_bytes_len);
}

bool bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx) {
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_odd = bit_idx % 8;
    return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd));
}

int bitmap_scan(struct bitmap *btmp, uint32_t cnt) {
    uint32_t byte_idx = 0;
    while ((0xff == btmp->bits[byte_idx]) && (byte_idx < btmp->btmp_bytes_len)) {
        byte_idx++;
    }
    ASSERT(byte_idx <= btmp->btmp_bytes_len);
    if (byte_idx == btmp->btmp_bytes_len) {
        return -1;
    }

    int bit_idx = 0;
    while ((uint8_t)(BITMAP_MASK << bit_idx) & btmp->bits[byte_idx]) {
        bit_idx++;
    }

    int bit_idx_start = byte_idx * 8 + bit_idx;
    if (cnt == 1) {
        return bit_idx_start;
    }

    uint32_t bit_left = (btmp->btmp_bytes_len * 8 - bit_idx_start);
    uint32_t bit_next = bit_idx_start + 1;
    uint32_t count = 1;

    bit_idx_start = -1;
    while (bit_left-- > 0) {
        if (!(bitmap_scan_test(btmp, bit_next))) {
            count++;
        } else {
            count = 0;
        }
        if (count == cnt) {
            bit_idx_start = bit_next - cnt + 1;
            break;
        }
        bit_next++;
    }
    return bit_idx_start;
}

void bitmap_set(struct bitmap *btmp, uint32_t bit_idx, int8_t value) {
    ASSERT((value == 0) || (value == 1));
    uint32_t byte_idx = bit_idx / 8;
    uint32_t bit_odd = bit_idx % 8;

    if (value) {
        btmp->bits[byte_idx] |= (BITMAP_MASK << bit_odd);
    } else {
        btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_odd);
    }   
}