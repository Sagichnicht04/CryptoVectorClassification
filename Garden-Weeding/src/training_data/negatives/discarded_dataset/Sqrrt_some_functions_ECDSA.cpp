#include "ECDSA.h"

using namespace std;







pair<BIGNUM*, BIGNUM*> sign_message(BIGNUM* message, BIGNUM* private_key, Point p, BIGNUM* n) {

    BIGNUM* rn = BN_new();
    BIGNUM* zero = BN_new();
    BIGNUM* rn_inv = BN_new();
    BIGNUM* res = BN_new();
    BN_set_word(zero,0);
    BN_CTX* ctx = BN_CTX_new();

    do {
        if (!BN_rand_range(rn, n)) {
            std::cerr << "Error generation" << std::endl;
            break;
        }
    } while (BN_is_zero(rn));

    Point r_point = p.multiply(rn);
    BIGNUM* r = BN_new();
    BN_mod(r, r_point.getX(), n, ctx);
    if (BN_cmp(r, zero) == 0) {
        return sign_message(message, private_key, p, n);
    }

    BN_mod_inverse(rn_inv, rn,n,ctx);
    BN_mul(res,r,private_key,ctx);
    BN_add(res,res,message);
    BN_mul(res,rn_inv, res, ctx);
    BN_mod(res,res,n,ctx);

    BN_free(rn);
    BN_free(zero);
    BN_free(rn_inv);
    BN_CTX_free(ctx);
    return {r, res};
}



bool verify_signature(pair<BIGNUM*,BIGNUM*> signature, BIGNUM* message, Point public_key, BIGNUM* n, Point point) {
    pair<BIGNUM *, BIGNUM *> p = signature;
    BIGNUM* s_inv = BN_new();
    BIGNUM* u = BN_new();
    BIGNUM* v = BN_new();
    BN_CTX* ctx = BN_CTX_new();

    BN_mod_inverse(s_inv,p.second,n,ctx);

    BN_mul(u,message,s_inv,ctx);
    BN_mod(u,u,n,ctx);
    BN_mul(v,p.first,s_inv,ctx);
    BN_mod(v,v,n,ctx);
    Point point_r = point.multiply(u).add(public_key.multiply(v));

    bool res = (BN_cmp(point_r.getX(), p.first) == 0);
    BN_free(s_inv);
    BN_free(u);
    BN_free(v);
    BN_CTX_free(ctx);
    return res;
}
