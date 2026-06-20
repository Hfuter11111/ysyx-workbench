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

#include <common.h>
#include <stdio.h>
#include <threads.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char *e, bool *success);

void test_expr(char *filename) {
  int total = 0; // 记录测试总数目
  int pass = 0; // 通过总数目
  char *endptr;
  word_t expected, result;

  FILE *fp = fopen(filename, "r");
  Assert(fp, "Can not open '%s'", filename);

  char buf[65536] = {}; // 用来存储每一行的字符
  while(fgets(buf, sizeof(buf), fp)) {
    bool success = true;
    // 去掉换行符
    char *p = strchr(buf, '\n');
      if (p != NULL) {
      *p = '\0';
    }
    // 跳过空行
    if (buf[0] == '\0') {
      continue;
    }
    total++;
    expected = strtoul(buf, &endptr, 10); // 保存结果,endptr指向结果后面未被识别的第一个字符
    // 去空格
    while(*endptr == ' ') {
      endptr++;
    }
    result = expr(endptr, &success);

    if(result != expected || success == false) {
      printf("line %d failed\n", total);
      printf("expr: %s\n", endptr);
      printf("expected = %u\n", expected);
      printf("result   = %u\n", result);
      assert(0);
    }

    pass++;
  }
  fclose(fp);
  printf("expr test passed: %d/%d\n", pass, total);
}

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
/* 
  test_expr("input");
  return 0;
*/
  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
