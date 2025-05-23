#!/bin/bash

echo "🔍 Verificando archivos del asistente de instalación..."
echo "=================================================="

# Función para verificar archivo y mostrar primeras líneas
check_file() {
    local file="$1"
    local expected_content="$2"
    
    echo ""
    echo "📄 Verificando: $file"
    echo "------------------------"
    
    if [ -f "$file" ]; then
        echo "✅ Archivo existe"
        echo "📝 Primeras 5 líneas:"
        head -5 "$file" | sed 's/^/   /'
        
        if [ -n "$expected_content" ]; then
            if grep -q "$expected_content" "$file"; then
                echo "✅ Contenido correcto detectado"
            else
                echo "❌ Contenido incorrecto o faltante"
            fi
        fi
    else
        echo "❌ Archivo NO existe"
        echo "💡 Necesitas crear: $file"
    fi
}

# Verificar archivos en raíz
echo "🔍 ARCHIVOS EN RAÍZ:"
check_file "installer_context.h" "#ifndef INSTALLER_CONTEXT_H"
check_file "installer_context.c" "critical_installer_commands"
check_file "main_installer.c" "MODULE_NAME.*Instalación Arch"

# Verificar archivos del módulo
echo ""
echo "🔍 MÓDULO arch_installer:"
check_file "modulos/arch_installer/config.ini" "MODEL=gpt-4o-mini"
check_file "modulos/arch_installer/role.txt" "ESPECIALISTA en instalación"
check_file "modulos/arch_installer/executor.c" "extract_command_arch_installer"
check_file "modulos/arch_installer/executor.h" "#ifndef EXECUTOR_ARCH_INSTALLER_H"

# Verificar Makefile.mcp
echo ""
echo "🔍 MAKEFILE:"
check_file "Makefile.mcp" "arch_installer:"

# Verificar archivos base necesarios
echo ""
echo "🔍 ARCHIVOS BASE NECESARIOS:"
check_file "mcp_client.h" "MCPClient"
check_file "mcp_client.c" "mcp_create_client"
check_file "MCPBridge_native" ""

# Resumen final
echo ""
echo "=================================================="
echo "📊 RESUMEN:"

missing_files=0
required_files=(
    "installer_context.h"
    "installer_context.c" 
    "main_installer.c"
    "modulos/arch_installer/config.ini"
    "modulos/arch_installer/role.txt"
    "modulos/arch_installer/executor.c"
    "modulos/arch_installer/executor.h"
    "Makefile.mcp"
)

for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Falta: $file"
        ((missing_files++))
    fi
done

if [ $missing_files -eq 0 ]; then
    echo "✅ Todos los archivos están presentes"
    echo "🚀 Listo para compilar: make arch_installer"
else
    echo "⚠️  Faltan $missing_files archivos"
    echo "📝 Crear los archivos faltantes antes de compilar"
fi

echo "=================================================="
