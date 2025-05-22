CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I. -Icommon/includes -Iapi

# Descubrir todos los archivos .c en common
COMMON_SRCS := $(shell find common -name "*.c")
API_SRCS := api/openai.c
MODULES_DIR = modulos

# Detectar automáticamente todos los módulos disponibles
AVAILABLE_MODULES := $(notdir $(wildcard $(MODULES_DIR)/*))

# Objetivo predeterminado
all: $(AVAILABLE_MODULES)

# Regla dinámica para compilar cualquier módulo
$(AVAILABLE_MODULES):
	@echo "Compilando módulo: $@"
	$(CC) $(CFLAGS) -DMODO_$(shell echo $@ | tr a-z A-Z) \
		$(INCLUDES) -I$(MODULES_DIR)/$@ \
		-o gpt_$@ main.c $(COMMON_SRCS) $(API_SRCS) $(MODULES_DIR)/$@/executor.c
	@echo "✅ Módulo $@ compilado como: gpt_$@"

# Add a default CONFIG_FILE definition for cases where no module is specified
.DEFAULT:
	$(CC) $(CFLAGS) -DDEFAULT_CONFIG_FILE="default/config.ini" \
		$(INCLUDES) -o gpt_default main.c $(COMMON_SRCS) $(API_SRCS)

# Listar módulos disponibles
list:
	@echo "Módulos disponibles para compilar:"
	@for module in $(AVAILABLE_MODULES); do \
		echo "  - $$module (make $$module)"; \
	done

# Verificar API key
test_api:
	@echo "Probando conexión a la API de OpenAI..."
	@curl -s https://api.openai.com/v1/models -H "Authorization: Bearer $$(cat api/config.txt | grep API_KEY | cut -d= -f2)" | grep -q "gpt-3.5-turbo" && echo "✓ API Key válida y conexión establecida correctamente." || echo "✗ Error al conectar con la API. Verifica tu API Key y conexión a internet."

# Limpiar archivos compilados
clean:
	rm -f gpt_*
	rm -f req.json out.txt resp.json context.txt

help:
	@echo "Comandos disponibles:"
	@echo "  make          - Compila todos los módulos"
	@echo "  make [modulo] - Compila un módulo específico (ej: make chat)"
	@echo "  make list     - Muestra los módulos disponibles"
	@echo "  make clean    - Elimina archivos compilados y temporales"
	@echo "  make test_api - Verifica si la API key es válida"
	@echo "  make help     - Muestra esta ayuda"

.PHONY: all list clean help test_api $(AVAILABLE_MODULES)
# Incluir reglas MCP (opcional)
-include Makefile.mcp
