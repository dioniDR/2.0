#ifndef INSTALLER_CONTEXT_H
#define INSTALLER_CONTEXT_H

// Constantes de optimización
#define MAX_INSTALLER_CONTEXT 20
#define MAX_CRITICAL_COMMANDS 15

// Arrays de información
extern const char* critical_installer_commands[];
extern const char* phase_descriptions[];

// Funciones principales
int is_critical_installer_command(const char* content);
int detect_installation_phase(const char* content);
void optimize_installer_context();
void add_installer_context(const char* role, const char* content);
int count_context_lines();
void show_installation_progress();

#endif