using System.Text.Json;
using System.Text.Json.Serialization;
using System.Diagnostics;

namespace MCPBridge;

// Source Generator Context para Native AOT
[JsonSerializable(typeof(MCPCommand))]
[JsonSerializable(typeof(MCPResponse))]
[JsonSerializable(typeof(SystemInfo))]
[JsonSerializable(typeof(TextAnalysis))]
public partial class JsonContext : JsonSerializerContext { }

public class MCPCommand
{
    public string Action { get; set; } = "";
    public string? Data { get; set; }
}

public class MCPResponse
{
    public bool Success { get; set; }
    public string? Result { get; set; }
    public string? Error { get; set; }
}

public class SystemInfo
{
    public string OS { get; set; } = "";
    public string Machine { get; set; } = "";
    public string User { get; set; } = "";
    public string WorkingDirectory { get; set; } = "";
    public int ProcessorCount { get; set; }
}

public class TextAnalysis
{
    public bool IsCommand { get; set; }
    public string Text { get; set; } = "";
    public string CommandType { get; set; } = "";
}

class Program
{
    static async Task Main(string[] args)
    {
        if (args.Length > 0 && args[0] == "--help")
        {
            Console.WriteLine("MCPBridge Native - Bridge nativo para MCP");
            Console.WriteLine("Uso: ./MCPBridge");
            return;
        }

        try
        {
            while (true)
            {
                var line = Console.ReadLine();
                if (string.IsNullOrEmpty(line) || line == "EXIT")
                    break;

                var command = JsonSerializer.Deserialize(line, JsonContext.Default.MCPCommand);
                if (command != null)
                {
                    var response = await ProcessCommand(command);
                    var jsonResponse = JsonSerializer.Serialize(response, JsonContext.Default.MCPResponse);
                    Console.WriteLine(jsonResponse);
                    Console.Out.Flush();
                }
            }
        }
        catch (Exception ex)
        {
            var errorResponse = new MCPResponse 
            { 
                Success = false, 
                Error = $"Bridge error: {ex.Message}" 
            };
            var jsonError = JsonSerializer.Serialize(errorResponse, JsonContext.Default.MCPResponse);
            Console.WriteLine(jsonError);
        }
    }

    static async Task<MCPResponse> ProcessCommand(MCPCommand command)
    {
        try
        {
            return command.Action.ToLower() switch
            {
                "execute_command" => await ExecuteSystemCommand(command.Data ?? ""),
                "analyze_text" => AnalyzeText(command.Data ?? ""),
                "get_system_info" => GetSystemInfo(),
                "arch_diagnostics" => await ArchDiagnostics(),
                _ => new MCPResponse 
                { 
                    Success = false, 
                    Error = $"Comando desconocido: {command.Action}" 
                }
            };
        }
        catch (Exception ex)
        {
            return new MCPResponse 
            { 
                Success = false, 
                Error = ex.Message 
            };
        }
    }

    static async Task<MCPResponse> ExecuteSystemCommand(string command)
    {
        try
        {
            using var process = new Process();
            process.StartInfo = new ProcessStartInfo
            {
                FileName = "/bin/bash",
                Arguments = $"-c \"{command.Replace("\"", "\\\"")}\"",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            };
            
            process.Start();
            
            var outputTask = process.StandardOutput.ReadToEndAsync();
            var errorTask = process.StandardError.ReadToEndAsync();
            
            await process.WaitForExitAsync();
            
            var output = await outputTask;
            var error = await errorTask;
            
            var result = output;
            if (!string.IsNullOrEmpty(error))
                result += $"\n[stderr]: {error}";
            
            result += $"\n[exit_code]: {process.ExitCode}";
            
            return new MCPResponse 
            { 
                Success = process.ExitCode == 0, 
                Result = result.Trim()
            };
        }
        catch (Exception ex)
        {
            return new MCPResponse 
            { 
                Success = false, 
                Error = $"Error ejecutando: {ex.Message}" 
            };
        }
    }

    static MCPResponse AnalyzeText(string text)
    {
        var isCommand = IsLikelyCommand(text);
        var commandType = isCommand ? DetectCommandType(text) : "none";
        
        var analysis = new TextAnalysis
        {
            IsCommand = isCommand,
            Text = text,
            CommandType = commandType
        };

        var analysisJson = JsonSerializer.Serialize(analysis, JsonContext.Default.TextAnalysis);
        return new MCPResponse 
        { 
            Success = true, 
            Result = analysisJson 
        };
    }

    static MCPResponse GetSystemInfo()
    {
        try
        {
            var info = new SystemInfo
            {
                OS = Environment.OSVersion.ToString(),
                Machine = Environment.MachineName,
                User = Environment.UserName,
                WorkingDirectory = Environment.CurrentDirectory,
                ProcessorCount = Environment.ProcessorCount
            };

            var infoJson = JsonSerializer.Serialize(info, JsonContext.Default.SystemInfo);
            return new MCPResponse 
            { 
                Success = true, 
                Result = infoJson 
            };
        }
        catch (Exception ex)
        {
            return new MCPResponse 
            { 
                Success = false, 
                Error = ex.Message 
            };
        }
    }

    static async Task<MCPResponse> ArchDiagnostics()
    {
        var commands = new[]
        {
            "uname -a",
            "lsblk -f", 
            "df -h",
            "free -h"
        };

        var results = new List<string>();
        
        foreach (var cmd in commands)
        {
            var response = await ExecuteSystemCommand(cmd);
            results.Add($"=== {cmd} ===");
            results.Add(response.Result ?? "Error");
            results.Add("");
        }

        return new MCPResponse
        {
            Success = true,
            Result = string.Join("\n", results)
        };
    }

    static bool IsLikelyCommand(string text)
    {
        if (string.IsNullOrWhiteSpace(text)) return false;
        
        text = text.Trim();
        
        var commandPatterns = new[]
        {
            "ls", "dir", "pwd", "cd", "cat", "grep", "find", "ps", "top", 
            "df", "du", "free", "uname", "pacman", "yay", "systemctl", 
            "journalctl", "sudo", "su", "chmod", "mkdir", "rm", "cp", "mv"
        };

        return commandPatterns.Any(cmd => 
            text.StartsWith(cmd, StringComparison.OrdinalIgnoreCase) &&
            (text.Length == cmd.Length || char.IsWhiteSpace(text[cmd.Length])));
    }

    static string DetectCommandType(string text)
    {
        text = text.Trim().ToLower();
        
        return text switch
        {
            var t when t.StartsWith("pacman") || t.StartsWith("yay") => "package_manager",
            var t when t.StartsWith("systemctl") => "systemd", 
            var t when t.StartsWith("ls") || t.StartsWith("find") => "file_system",
            var t when t.StartsWith("ps") || t.StartsWith("top") => "process",
            var t when t.StartsWith("cd") || t.StartsWith("pwd") => "navigation",
            _ => "general"
        };
    }
}