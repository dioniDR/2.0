#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

// Inclusiones comunes
#include "common/includes/utils.h"
#include "common/includes/context.h"
#include "api/openai.h"

// Banner del programa
void print_banner(const char* module_name) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║           GPT Terminal Assistant v1.1            ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║ Modo: %-43s ║\n", module_name);
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║ Comandos internos:                              ║\n");
    printf("║ !ayuda    - Muestra esta ayuda                  ║\n");
    printf("║ !salir    - Salir del programa                  ║\n");
    printf("║ !limpiar  - Limpiar la pantalla                 ║\n");
    printf("║ !contexto - Mostrar el contexto actual          ║\n");
    printf("║ !reset    - Reiniciar el contexto               ║\n");
    printf("║ !version  - Muestra la versión del programa     ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");
}

// Comando para mostrar la ayuda
void show_help(const char* module_name) {
    print_banner(module_name);
}

// Comando para limpiar el contexto
void reset_context() {
    FILE *f = fopen("context.txt", "w");
    if (f) fclose(f);
    printf("✓ Contexto reiniciado correctamente.\n");
}

// Mostrar el contexto actual
void show_context() {
    FILE *f = fopen("context.txt", "r");
    if (!f) {
        printf("No hay contexto guardado.\n");
        return;
    }
    
    printf("\n===== CONTEXTO ACTUAL =====\n");
    char line[2048];
    while (fgets(line, sizeof(line), f)) {
        char role[16], content[2000];
        if (sscanf(line, "%15[^\t]\t%[^\n]", role, content) == 2) {
            printf("[%s]: %s\n", role, content);
        }
    }
    printf("===========================\n\n");
    fclose(f);
}

// Manejador de señales (para Ctrl+C)
void handle_signal(int sig) {
    printf("\n¡Hasta pronto! Para salir completamente, usa !salir\n");
}

#ifdef MODO_ARCH
#include "modulos/arch/executor.h"
#define MODULE_NAME "Asistente Arch Linux"
#define ROLE_FILE "modulos/arch/role.txt"
#define extract_command extract_command_arch
#define run_command run_command_arch
#endif

#ifdef MODO_CHAT
#include "modulos/chat/executor.h"
#define MODULE_NAME "Asistente Conversacional"
#define ROLE_FILE "modulos/chat/role.txt"
#define extract_command extract_command_chat
#define run_command run_command_chat
#endif

#ifdef MODO_CREATOR
#include "modulos/creator/executor.h"
#define MODULE_NAME "Generador de Estructuras"
#define ROLE_FILE "modulos/creator/role.txt"
#define extract_command extract_command_creator
#define run_command run_command_creator
#endif

int main() {
    // Configurar manejador de señales
    signal(SIGINT, handle_signal);
    
    // Inicializar
    load_context();
    char input[2048];

    // Mostrar banner de bienvenida
    print_banner(MODULE_NAME);

    // Bucle principal
    while (1) {
        printf("\n> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        
        // Comandos internos
        if (strncmp(input, "!salir", 6) == 0 || strcmp(input, "salir") == 0) {
            printf("¡Hasta pronto!\n");
            break;
        } else if (strncmp(input, "!ayuda", 6) == 0) {
            show_help(MODULE_NAME);
            continue;
        } else if (strncmp(input, "!limpiar", 8) == 0) {
            system("clear");
            continue;
        } else if (strncmp(input, "!contexto", 9) == 0) {
            show_context();
            continue;
        } else if (strncmp(input, "!reset", 6) == 0) {
            reset_context();
            continue;
        } else if (strncmp(input, "!version", 8) == 0) {
            printf("GPT Terminal Assistant v1.1\n");
            continue;
        }

        // Enviar mensaje a GPT
        printf("⏳ Procesando solicitud...\n");
        char* respuesta = send_prompt(input, ROLE_FILE);
        if (!respuesta || strncmp(respuesta, "Error", 5) == 0) {
            printf("❌ %s\n", respuesta ? respuesta : "Error desconocido al procesar la solicitud.");
            free(respuesta);
            continue;
        }
        
        printf("\n[GPT]:\n%s\n", respuesta);

        // Detectar y ejecutar comandos
        char* comando = extract_command(respuesta);

        if (comando) {
            printf("\n[Shell] Ejecutando:\n%s\n", comando);
            
            // Preguntar confirmación para comandos potencialmente peligrosos
            if (strstr(comando, "rm ") || strstr(comando, "mkfs") || 
                strstr(comando, "dd ") || strstr(comando, "fdisk")) {
                char confirm[10];
                printf("⚠️  Advertencia: Este comando puede ser destructivo.\n");
                printf("¿Deseas ejecutarlo? (s/n): ");
                if (!fgets(confirm, sizeof(confirm), stdin) || 
                    (confirm[0] != 's' && confirm[0] != 'S')) {
                    printf("Comando cancelado por el usuario.\n");
                    free(comando);
                    free(respuesta);
                    continue;
                }
            }
            
            // Ejecutar el comando
            char* salida = run_command(comando);
            
            printf("\n[Salida]:\n%s\n", salida);
            append_to_context(comando, salida);
            free(salida);
            free(comando);
        } else {
            printf("ℹ️  No se detectó comando ejecutable.\n");
        }
        free(respuesta);
    }
    return 0;
}