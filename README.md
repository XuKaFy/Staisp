# Staisp

## 关于 Staisp

Staisp（STAtic lISP），即静态语法表，用于用一种相比于 IR 更高级的语言直接编写语法树。

```
DEFVAR i32:n 1

DEFFUNC i32:fib ( i32 CONST:n ) BLOCK {
    IF OR EQ n 1
          EQ n 2
        RETURN 1
    RETURN ADD fib ( SUB n 1 )
               fib ( SUB n 2 )
}

DEFFUNC i32:main ( ) {
    DEFVAR i32:ans 0
    ASSIGN n 10
    WHILE LE n 10
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
* `i1|i8|i16|i32|i64|u1|u8|i16|u32|u64|f32|f64`
* `CAST`

以及类型说明符

* `type *`：指针
* `type [const_number]`：数组

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
           | "ASSIGN"    left_value  value
           | "IF"        value       statement
           | "IFE"       value       statement       statement
           | "WHILE"     value       statement
           | "RETURN"    value
           | "DEFPTR"    typed_sym
           | "DEFARRAY"  typed_sym   array_def
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
typed_sym   -> type:sym
type        -> num_type
             | type CONST
             | type*
             | type[value]
num_type    -> i64|i32|i16|i8|i1|void
value_list  -> ( )
             | ( value, value_list )
typed_sym_list -> ( )
                | ( typed_sym, typed_sym_list )
stat_list -> { }
           | { statement, stat_list }
left_value -> sym
            | "DEREF"   value
            | "ITEM"    value       array_def
value -> function
       | const_val
       | variant
       | CAST   type        value
       | "REF"  left_value
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
mkdir build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## 使用

```
compiler [file_name]
```

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
11. `[Parser] error 11: too many CONSTs`：输入了太多 CONST
12. `[Parser] error 12: expected to be a left-value`：此处应为左值
13. `[Parser] error 13: type of index should be integer`：数组长度必须是整型
14. `[Parser] error 14: CONST of array type is meaningless`：数组无需 CONST 修饰

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
12. `[Convertor] error 12: expected to be a left-value`：此处应为左值
13. `[Convertor] error 13: not castable`：隐式转换失败
14. `[Convertor] error 14: type of index should be integer`：寻址的参数必须是整型
15. `[Convertor] error 15: array should be initialized by a list`：必须用列表来初始化
16. `[Convertor] error 16: list should initialize type that is pointed`：列表只能初始化指针类型指向的值
17. `[Convertor] error 17: list doesn't match the expected value`：列表中的值类型与数组不符
18. `[Convertor] error 18: type has no joined type`：两类型不可计算

### Executor 运行时报错

1. `[Executor] error 1: function is not declared as DEFCONSTFUNC`：尝试执行非常数函数的函数
2. `[Executor] error 2: function not found`：找不到函数
3. `[Executor] error 3: variant not found`：找不到变量
4. `[Executor] error 4: wrong count of arguments`：函数参数列表长度不同
5. `[Executor] error 5: function nested`：尝试执行函数内的函数
6. `[Executor] error 6: function has no return`：函数没有返回值
7. `[Executor] error 7: empty value`：计算时含有 VOID
