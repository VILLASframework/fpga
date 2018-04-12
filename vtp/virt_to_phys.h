#pragma once

#include <stdint.h>
#include <fcntl.h>

extern "C" {

int virt_to_phys_user(uintptr_t *paddr, pid_t pid, uintptr_t vaddr);

// for current process
int virt_to_phys(uintptr_t* paddr, uintptr_t vaddr);

}
