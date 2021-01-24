/*
+++ update of common.h file (from HPNN - libhpnn) +++
    Copyright (C) 2019  Okadome Valencia Hubert

    common.h is a free header: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    common.h is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef COMMON_H
#define COMMON_H
#ifdef USE_GLIB
#include <glib.h>
#include <glib/gstdio.h>
#else
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#endif //USE_GLIB

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
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)) \
    || (defined(__cplusplus) && (__cplusplus >= 201103))
#define FUNCTION __func__
#elif defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)
#define FUNCTION __FUNCTION__
#elif defined(__FUNCSIG__)
#define FUNCTION __FUNCSIG__
#else
#define FUNCTION "???"
#endif
#define PREP_READLINE() size_t _readline_len=0
#define READLINE(fp,buffer) do{\
    ssize_t _read_count=0;\
    _read_count=getline(&buffer,&_readline_len,fp);\
    if(_read_count==-1) fprintf(stderr,"READ ERROR: getline failed (function %s, line %i)\n",FUNCTION,__LINE__);\
}while(0)
#define QUOTE(a) #a
#define QUOTE2(a,b) TH_QUOTE(a ## b)
#define TINY 1E-14
/*outputs*/
#ifdef _MPI
#define _OUT(_file,...) do{\
    int _rank;\
    MPI_Comm_rank(MPI_COMM_WORLD,&_rank);\
    if(_rank==0) fprintf((_file), __VA_ARGS__);\
}while(0)
#else /*_MPI*/
#define _OUT(_file,...) do{\
    fprintf((_file), __VA_ARGS__);\
}while(0)
#endif /*_MPI*/
/*USING GLIB?*/
#ifdef USE_GLIB
#define DIR_S GDir
#define CHAR gchar
#define UCHAR guchar
#define SHORT gshort
#define UINT guint
#define UINT64 guint64
#define DOUBLE gdouble
#define BOOL gboolean
#define STRFIND(a,b) g_strstr(b,a)
#define ISDIGIT g_ascii_isdigit
#define ISGRAPH g_ascii_isgraph
#define ISSPACE g_ascii_isspace
#define STR2ULL g_ascii_strtoull
#define STR2D g_ascii_strtod
#define ALLOC(pointer,size,type) do{\
    pointer=g_malloc0((size)*sizeof(type));\
    if(pointer==NULL) {\
        _OUT(stderr,"Alloc error (function %s, line %i)\n",FUNCTION,__LINE__);\
        exit(-1);\
    }\
}while(0)
#define FREE(pointer) do{\
    g_free(pointer);\
    pointer=NULL;\
}while(0)
#define STRDUP(src,dest) do{\
    dest=g_strdup(str);\
}while(0)
#define STRDUP_REPORT(src,dest,mem) do{\
    UINT __len=0;\
    STRLEN(src,__len);\
    STRDUP(src,dest);\
    mem+=(__len)*sizeof(CHAR);\
}while(0)
#define STRCAT(dest,src1,src2) do{\
    dest=g_strconcat(src1,src2,NULL);\
}while(0)
/*files*/
#define GET_CWD(cwd) do{\
    cwd=g_get_current_dir(void);\
}while(0)
#define OPEN_DIR(dir,path) do{\
    dir=g_dir_open(path,0,NULL);\
}while(0)
#define FILE_FROM_DIR(dir,file) do{\
    file=g_dir_read_name(dir);\
}while(0)
#define CLOSE_DIR(dir,ok) do{\
    g_dir_close(dir);\
    ok=0;\
}while(0)

#else /*USE_GLIB*/
#define DIR_S DIR
#define CHAR char
#define UCHAR unsigned char
#define SHORT short
#define UINT unsigned int
#define UINT64 uint64_t
#define DOUBLE double
#define BOOL int
#define STRFIND(a,b) strstr(b,a)
#define ISDIGIT(a) isdigit(a)
#define ISGRAPH(a) isgraph(a)
#define ISSPACE(a) isspace(a)
#define STR2ULL strtoull
#define STR2D strtod
#define ALLOC(pointer,size,type) do{\
    pointer=(type *)calloc((size),sizeof(type));\
    if(pointer==NULL) {\
        _OUT(stderr,"Alloc error (function %s, line %i)\n",FUNCTION,__LINE__);\
        exit(-1);\
    }\
}while(0)
#define FREE(pointer) do{\
    if(pointer!=NULL) free(pointer);\
    pointer=NULL;\
}while(0)
#define TRUE (1==1)
#define FALSE (1==0)
#define STRLEN(src,len) while((len<UINT_LEAST32_MAX)&&(src[len]!='\0')) len++
/*the len+1 in line 4 is a place for '\0' similarily to strcpy*/
#define STRDUP(src,dest) do{\
    UINT __len=0;\
    STRLEN(src,__len);\
    if(__len!=0){\
        ALLOC(dest,__len+1,CHAR);\
        __len--;\
        while(__len>0) {\
            dest[__len]=src[__len];\
            __len--;\
        }\
        dest[0]=src[0];\
    } else {\
        dest=NULL;\
    }\
}while(0)
#define STRDUP_REPORT(src,dest,mem) do{\
    UINT __len=0;\
    STRLEN(src,__len);\
    if(__len!=0){\
        ALLOC(dest,__len+1,CHAR);\
        mem+=(__len)*sizeof(CHAR);\
        __len--;\
        while(__len>0) {\
            dest[__len]=src[__len];\
            __len--;\
        }\
        dest[0]=src[0];\
    } else {\
        dest=NULL;\
    }\
}while(0)
#define STRCAT(dest,src1,src2) do{\
    UINT __acc=0,__len=0;\
    STRLEN(src1,__acc);\
    STRLEN(src2,__len);\
    __acc+=__len;\
    if((__acc>0)&&(__len>0)){\
        ALLOC(dest,__acc+1,CHAR);\
        dest[0]='\0';\
        strcat(dest,src1);\
        strcat(dest,src2);\
    } else {\
        dest=NULL;\
    }\
}while(0)


#define GET_CWD(cwd) do{\
    cwd=getcwd(NULL,0);\
}while(0)
#define OPEN_DIR(dir,path) do{\
        dir=opendir(path);\
}while(0)
#define FILE_FROM_DIR(dir,file) do{\
    struct dirent *_entry;\
    _entry=readdir(dir);\
    if(_entry==NULL) file=NULL;\
    else {\
        STRDUP(_entry->d_name,file);\
    }\
}while(0)
#define CLOSE_DIR(dir,ok) do{\
    ok=closedir(dir);\
}while(0)
#endif //USE_GLIB
/**/


/*report memory usage*/
#define ALLOC_REPORT(pointer,size,type,mem) do{\
    ALLOC(pointer,size,type);\
    mem+=(size)*sizeof(type);\
}while(0)
/*useful*/
#define SKIP_BLANK(pointer) \
    while((!ISGRAPH(*pointer))&&(*pointer!='\n')&&(*pointer!='\0')) pointer++
#define SKIP_NUM(pointer) \
    while((ISDIGIT(*pointer))&&(*pointer!='\n')&&(*pointer!='\0')) pointer++
#define STR_CLEAN(pointer) do{\
    CHAR *_ptr=pointer;\
    while(*_ptr!='\0'){\
        if(*_ptr=='\t') *_ptr='\0';\
        if(*_ptr==' ') *_ptr='\0';\
        if((*_ptr=='\n')||(*_ptr=='#')) *_ptr='\0';\
        else _ptr++;\
    }\
}while(0)
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
#define ARRAY_CP(src,dest,n) do{\
    if(src!=NULL){\
        UINT _i;\
        for(_i=0;_i<(n);_i++) dest[_i]=src[_i];\
    }\
}while(0)

#define ASSERTPTR(pointer,retval) do{\
    if(pointer==NULL){\
        _OUT(stderr,"Error: NULL pointer (function %s, line %i):\n%s=NULL\n",\
            FUNCTION,__LINE__,QUOTE(pointer));\
        return retval;\
    }\
}while(0)

#define ASSERT_GOTO(pointer,label) do{\
    if(pointer==NULL){\
        _OUT(stderr,"Error: NULL pointer (function %s, line %i):\n%s=NULL\n",\
            FUNCTION,__LINE__,QUOTE(pointer));\
        goto label;\
    }\
}while(0)

#define ASSERT_READ(ptr,ptr2,label,error) do{\
        if(ptr==ptr2){\
                fprintf(stderr,"READ ERROR: %s\n",error);\
                fprintf(stderr,"-- in function %s, line %i \n",FUNCTION,__LINE__);\
                goto label;\
        }\
}while(0)

#endif//COMMON_H
