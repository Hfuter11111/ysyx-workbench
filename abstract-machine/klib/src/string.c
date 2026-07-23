#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while(s[len]) {
    len++;
  }

  return len;
}

char *strcpy(char *dst, const char *src) {
  char *result = dst;

  while ((*dst++ = *src++) != '\0') {
  }

  return result;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i = 0;
  while(i < n && src[i]) {
    dst[i] = src[i];
    i++;
  }

  while(i < n) {
    dst[i] = '\0';
    i++;
  }

  return dst;
}

char *strcat(char *dst, const char *src) {
  char *result = dst;

  while(*dst != '\0') {
    dst++;
  }

  while((*dst++ = *src++) != '\0');

  return result;  
}

int strcmp(const char *s1, const char *s2) {
  size_t i = 0;
  while(s1[i] != '\0' && s1[i] == s2[i]) {
    i++;
  }

  return (unsigned char)s1[i] - (unsigned char)s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  while(i < n && s1[i] != '\0' && s1[i] == s2[i]) {
    i++;
  }

  if (i == n) {
    return 0;
  }

  return (int)((unsigned char)s1[i] - (unsigned char)s2[i]);
} 

void *memset(void *s, int c, size_t n) {
  size_t i = 0;
  unsigned char *s_ = (unsigned char *)s;
  unsigned char value = (unsigned char)c;
  while(i < n) {
    s_[i] = value;
    i++;
  }

  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  if(d == s || n == 0) {
    return dst;
  }

  if(d < s) {
    size_t i = 0;
    while(i < n) {
      d[i] = s[i];
      i++;
    }
  } else {
    size_t i = n;
    while(i > 0) {
      i--;
      d[i] = s[i]; 
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t i = 0;
  unsigned char *dest = (unsigned char *)out;
  const unsigned char *src = (const unsigned char *)in;
  while(i < n) {
    dest[i] = src[i];
    i++;
  }

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i = 0;
  const unsigned char *s1_ = (const unsigned char *)s1;
  const unsigned char *s2_ = (const unsigned char *)s2;
  while(i < n && s1_[i] == s2_[i]) {
    i++;
  }

  if(i == n) {
    return 0;
  }

  return (int)(s1_[i] - s2_[i]);

}

#endif
