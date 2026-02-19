#include <iostream>
#include <cstring>
#include <string>

#define ENABLE_ADD_STRUCT_INSPECTOR
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
    uint32_t value;
};

int main()
{
    ConstStructInspector inspector;
    A a = {};

    auto bind_a =[](ConstStructInspector& inspector, const A& a)
    {
        ADD_STRUCT_INSPECTOR(inspector, a.value);
        for (int i = 0; i < B_SIZE; i++)
        {
            ADD_STRUCT_INSPECTOR(inspector, a.b[i].value, i);
            for (int j = 0; j < C_SIZE; j++)
            {
                ADD_STRUCT_INSPECTOR(inspector, a.b[i].c[j].value, i, j);
                for (int k = 0; k < D_SIZE; k++)
                {
                    ADD_STRUCT_INSPECTOR(inspector, a.b[i].c[j].d[k].value, i, j, k);
                }
            }
        }
    };

    std::string key;
    int count = 0;
    while (true)
    {
        count++;
        auto copy_a = a;
        for (int i = 0; i < B_SIZE; i++)
        {
            copy_a.b[i].value = (count * 10 + i * 1) % 2;
            for (int j = 0; j < C_SIZE; j++)
            {
                copy_a.b[i].c[j].value = count * 100 + i * 10 + j * 1;
                for (int k = 0; k < D_SIZE; k++)
                {
                    copy_a.b[i].c[j].d[k].value = std::to_string(count * 1000 + i * 100 + j * 10 + k);
                }
            }
        }

        bind_a(inspector, copy_a);

        std::cout << "input struct member full path : " << std::flush;
        std::cin >> key;
        if (inspector.contains(key))
        {
            std::cout << key << " ... " << inspector.str(key) << " (type=" << inspector.type(key) << ")" << std::endl;
        }
        else
        {
            std::cout << key << " is not found!" << std::endl;
        }
    }

    return 0;
}