#include <iostream>
#include "struct_inspector.hpp"

static constexpr int B_SIZE = 8;
static constexpr int C_SIZE = 4;
static constexpr int D_SIZE = 2;

struct A
{
    struct B
    {
        struct C
        {
            struct D
            {
                std::string value;
            };
            D d[D_SIZE];
            int value;
        };
        C c[C_SIZE];
        bool value;
    };

    B b[B_SIZE];
    int value;
};

// D
REGISTER_INSPECTOR(A::B::C::D)
    FIELD(value)
REGISTER_END()

// C
REGISTER_INSPECTOR(A::B::C)
    FIELD(d)       // d[0], d[1], ...
    FIELD(value)
REGISTER_END()

// B
REGISTER_INSPECTOR(A::B)
    FIELD(c)       // c[0], c[1], ...
    FIELD(value)
REGISTER_END()

// A
REGISTER_INSPECTOR(A)
    FIELD(b)       // b[0], b[1], ...
    FIELD(value)
REGISTER_END()

int main()
{
    A a = {};

    ConstStructInspector ins;
    ins.bind_struct(a, "a");

    std::string key;
    int count = 0;
    while (true)
    {
        count++;
        for (int i = 0; i < B_SIZE; i++)
        {
            a.b[i].value = (count * 10 + i * 1) % 2;
            for (int j = 0; j < C_SIZE; j++)
            {
                a.b[i].c[j].value = count * 100 + i * 10 + j * 1;
                for (int k = 0; k < D_SIZE; k++)
                {
                    a.b[i].c[j].d[k].value = std::to_string(count * 1000 + i * 100 + j * 10 + k);
                }
            }
        }

        std::cout << "input struct member full path : " << std::flush;
        std::cin >> key;
        if (ins.contains(key))
        {
            std::cout << key << " ... " << ins.str(key) << " (type=" << ins.type(key) << ")" << std::endl;
        }
        else
        {
            std::cout << key << " is not found!" << std::endl;
        }
    }

    return 0;
}