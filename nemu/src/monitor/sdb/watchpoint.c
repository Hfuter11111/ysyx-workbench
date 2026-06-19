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

#include "sdb.h"
#include <assert.h>

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  // 如果已无空闲监视点结构，那么直接终止程序
    Assert(free_, "No free watchpoint is available.\n");
  // 将空闲监视点结构从free_链表中摘下
  WP *wp = free_;
  free_ = free_->next;
  // 插入到head链表中
  wp->next = head;
  head = wp;
  return wp;
}

void free_wp(WP *wp) {
  Assert(wp,"wp is NULL\n");

  bool found = false;

  // 如果wp是head节点
  if(wp == head) {
    head = wp->next;
    found = true;
  } else {
    WP *pre; // 前驱节点

    for(pre = head; pre != NULL && pre->next != NULL; pre = pre->next) {
      if(pre->next == wp) {
        // 将wp从head链表中摘下
        pre->next = wp->next;
        found = true;
        break;
      }
    }
  }

  // 如果没找到，说明传入的wp不在正在使用的监视点链表中
  Assert(found, "watchpoint is not in the active list\n");

  // 将wp归还到free_链表
  wp->next = free_;
  free_ = wp;
}

WP* search_wp(int no){
  WP *wp;
  for(wp = head; wp; wp = wp->next) {
    if(wp->NO == no) {
      return wp;
    }
  }
  return  NULL;
}

bool check_watchpoints() {
  bool triggered = false;
  WP *wp;
  for(wp = head; wp; wp = wp->next) {
    bool success = true;
    word_t new_value = expr(wp->expr_str, &success);
    // 如果表达式求值有错误，程序停止
    if(success == false) {
      printf("Bad expression: %s\n", wp->expr_str);
      triggered = true;
      continue;
    }

    if(new_value != wp->old_value) {
      printf("Watchpoint %d triggered\n", wp->NO);
      printf("expr: %s\n", wp->expr_str);
      printf("old value = " FMT_WORD "\n", wp->old_value);
      printf("new value = " FMT_WORD "\n", new_value);

      wp->old_value = new_value;
      triggered = true;
    }
  }

  return triggered;
}

void display_watchpoints() {
  // head链表里无监视点
  if(head == NULL) {
    printf("No Watchpoints\n");
    return;
  }

  WP *wp;
  printf("Num\tType\t\tWhat\tValue\n");
  for(wp = head; wp; wp = wp->next) {
    printf("%d\tWatchpoint\t%s\t" FMT_WORD "\n", 
      wp->NO, wp->expr_str, wp->old_value);
  }
}
