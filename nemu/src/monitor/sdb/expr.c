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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>


enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus '\\'在c语言里对应‘\’，/+在正则表达式里对应’+‘
  {"-", '-'},           // sub
  {"\\*", '*'},         // mul  在正则表达式里有特殊含义
  {"/", '/'},           // div  
  {"\\(", '('},         // '('  有特殊含义
  {"\\)",')'},          //')'
  {"[0-9]+", TK_NUM},   // 十进制整数
  {"==", TK_EQ},        // equal
  
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

// 按顺序存放已经被识别出的token信息
static Token tokens[32] __attribute__((used)) = {};

// 已经被识别出的token数目
static int nr_token __attribute__((used))  = 0;

// bool返回值为布尔类型，只能返回false或true
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch; // regmatch_t有两个成员：rm_so和rm_eo,其中rm_so表示匹配字符串的起始位置，rm_eo表示匹配结束后下一个字符的位置

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      // regexec匹配成功返回0
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start); //%.*s 从substr_start开始打印substr_len个字符

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: 
            break;
          case TK_NUM:
            if(nr_token >= ARRLEN(tokens)) {
              printf("too many tokens\n");
              return false;
            }

            if(substr_len >= sizeof(tokens[nr_token].str)) {
              printf("token too long: %.*s\n", substr_len, substr_start);
              return false;
            }

            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          default:  
            if(nr_token >= ARRLEN(tokens)) {
              printf("too many tokens\n");
              return false;
            }          
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true; 
}

static bool check_parentheses(int p, int q, bool *success) {
  int i;
  int match = 0;
  *success = true;
  //检查表达式是否合法（括号是否匹配）
  for(i = p; i <= q; i++){
    // 出现一个左括号，match加1
    if(tokens[i].type == '(') {
      match++;
    }
    // 出现一个右括号，match减1
    else if(tokens[i].type == ')') {
      match--;
      // 小于0说明右括号多了
      if(match < 0) {
        *success = false;
        return false;
      }
    }
  }
  // 大于0说明左括号多了
  if(match != 0) {
    *success = false;
    return false;
  }

  // 判断是否符合 "(" <expr> ")" 规定的表达式形式，这*success = true 表达式是合法的但不一定满足规定的形式
  if(tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  }

  // 判断首尾括号是否匹配
  for(i = p, match = 0; i <= q; i++){
    if(tokens[i].type == '(') {
      match++;
    }
    // 如果该token为')'且match等于0且该token位置不是最右边则说明首尾括号不匹配
    else if(tokens[i].type == ')') {
      match--;
      if(match == 0 && i != q) {
        return false;
      }
    }
  }    

  return true;
}

// 用来确定token的优先级
static int get_priority(int type) {
  switch(type) {
    case '+': 
    case '-':
      return 1;
    case '*':
    case '/':
      return 2;
    default:
      return 0;
  }
}

int find_op(int p, int q) {
  int i;
  int match = 0;
  int pr;
  int pr_min = 100;
  int op = -1;
  for(i = p; i <= q; i++) {
    // 不考虑括号里的token
    if(tokens[i].type == '(') {
      match++;
    }
    else if(tokens[i].type == ')') {
      match--;
    }
    // pr为优先级，非运算符优先级为0,不考虑非运算符
    if(match == 0) {
      pr = get_priority(tokens[i].type);
      if(pr != 0 && pr <= pr_min) {
        pr_min = pr;
        op = i;
      }
    }
  }
  return op;
}

word_t eval(int p, int q, bool *success) {
  if (p > q) {
    /* Bad expression */
    *success = false;
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if(tokens[p].type != TK_NUM) {
      *success = false;
      return 0;
    }
    return strtoul(tokens[p].str, NULL, 10);

  }
  else if (check_parentheses(p, q, success) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  }
  else {
    int op;
    word_t val1, val2;
    // 如果check_parentheses()里检查出匹配错误，那么此时success是false，之间返回0
    if(*success == false) {
      return 0;
    }
    op = find_op(p, q);
    if(op == -1) {
      *success = false;
      return 0;
    }
    val1 = eval(p, op - 1, success);
    // 如果val1出错就不再计算val2
    if(*success == false)
      return 0;
    val2 = eval(op + 1, q, success);
    // 如果val2出错直接返回
    if(*success == false)
      return 0;

    switch (tokens[op].type) {
      case '+': 
        return val1 + val2;
      case '-': 
        return val1 - val2;
      case '*': 
        return val1 * val2;
      case '/': 
        // 避免除数为0
        if(val2 == 0) {
          *success = false;
          return 0;
        }
        return val1 / val2;
      default: 
        *success = false;
        return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if(!make_token(e)) {
    *success = false;
    return 0;
  }

  if(!nr_token) {
    *success = false;
    return 0;
  }

  return eval(0, nr_token - 1, success);

}
