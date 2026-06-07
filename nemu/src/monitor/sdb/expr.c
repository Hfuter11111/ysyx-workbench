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
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>


enum {
  TK_NOTYPE = 256,  TK_DEC, TK_HEX, TK_REG, TK_EQ, TK_NEQ, TK_AND, TK_DEREF

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
  {"\\*", '*'},         // mul或指针解引用，make_token中再做区分  *在正则表达式里有特殊含义
  {"/", '/'},           // div  
  {"\\(", '('},         // '('  有特殊含义
  {"\\)",')'},          //')'
  {"0[xX][0-9a-fA-F]+u?", TK_HEX}, // 十六进制整数，十六进制必须放在十进制前边否则例如0x10,会被识别成十进制x10
  {"[0-9]+u?", TK_DEC}, // 十进制整数，由于表达式生成器要保证无符号数运算(加上u表示常量是无符号数)，所以正则要可以识别u，
                                               // 同时直接用strtoul(tokens[p].str, NULL, 10);因为从字符串开头开始，尽可能多地解析合法数字字符；一旦遇到当前进制下不合法的字符，就停止转换。
  {"\\$[a-zA-Z0-9]+", TK_REG}, // 寄存器
  {"==", TK_EQ},       // equal
  {"!=", TK_NEQ},      // no equal
  {"&&", TK_AND},      // and

  
  
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
static Token tokens[65536] __attribute__((used)) = {};

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
          // 十进制，十六进制和寄存器区分token可共用
          case TK_DEC:
          case TK_HEX:
          case TK_REG:
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
    case TK_AND:
      return 1;
    case TK_EQ:
    case TK_NEQ:
      return 2;
    case '+': 
    case '-':
      return 3;
    case '*':
    case '/':
      return 4;
    default:
      return 0; //非双目运算符，TK_DEREF也为0,一元和二元在求值函数里是分开的，优先考虑二元为主运算符，因为二元优先级低
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
    bool reg_success = true;
    word_t reg_result;
    switch (tokens[p].type) {
      case TK_DEC:
      case TK_HEX:
        return strtoul(tokens[p].str, NULL, 0); //自动识别
      case TK_REG:
        reg_result = isa_reg_str2val(tokens[p].str + 1, &reg_success); 
        if(reg_success == true) {
          return reg_result;
        } else {
          *success = false;
          return 0;
        }
      default:
        *success = false;
        return 0;
    }
  }

  else if(check_parentheses(p, q, success) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  }
  
  else {
    int op;
    word_t val1, val2;
    // 如果check_parentheses()里检查出匹配错误，那么此时success是false，直接返回0
    if(*success == false) {
      return 0;
    }
    op = find_op(p, q);
    if(op !=-1) {
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
            printf("Division by zero detected! val1=%u, val2=%u\n", val1, val2);
            *success = false;
            return 0;
          }
          return val1 / val2;
        case TK_EQ:
          return val1 == val2;
        case TK_NEQ:
          return val1 != val2;
        case TK_AND:
          return val1 && val2;
        default: 
          *success = false;
          return 0;
      }
    }

    // 找不到双目运算符，再处理单目指针解引用(右结合)
    if(tokens[p].type == TK_DEREF) {
      word_t addr = eval(p + 1, q, success);
      if(*success == false)
        return 0;
      return vaddr_read(addr, 4);
    }

    // 既不是双目表达式，也不是指针解引用表达式，说明非法
    *success = false;
    return 0;
  }
}

word_t expr(char *e, bool *success) {
  *success = true;

  if(!make_token(e)) {
    *success = false;
    return 0;
  }

  if(!nr_token) {
    *success = false;
    return 0;
  }
  // 区分该token为*还是指针解引用，如果前边不是一个完整表达式则为指针解引用，如果前边是一个完整表达式则为*，前边token是一个十六进制或十进制或寄存器值或右括号则为完整表达式
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*') {
      if (i == 0 ||
          !(tokens[i - 1].type == TK_DEC ||
            tokens[i - 1].type == TK_HEX ||
            tokens[i - 1].type == TK_REG ||
            tokens[i - 1].type == ')')) {
        tokens[i].type = TK_DEREF;
      }
    }
  }

  return eval(0, nr_token - 1, success);

}
