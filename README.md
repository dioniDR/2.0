# 🚀 GPT Terminal Assistant with MCP

A modular, intelligent terminal assistant that combines GPT's conversational AI with Model Context Protocol (MCP) for direct system command execution. Specialized for Arch Linux but extensible to other use cases.

## ✨ Features

- **🤖 Hybrid Chat**: Combines GPT conversation with direct command execution
- **🔧 Auto-detection**: Recognizes user commands and executes them automatically
- **🌉 Native MCP Bridge**: Implementation with Native AOT (no external dependencies)
- **📊 Specialized Diagnostics**: Arch Linux specific tools and commands
- **🔄 Continuous Context**: Maintains command and response history
- **📁 Modular Architecture**: Easy to extend for other use cases

## 🏗️ Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   gpt_arch_mcp  │◄──►│  MCPBridge_native │◄──►│  System         │
│   (C Program)   │    │   (.NET Bridge)   │    │  Commands       │
└─────────────────┘    └──────────────────┘    └─────────────────┘
        │                                                │
        ▼                                                │
┌─────────────────┐                                     │
│  OpenAI API     │                                     │
│   (GPT-4o)      │                                     │
└─────────────────┘                                     │
                                                         │
┌────────────────────────────────────────────────────────┘
│                    Workflow
│  1. User types: "ls" → Executes directly
│  2. User asks: "How to check disk space?" → GPT responds + suggests "df -h"
│  3. Special commands: /status, /diag, /help
└────────────────────────────────────────────────────────
```

## 📦 Prerequisites

- **Operating System**: Linux (tested on Ubuntu and Arch Linux)
- **.NET 8.0 SDK**: For building the bridge (not required on final system)
- **GCC**: For compiling C code
- **jq**: For JSON processing
- **curl**: For OpenAI API communication

## 🚀 Quick Start

> **⚡ Want to start immediately?** See [QUICKSTART.md](QUICKSTART.md) for 5-minute setup

### For Users (Download Binary)
```bash
# Download pre-built version (no .NET needed)
wget https://github.com/your-username/gpt-terminal-assistant/releases/latest/download/gpt-terminal-assistant-binary.tar.gz
tar -xzf gpt-terminal-assistant-binary.tar.gz && cd gpt-terminal-assistant/
cp api/config.txt.example api/config.txt && nano api/config.txt  # Add API key
./gpt_arch_mcp
```

### For Developers (Build from Source)
```bash
# Clone and build
git clone https://github.com/your-username/gpt-terminal-assistant.git
cd gpt-terminal-assistant
sudo apt install -y dotnet-sdk-8.0 gcc jq curl  # Install dependencies
make arch_mcp  # Build everything
cp api/config.txt.example api/config.txt && nano api/config.txt  # Add API key
./gpt_arch_mcp
```

> **📖 Need detailed build instructions?** See [BUILD.md](BUILD.md)

## 🎮 Usage

### Direct Commands
Commands are detected and executed automatically:

```bash
🤖 > ls -la
🔧 Executing: ls -la
--- Result ---
total 1024
drwxr-xr-x  8 user user  4096 jan 25 10:30 .
[... command output ...]
--- End ---
```

### Conversation with GPT
Questions are automatically sent to GPT:

```bash
🤖 > How can I check disk space?
🤖 Processing with GPT...
--- 💬 GPT Response ---
To check disk space, you can use the `df -h` command:

```bash
df -h
```

This command shows disk usage in human-readable format.
--- End ---

💡 GPT suggests running: df -h
Do you want to execute it? [y/N]: y
```

### Special Commands

- `/help` - Show complete help
- `/status` - System information via MCP
- `/diag` - Complete Arch Linux diagnostics
- `/clear` - Clear conversation context
- `/mcp` - MCP bridge status
- `exit/salir/quit` - Exit program

## 🧩 Available Modules

### arch_mcp (Main)
- **Specialization**: Arch Linux installation and maintenance
- **Features**: Specific diagnostics, Arch command detection
- **Configuration**: `modulos/arch_mcp/config.ini`

### arch (Original)
- **Description**: Original version without MCP
- **Usage**: `make arch && ./gpt_arch`
- **Purpose**: Comparison and fallback

### chat
- **Description**: General conversational assistant
- **Usage**: `make chat && ./gpt_chat`

### creator
- **Description**: Project structure generator
- **Usage**: `make creator && ./gpt_creator`

## 🔧 Development Commands

```bash
# Compilation
make arch_mcp              # Build MCP version
make arch                  # Build original version
make list                  # See all modules

# Testing
make test_mcp              # Test MCP bridge
make test_api              # Test API key
make check_mcp_deps        # Check dependencies

# Cleanup
make clean                 # Clean compiled files
make clean_mcp             # Clean MCP files only

# Help
make help                  # General help
make help_mcp              # MCP specific help
```

## 📁 Project Structure

```
gpt-terminal-assistant/
├── 📂 api/                     # OpenAI API configuration
│   ├── openai.c                # API client
│   ├── openai.h
│   └── config.txt.example      # API key template
├── 📂 common/                  # Shared utilities
│   ├── includes/
│   ├── utils.c                 # Utility functions
│   ├── config_manager.c        # Configuration manager
│   └── context.c               # Context handling
├── 📂 modulos/                 # Specialized modules
│   ├── arch/                   # Original Arch module
│   ├── arch_mcp/              # Arch module with MCP
│   ├── chat/                   # Conversational module
│   └── creator/                # Generator module
├── 📄 main.c                   # Original main
├── 📄 main_mcp.c               # Extended main with MCP
├── 📄 mcp_client.h/c           # MCP client in C
├── 📄 MCPBridge.cs             # MCP bridge in C#
├── 📄 MCPBridge.csproj         # .NET project
├── 📄 Makefile                 # Main build
├── 📄 Makefile.mcp             # MCP build
└── 📄 README.md                # This documentation
```

## 🔧 Creating New Modules

Use the interactive module creator:

```bash
./create_module.sh
```

Or manually create the structure:

```
modulos/new_module/
├── config.ini          # Module configuration
├── role.txt            # Specific role for GPT
├── executor.h          # Executor header
└── executor.c          # Executor implementation
```

## 🚀 Distribution

To create a distribution package:

```bash
# Create distribution package
make dist_installer

# Result: gpt_arch_installer_dist.tar.gz
# Contains only necessary files to run
```

### Files needed for distribution:
- `gpt_arch_mcp` - Main executable
- `MCPBridge_native` - MCP bridge (no dependencies)
- `api/config.txt` - API configuration
- `modulos/arch_mcp/` - Module configuration

## 🤝 Contributing

1. Fork the project
2. Create your feature branch (`git checkout -b feature/new-feature`)
3. Commit your changes (`git commit -am 'Add new feature'`)
4. Push to the branch (`git push origin feature/new-feature`)
5. Create a Pull Request

## 📄 License

MIT License - see `LICENSE` file for details.

## 🙏 Acknowledgments

- **OpenAI**: For providing the GPT-4o API
- **Microsoft**: For .NET Native AOT
- **Anthropic**: For the Model Context Protocol concept
- **Arch Linux Community**: For inspiration and testing

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/your-username/gpt-terminal-assistant/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-username/gpt-terminal-assistant/discussions)

---

**⭐ If you like this project, give it a star on GitHub!**