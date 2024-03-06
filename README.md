# Staisp

## 关于 Staisp

Staisp（STAtic lISP），即静态语法表，用于用一种相比于 IR 更高级的语言直接编写语法树。

```
DEFVAR i32:n 1

DEFFUNC i32:fib ( CONST i32:n ) BLOCK {
    IF OR EQ n 1
          EQ n 2
        RETURN 1
    RETURN ADD fib ( SUB n 1 )
               fib ( SUB n 2 )
}

DEFFUNC i32:main ( ) {
    DEFVAR i32:ans 0
    ASSIGN n 10
    WHILE ULE n 10
        ASSIGN n ADD n 1
    BLOCK {
        ASSIGN ans fib ( n )
    }
    RETURN ans
}
```

### 关键字

#### A 组：基本的数据操作与数据类型

* `DEFVAR`
* `ASSIGN`
* `i1|i8|i16|i32|i64`

#### B 组：控制流

* `IF`
* `IFE`
* `WHILE`
* `RETURN`
* `CONTINUE`
* `BREAK`

#### C 组：函数与语句块

* `CALL`（因为可以省略，因此暂未加入 Parser）
* `DEFFUNC`
* `BLOCK`

#### D 组：常量表达式

* `CONST`
* `CONSTEXPR`
* `DEFCONSTFUNC`

#### E 组：指针与数组

* `DEFPTR`
* `REF`
* `DEREF`
* `DEFARRAY`
* `ITEM`

### 语法规则

每一个句子的开头都是一个谓词，表示本句的动作。除自定义函数，句子后面直接跟上相等数量的参数，无需括号。自定义函数需要跟上不定长列表。

```
program -> *statement
statement -> "DEFFUNC"          typed_sym   typed_sym_list  statement
           | "DEFCONSTFUNC"     typed_sym   typed_sym_list  statement
           | stat_list
           | "BLOCK"     stat_list
           | "DEFVAR"    typed_sym   vaimage.pnglue
           | "ASSIGN"    sym         value
           | "IF"        value       statement
           | "IFE"       value       statement       statement
           | "WHILE"     value       statement
           | "RETURN"    value
           | "DEFPTR"    typed_sym
           | "DEFARRAY"  typed_sym   array_def
           | "REF"       sym
           | "DEREF"     value
           | "ITEM"      value       number
           | "CALL"      function
           | "BREAK"
           | "CONTINUE"
           | function
function -> basic_function
          | defined_function    value_list
basic_function -> unary_func    value
               -> binary_func   value   value
               -> triple        value   value   value
basic_function_const -> unary_func    const_val
                     -> binary_func   const_val   const_val
                     -> triple        const_val   const_val   const_val
typed_sym   -> CONST num_type:sym
            -> num_type:sym
num_type    -> i64|i32|i16|i8|i1|void
value_list  -> ( )
             | ( value, value_list )
typed_sym_list -> ( )
                | ( typed_sym, typed_sym_list )
stat_list -> { }
           | { statement, stat_list }
value -> function
       | const_val
       | variant
const_val -> basic_function_const
           | integer
array_def -> [ const_val ]
           | [ const_val, array_def ]
variant -> sym
unary_func  -> ......
binary_func -> ......
triple_func -> ......
defined_function -> sym

interger    -> 0 | (1|2|3|4|5|6|7|8|9)*(0|1|2|3|4|5|6|7|8|9)
sym         -> [a-zA-Z_]*[a-zA-Z_0-9]
```

## 编译

```
cd src
make
```

## 使用

```
compiler [file_name]
```

## 特性

* Variable Shadowing

## 报错

### Lexer 词法错误

1. `[Lexer] error 1: not a number`：解析到一个以数字开头但是混杂数字的元素

### Parser 语法错误

1. `[Parser] error 1: token out of expectation`：读取到非预期的 token
2. `[Parser] error 2: not a symbol`：预期为变量名，但最终输入不是变量名
3. `[Parser] error 3: not a type`：预期为类型名，但最终输入不是类型名。常见于 `[type]:[name]` 有类型变量名输入有误
4. `[Parser] error 4: function not found`：要执行的函数未定义
5. `[Parser] error 5: variable not found`：要使用的变量未定义
6. `[Parser] error 6: calling a variable`：尝试调用变量
7. `[Parser] error 7: definition existed`：尝试重定义变量或函数
8. `[Parser] error 8: function nested`：尝试在函数内定义函数
9. `[Parser] error 9: beginning of a statement must be a symbol`：不能以非名称作为语句开头，例如使用数字
10. `[Parser] error 10: beginning of a statement cannot be a type`：不能以数据类型作为开头，例如 CONST

### Convertor 语义错误

以 `- impossible` 结尾的报错为未经 Parser 直接生成 AST 才可能出现的错误，非正式错误，无标号

1. `[Convertor] error 1: left value cannot be found`：找不到左值
2. `[Convertor] error 2: operation not implemented`：该操作未被 IR 实现
3. `[Convertor] error 3: node not calculatable`：尝试对无值的内容求值
4. `[Convertor] error 4: expression outside a function`：尝试在全局环境中执行其他内容
5. `[Convertor] error 5: global operation has type that not implemented`：全局中不可以使用该操作
6. `[Convertor] error 6: binary operation conversion from ast to ir not implemented`：该二元算数操作未被 IR 实现
7. `[Convertor] error 7: comparasion operation conversion from ast to ir not implemented`：该比较操作未被 IR 实现
8. `[Convertor] error 8: wrong count of arguments`：函数参数列表长度不同
9. `[Convertor] error 9: no outer loops`：BREAK 或 CONTINUE 外层无循环
10. `[Convertor] error 10: assignment to a local const value`：为局部 CONST 变量赋值
11. `[Convertor] error 11: assignment to a global const value`：为全局 CONST 变量赋值
