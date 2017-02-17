#include <stdlib.h>
#include <stdint.h>

void start_kernel(void)
{
	void (*kernel)(uint32_t reserved, uint32_t mach, uint32_t dt) = (void (*)(uint32_t, uint32_t, uint32_t))(KERNEL_ADDR | 1);

	kernel(0, ~0UL, DTB_ADDR);
}
