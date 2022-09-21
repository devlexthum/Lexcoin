Copyright (c) 2017-2022 The Lex Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CURVE25519_REF10_H
#define CURVE25519_REF10_H

int crypto_scalarmult_base_ref10(unsigned char *q,const unsigned char *n);
int crypto_scalarmult_ref10(unsigned char *q, const unsigned char *n, const unsigned char *p);

#endif /* CURVE25519_REF10_H */

