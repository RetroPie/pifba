#ifndef _GP2X_MEM_FUNCS_
#define _GP2X_MEM_FUNCS_

#ifdef __cplusplus
extern "C" {
#endif
extern void *gp2x_memcpy(void *s1, const void *s2, size_t n);
//extern void *gp2x_memset(void *s, int c, size_t n);
#define gp2x_memset memset
#ifdef __cplusplus
} /* End of extern "C" */
#endif
#endif
