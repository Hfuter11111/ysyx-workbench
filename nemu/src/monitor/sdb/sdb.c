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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT; //如果没有这一行，nemu_state还是被初始化的时候的NEMU_STOP,没有被赋值为NEMU_QUIT，is_exit_status_bad()这个函数没有匹配到END或quit状态，故main最终返回为1,而返回1就会报错（约定） 
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  int n = 1;
  if(args != NULL) {
    n = atoi(args);
    if (n <= 0) {
      printf("Usage: si N, N should be a positive integer\n"); //输入abc类似的字符串，atoi会返回0
      return 0;
    }
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  if((args != NULL) && (strcmp(args,"r") == 0)){
    isa_reg_display();
  }
  else if((args != NULL) && (strcmp(args, "w") == 0)) {
    display_watchpoints();
  }
  else {
    printf("Usage: info SUBCMD, SUBCMD should be 'r' or 'w'\n");
  }

  return 0;
}

static int cmd_x(char *args) {
  int n;
  int i;
  word_t expr_;
  char *N;
  char *EXPR;
  bool success = true;
  if(args == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  N = strtok(args, " ");
  EXPR = strtok(NULL, "");
  if (N == NULL || EXPR == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  while (*EXPR == ' ') EXPR++; // 把表达式前边的空格去掉

  if (*EXPR == '\0') {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  n = atoi(N);
  expr_ = expr(EXPR, &success);
  if (success == false) {
    printf("Bad expression: %s\n", EXPR);
    return 0;
  }
  //expr_ = strtoul(EXPR, NULL, 0);  // 自动识别十进制 八进制 十六进制
  if(n <= 0) {
    printf("Usage: x N EXPR, N should be a positive integer\n");
    return 0;
  }
  for(i = 0; i < n; i++){
    word_t addr = expr_ + i * 4;
    word_t data = vaddr_read(addr, 4);
    printf(FMT_PADDR ":    " FMT_WORD " \n", addr, data);
  }
  return 0;
}

static int cmd_p(char *args) {
  if(args == NULL) {
    printf("Usage: p EXPR\n");
    return 0;
  }

  bool success = true;
  word_t result = expr(args, &success);

  if(success == true) {
    printf("%s: " FMT_WORD "\n", args, result);
  } else {
    printf("Bad expression: %s\n", args);
  }
  return 0;
}

static int cmd_w(char *args) {
  if(args == NULL) {
    printf("Usage: w EXPR\n");
    return 0;
  }
  // 
  bool success = true;
  word_t result = expr(args, &success);

  if(success == true) {
    WP *wp = new_wp();
    wp->old_value = result; // 保存当前值
    strncpy(wp->expr_str, args, sizeof(wp->expr_str) - 1);
    wp->expr_str[sizeof(wp->expr_str) - 1] = '\0';

    printf("Watchpoint %d: %s = " FMT_WORD "\n",
         wp->NO, wp->expr_str, wp->old_value);
    
  } else {
    printf("Bad expression: %s\n", args);
  }
  return 0;
}

static int cmd_d(char *args) {
  if(args == NULL) {
    printf("Usage: d N\n");
    return 0;
  }
  char *endptr = NULL;
  int N = strtol(args, &endptr, 0);
  // 如果输入纯字母，则输出0,可能导致误删，根据endptr是否指向字符串开头来确定是否为这种错误情况
  if(endptr == args) {
    printf("Usage: d N\n");
    return 0;
  }
  // 在head链表中找NO=N的节点
  WP *wp = search_wp(N);
  if(wp == NULL) {
    printf("No watchpoint number %d\n", N);
    return 0;
  }
  free_wp(wp);
  printf("Delete watchpoint %d\n", N);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Single instruction N", cmd_si},
  {"info", "Display program status", cmd_info},
  {"x", "Examine memory", cmd_x},
  {"p", "Expression evaluation", cmd_p},
  {"w", "Set watchpoint", cmd_w},
  {"d", "Delect watchpoint",cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }                                          //没有输入参数的话，args要比str_end大1,故给args赋NULL

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
