# ⚡ Quick Start Guide

Want to get started **immediately**? This guide gets you running in 5 minutes.

## 🎯 What You Need

1. **Linux system** (Ubuntu, Arch, etc.)
2. **OpenAI API key** ([get one here](https://platform.openai.com/api-keys))
3. **5 minutes** of your time

## 🚀 Option 1: Download Binary (Easiest)

**No compilation needed!** Download pre-built version:

```bash
# Download latest release
wget https://github.com/your-username/gpt-terminal-assistant/releases/latest/download/gpt-terminal-assistant-binary.tar.gz

# Extract
tar -xzf gpt-terminal-assistant-binary.tar.gz
cd gpt-terminal-assistant/

# Install minimal dependencies (if needed)
sudo apt install -y jq curl  # Ubuntu/Debian
# OR
sudo pacman -S jq curl       # Arch Linux

# Configure API key
cp api/config.txt.example api/config.txt
nano api/config.txt  # Add your OpenAI API key

# Run!
./gpt_arch_mcp
```

## 🔨 Option 2: Build from Source (Latest)

**Requires .NET SDK** but gives you the latest version:

```bash
# Clone repository
git clone https://github.com/your-username/gpt-terminal-assistant.git
cd gpt-terminal-assistant/

# Install build dependencies
sudo apt install -y dotnet-sdk-8.0 gcc jq curl make  # Ubuntu/Debian
# OR  
sudo pacman -S dotnet-sdk gcc jq curl make            # Arch Linux

# Build (takes ~2 minutes)
make arch_mcp

# Configure API key  
cp api/config.txt.example api/config.txt
nano api/config.txt  # Add your OpenAI API key

# Run!
./gpt_arch_mcp
```

## 🔑 Configure Your API Key

1. **Get OpenAI API Key**: Visit [OpenAI Platform](https://platform.openai.com/api-keys)

2. **Add to config**:
   ```bash
   nano api/config.txt
   ```
   
3. **Replace the placeholder**:
   ```
   API_KEY=sk-your-actual-openai-api-key-here
   ```

4. **Save and exit** (Ctrl+X, then Y, then Enter in nano)

## 🎮 First Run

```bash
./gpt_arch_mcp
```

Try these commands:
- `ls` → Executes directly
- `"How do I check disk space?"` → GPT suggests `df -h`
- `/help` → Shows all commands
- `/status` → System information
- `exit` → Quit

## 🎯 What Can It Do?

### Direct Commands (Execute Immediately)
```bash
🤖 > ls -la
🔧 Executing: ls -la
[shows file listing]

🤖 > df -h  
🔧 Executing: df -h
[shows disk usage]
```

### Ask GPT Questions
```bash
🤖 > How do I find large files?
🤖 Processing with GPT...
--- 💬 GPT Response ---
You can find large files using the `find` command:
find /path -size +100M -type f
--- End ---
💡 GPT suggests running: find . -size +100M -type f
Execute? [y/N]: y
```

### Special Commands
```bash
🤖 > /help     # Show all commands
🤖 > /status   # System information  
🤖 > /diag     # Arch Linux diagnostics
🤖 > /clear    # Clear conversation history
```

## 🔍 Troubleshooting

### "Command not found: jq"
```bash
# Ubuntu/Debian
sudo apt install jq

# Arch Linux
sudo pacman -S jq
```

### "Permission denied"
```bash
chmod +x gpt_arch_mcp
```

### "API Error"
- Check your API key in `api/config.txt`
- Ensure you have OpenAI credits
- Test with: `make test_api`

### "Bridge not responding"
```bash
# Rebuild the bridge
make build_mcp_bridge

# Test bridge independently  
echo '{"Action":"get_system_info"}' | ./MCPBridge_native
```

## 🆘 Need Help?

1. **Check logs**: Any errors are shown directly in the terminal
2. **Read the docs**: See [README.md](README.md) for full documentation
3. **Build issues**: See [BUILD.md](BUILD.md) for detailed build instructions
4. **Open an issue**: [GitHub Issues](https://github.com/your-username/gpt-terminal-assistant/issues)

## 💡 Pro Tips

- **Save tokens**: Use `/clear` to reset conversation context
- **Arch Linux**: Use `/diag` for system diagnostics
- **Dangerous commands**: The system will ask for confirmation
- **Multiple modules**: Try `make chat` for general conversation mode

---

**⏱️ Total setup time: ~5 minutes**  
**🎉 You're ready to go!**