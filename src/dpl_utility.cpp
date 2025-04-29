#include "dpl_utility.h"

namespace dpl {

union TestMachineByteOrder
{
    char a;
    int b;
};

bool IsMachineBigedign(void)
{
    static TestMachineByteOrder t = { .a = 1 };

    return t.b == 0;
}

}
