#include "mcp_client.h"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

static char* read_json_response(FILE* stream) {
    char* buffer = malloc(8192);
    if (!buffer) return NULL;
    
    if (fgets(buffer, 8192, stream)) {
        buffer[strcspn(buffer, "\n")] = 0;
        return buffer;
    }
    
    free(buffer);
    return NULL;
}

MCPClient* mcp_create_client() {
    MCPClient* client = malloc(sizeof(MCPClient));
    if (!client) return NULL;
    
    int to_bridge[2], from_bridge[2];
    
    if (pipe(to_bridge) == -1 || pipe(from_bridge) == -1) {
        free(client);
        return NULL;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(to_bridge[0]); close(to_bridge[1]);
        close(from_bridge[0]); close(from_bridge[1]);
        free(client);
        return NULL;
    }
    
    if (pid == 0) {
        // Proceso hijo - ejecutar el bridge
        close(to_bridge[1]);
        close(from_bridge[0]);
        
        dup2(to_bridge[0], STDIN_FILENO);
        dup2(from_bridge[1], STDOUT_FILENO);
        
        close(to_bridge[0]);
        close(from_bridge[1]);
        
        // Ejecutar el bridge nativo
        execl("./MCPBridge_native", "MCPBridge_native", NULL);
        exit(1);
    }
    
    // Proceso padre
    close(to_bridge[0]);
    close(from_bridge[1]);
    
    client->bridge_in = fdopen(to_bridge[1], "w");
    client->bridge_out = fdopen(from_bridge[0], "r");
    client->bridge_pid = pid;
    
    if (!client->bridge_in || !client->bridge_out) {
        mcp_cleanup(client);
        return NULL;
    }
    
    return client;
}

void mcp_cleanup(MCPClient* client) {
    if (!client) return;
    
    if (client->bridge_in) {
        fprintf(client->bridge_in, "EXIT\n");
        fflush(client->bridge_in);
        fclose(client->bridge_in);
    }
    
    if (client->bridge_out) {
        fclose(client->bridge_out);
    }
    
    if (client->bridge_pid > 0) {
        kill(client->bridge_pid, SIGTERM);
        waitpid(client->bridge_pid, NULL, 0);
    }
    
    free(client);
}

MCPResponse* mcp_send_command(MCPClient* client, const char* action, const char* data) {
    if (!client || !action) return NULL;
    
    // Construir JSON para el comando
    fprintf(client->bridge_in, "{\"Action\":\"%s\"", action);
    if (data) {
        // Escapar datos para JSON básico
        fprintf(client->bridge_in, ",\"Data\":\"");
        for (const char* p = data; *p; p++) {
            if (*p == '"' || *p == '\\') {
                fputc('\\', client->bridge_in);
            }
            fputc(*p, client->bridge_in);
        }
        fprintf(client->bridge_in, "\"");
    }
    fprintf(client->bridge_in, "}\n");
    fflush(client->bridge_in);
    
    // Leer respuesta
    char* json_response = read_json_response(client->bridge_out);
    if (!json_response) return NULL;
    
    // Parsear respuesta JSON básica
    MCPResponse* response = malloc(sizeof(MCPResponse));
    memset(response, 0, sizeof(MCPResponse));
    
    // Buscar campos en el JSON
    if (strstr(json_response, "\"Success\":true")) {
        response->success = 1;
    }
    
    // Extraer resultado
    char* result_start = strstr(json_response, "\"Result\":\"");
    if (result_start) {
        result_start += 10;
        char* result_end = strstr(result_start, "\",");
        if (!result_end) result_end = strstr(result_start, "\"}");
        
        if (result_end) {
            size_t len = result_end - result_start;
            response->result = malloc(len + 1);
            strncpy(response->result, result_start, len);
            response->result[len] = '\0';
            
            // Desescapar JSON básico
            char* src = response->result;
            char* dst = response->result;
            while (*src) {
                if (*src == '\\' && *(src+1)) {
                    src++;
                    if (*src == 'n') *dst = '\n';
                    else if (*src == 't') *dst = '\t';
                    else *dst = *src;
                } else {
                    *dst = *src;
                }
                src++; dst++;
            }
            *dst = '\0';
        }
    }
    
    // Extraer error
    char* error_start = strstr(json_response, "\"Error\":\"");
    if (error_start) {
        error_start += 9;
        char* error_end = strstr(error_start, "\",");
        if (!error_end) error_end = strstr(error_start, "\"}");
        
        if (error_end) {
            size_t len = error_end - error_start;
            response->error = malloc(len + 1);
            strncpy(response->error, error_start, len);
            response->error[len] = '\0';
        }
    }
    
    free(json_response);
    return response;
}

MCPResponse* mcp_execute_command(MCPClient* client, const char* command) {
    return mcp_send_command(client, "execute_command", command);
}

MCPResponse* mcp_analyze_text(MCPClient* client, const char* text) {
    return mcp_send_command(client, "analyze_text", text);
}

MCPResponse* mcp_get_system_info(MCPClient* client) {
    return mcp_send_command(client, "get_system_info", NULL);
}

MCPResponse* mcp_arch_diagnostics(MCPClient* client) {
    return mcp_send_command(client, "arch_diagnostics", NULL);
}

void mcp_free_response(MCPResponse* response) {
    if (!response) return;
    
    if (response->result) free(response->result);
    if (response->error) free(response->error);
    free(response);
}

int is_user_command(const char* text) {
    if (!text) return 0;
    
    // Saltar espacios iniciales
    while (isspace(*text)) text++;
    
    // Lista de comandos comunes
    const char* commands[] = {
        "ls", "dir", "pwd", "cd", "cat", "grep", "find", "ps", "top", 
        "df", "du", "free", "uname", "which", "whereis", "locate",
        "pacman", "yay", "makepkg", "systemctl", "journalctl",
        "sudo", "su", "chmod", "chown", "mkdir", "rmdir", "rm",
        "cp", "mv", "ln", "tar", "gzip", "unzip", "wget", "curl",
        "lsblk", "fdisk", "cfdisk", "mount", "umount",
        NULL
    };
    
    for (int i = 0; commands[i]; i++) {
        size_t len = strlen(commands[i]);
        if (strncmp(text, commands[i], len) == 0) {
            if (text[len] == '\0' || isspace(text[len]) || text[len] == '-') {
                return 1;
            }
        }
    }
    
    return 0;
}
