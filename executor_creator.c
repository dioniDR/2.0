#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common_utils.h"

// Función específica para extraer comandos en modo Creator
char* extract_command_creator(const char *text) {
    // Usar la función mejorada con parámetro específico para bash
    return extract_command_improved(text, "bash");
}

// Función específica para ejecutar comandos en modo Creator
char* run_command_creator(const char *cmd) {
    // Usar la función mejorada para ejecutar comandos
    return run_command_improved(cmd);
}