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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

word_t expr(char *e, bool *success);

// Watchpoint 相关声明
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char expr_str[256];   // 保存监视的表达式
  word_t old_value;     // 保存上一次表达式求值结果，用于与新值比较

} WP;

void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);
WP* search_wp(int no);
bool check_watchpoints();
void display_watchpoints();
#endif
