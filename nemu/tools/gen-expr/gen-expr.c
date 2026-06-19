/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static int buf_index = 0;
// %s用buf替代，%%，因为%u有特殊含义，\"代表"
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static uint32_t choose(uint32_t n) {
  return rand() % n;
}

static int get_remain() {
  return sizeof(buf) - buf_index;
}

void gen(char c) {
  if(buf_index < sizeof(buf) - 1) {
    buf[buf_index] = c;
    buf_index++;
    buf[buf_index] = '\0';
  }
}

// 用于产生空格，随机产生0～3个空格
void gen_space() {
  int n = choose(4);
  int i;
  for(i = 0; i < n; i++) {
    gen(' ');
  }
}

// 如果生成的数超过剩余空间则不更新 buf_index
void gen_num() {
  uint32_t num = choose(100);
  int len = snprintf(buf+buf_index, get_remain(), "%uu", num); // 自动加'\0'，返回值为格式化后字符串长度，%uu 存入的是[0-9]+u,u表示该常量为无符号数，
  if(len > 0 && len < get_remain()) {                                            // 有符号数溢出为未定义行为，无符号溢出会回绕
    buf_index += len;
  }
}
// 用于生成一个非零数用作除数
void gen_nonzero_num() {
  uint32_t num = choose(99) + 1;  // 1~99，保证不是0
  int len = snprintf(buf + buf_index, get_remain(), "%uu", num);

  if(len > 0 && len < get_remain()) {
    buf_index += len;
  }
}

static char gen_rand_op() {
  switch (choose(4)) {
    case 0: return '+';
    case 1: return '-';
    case 2: return '*';
    default: return '/';
  }
}

// 防止长表达式溢出，get_remain() <= 100 则强制生成一个数字并终止递归，必须加gen_num()不然可能会产生非法表达式，限制表达式生成深度，最多20
static void gen_rand_expr(int depth) {
  char op;
  if(get_remain() <= 100 || depth >= 20) {
    gen_num();
    return;
  } 
  switch (choose(3)) {
    case 0: 
      gen_space(); 
      gen_num();
      gen_space();
      break;
    case 1: 
      gen_space();
      gen('('); 
      gen_space();
      gen_rand_expr(depth + 1); 
      gen_space();
      gen(')'); 
      gen_space();
      break;
    default: 
      gen_space();
      gen_rand_expr(depth + 1);
      gen_space();
      op = gen_rand_op();
      gen(op);
      gen_space();
      // 如果遇到除号那么右边表达式就不再生成复杂表达式(避免除数为0)而是生成一个非零数1作除数
      if(op != '/')
        gen_rand_expr(depth + 1);
      else 
        gen_nonzero_num();
      gen_space();
      break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf_index = 0;
    buf[0] = '\0';
    gen_rand_expr(0);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
    
    // 编译成功返回0,编译失败则跳过这个表达式
    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) {
      continue;
    }

    fp = popen("/tmp/.expr", "r"); // 将程序的标准输出接到管道里并读取内容
    assert(fp != NULL);

    unsigned result;
    int status;
    // fscanf返回读入的个数，正常运行应该是1,如果出现除0,ret就不为1
    ret = fscanf(fp, "%u", &result);
    // pclose会关闭管道并等待子进程完成，如果子进程正常完成则返回0,否则非0,如果出现除0,则子进程非正常完成
    status = pclose(fp);
    // 如果出现除0跳过
    if(ret != 1 || status != 0) {
      continue;
    }
    printf("%u %s\n", result, buf);
  }
  return 0;
}
