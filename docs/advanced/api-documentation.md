# 📚 API Documentation - GPT Assistant MCP

## 🏗️ Arquitectura del Sistema

### Componentes principales

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│   Cliente C     │ ←→  │   Bridge .NET    │ ←→  │ Sistema/OpenAI  │
│  (gpt_arch_mcp) │     │ (MCPBridge_native)│     │                 │
└─────────────────┘     └──────────────────┘     └─────────────────┘
         ↓                       ↓                         ↓
    User Input           JSON Protocol              Commands/API
```

## 🔌 MCP Client API (C)

### Estructuras de datos

```c
typedef struct {
    FILE* bridge_in;      // Stream de entrada al bridge
    FILE* bridge_out;     // Stream de salida del bridge
    pid_t bridge_pid;     // PID del proceso bridge
} MCPClient;

typedef struct {
    int success;          // 1 = éxito, 0 = error
    char* result;         // Resultado de la operación (puede ser NULL)
    char* error;          // Mensaje de error (puede ser NULL)
} MCPResponse;
```

### Funciones principales

#### `MCPClient* mcp_create_client()`
Crea e inicializa un cliente MCP.

**Retorna:**
- Puntero a `MCPClient` en caso de éxito
- `NULL` en caso de error

**Ejemplo:**
```c
MCPClient* client = mcp_create_client();
if (!client) {
    fprintf(stderr, "Error al crear cliente MCP\n");
    return 1;
}
```

#### `void mcp_cleanup(MCPClient* client)`
Libera recursos y termina el bridge.

**Parámetros:**
- `client`: Cliente a limpiar

**Ejemplo:**
```c
mcp_cleanup(client);
```

#### `MCPResponse* mcp_execute_command(MCPClient* client, const char* command)`
Ejecuta un comando del sistema a través del bridge.

**Parámetros:**
- `client`: Cliente MCP activo
- `command`: Comando a ejecutar

**Retorna:**
- `MCPResponse` con el resultado

**Ejemplo:**
```c
MCPResponse* response = mcp_execute_command(client, "ls -la");
if (response && response->success) {
    printf("Resultado: %s\n", response->result);
}
mcp_free_response(response);
```

#### `MCPResponse* mcp_analyze_text(MCPClient* client, const char* text)`
Analiza texto para detectar si es un comando.

**Parámetros:**
- `client`: Cliente MCP activo
- `text`: Texto a analizar

**Retorna:**
- `MCPResponse` con análisis JSON

#### `MCPResponse* mcp_get_system_info(MCPClient* client)`
Obtiene información del sistema.

**Retorna:**
- `MCPResponse` con información JSON

#### `int is_user_command(const char* text)`
Detecta si el texto es un comando (función local, no usa MCP).

**Parámetros:**
- `text`: Texto a verificar

**Retorna:**
- 1 si es comando, 0 si no

## 🌉 Bridge Protocol (JSON)

### Formato de comandos

```json
{
    "Action": "nombre_accion",
    "Data": "datos_opcionales"
}
```

### Formato de respuestas

```json
{
    "Success": true|false,
    "Result": "resultado_si_exitoso",
    "Error": "mensaje_error_si_fallo"
}
```

### Acciones disponibles

#### `execute_command`
Ejecuta un comando del sistema.

**Request:**
```json
{
    "Action": "execute_command",
    "Data": "ls -la /home"
}
```

**Response:**
```json
{
    "Success": true,
    "Result": "total 24\ndrwxr-xr-x 3 user user...\n[exit_code]: 0"
}
```

#### `analyze_text`
Analiza si un texto es un comando.

**Request:**
```json
{
    "Action": "analyze_text",
    "Data": "pacman -Syu"
}
```

**Response:**
```json
{
    "Success": true,
    "Result": "{\"IsCommand\":true,\"Text\":\"pacman -Syu\",\"CommandType\":\"package_manager\"}"
}
```

#### `get_system_info`
Obtiene información del sistema.

**Request:**
```json
{
    "Action": "get_system_info"
}
```

**Response:**
```json
{
    "Success": true,
    "Result": "{\"OS\":\"Unix 6.5.0.0\",\"Machine\":\"hostname\",\"User\":\"username\",...}"
}
```

#### `arch_diagnostics`
Ejecuta diagnóstico completo de Arch Linux.

**Request:**
```json
{
    "Action": "arch_diagnostics"
}
```

## 🔧 Creación de Módulos

### Estructura de un módulo

```
modulos/mi_modulo/
├── config.ini      # Configuración del módulo
├── role.txt        # Rol del sistema para GPT
├── executor.h      # Header con declaraciones
└── executor.c      # Implementación
```

### API del módulo

Cada módulo debe implementar:

```c
// executor.h
char* extract_command_MODULO(const char* text);
char* run_command_MODULO(const char* cmd);
```

### Integración en main.c

```c
#ifdef MODO_MI_MODULO
#include "modulos/mi_modulo/executor.h"
#define MODULE_NAME "Mi Módulo Personalizado"
#define CONFIG_FILE "modulos/mi_modulo/config.ini"
#define extract_command extract_command_mi_modulo
#define run_command run_command_mi_modulo
#endif
```

## 🛠️ OpenAI API Integration

### Función principal

```c
char* send_prompt(const char* prompt, const char* config_file);
```

**Parámetros:**
- `prompt`: Texto a enviar a GPT
- `config_file`: Ruta al archivo de configuración

**Retorna:**
- String con la respuesta (debe ser liberado con `free()`)

### Configuración

El archivo `config.ini` soporta:

```ini
MODEL=gpt-4o
TEMPERATURE=0.7
MAX_TOKENS=2000
API_KEY_FILE=api/config.txt
ROLE_FILE=modulos/mi_modulo/role.txt
SYSTEM_ROLE=system
SYSTEM_CONTENT=Descripción del asistente
```

## 📦 Funciones Utilitarias

### `char* trim(char* str)`
Elimina espacios al inicio y final.

### `char* extract_command_improved(const char* text, const char* language)`
Extrae comandos de bloques de código.

**Parámetros:**
- `text`: Texto donde buscar
- `language`: Lenguaje del bloque (ej: "bash")

### `char* run_command_improved(const char* cmd)`
Ejecuta comando capturando stdout y stderr.

## 🔒 Consideraciones de Seguridad

### Validación de comandos

El bridge implementa validaciones básicas:

1. **Lista negra de comandos peligrosos**
2. **Límite de longitud de comando** (1024 chars)
3. **Timeout de ejecución** (30 segundos)
4. **Logging de auditoría**

### Sanitización de entrada

```c
// Ejemplo de escape para JSON
for (const char* p = data; *p; p++) {
    if (*p == '"' || *p == '\\') {
        fputc('\\', stream);
    }
    fputc(*p, stream);
}
```

## 🧪 Testing

### Test unitario de función

```c
// test_is_command.c
#include <assert.h>
#include "mcp_client.h"

int main() {
    assert(is_user_command("ls") == 1);
    assert(is_user_command("¿qué es ls?") == 0);
    assert(is_user_command("pacman -Syu") == 1);
    return 0;
}
```

### Test de integración

```bash
# test_integration.sh
echo '{"Action":"execute_command","Data":"echo test"}' | ./MCPBridge_native | jq .
```

## 📈 Métricas y Monitoreo

### Logs del sistema

- **context.txt**: Historial de conversación
- **mcp_audit.log**: Comandos ejecutados (si se habilita)
- **req.json/resp.json**: Última comunicación con OpenAI

### Performance

- Tiempo de respuesta del bridge: < 100ms
- Overhead de IPC: ~1ms
- Memoria base: ~50MB

## 🚀 Ejemplos de Uso Avanzado

### Cliente personalizado

```c
#include "mcp_client.h"

int main() {
    MCPClient* client = mcp_create_client();
    if (!client) return 1;
    
    // Ejecutar múltiples comandos
    const char* commands[] = {"uname -a", "df -h", "free -m", NULL};
    
    for (int i = 0; commands[i]; i++) {
        MCPResponse* resp = mcp_execute_command(client, commands[i]);
        if (resp && resp->success) {
            printf("=== %s ===\n%s\n", commands[i], resp->result);
        }
        mcp_free_response(resp);
    }
    
    mcp_cleanup(client);
    return 0;
}
```

### Extensión del bridge

```csharp
// Agregar nueva acción al bridge
case "custom_analysis":
    return PerformCustomAnalysis(command.Data);

static MCPResponse PerformCustomAnalysis(string data) {
    // Implementación personalizada
    return new MCPResponse {
        Success = true,
        Result = "Análisis completado"
    };
}
```

## 📞 Soporte

- **Issues**: GitHub Issues del proyecto
- **Documentación**: `/docs` en el repositorio
- **Ejemplos**: `/examples` en el repositorio