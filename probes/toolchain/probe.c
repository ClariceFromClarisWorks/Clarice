/* Build/link probe only; this is not the terminal or a performance test. */
#include <Processes.h>
#include <OSUtils.h>
typedef char int_is_32_bits[sizeof(int) == 4 ? 1 : -1];
typedef char pointer_is_32_bits[sizeof(void *) == 4 ? 1 : -1];
extern unsigned long sys6_abi_probe(unsigned long value);
int main(void)
{
    if (sys6_abi_probe(41UL) != 42UL)
        SysBeep(30);
    ExitToShell();
    return 0;
}
