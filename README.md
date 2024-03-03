# Staisp

## 关于 Staisp

Staisp（STAtic lISP），即静态语法表，用于用一种相比于 IR 更高级的语言直接编写语法树。

```
DEFVAR i32:n 50

DEFFUNC i32:fib ( i32:n ) BLOCK {
    IF OR EQ n i32:1
          EQ n i32:2
        RETURN i32:1
    RETURN ADD fib ( SUB n 1 )
               fib ( SUB n 2 )
}

DEFFUNC main ( ) BLOCK {
    ASSIGN n 10
    DEFVAR i32:ans 10
    WHILE ULE n 20
    ASSIGN n ADD n 1
    BLOCK {
        ASSIGN ans fib ( n )
    }
    RETURN ans
}
```

每一个句子的开头都是一个谓词，表示本句的动作。除自定义函数，句子后面直接跟上相等数量的参数，无需括号。自定义函数需要跟上不定长列表。

```
program -> *statement
statement -> "DEFVAR"    typed_sym   vaimage.pnglue
           | "DEFFUNC"   typed_sym   typed_sym_list  statement
           | "ASSIGN"    sym         value
           | "BLOCK"     stat_list
           | "IF"        value       statement
           | "IFE"       value       statement       statement
           | "WHILE"     value       statement
           | "RETURN"    value
           | function
function -> basic_function
          | defined_function    value_list
basic_function -> unary_func    value
               -> binary_func   value   value
               -> triple        value   value   value
typed_sym   -> num_type:sym
num_type    -> i64|i32|i16|i8|i1|void
value_list  -> ( )
             | ( value, value_list )
typed_sym_list -> ( )
                | ( typed_sym, typed_sym_list )
stat_list -> { }
           | { statement, stat_list }
value -> function
       | interger
       | variant
variant -> sym
unary_func  -> ......
binary_func -> ......
triple_func -> ......
defined_function -> sym

interger    -> 0 | (1|2|3|4|5|6|7|8|9)*(0|1|2|3|4|5|6|7|8|9)
sym         -> [a-zA-Z_]*[a-zA-Z_0-9]
```

## 运行

```
cd src
make
```
