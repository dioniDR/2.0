#!/bin/bash
# create_module.sh - Script para crear nuevos módulos GPT

set -e

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== 🚀 Creador de Módulos GPT ===${NC}\n"

# Solicitar nombre del módulo
read -p "Nombre del módulo (ej: docker, python, security): " MODULE_NAME

# Validar nombre
if [[ ! "$MODULE_NAME" =~ ^[a-z][a-z0-9_]*$ ]]; then
    echo -e "${RED}❌ Error: El nombre debe empezar con letra minúscula y contener solo letras, números y guiones bajos${NC}"
    exit 1
fi

# Verificar si ya existe
if [ -d "modulos/$MODULE_NAME" ]; then
    echo -e "${RED}❌ Error: El módulo '$MODULE_NAME' ya existe${NC}"
    exit 1
fi

# Solicitar información del módulo
echo -e "\n${YELLOW}📝 Información del módulo:${NC}"
read -p "Descripción breve: " MODULE_DESC
read -p "Modelo GPT (default: gpt-4o): " MODEL
MODEL=${MODEL:-gpt-4o}
read -p "Temperatura (0.0-1.0, default: 0.7): " TEMPERATURE
TEMPERATURE=${TEMPERATURE:-0.7}
read -p "Max tokens (default: 2000): " MAX_TOKENS
MAX_TOKENS=${MAX_TOKENS:-2000}

# Preguntar si incluir MCP
read -p "¿Incluir soporte MCP? [s/N]: " INCLUDE_MCP
INCLUDE_MCP=${INCLUDE_MCP,,} # lowercase

# Crear estructura de directorios
echo -e "\n${BLUE}📁 Creando estructura del módulo...${NC}"
mkdir -p "modulos/$MODULE_NAME"

# Crear config.ini
cat > "modulos/$MODULE_NAME/config.ini" << EOF
# Configuración del módulo $MODULE_NAME
# $MODULE_DESC

# Configuración del modelo
MODEL=$MODEL
TEMPERATURE=$TEMPERATURE
MAX_TOKENS=$MAX_TOKENS

# Rutas de archivos
API_KEY_FILE=api/config.txt
ROLE_FILE=modulos/$MODULE_NAME/role.txt

# Configuración de respaldo (se usa si no existe ROLE_FILE)
SYSTEM_ROLE=system
SYSTEM_CONTENT=$MODULE_DESC
EOF

# Crear role.txt
cat > "modulos/$MODULE_NAME/role.txt" << EOF
system
$MODULE_DESC
EOF

# Crear executor.h
MODULE_NAME_UPPER=$(echo "$MODULE_NAME" | tr '[:lower:]' '[:upper:]')
cat > "modulos/$MODULE_NAME/executor.h" << EOF
#ifndef EXECUTOR_${MODULE_NAME_UPPER}_H
#define EXECUTOR_${MODULE_NAME_UPPER}_H

// Función específica para extraer comandos en modo $MODULE_NAME
char* extract_command_${MODULE_NAME}(const char* text);

// Función específica para ejecutar comandos en modo $MODULE_NAME
char* run_command_${MODULE_NAME}(const char* cmd);

#endif /* EXECUTOR_${MODULE_NAME_UPPER}_H */
EOF

# Crear executor.c
cat > "modulos/$MODULE_NAME/executor.c" << EOF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../common/includes/utils.h"

// Función específica para extraer comandos en modo $MODULE_NAME
char* extract_command_${MODULE_NAME}(const char *text) {
    // Usar la función mejorada con parámetro específico para bash
    return extract_command_improved(text, "bash");
}

// Función específica para ejecutar comandos en modo $MODULE_NAME
char* run_command_${MODULE_NAME}(const char *cmd) {
    // TODO: Implementar lógica específica del módulo si es necesaria
    // Por ejemplo, validaciones especiales, logging, etc.
    
    // Por defecto, usar la función mejorada para ejecutar comandos
    return run_command_improved(cmd);
}
EOF

# Si incluye MCP, crear versión MCP
if [ "$INCLUDE_MCP" = "s" ]; then
    echo -e "${BLUE}🔌 Creando versión MCP del módulo...${NC}"
    
    MODULE_MCP="${MODULE_NAME}_mcp"
    mkdir -p "modulos/$MODULE_MCP"
    
    # Copiar archivos base con modificaciones
    sed "s/$MODULE_NAME/$MODULE_MCP/g" "modulos/$MODULE_NAME/config.ini" > "modulos/$MODULE_MCP/config.ini"
    
    cat > "modulos/$MODULE_MCP/role.txt" << EOF
system
$MODULE_DESC con capacidades MCP extendidas. Tienes acceso directo a comandos del sistema y puedes ejecutarlos automáticamente cuando el usuario los escriba.
EOF
    
    sed "s/$MODULE_NAME/$MODULE_MCP/g; s/${MODULE_NAME_UPPER}/${MODULE_NAME_UPPER}_MCP/g" \
        "modulos/$MODULE_NAME/executor.h" > "modulos/$MODULE_MCP/executor.h"
    
    sed "s/$MODULE_NAME/$MODULE_MCP/g" \
        "modulos/$MODULE_NAME/executor.c" > "modulos/$MODULE_MCP/executor.c"
fi

# Actualizar main.c para incluir el nuevo módulo
echo -e "\n${BLUE}📝 Actualizando main.c...${NC}"

# Crear backup de main.c
cp main.c main.c.bak

# Buscar la línea donde insertar el nuevo módulo
LINE_NUMBER=$(grep -n "^#ifdef MODO_" main.c | tail -1 | cut -d: -f1)
LINE_NUMBER=$((LINE_NUMBER + 7))

# Preparar el bloque a insertar
BLOCK_TO_INSERT=$(cat << EOF

#ifdef MODO_${MODULE_NAME_UPPER}
#include "modulos/$MODULE_NAME/executor.h"
#define MODULE_NAME "$MODULE_DESC"
#define CONFIG_FILE "modulos/$MODULE_NAME/config.ini"
#define extract_command extract_command_${MODULE_NAME}
#define run_command run_command_${MODULE_NAME}
#endif
EOF
)

# Insertar el bloque en main.c
head -n $LINE_NUMBER main.c > main.c.tmp
echo "$BLOCK_TO_INSERT" >> main.c.tmp
tail -n +$((LINE_NUMBER + 1)) main.c >> main.c.tmp
mv main.c.tmp main.c

# Si incluye MCP, actualizar main_mcp.c también
if [ "$INCLUDE_MCP" = "s" ]; then
    echo -e "${BLUE}📝 Actualizando main_mcp.c...${NC}"
    
    cp main_mcp.c main_mcp.c.bak
    
    LINE_NUMBER=$(grep -n "^#ifdef MODO_" main_mcp.c | tail -1 | cut -d: -f1)
    LINE_NUMBER=$((LINE_NUMBER + 7))
    
    BLOCK_TO_INSERT=$(cat << EOF

#ifdef MODO_${MODULE_NAME_UPPER}_MCP
#include "modulos/${MODULE_MCP}/executor.h"
#define MODULE_NAME "🚀 $MODULE_DESC MCP"
#define CONFIG_FILE "modulos/${MODULE_MCP}/config.ini"
#define extract_command extract_command_${MODULE_MCP}
#define run_command run_command_${MODULE_MCP}
#endif
EOF
)
    
    head -n $LINE_NUMBER main_mcp.c > main_mcp.c.tmp
    echo "$BLOCK_TO_INSERT" >> main_mcp.c.tmp
    tail -n +$((LINE_NUMBER + 1)) main_mcp.c >> main_mcp.c.tmp
    mv main_mcp.c.tmp main_mcp.c
fi

# Crear ejemplo de uso
cat > "modulos/$MODULE_NAME/README.md" << EOF
# Módulo $MODULE_NAME

$MODULE_DESC

## Compilación

\`\`\`bash
make $MODULE_NAME
\`\`\`

## Uso

\`\`\`bash
./gpt_$MODULE_NAME
\`\`\`

## Configuración

Edita \`modulos/$MODULE_NAME/config.ini\` para ajustar:
- Modelo GPT
- Temperatura
- Tokens máximos

## Personalización del rol

Modifica \`modulos/$MODULE_NAME/role.txt\` para cambiar el comportamiento del asistente.
EOF

if [ "$INCLUDE_MCP" = "s" ]; then
    cat >> "modulos/$MODULE_NAME/README.md" << EOF

## Versión MCP

También se ha creado una versión con soporte MCP:

\`\`\`bash
make ${MODULE_MCP}
./gpt_${MODULE_MCP}
\`\`\`

Esta versión permite ejecutar comandos directamente sin confirmación.
EOF
fi

# Resumen final
echo -e "\n${GREEN}✅ Módulo '$MODULE_NAME' creado exitosamente!${NC}"
echo -e "\n${YELLOW}📋 Resumen:${NC}"
echo -e "  • Directorio: modulos/$MODULE_NAME/"
echo -e "  • Configuración: modulos/$MODULE_NAME/config.ini"
echo -e "  • Rol: modulos/$MODULE_NAME/role.txt"
echo -e "  • Código: modulos/$MODULE_NAME/executor.{h,c}"

if [ "$INCLUDE_MCP" = "s" ]; then
    echo -e "  • Versión MCP: modulos/${MODULE_MCP}/"
fi

echo -e "\n${YELLOW}🔨 Para compilar:${NC}"
echo -e "  make $MODULE_NAME"
if [ "$INCLUDE_MCP" = "s" ]; then
    echo -e "  make ${MODULE_MCP}"
fi

echo -e "\n${YELLOW}📝 Próximos pasos:${NC}"
echo -e "  1. Edita modulos/$MODULE_NAME/role.txt para personalizar el comportamiento"
echo -e "  2. Modifica executor.c si necesitas lógica especial"
echo -e "  3. Compila con 'make $MODULE_NAME'"
echo -e "  4. Ejecuta con './gpt_$MODULE_NAME'"

echo -e "\n${GREEN}¡Listo para usar!${NC}"