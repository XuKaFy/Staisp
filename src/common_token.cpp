#include "common_token.h"

#include <utility>

Token::Token(pCode code, String::iterator token_begin,
             String::iterator token_end, int line)
    : p_code(std::move(std::move(code))), token_begin(token_begin),
      token_end(token_end), line(line) {}

void Token::print_message(const String &type, int id, const String &object,
                          const String &error_code) {
    String::iterator line_begin = token_begin;
    String::iterator line_end = token_end;
    // 暴力找到当前行的开头和结尾
    while (*line_begin != '\n' && line_begin != p_code->p_code->begin()) {
        --line_begin;
    }
    while (*line_end != '\n' && line_end != p_code->p_code->end()) {
        ++line_end;
    }
    if (*line_begin == '\n') {
        ++line_begin;
    }

    printf("%s:%d:%llu: [%s] %s %d: %s\n", p_code->file_name.c_str(), line,
           static_cast<unsigned long long>(token_begin - line_begin + 1),
           object.c_str(), type.c_str(), id, error_code.c_str());
    // 暂时id设计为：代码最多为 99999 行
    printf("%5d | %s\n        ", line, String(line_begin, line_end).c_str());
    for (String::difference_type i = 0; i < token_begin - line_begin; ++i) {
        putchar(' ');
    }
    for (String::difference_type i = 0; i < token_end - token_begin; ++i) {
        putchar('^');
    }
    puts("");
}

void Token::throw_error(int id, const String &object,
                        const String &error_code) {
    print_message("error", id, object, error_code);
    throw Exception(id, object, error_code);
}

void Token::print_warning(int id, const String &object,
                          const String &error_code) {
    print_message("warning", id, object, error_code);
}
