#include <ap_int.h>
#include "poly1305.hpp"

extern "C" {
void poly1305_top(
    ap_uint<256> key,
    const ap_uint<128>* msg,
    ap_uint<64> msg_len,
    ap_uint<128>& tag_out
) {
#pragma HLS INTERFACE s_axilite port=key       bundle=CTRL
#pragma HLS INTERFACE m_axi     port=msg       offset=slave bundle=GMEM
#pragma HLS INTERFACE s_axilite port=msg       bundle=CTRL
#pragma HLS INTERFACE s_axilite port=msg_len   bundle=CTRL
#pragma HLS INTERFACE s_axilite port=tag_out   bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return    bundle=CTRL

    ap_uint<132> acc = 0;
    ap_uint<128> tag_temp;

    if (msg_len == 0) {
        tag_out = key.range(255, 128);
        return;
    }

    ap_uint<64> num_blocks = (msg_len + 15) / 16;
    ap_int<64> remaining = msg_len;

    for (ap_uint<64> i = 0; i < num_blocks; i++) {
    #pragma HLS PIPELINE off

        ap_uint<128> block = msg[i];

        ap_uint<32> block_len;
        if (remaining >= 16) {
            block_len = 16;
        } else {
            block_len = remaining;
        }

        xf::security::internal::poly1305Imp(acc, key, block, block_len, tag_temp);

        remaining -= 16;
    }

    tag_out = tag_temp;
}
}
