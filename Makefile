CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I. -Icommon/includes -Iapi

# Directorio de salida para todos los binarios
OUT_DIR = out

# Descubrir todos los archivos .c en common
COMMON_SRCS := $(shell find common -name "*.c")
API_SRCS := api/openai.c
MODULES_DIR = modulos

# Detectar automáticamente todos los módulos disponibles
AVAILABLE_MODULES := $(notdir $(wildcard $(MODULES_DIR)/*))

# Objetivo predeterminado
all: $(AVAILABLE_MODULES)

# Crear directorio de salida si no existe
$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

# Regla dinámica para compilar cualquier módulo
$(AVAILABLE_MODULES): $(OUT_DIR)
	@echo "🔨 Compilando módulo: $@"
	$(CC) $(CFLAGS) -DMODO_$(shell echo $@ | tr a-z A-Z) \
		$(INCLUDES) -I$(MODULES_DIR)/$@ \
		-o $(OUT_DIR)/gpt_$@ main.c $(COMMON_SRCS) $(API_SRCS) $(MODULES_DIR)/$@/executor.c
	@echo "✅ Módulo $@ compilado como: $(OUT_DIR)/gpt_$@"

# Regla predeterminada
.DEFAULT: $(OUT_DIR)
	$(CC) $(CFLAGS) -DDEFAULT_CONFIG_FILE="default/config.ini" \
		$(INCLUDES) -o $(OUT_DIR)/gpt_default main.c $(COMMON_SRCS) $(API_SRCS)

# Listar módulos disponibles
list:
	@echo "📋 Módulos disponibles para compilar:"
	@for module in $(AVAILABLE_MODULES); do \
		echo "  - $$module → $(OUT_DIR)/gpt_$$module"; \
	done

# Verificar API key
test_api:
	@echo "🔍 Probando conexión a la API de OpenAI..."
	@if [ ! -f api/config.txt ]; then \
		echo "❌ No se encontró api/config.txt. Crea uno basado en api/config.txt.example"; \
		exit 1; \
	fi
	@curl -s https://api.openai.com/v1/models -H "Authorization: Bearer $$(cat api/config.txt | grep API_KEY | cut -d= -f2)" | grep -q "gpt-3.5-turbo" && echo "✅ API Key válida y conexión establecida correctamente." || echo "❌ Error al conectar con la API. Verifica tu API Key y conexión a internet."

# Limpiar archivos compilados
clean:
	@echo "🧹 Limpiando archivos compilados..."
	rm -rf $(OUT_DIR)/
	rm -f req.json out.txt resp.json context.txt *.tar.gz
	@echo "✅ Directorio $(OUT_DIR)/ eliminado"

# Crear script de ejecución para facilidad de uso
create_runners: $(OUT_DIR)
	@echo "📝 Creando scripts de ejecución..."
	@for exe in $(OUT_DIR)/gpt_*; do \
		if [ -f "$$exe" ]; then \
			name=$$(basename $$exe); \
			echo "#!/bin/bash" > $$name.sh; \
			echo "cd \"\$$(dirname \"\$$0\")\"" >> $$name.sh; \
			echo "./$(OUT_DIR)/$$name \"\$$@\"" >> $$name.sh; \
			chmod +x $$name.sh; \
			echo "  - $$name.sh → $$exe"; \
		fi \
	done
	@echo "✅ Scripts de ejecución creados en el directorio raíz"

help:
	@echo "🚀 Comandos disponibles:"
	@echo "  make                - Compila todos los módulos en $(OUT_DIR)/"
	@echo "  make [modulo]       - Compila un módulo específico (ej: make chat)"
	@echo "  make list           - Muestra los módulos disponibles"
	@echo "  make clean          - Elimina $(OUT_DIR)/ y archivos temporales"
	@echo "  make test_api       - Verifica si la API key es válida"
	@echo "  make create_runners - Crea scripts .sh para ejecutar desde raíz"
	@echo "  make help           - Muestra esta ayuda"
	@echo ""
	@echo "📁 Estructura de salida:"
	@echo "  $(OUT_DIR)/gpt_arch     - Versión original de Arch"
	@echo "  $(OUT_DIR)/gpt_arch_mcp - Versión Arch con MCP (requiere make arch_mcp)"
	@echo "  $(OUT_DIR)/gpt_chat     - Módulo conversacional"
	@echo "  $(OUT_DIR)/gpt_creator  - Generador de proyectos"
	@echo ""
	@echo "💡 Para usar MCP: make -f Makefile.mcp arch_mcp"

.PHONY: all list clean help test_api create_runners $(AVAILABLE_MODULES)

# Incluir reglas MCP (opcional)
-include Makefile.mcp