#ifndef _BASE85_INCLUDE_H_
#define _BASE85_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif
	
int base85_encode( unsigned char *dst, int *dlen, unsigned char *src, int slen );
int base85_decode( unsigned char *dst, int *dlen, unsigned char *src, int slen );

#ifdef __cplusplus
}
#endif
#endif

