#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "installer_context.h"

// Comandos críticos para instalación de Arch
const char* critical_installer_commands[] = {
    "lsblk", "fdisk", "cfdisk", "mkfs", "mount", "umount",
    "pacstrap", "genfstab", "arch-chroot", 
    "grub-install", "grub-mkconfig", "bootctl",
    "passwd", "useradd", "systemctl enable",
    "timedatectl", "loadkeys", "setfont",
    "iwctl", "dhcpcd", "ping",
    NULL
};

// Fases de instalación
const char* phase_descriptions[] = {
    "Preparación inicial",
    "Particionado de disco", 
    "Formateo y montaje",
    "Instalación del sistema base",
    "Configuración básica",
    "Instalación del bootloader",
    "Configuración final"
};

int is_critical_installer_command(const char* content) {
    if (!content) return 0;
    
    for (int i = 0; critical_installer_commands[i]; i++) {
        if (strstr(content, critical_installer_commands[i])) {
            return 1;
        }
    }
    return 0;
}

int detect_installation_phase(const char* content) {
    if (strstr(content, "lsblk") || strstr(content, "timedatectl") || strstr(content, "iwctl")) {
        return 1; // Preparación
    }
    if (strstr(content, "fdisk") || strstr(content, "cfdisk") || strstr(content, "parted")) {
        return 2; // Particionado
    }
    if (strstr(content, "mkfs") || strstr(content, "mount")) {
        return 3; // Formateo/montaje
    }
    if (strstr(content, "pacstrap") || strstr(content, "genfstab")) {
        return 4; // Instalación base
    }
    if (strstr(content, "arch-chroot") || strstr(content, "passwd") || strstr(content, "useradd")) {
        return 5; // Configuración
    }
    if (strstr(content, "grub") || strstr(content, "bootctl")) {
        return 6; // Bootloader
    }
    return 0; // General
}

void optimize_installer_context() {
    FILE *original = fopen("context.txt", "r");
    if (!original) return;
    
    FILE *optimized = fopen("context_optimized.txt", "w");
    if (!optimized) {
        fclose(original);
        return;
    }
    
    char line[2048];
    char critical_lines[20][2048];  // Máximo 20 líneas críticas
    int critical_count = 0;
    int total_lines = 0;
    
    // Leer todas las líneas y filtrar críticas
    while (fgets(line, sizeof(line), original) && critical_count < 20) {
        total_lines++;
        
        // Extraer contenido después del tab
        char *content = strchr(line, '\t');
        if (content) {
            content++; // Saltar el tab
            
            // Si es comando crítico, guardarlo
            if (is_critical_installer_command(content)) {
                strcpy(critical_lines[critical_count], line);
                critical_count++;
            }
        }
    }
    
    // Escribir resumen + comandos críticos
    fprintf(optimized, "system\t[INSTALACIÓN ARCH] Sesión de instalación en progreso. Comandos críticos ejecutados: %d\n", critical_count);
    
    // Escribir solo las últimas 15 líneas críticas
    int start_index = (critical_count > 15) ? critical_count - 15 : 0;
    for (int i = start_index; i < critical_count; i++) {
        fputs(critical_lines[i], optimized);
    }
    
    fclose(original);
    fclose(optimized);
    
    // Reemplazar archivo original
    rename("context_optimized.txt", "context.txt");
    
    printf("🎯 Contexto optimizado: %d→%d líneas (ahorro ~%d tokens)\n", 
           total_lines, critical_count + 1, (total_lines - critical_count) * 4);
}

void add_installer_context(const char* role, const char* content) {
    FILE *ctx = fopen("context.txt", "a");
    if (!ctx) return;
    
    // Solo agregar si es crítico O si hay pocas líneas
    if (is_critical_installer_command(content) || count_context_lines() < 10) {
        fprintf(ctx, "%s\t%s\n", role, content);
        
        // Detectar fase de instalación
        int phase = detect_installation_phase(content);
        if (phase > 0) {
            fprintf(ctx, "system\t[FASE %d] %s\n", phase, phase_descriptions[phase - 1]);
        }
    }
    
    fclose(ctx);
    
    // Optimizar si hay demasiadas líneas
    if (count_context_lines() > 25) {
        optimize_installer_context();
    }
}

int count_context_lines() {
    FILE *f = fopen("context.txt", "r");
    if (!f) return 0;
    
    int lines = 0;
    char buffer[2048];
    while (fgets(buffer, sizeof(buffer), f)) {
        lines++;
    }
    fclose(f);
    return lines;
}

void show_installation_progress() {
    printf("\n📋 Progreso de instalación:\n");
    
    FILE *ctx = fopen("context.txt", "r");
    if (!ctx) return;
    
    char line[2048];
    int phases_completed[7] = {0}; // Fases 0-6
    
    while (fgets(line, sizeof(line), ctx)) {
        char *content = strchr(line, '\t');
        if (content) {
            content++;
            int phase = detect_installation_phase(content);
            if (phase > 0) {
                phases_completed[phase] = 1;
            }
        }
    }
    fclose(ctx);
    
    for (int i = 1; i <= 6; i++) {
        printf("  %s [%s] %s\n", 
               phases_completed[i] ? "✅" : "⭕", 
               phases_completed[i] ? "COMPLETADO" : "PENDIENTE",
               phase_descriptions[i - 1]);
    }
    printf("\n");
}