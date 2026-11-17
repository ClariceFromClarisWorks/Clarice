#ifndef CLARICE_HARDWARE_H
#define CLARICE_HARDWARE_H
#include <stdint.h>
/* Runtime 1.3. Snapshot, no ownership or retained allocation. Zero denotes
 * unknown except FPU (-2 present/type unknown, -1 unknown, 0 none). CPU/FPU codes are native Gestalt
 * codes; model_namespace is 1 for SysEnvirons, 2 for Gestalt. Primary display.
 * Callers pass sizeof(ClariceHardware); older/smaller records are rejected. */
typedef struct {
 uint16_t version,size,system,cpu;
 int16_t fpu,model_namespace;
 int32_t model;
 uint32_t ram;
 uint16_t width,height,depth,reserved;
 char model_name[48];
} ClariceHardware;
int cli_hardware(ClariceHardware *,unsigned long);
#endif
