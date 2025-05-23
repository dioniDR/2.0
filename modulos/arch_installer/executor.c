#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../common/includes/utils.h"

// Función específica para extraer comandos en modo installer
char* extract_command_arch_installer(const char *text) {
    return extract_command_improved(text, "bash");
}

// Función específica para ejecutar comandos en modo installer  
char* run_command_arch_installer(const char *cmd) {
    return run_command_improved(cmd);
}