# 🚀 GPT Assistant con Model Context Protocol (MCP)

Un asistente de terminal inteligente especializado en Arch Linux que combina la potencia de GPT con capacidades MCP (Model Context Protocol) para ejecución directa de comandos del sistema.

## 🎯 Características principales

- **🤖 Chat híbrido**: Combina conversación con GPT y ejecución directa de comandos
- **🔧 Detección automática**: Reconoce comandos del usuario y los ejecuta sin confirmación
- **🌉 Bridge MCP nativo**: Implementación Native AOT sin dependencias externas
- **📊 Diagnósticos especializados**: Herramientas específicas para Arch Linux
- **🔄 Contexto continuo**: Mantiene historial de comandos y respuestas
- **📁 Arquitectura modular**: Fácil extensión para otros casos de uso

## 🏗️ Arquitectura del sistema

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   gpt_arch_mcp  │◄──►│  MCPBridge_native │◄──►│  Comandos del   │
│   (Programa C)  │    │   (Bridge .NET)   │    │     Sistema     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
        │                                                │
        ▼                                                │
┌─────────────────┐                                     │
│  API OpenAI     │                                     │
│   (GPT-4o)      │                                     │
└─────────────────┘                                     │
                                                         │
┌────────────────────────────────────────────────────────┘
│                    Flujo de trabajo
│  1. Usuario escribe: "ls" → Se ejecuta directamente
│  2. Usuario pregunta: "¿Cómo ver el espacio?" → GPT responde + sugiere "df -h"
│  3. Comandos especiales: /status, /diag, /help
└────────────────────────────────────────────────────────
```

## 🛠️ Tecnologías utilizadas

- **C**: Programa principal y cliente MCP
- **C# + .NET 8**: Bridge MCP con Native AOT
- **OpenAI API**: Integración con GPT-4o
- **JSON**: Comunicación entre componentes
- **Bash**: Ejecución de comandos del sistema

## 📦 Instalación y configuración

### Requisitos previos

- **Sistema operativo**: Linux (probado en Ubuntu y Arch Linux)
- **.NET 8.0 SDK**: Para compilar el bridge (no necesario en el sistema final)
- **GCC**: Para compilar el código C
- **jq**: Para procesamiento JSON
- **curl**: Para comunicación con API de OpenAI

### Instalación rápida

```bash
# 1. Clonar el repositorio
git clone https://github.com/tu-usuario/2.0.git
cd 2.0

# 2. Instalar dependencias
sudo apt update && sudo apt install -y dotnet-sdk-8.0 gcc jq curl

# 3. Configurar API key
echo "API_KEY=tu_openai_api_key_aqui" > api/config.txt

# 4. Compilar todo
make arch_mcp

# 5. Ejecutar
./gpt_arch_mcp
```

### Configuración avanzada

El sistema es completamente modular. Puedes crear nuevos módulos copiando la estructura de `modulos/arch_mcp/`:

```bash
# Crear nuevo módulo
make create_module
# Seguir las instrucciones interactivas
```

## 🎮 Uso del sistema

### Comandos directos
Los comandos se detectan y ejecutan automáticamente:

```bash
🤖 > ls -la
🔧 Ejecutando: ls -la
--- Resultado ---
total 1024
drwxr-xr-x  8 user user  4096 ene 25 10:30 .
[... salida del comando ...]
--- Fin ---
```

### Conversación con GPT
Las preguntas se envían automáticamente a GPT:

```bash
🤖 > ¿Cómo puedo ver el espacio en disco?
🤖 Procesando con GPT...
--- 💬 Respuesta GPT ---
Para ver el espacio en disco, puedes usar el comando `df -h`:

```bash
df -h
```

Este comando muestra el uso de disco en formato legible.
--- Fin ---

💡 GPT sugiere ejecutar: df -h
¿Deseas ejecutarlo? [s/N]: s
```

### Comandos especiales

- `/help` - Mostrar ayuda completa
- `/status` - Información del sistema vía MCP
- `/diag` - Diagnóstico completo de Arch Linux
- `/clear` - Limpiar contexto de conversación
- `/mcp` - Estado del bridge MCP
- `salir/exit/quit` - Terminar programa

## 🧩 Módulos disponibles

### arch_mcp (Principal)
- **Especialización**: Instalación y mantenimiento de Arch Linux
- **Características**: Diagnósticos específicos, detección de comandos Arch
- **Configuración**: `modulos/arch_mcp/config.ini`

### arch (Original)
- **Descripción**: Versión original sin MCP
- **Uso**: `make arch && ./gpt_arch`
- **Propósito**: Comparación y fallback

### chat
- **Descripción**: Asistente conversacional general
- **Uso**: `make chat && ./gpt_chat`

### creator
- **Descripción**: Generador de estructuras de proyecto
- **Uso**: `make creator && ./gpt_creator`

## 🔧 Comandos de desarrollo

```bash
# Compilación
make arch_mcp              # Compilar versión MCP
make arch                  # Compilar versión original
make list                  # Ver todos los módulos

# Testing
make test_mcp              # Probar bridge MCP
make test_api              # Probar API key
make check_mcp_deps        # Verificar dependencias

# Limpieza
make clean                 # Limpiar archivos compilados
make clean_mcp             # Limpiar solo archivos MCP

# Ayuda
make help                  # Ayuda general
make help_mcp              # Ayuda específica MCP
```

## 📁 Estructura del proyecto

```
gpt-assistant/
├── 📂 api/                     # Configuración API OpenAI
│   ├── openai.c                # Cliente API
│   ├── openai.h
│   └── config.txt              # API key (no incluido en repo)
├── 📂 common/                  # Utilidades compartidas
│   ├── includes/
│   ├── utils.c                 # Funciones de utilidad
│   ├── config_manager.c        # Gestor de configuración
│   └── context.c               # Manejo de contexto
├── 📂 modulos/                 # Módulos especializados
│   ├── arch/                   # Módulo original Arch
│   ├── arch_mcp/              # Módulo Arch con MCP
│   ├── chat/                   # Módulo conversacional
│   └── creator/                # Módulo generador
├── 📄 main.c                   # Main original
├── 📄 main_mcp.c               # Main extendido con MCP
├── 📄 mcp_client.h/c           # Cliente MCP en C
├── 📄 MCPBridge.cs             # Bridge MCP en C#
├── 📄 MCPBridge.csproj         # Proyecto .NET
├── 📄 Makefile                 # Build principal
├── 📄 Makefile.mcp             # Build MCP
└── 📄 README.md                # Esta documentación
```

## 🚀 Distribución

Para distribuir el asistente compilado:

```bash
# Crear paquete de distribución
make dist

# Resultado: gpt_assistant_dist.tar.gz
# Contiene solo los archivos necesarios para ejecutar
```

### Archivos necesarios para distribución:
- `gpt_arch_mcp` - Ejecutable principal
- `MCPBridge_native` - Bridge MCP (sin dependencias)
- `api/config.txt` - Configuración API
- `modulos/arch_mcp/` - Configuración del módulo

## ⚠️ Problemas conocidos

1. **Vulnerabilidades en System.Text.Json 8.0.0**: Advertencias durante compilación (no afectan funcionalidad)
2. **Comandos múltiples**: Los comandos en la misma línea se interpretan como preguntas a GPT
3. **Dependencia de jq**: Requerido para procesamiento JSON de respuestas OpenAI

## 🤝 Contribución

1. Fork el proyecto
2. Crea una branch para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. Commit tus cambios (`git commit -am 'Agregar nueva funcionalidad'`)
4. Push a la branch (`git push origin feature/nueva-funcionalidad`)
5. Crea un Pull Request

### Estructura para nuevos módulos:
```
modulos/nuevo_modulo/
├── config.ini          # Configuración del módulo
├── role.txt            # Rol específico para GPT
├── executor.h          # Header del executor
└── executor.c          # Implementación del executor
```

## 📄 Licencia

MIT License - ver archivo `LICENSE` para detalles.

## 🙏 Agradecimientos

- **OpenAI**: Por proporcionar la API GPT-4o
- **Microsoft**: Por .NET Native AOT
- **Anthropic**: Por el concepto Model Context Protocol
- **Comunidad Arch Linux**: Por la inspiración y testing

## 📞 Soporte

- **Issues**: [GitHub Issues](https://github.com/tu-usuario/2.0/issues)
- **Discusiones**: [GitHub Discussions](https://github.com/tu-usuario/2.0/discussions)
- **Email**: tu-email@ejemplo.com

---

**⭐ Si te gusta el proyecto, dale una estrella en GitHub!**