# EBNF语法描述
- 支持赋值，条件判断，分支循环，函数调用（采用值传递方式）
```EBNF
<program> = "program" <ident> "{" { <func_def> } "main" "{" <stmt_list> "}" "}" 
<func_def> = "func" <ident> "(" [ <param_list> ] ")" "{" <stmt_list> "return" <expr> ";" "}" 
<param_list> = <ident> { "," <ident> } 
<stmt_list> = <stmt> ";" { <stmt> ";" }     
<stmt> = <declare_stmt> | <assign_stmt> | <if_stmt> | <while_stmt> | <input_stmt> | <output_stmt> 
| <func_call> 
<declare_stmt> = "let" <ident> [ "=" <expr> ] 
<assign_stmt> = <ident> "=" <expr> 
<if_stmt> = "if" "(" <bool_expr> ")" "{" <stmt_list> "}" [ "else" "{" <stmt_list> "}" ] 
<while_stmt> = "while" "(" <bool_expr> ")" "{" <stmt_list> "}" 
<func_call> = <ident> "(" [ <arg_list> ] ")" 
<arg_list> = <expr> { "," <expr> } 
<input_stmt> = "input" "(" <ident> { "," <ident> } ")" 
<output_stmt> = "output" "(" <expr> { "," <expr> } ")" 
<bool_expr> = <expr> ("==" | "!=" | "<" | "<=" | ">" | ">=") <expr> 
<expr> = [ "+" | "-" ] <term> { ("+" | "-") <term> } 
<term> = <factor> { ("*" | "/") <factor> } 
<factor> = <ident> | <number> | "(" <expr> ")" | <func_call> 
<ident> = <letter> { <letter> | <digit> } 
<number> = <digit> { <digit> } 
<letter> = "a" | "b" | ... | "z" | "A" | "B" | ... | "Z" 
<digit> = "0" | "1" | ... | "9"
```
# 具体实现
- 采用类pl0Compiler方式
- 文件描述
  - main.c为编译程序的主入口
  - lexer.c进行词法分析
  - block.c进行自顶向下的语法分析，并结合code.c生成中间代码
  - code.c:生成中间代码
  - interprete.c：栈式解释执行
  - common.h：定义的资源常量
