#ifndef COMMON_H
#define COMMON_H
#ifdef USE_GLIB
#include <glib.h>
#else
#include <string.h>
#include <ctype.h>
#endif //USE_GLIB
#include <math.h>

/* A header for the common useful C functions, and
 * most used defines.
 *
 * Linux users can import common.h as usual, there
 * should be no problem doing:
 * #include "common.h"
 * For portability, a version can also be found of
 * common.h defines using glib.h
 * In such cases, prior to importing common.h, the
 * users are required to #define USE_GLIB for pre-
 * compiler:
 * #define USE_GLIB
 * #include "common.h"
 *
 * ------------------- (c) OVHPA: Okadome Valencia
 * mail: hubert.valencia _at_ imass.nagoya-u.ac.jp */

/*defines*/
#if defined(__GNUC__) || (defined(__ICC) && (__ICC >= 600))
#define FUNCTION __PRETTY_FUNCTION__
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)) || (defined(__cplusplus) && (__cplusplus >= 201103))
#define FUNCTION __func__
#elif defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)
#define FUNCTION __FUNCTION__
#elif defined(__FUNCSIG__)
#define FUNCTION __FUNCSIG__
#else
#define FUNCTION "???"
#endif
#define READLINE(fp,buffer) getline(&buffer,&len,fp)
#ifdef USE_GLIB
#define CHAR gchar
#define UINT g_uint
#define DOUBLE gdouble
#define BOOL gboolean
#define STRFIND(a,b) g_strstr(b,a)
#define ISDIGIT g_ascii_isdigit
#define ISGRAPH g_ascii_isgraph
#define STR2ULL g_ascii_strtoull
#define STR2D g_ascii_strtod
#define ALLOC(pointer,size,type) do{\
	pointer=g_malloc0((size)*sizeof(type));\
	if(pointer==NULL) {\
		fprintf(stderr,"Allocation error (function %s, line %i)\n",FUNCTION,__LINE__);\
		exit(-1);\
	}\
}while(0)
#define FREE(pointer) g_free(pointer)
#else
#define CHAR char
#define UINT unsigned int
#define DOUBLE double
#define BOOL int
#define STRFIND(a,b) strstr(b,a)
#define ISDIGIT(a) isdigit(a)
#define ISGRAPH(a) isgraph(a)
#define STR2ULL strtoull
#define STR2D strtod
#define ALLOC(pointer,size,type) do{\
	pointer=calloc((size),sizeof(type));\
	if(pointer==NULL) {\
		fprintf(stderr,"Allocation error (function %s, line %i)\n",FUNCTION,__LINE__);\
		exit(-1);\
	}\
}while(0)
#define FREE(pointer) free(pointer)
#define TRUE (1==1)
#define FALSE (1==0)
#endif //USE_GLIB

/*useful*/
#define SKIP_BLANK(pointer) while(!ISGRAPH(*pointer)) pointer++
#define SKIP_NUM(pointer) while(ISDIGIT(*pointer)) pointer++
#define GET_LAST_LINE(fp,buffer) do{\
	fseek(fp,-2,SEEK_END);\
	while(fgetc(fp)!='\n') fseek(fp,-2,SEEK_CUR);\
	fseek(fp,+1,SEEK_CUR);\
	READLINE(fp,buffer);\
}while(0)
#define GET_UINT(i,in,out) do{\
	i=(UINT)STR2ULL(in,&(out),10);\
}while(0)
#define GET_DOUBLE(d,in,out) do{\
	d=(DOUBLE)STR2D(in,&(out));\
}while(0)

/*debug*/
//#define _DEB_

#endif//COMMON_H
