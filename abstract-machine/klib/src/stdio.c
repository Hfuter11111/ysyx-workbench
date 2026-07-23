#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  int i = 0;
  va_list vp;                             // 保存可变参数的访问状态
  va_start(vp, fmt);                      // 使vp指向第一个参数
  while(*fmt != '\0') {
    if(*fmt != '%') {                     // 判断是否为%，不是则直接赋，是则格式化处理
      out[i++] = *fmt;
      fmt++;
    } else {
      fmt++;
      switch(*fmt) {
        case 's': {
          const char *str = va_arg(vp, char *);   
          while(*str != '\0') {
            out[i++] = *str++;
          }
          break;
        }
        case 'd': {
          int num = va_arg(vp, int);
          char temp[16];                    // 4字节有符号整形可表示的十进制为十位数，还需考虑负号，11位
          int len = 0;
          if(num < 0) {                     // 负数转化为正数，并加入负号
            num = -num;
            out[i++] = '-';
          }
          if(num == 0) {                    // 为0则直接给'0'
            out[i++] = '0';
          }
          while(num > 0) {                  // 由于我们先求出来的是个十百，而填入则是百十个，故需要暂存
            temp[len++] = '0' + num % 10;   // 从字符'0'开始加
            num /= 10;
          }
          while(len > 0) {
            out[i++] = temp[--len];         // 倒序放入
          }
          break;
        }
      }
      fmt++;                                // 接着处理下一个
    }
  }
  out[i] = '\0';                           // sprintf自动补'\0'
  va_end(vp);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
