#include "staisp_lexer.h"

#include <utility>


namespace Staisp {

StaispToken::StaispToken(ImmValue val, pCode code, String::iterator token_begin,
                         String::iterator token_end, int line)
    : Token(std::move(code), token_begin, token_end, line), t(TOKEN_INT), val(val) {}

StaispToken::StaispToken(String sym, pCode code, String::iterator token_begin,
                         String::iterator token_end, int line)
    : Token(std::move(code), token_begin, token_end, line), t(TOKEN_SYM), sym(std::move(std::move(sym))) {}

StaispToken::StaispToken(TokenType t, String::value_type c, pCode code,
                         String::iterator token_begin,
                         String::iterator token_end, int line)
    : Token(std::move(code), token_begin, token_end, line), t(t), val(c) {}

String::value_type Lexer::get_char() {
    if (!has_char()) {
        my_assert(false, "Error: no more chars.");
    }
    if (peek() == '\n') {
        ++_line_count;
}
    return *(_current++);
}

String::value_type Lexer::peek() { return *(_current); }

bool Lexer::has_char() const { return _current != _end; }

void Lexer::jump_empty() {
    while (has_char() && (isspace(peek()) != 0)) {
        get_char();
}
}

void Lexer::jump_comment() {
    if (has_char() && peek() == '#') {
        while (has_char() && peek() != '\n') {
            get_char();
}
}
}

TokenList Lexer::lexer(const pCode& code) {
    TokenList list;
    _current = code->p_code->begin();
    _end = code->p_code->end();
    _line_count = 1;
    _p_code = code;
    while (true) {
        while (has_char() && ((isspace(peek()) != 0) || peek() == '#')) {
            jump_empty();
            jump_comment();
        }
        if (!has_char()) {
            break;
}
        _begin = _current;
        list.push_back(lexer_one_token());
        // list.back().print();
    }
    _p_code = pCode();
    return list;
}

pToken Lexer::lexer_number(String::value_type head) {
    long long var = head - '0';
    while (has_char() && (isspace(peek()) == 0)) {
        if (peek() == '.') {
            get_char();
            return lexer_float(var);
        }
        if (isalpha(peek()) != 0) {
            StaispToken(var, _p_code, _begin, _current, _line_count)
                .throw_error(1, "Lexer", "not a number");
        }
        if (isdigit(peek()) == 0) {
            break;
}
        var = var * 10 + get_char() - '0';
    }
    if (var >= INT_MAX) {
        return pToken(new StaispToken(ImmValue(var, IMM_I64), _p_code, _begin,
                                      _current, _line_count));
    }
    return pToken(new StaispToken(ImmValue(var, IMM_I32), _p_code, _begin,
                                  _current, _line_count));
}

pToken Lexer::lexer_float(long long head) {
    double var = head;
    double small = 0.1;
    while (has_char() && (isspace(peek()) == 0)) {
        if (isalpha(peek()) != 0) {
            StaispToken(var, _p_code, _begin, _current, _line_count)
                .throw_error(1, "Lexer", "error 1: not a number");
        }
        if (isdigit(peek()) == 0) {
            break;
}
        var += small * (get_char() - '0');
        small /= 10;
    }
    return pToken(
        new StaispToken(ImmValue(var), _p_code, _begin, _current, _line_count));
}

pToken Lexer::lexer_sym(String::value_type head) {
    String sym;
    sym += head;
    while (has_char() && (isspace(peek()) == 0)) {
        if ((isalnum(peek()) == 0) && peek() != '_') {
            break;
}
        sym += get_char();
    }
    return pToken(new StaispToken(sym, _p_code, _begin, _current, _line_count));
}

pToken Lexer::lexer_one_token() {
    String::value_type ch = get_char();
    switch (ch) {
    case '(':
        return pToken(new StaispToken(TOKEN_LB_S, '(', _p_code, _begin,
                                      _current, _line_count));
    case ')':
        return pToken(new StaispToken(TOKEN_RB_S, ')', _p_code, _begin,
                                      _current, _line_count));
    case '[':
        return pToken(new StaispToken(TOKEN_LB_M, '[', _p_code, _begin,
                                      _current, _line_count));
    case ']':
        return pToken(new StaispToken(TOKEN_RB_M, ']', _p_code, _begin,
                                      _current, _line_count));
    case '{':
        return pToken(new StaispToken(TOKEN_LB_L, '{', _p_code, _begin,
                                      _current, _line_count));
    case '}':
        return pToken(new StaispToken(TOKEN_RB_L, '}', _p_code, _begin,
                                      _current, _line_count));
    case ':':
        return pToken(new StaispToken(TOKEN_2DOT, ':', _p_code, _begin,
                                      _current, _line_count));
    case '*':
        return pToken(new StaispToken(TOKEN_FLWR, '*', _p_code, _begin,
                                      _current, _line_count));
    }
    if (isdigit(ch) != 0) {
        return lexer_number(ch);
    }
    return lexer_sym(ch);
}

} // namespace Staisp
