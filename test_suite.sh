#!/bin/bash
# test_suite.sh - Suite de pruebas para GPT Assistant

set -e

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Contadores
TESTS_PASSED=0
TESTS_FAILED=0

# Función para ejecutar test
run_test() {
    local test_name="$1"
    local command="$2"
    local expected_pattern="$3"
    
    echo -n "🧪 $test_name... "
    
    if output=$($command 2>&1); then
        if echo "$output" | grep -q "$expected_pattern"; then
            echo -e "${GREEN}✓${NC}"
            ((TESTS_PASSED++))
            return 0
        else
            echo -e "${RED}✗ (salida no coincide)${NC}"
            echo "  Esperado: $expected_pattern"
            echo "  Obtenido: $output" | head -n 3
            ((TESTS_FAILED++))
            return 1
        fi
    else
        echo -e "${RED}✗ (comando falló)${NC}"
        echo "  Error: $output"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Función para test interactivo
test_interactive() {
    local test_name="$1"
    local executable="$2"
    local input="$3"
    local expected="$4"
    
    echo -n "🧪 $test_name... "
    
    if output=$(echo -e "$input\nsalir" | timeout 10 "./$executable" 2>&1); then
        if echo "$output" | grep -q "$expected"; then
            echo -e "${GREEN}✓${NC}"
            ((TESTS_PASSED++))
            return 0
        else
            echo -e "${RED}✗${NC}"
            echo "  Esperado: $expected"
            ((TESTS_FAILED++))
            return 1
        fi
    else
        echo -e "${RED}✗ (timeout o error)${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

echo -e "${BLUE}=== 🧪 Suite de Tests GPT Assistant ===${NC}\n"

# Verificar dependencias
echo -e "${YELLOW}📋 Verificando dependencias...${NC}"
run_test "GCC disponible" "which gcc" "gcc"
run_test ".NET SDK disponible" "which dotnet" "dotnet"
run_test "jq disponible" "which jq" "jq"
run_test "API key configurada" "test -f api/config.txt && grep -q API_KEY api/config.txt && echo OK" "OK"

echo -e "\n${YELLOW}🔨 Compilación...${NC}"

# Test de compilación básica
run_test "Compilar módulo arch" "make arch" "✅ Módulo arch compilado"
run_test "Compilar bridge MCP" "make build_mcp_bridge" "✅ Bridge nativo compilado"
run_test "Compilar módulo arch_mcp" "make arch_mcp" "✅ Módulo arch_mcp compilado"

echo -e "\n${YELLOW}🌉 Tests del Bridge MCP...${NC}"

# Test del bridge
run_test "Bridge responde a ping" \
    "echo '{\"Action\":\"get_system_info\"}' | ./MCPBridge_native | jq -r .Success" \
    "true"

run_test "Bridge ejecuta comandos" \
    "echo '{\"Action\":\"execute_command\",\"Data\":\"echo test\"}' | ./MCPBridge_native | jq -r .Result | grep test" \
    "test"

run_test "Bridge maneja errores" \
    "echo '{\"Action\":\"invalid_action\"}' | ./MCPBridge_native | jq -r .Success" \
    "false"

echo -e "\n${YELLOW}🤖 Tests de módulos...${NC}"

# Test módulo arch básico
if [ -f "gpt_arch" ]; then
    test_interactive "Módulo arch responde" "gpt_arch" "pwd" "¿Deseas ejecutar"
fi

# Test módulo arch_mcp
if [ -f "gpt_arch_mcp" ]; then
    test_interactive "Módulo arch_mcp detecta comandos" "gpt_arch_mcp" "ls" "Ejecutando: ls"
    test_interactive "Módulo arch_mcp procesa preguntas" "gpt_arch_mcp" "¿qué es Linux?" "Procesando con GPT"
fi

echo -e "\n${YELLOW}🔍 Tests de funcionalidades específicas...${NC}"

# Test detección de comandos
cat > test_command_detection.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include "mcp_client.h"

int main() {
    // Test comandos que deben ser detectados
    const char* commands[] = {
        "ls", "ls -la", "pacman -Syu", "systemctl status",
        "df -h", "free -m", NULL
    };
    
    for (int i = 0; commands[i]; i++) {
        if (is_user_command(commands[i])) {
            printf("✓ %s\n", commands[i]);
        } else {
            printf("✗ %s\n", commands[i]);
            return 1;
        }
    }
    
    // Test strings que NO deben ser detectados como comandos
    const char* non_commands[] = {
        "¿qué es ls?", "lista de archivos", "lsd no es comando",
        "sistema operativo", NULL
    };
    
    for (int i = 0; non_commands[i]; i++) {
        if (!is_user_command(non_commands[i])) {
            printf("✓ NO: %s\n", non_commands[i]);
        } else {
            printf("✗ FALSO POSITIVO: %s\n", non_commands[i]);
            return 1;
        }
    }
    
    return 0;
}
EOF

gcc -o test_command_detection test_command_detection.c mcp_client.c -I.
run_test "Detección de comandos" "./test_command_detection" "✓"
rm -f test_command_detection test_command_detection.c

echo -e "\n${YELLOW}🔒 Tests de seguridad...${NC}"

# Test comandos peligrosos bloqueados
if [ -f "MCPBridge_native" ]; then
    run_test "Bridge bloquea rm -rf /" \
        "echo '{\"Action\":\"execute_command\",\"Data\":\"rm -rf /\"}' | ./MCPBridge_native | jq -r .Success" \
        "false"
        
    run_test "Bridge permite comandos seguros" \
        "echo '{\"Action\":\"execute_command\",\"Data\":\"date\"}' | ./MCPBridge_native | jq -r .Success" \
        "true"
fi

echo -e "\n${YELLOW}💾 Tests de persistencia...${NC}"

# Test contexto
rm -f context.txt
echo -e "pwd\nsalir" | timeout 5 ./gpt_arch_mcp > /dev/null 2>&1
run_test "Contexto se crea" "test -f context.txt && echo OK" "OK"
run_test "Contexto contiene comandos" "grep -q 'pwd' context.txt && echo OK" "OK"

echo -e "\n${YELLOW}⚡ Tests de rendimiento...${NC}"

# Test velocidad del bridge
if [ -f "MCPBridge_native" ]; then
    echo -n "🧪 Tiempo de respuesta del bridge... "
    start_time=$(date +%s%N)
    echo '{"Action":"get_system_info"}' | ./MCPBridge_native > /dev/null
    end_time=$(date +%s%N)
    elapsed=$((($end_time - $start_time) / 1000000))
    
    if [ $elapsed -lt 100 ]; then
        echo -e "${GREEN}✓ ${elapsed}ms${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${YELLOW}⚠ ${elapsed}ms (lento)${NC}"
        ((TESTS_FAILED++))
    fi
fi

echo -e "\n${YELLOW}🧹 Tests de limpieza...${NC}"

# Verificar que make clean funciona
make clean > /dev/null 2>&1
run_test "Make clean elimina ejecutables" "! test -f gpt_arch && echo OK" "OK"
run_test "Make clean elimina temporales" "! test -f req.json && echo OK" "OK"

# Restaurar estado
make arch_mcp > /dev/null 2>&1

echo -e "\n${YELLOW}📊 Tests de integración...${NC}"

# Test pipeline completo
cat > test_integration.sh << 'EOF'
#!/bin/bash
# Simular sesión completa

# 1. Comando directo
echo "ls" | timeout 5 ./gpt_arch_mcp | grep -q "Ejecutando: ls" || exit 1

# 2. Pregunta a GPT
echo -e "¿cómo ver el espacio en disco?\nsalir" | timeout 10 ./gpt_arch_mcp | grep -q "df" || exit 1

# 3. Comando especial
echo -e "/status\nsalir" | timeout 5 ./gpt_arch_mcp | grep -q "Estado del Sistema" || exit 1

echo "OK"
EOF

chmod +x test_integration.sh
run_test "Pipeline completo" "./test_integration.sh" "OK"
rm -f test_integration.sh

echo -e "\n${YELLOW}🐛 Tests de manejo de errores...${NC}"

# Test errores comunes
test_interactive "Manejo de comando inválido" "gpt_arch_mcp" "comando_que_no_existe" "command not found"

# Test sin API key
mv api/config.txt api/config.txt.bak 2>/dev/null || true
test_interactive "Error sin API key" "gpt_arch_mcp" "test" "Error"
mv api/config.txt.bak api/config.txt 2>/dev/null || true

echo -e "\n${BLUE}=== 📊 Resumen de Tests ===${NC}"
echo -e "✅ Pasados: ${GREEN}$TESTS_PASSED${NC}"
echo -e "❌ Fallidos: ${RED}$TESTS_FAILED${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 ¡Todos los tests pasaron!${NC}"
    exit 0
else
    echo -e "\n${RED}⚠️  Algunos tests fallaron${NC}"
    exit 1
fi