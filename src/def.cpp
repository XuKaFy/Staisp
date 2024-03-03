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

ImmType join_type(ImmType a, ImmType b)
{
    return std::max(a, b);
}
