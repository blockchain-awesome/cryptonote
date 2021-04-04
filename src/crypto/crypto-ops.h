// Copyright (c) 2014-2018, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

 #ifndef CRYPTO_OPS_H_
 #define CRYPTO_OPS_H_

 #ifdef __cplusplus
extern "C"
{
#endif

/* From fe.h */

typedef int32_t fe[10];

/* From ge.h */

typedef struct {
  fe X;
  fe Y;
  fe Z;
} ge_p2;

typedef struct {
  fe X;
  fe Y;
  fe Z;
  fe T;
} ge_p3;

typedef struct {
  fe X;
  fe Y;
  fe Z;
  fe T;
} ge_p1p1;

typedef struct {
  fe yplusx;
  fe yminusx;
  fe xy2d;
} ge_precomp;

typedef struct {
  fe YplusX;
  fe YminusX;
  fe Z;
  fe T2d;
} ge_cached;

/* From ge_add.c */

extern void ge_add(ge_p1p1 *, const ge_p3 *, const ge_cached *);

/* From ge_double_scalarmult.c, modified */

typedef ge_cached ge_dsmp[8];
extern const ge_precomp ge_Bi[8];
extern void ge_dsm_precomp(ge_dsmp r, const ge_p3 *s);
extern void ge_double_scalarmult_base_vartime(ge_p2 *, const unsigned char *, const ge_p3 *, const unsigned char *);
extern void ge_double_scalarmult_base_vartime_p3(ge_p3 *, const unsigned char *, const ge_p3 *, const unsigned char *);

/* From ge_frombytes.c, modified */

extern const fe fe_sqrtm1;
extern const fe fe_d;
extern int ge_frombytes_vartime(ge_p3 *, const unsigned char *);

/* From ge_p1p1_to_p2.c */

extern void ge_p1p1_to_p2(ge_p2 *, const ge_p1p1 *);

/* From ge_p1p1_to_p3.c */

extern void ge_p1p1_to_p3(ge_p3 *, const ge_p1p1 *);

/* From ge_p2_dbl.c */

extern void ge_p2_dbl(ge_p1p1 *, const ge_p2 *);

/* From ge_p3_to_cached.c */

extern const fe fe_d2;
extern void ge_p3_to_cached(ge_cached *, const ge_p3 *);

/* From ge_p3_to_p2.c */

extern void ge_p3_to_p2(ge_p2 *, const ge_p3 *);

/* From ge_p3_tobytes.c */

extern void ge_p3_tobytes(unsigned char *, const ge_p3 *);

/* From ge_scalarmult_base.c */

extern const ge_precomp ge_base[32][8];
extern void ge_scalarmult_base(ge_p3 *, const unsigned char *);

/* From ge_tobytes.c */

extern void ge_tobytes(unsigned char *, const ge_p2 *);

/* From sc_reduce.c */

extern void sc_reduce(unsigned char *);

/* New code */

extern void ge_scalarmult(ge_p2 *, const unsigned char *, const ge_p3 *);
extern void ge_scalarmult_p3(ge_p3 *, const unsigned char *, const ge_p3 *);
extern void ge_double_scalarmult_precomp_vartime(ge_p2 *, const unsigned char *, const ge_p3 *, const unsigned char *, const ge_dsmp);
extern void ge_double_scalarmult_precomp_vartime2(ge_p2 *, const unsigned char *, const ge_dsmp, const unsigned char *, const ge_dsmp);
extern void ge_double_scalarmult_precomp_vartime2_p3(ge_p3 *, const unsigned char *, const ge_dsmp, const unsigned char *, const ge_dsmp);
extern void ge_mul8(ge_p1p1 *, const ge_p2 *);
extern const fe fe_ma2;
extern const fe fe_ma;
extern const fe fe_fffb1;
extern const fe fe_fffb2;
extern const fe fe_fffb3;
extern const fe fe_fffb4;
extern const ge_p3 ge_p3_identity;
extern const ge_p3 ge_p3_H;
extern void ge_fromfe_frombytes_vartime(ge_p2 *, const unsigned char *);
extern void sc_0(unsigned char *);
extern void sc_reduce32(unsigned char *);
extern void sc_add(unsigned char *, const unsigned char *, const unsigned char *);
extern void sc_sub(unsigned char *, const unsigned char *, const unsigned char *);
extern void sc_mulsub(unsigned char *, const unsigned char *, const unsigned char *, const unsigned char *);
extern void sc_mul(unsigned char *, const unsigned char *, const unsigned char *);
extern void sc_muladd(unsigned char *s, const unsigned char *a, const unsigned char *b, const unsigned char *c);
extern int sc_check(const unsigned char *);
extern int sc_isnonzero(const unsigned char *); /* Doesn't normalize */

// internal
extern uint64_t load_3(const unsigned char *in);
extern uint64_t load_4(const unsigned char *in);
extern void ge_sub(ge_p1p1 *r, const ge_p3 *p, const ge_cached *q);
extern void fe_add(fe h, const fe f, const fe g);
extern void fe_tobytes(unsigned char *, const fe);
extern void fe_invert(fe out, const fe z);

extern int ge_p3_is_point_at_infinity(const ge_p3 *p);
#ifdef __cplusplus
} // extern "C"
#endif
#endif