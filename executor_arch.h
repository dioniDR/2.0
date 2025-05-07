#ifndef EXECUTOR_ARCH_H
#define EXECUTOR_ARCH_H
#include "common_utils.h"

// Función específica para extraer comandos en modo Arch
char* extract_command_arch(const char* text);

// Función específica para ejecutar comandos en modo Arch
char* run_command_arch(const char* cmd);

#endif /* EXECUTOR_ARCH_H */