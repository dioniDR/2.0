#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api/openai.h"
#include "common/includes/utils.h"
#include "common/includes/context.h"
#include "common/includes/config_manager.h"
#include "mcp_client.h"
#include "installer_context.h"

#define MODULE_NAME "🚀 Asistente de Instalación Arch Linux"
#define CONFIG_FILE "modulos/arch_installer/config.ini"

// Función para mostrar ayuda específica de instalación
void show_installer_help() {
    printf("\n=== 📋 Comandos de Instalación Arch ===\n");
    printf("💡 Comandos directos (se ejecutan automáticamente):\n");
    printf("• lsblk, fdisk, cfdisk - Gestión de discos\n");
    printf("• mkfs.ext4, mkfs.fat - Formateo\n"); 
    printf("• mount, umount - Montaje de particiones\n");
    printf("• pacstrap - Instalación del sistema base\n");
    printf("• arch-chroot - Entrar al sistema instalado\n");
    printf("• grub-install, grub-mkconfig - Bootloader\n\n");
    printf("🔧 Comandos especiales:\n");
    printf("• /help - Mostrar esta ayuda\n");
    printf("• /progress - Ver progreso de instalación\n");
    printf("• /phase - Información de fase actual\n");
    printf("• /optimize - Optimizar contexto\n");
    printf("• /diag - Diagnóstico del sistema\n");
    printf("• salir - Terminar asistente\n\n");
    printf("❓ O simplemente pregunta: '¿Cómo empiezo la instalación?'\n\n");
}

// Función para procesar comandos especiales de instalación
int process_installer_command(const char* input, MCPClient* mcp_client) {
    if (strcmp(input, "/help") == 0) {
        show_installer_help();
        return 1;
    }
    
    if (strcmp(input, "/progress") == 0) {
        show_installation_progress();
        return 1;
    }
    
    if (strcmp(input, "/phase") == 0) {
        printf("🔍 Detectando fase actual de instalación...\n");
        FILE *ctx = fopen("context.txt", "r");
        if (ctx) {
            char line[2048];
            int last_phase = 0;
            while (fgets(line, sizeof(line), ctx)) {
                if (strstr(line, "[FASE")) {
                    sscanf(line, "%*[^[]FASE %d", &last_phase);
                }
            }
            fclose(ctx);
            
            if (last_phase > 0) {
                printf("📍 Fase actual: %d - %s\n\n", last_phase, 
                       (last_phase <= 6) ? phase_descriptions[last_phase-1] : "Completado");
            } else {
                printf("📍 Fase: Preparación inicial\n\n");
            }
        }
        return 1;
    }
    
    if (strcmp(input, "/optimize") == 0) {
        optimize_installer_context();
        printf("✅ Contexto optimizado para reducir costos\n\n");
        return 1;
    }
    
    if (strcmp(input, "/diag") == 0) {
        if (mcp_client) {
            printf("🔍 Ejecutando diagnóstico de instalación...\n");
            MCPResponse* response = mcp_arch_diagnostics(mcp_client);
            if (response && response->success && response->result) {
                printf("=== 🩺 Diagnóstico del Sistema ===\n%s\n", response->result);
            } else {
                printf("❌ Error en el diagnóstico.\n");
            }
            mcp_free_response(response);
        } else {
            printf("🔍 Diagnóstico básico:\n");
            system("echo '=== Discos ===' && lsblk");
            system("echo '=== Memoria ===' && free -h");
            system("echo '=== Red ===' && ip link show");
        }
        return 1;
    }
    
    return 0; // No es un comando especial
}

// Función para manejar comandos de instalación con confirmación
void handle_installer_command(const char* command, MCPClient* mcp_client) {
    // Comandos destructivos que requieren confirmación especial
    const char* destructive_commands[] = {
        "fdisk", "mkfs", "dd", "wipefs", "sgdisk", NULL
    };
    
    int is_destructive = 0;
    for (int i = 0; destructive_commands[i]; i++) {
        if (strstr(command, destructive_commands[i])) {
            is_destructive = 1;
            break;
        }
    }
    
    if (is_destructive) {
        printf("\n⚠️  COMANDO DESTRUCTIVO DETECTADO: %s\n", command);
        printf("Este comando puede ELIMINAR datos. ¿Estás seguro?\n");
        printf("Escribe 'SI ESTOY SEGURO' para continuar: ");
        
        char confirm[50];
        fgets(confirm, sizeof(confirm), stdin);
        confirm[strcspn(confirm, "\n")] = 0;
        
        if (strcmp(confirm, "SI ESTOY SEGURO") != 0) {
            printf("❌ Comando cancelado por seguridad.\n\n");
            return;
        }
    }
    
    printf("\n🔧 Ejecutando: %s\n", command);
    printf("--- Resultado ---\n");
    
    if (mcp_client) {
        MCPResponse* response = mcp_execute_command(mcp_client, command);
        if (response) {
            if (response->success && response->result) {
                printf("%s\n", response->result);
            } else if (response->error) {
                printf("❌ Error: %s\n", response->error);
            }
            mcp_free_response(response);
        }
    } else {
        char* result = run_command_improved(command);
        printf("%s\n", result);
        free(result);
    }
    
    printf("--- Fin ---\n\n");
    
    // Agregar al contexto optimizado
    add_installer_context("system", command);
    
    // Sugerir verificación para comandos críticos
    if (strstr(command, "mount") || strstr(command, "mkfs") || strstr(command, "pacstrap")) {
        printf("💡 Sugerencia: Verifica el resultado antes de continuar\n\n");
    }
}

// Función principal
int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[]) {
    // Inicializar contexto de instalación
    load_context();
    
    // Crear cliente MCP
    printf("🔌 Inicializando asistente de instalación...\n");
    MCPClient* mcp_client = mcp_create_client();
    if (!mcp_client) {
        printf("⚠️  MCP no disponible. Modo básico activado.\n");
    } else {
        printf("✅ Asistente MCP listo.\n");
    }
    
    printf("\n=== %s ===\n", MODULE_NAME);
    printf("📋 Asistente especializado para instalación paso a paso\n");
    printf("💰 Optimizado para mínimo costo de tokens\n");
    printf("🔒 Comandos destructivos requieren confirmación\n");
    printf("💡 Usa /help para ver comandos disponibles\n\n");
    
    // Mostrar progreso si ya hay instalación en curso
    if (count_context_lines() > 5) {
        printf("🔄 Instalación en progreso detectada:\n");
        show_installation_progress();
    }
    
    char input[2048];
    
    while (1) {
        printf("🚀 > ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "salir") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        
        if (strlen(input) == 0) {
            continue;
        }
        
        // Procesar comandos especiales
        if (process_installer_command(input, mcp_client)) {
            continue;
        }
        
        // Detectar si es un comando del usuario
        if (is_user_command(input)) {
            handle_installer_command(input, mcp_client);
            continue;
        }
        
        // Si no es comando directo, optimizar contexto antes de enviar a GPT
        optimize_installer_context();
        
        printf("🤖 Consultando especialista en instalación...\n");
        char* respuesta = send_prompt(input, CONFIG_FILE);
        
        printf("\n--- 💬 Especialista ---\n%s\n\n", respuesta);
        
        // Verificar si GPT sugiere comandos
        char* comando_sugerido = extract_command_improved(respuesta, "bash");
        if (comando_sugerido) {
            printf("💡 Se sugiere ejecutar: %s\n", comando_sugerido);
            printf("¿Ejecutar este comando? [s/N]: ");
            
            char confirmar[10] = {0};
            fgets(confirmar, sizeof(confirmar), stdin);
            confirmar[strcspn(confirmar, "\n")] = 0;
            
            if (confirmar[0] == 's' || confirmar[0] == 'S') {
                handle_installer_command(comando_sugerido, mcp_client);
            }
            
            free(comando_sugerido);
        }
        
        // Agregar respuesta al contexto optimizado
        add_installer_context("assistant", respuesta);
        free(respuesta);
    }
    
    // Limpiar
    if (mcp_client) {
        mcp_cleanup(mcp_client);
    }
    
    printf("\n🎉 ¡Instalación completada! Reinicia para usar tu nuevo sistema Arch.\n");
    printf("📚 Para más configuración: https://wiki.archlinux.org/\n");
    return 0;
}