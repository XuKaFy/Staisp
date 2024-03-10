#include "def.h"

struct GlobalSymbols {
    ~GlobalSymbols() {
        for(auto i : s)
            delete i;
        s.clear();
    }
    Vector<const char*> s;
};

Symbol to_symbol(String s)
{
    static GlobalSymbols watcher;
    size_t n = s.size();
    char* m = new char[n + 1];
    strcpy(m, s.c_str());
    watcher.s.push_back(m);
    return Symbol(m);
}

pCode make_code(String code, String file_name)
{
    return pCode(new Code { pString(new String(code)), file_name });
}

void Memory::realloc(size_t len)
{
    this->len = len;
    auto j = new int8_t[len];
    memset(j, 0, sizeof(len));
    this->mem = RawMemory(j);
}
