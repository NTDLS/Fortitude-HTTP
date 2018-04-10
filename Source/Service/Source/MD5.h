#ifndef _MD5_H
#define _MD5_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	MD5.H - header file for MD5C.C

	Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
	rights reserved.

	License to copy and use this software is granted provided that it
	is identified as the "RSA Data Security, Inc. MD5 Message-Digest
	Algorithm" in all material mentioning or referencing this software
	or this function.

	License is also granted to make and use derivative works provided
	that such works are identified as "derived from the RSA Data
	Security, Inc. MD5 Message-Digest Algorithm" in all material
	mentioning or referencing the derived work.

	RSA Data Security, Inc. makes no representations concerning either
	the merchantability of this software or the suitability of this
	software for any particular purpose. It is provided "as is"
	without express or implied warranty of any kind.

	These notices must be retained in any copies of any part of this
	documentation and/or software.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <limits.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* MD5 context. */
typedef struct {
	unsigned int state[4];	  // State (ABCD).
	unsigned int count[2];    // Number of bits, modulo 2^64 (lsb first).
	unsigned char buffer[64]; // Input buffer.
} MD5_CTX;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MD5_Init (MD5_CTX *);
void MD5_Update (MD5_CTX *, unsigned char *, unsigned int);
void MD5_Final (unsigned char [16], MD5_CTX *);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
