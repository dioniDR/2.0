#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "../common/includes/utils.h"
#include "../common/includes/config_manager.h" // Nueva inclusión

// Función para escapar caracteres especiales en JSON (sin cambios)
char* escape_json(const char* input) {
    // ... [código existente sin cambios] ...
}

// Función modificada para usar GPTConfig
char* send_prompt(const char *prompt, const char *config_file) {
    // Inicializar la configuración con valores predeterminados
    GPTConfig config;
    config_init(&config);
    
    // Cargar configuración desde archivo
    if (config_file && access(config_file, F_OK) == 0) {
        if (!config_load_from_file(&config, config_file)) {
            fprintf(stderr, "Advertencia: No se pudo cargar la configuración desde %s, usando valores por defecto\n", config_file);
        }
    }
    
    // Cargar rol desde archivo si está especificado
    if (strlen(config.role_file) > 0) {
        if (!config_load_role(&config)) {
            fprintf(stderr, "Advertencia: No se pudo cargar la configuración desde %s, usando valores por defecto\n", config.role_file);
        }
    }
    
    FILE *ctx = fopen("context.txt", "a");
    if (ctx) {
        fprintf(ctx, "user\t%s\n", prompt);
        fclose(ctx);
    }

    FILE *rfile = fopen("req.json", "w");
    if (!rfile) {
        fprintf(stderr, "Error: No se pudo crear el archivo req.json\n");
        return strdup("Error: No se pudo crear el archivo de solicitud.");
    }
    
    // Escapar el prompt para JSON
    char* escaped_prompt = escape_json(prompt);
    if (!escaped_prompt) {
        fprintf(stderr, "Error: No se pudo escapar el prompt.\n");
        fclose(rfile);
        return strdup("Error: Problemas de memoria al procesar la solicitud.");
    }
    
    // Comenzar a escribir el JSON con el modelo de la configuración
    fprintf(rfile, "{\n  \"model\": \"%s\",\n", config.model);
    fprintf(rfile, "  \"temperature\": %.1f,\n", config.temperature);
    fprintf(rfile, "  \"max_tokens\": %d,\n", config.max_tokens);
    fprintf(rfile, "  \"messages\": [\n");

    // Agregar el rol del sistema de la configuración
    char* escaped_content = escape_json(config.system_content);
    if (escaped_content) {
        fprintf(rfile, "    {\"role\": \"%s\", \"content\": \"%s\"},\n", 
                config.system_role, escaped_content);
        free(escaped_content);
    }

    // Agregar el contexto previo (sin cambios)
    FILE *ctxin = fopen("context.txt", "r");
    if (ctxin) {
        char line[2048];
        int message_count = 0;
        
        while (fgets(line, sizeof(line), ctxin)) {
            char role[16] = {0}, content[2000] = {0};
            
            if (sscanf(line, "%15[^\t]\t%[^\n]", role, content) == 2) {
                char* escaped_content = escape_json(content);
                if (escaped_content) {
                    fprintf(rfile, "    {\"role\": \"%s\", \"content\": \"%s\"},\n", 
                            role, escaped_content);
                    free(escaped_content);
                    message_count++;
                }
            }
        }
        fclose(ctxin);
        
        // Si no hay mensajes en el contexto, agregar solo el prompt actual
        if (message_count == 0) {
            fprintf(rfile, "    {\"role\": \"user\", \"content\": \"%s\"}\n", escaped_prompt);
        } else {
            // Eliminar la última coma
            fseek(rfile, -2, SEEK_CUR);
            fprintf(rfile, "\n");
        }
    } else {
        // Si no hay contexto, agregar solo el prompt actual
        fprintf(rfile, "    {\"role\": \"user\", \"content\": \"%s\"}\n", escaped_prompt);
    }
    
    free(escaped_prompt);
    
    // Cerrar el JSON
    fprintf(rfile, "  ]\n}\n");
    fclose(rfile);
    
    // Enviar la solicitud
    char cmd[1024];
    
    // Obtener la clave API usando la configuración
    char *api_key = config_get_api_key(&config);
    if (!api_key) {
        return strdup("Error: No se pudo obtener la clave API.");
    }

    snprintf(cmd, sizeof(cmd),
        "curl -s -w \"\\nHTTP_STATUS:%%{http_code}\" https://api.openai.com/v1/chat/completions "
        "-H \"Authorization: Bearer %s\" "
        "-H \"Content-Type: application/json\" "
        "--data @req.json > resp.json", api_key);

    free(api_key);
    
    // Ejecutar la solicitud
    printf("Enviando solicitud a OpenAI con el modelo %s...\n", config.model);
    system(cmd);
    
    // Verificar código de estado HTTP y procesar respuesta
    // [resto del código sin cambios]
    
    FILE *resp = fopen("resp.json", "r");
    if (!resp) {
        return strdup("Error: No se pudo obtener respuesta de la API.");
    }
    
    // Buscar el código de estado HTTP
    char buffer[16384] = {0};
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, resp);
    fclose(resp);
    
    if (bytes_read == 0) {
        return strdup("Error: Respuesta vacía de la API.");
    }
    
    char *status_marker = strstr(buffer, "HTTP_STATUS:");
    if (status_marker) {
        int status_code = atoi(status_marker + 12);
        *status_marker = '\0'; // Truncar el buffer al marcador de estado
        
        if (status_code != 200) {
            // Si el código no es 200, intentar extraer el mensaje de error
            char error_msg[512] = "Error en la API de OpenAI";
            
            // Buscar mensaje de error en el JSON
            char *error_json = buffer;
            char *error_msg_start = strstr(error_json, "\"message\"");
            if (error_msg_start) {
                error_msg_start = strchr(error_msg_start, ':');
                if (error_msg_start) {
                    error_msg_start++; // Saltar los dos puntos
                    while (isspace((unsigned char)*error_msg_start)) error_msg_start++; // Saltar espacios
                    if (*error_msg_start == '\"') error_msg_start++; // Saltar comilla inicial
                    
                    char *error_msg_end = strchr(error_msg_start, '\"');
                    if (error_msg_end) {
                        size_t len = error_msg_end - error_msg_start;
                        if (len > sizeof(error_msg) - 20) len = sizeof(error_msg) - 20;
                        strncpy(error_msg, error_msg_start, len);
                        error_msg[len] = '\0';
                        sprintf(error_msg + strlen(error_msg), " (HTTP %d)", status_code);
                    } else {
                        sprintf(error_msg, "Error en la API de OpenAI (HTTP %d)", status_code);
                    }
                }
            } else {
                sprintf(error_msg, "Error en la API de OpenAI (HTTP %d)", status_code);
            }
            
            return strdup(error_msg);
        }
    }
    
    // Extraer el contenido de la respuesta with jq
    system("cat resp.json | jq -r '.choices[0].message.content' > out.txt 2>/dev/null");
    
    // Verificar si jq falló o no está instalado
    if (system("which jq > /dev/null 2>&1") != 0) {
        return strdup("Error: No se pudo procesar la respuesta. Por favor instala 'jq' (sudo apt install jq).");
    }
    
    FILE *r = fopen("out.txt", "r");
    if (!r) {
        return strdup("Error: No se pudo leer la respuesta procesada.");
    }
    
    char response[8192] = {0};
    bytes_read = fread(response, 1, sizeof(response) - 1, r);
    fclose(r);
    
    if (bytes_read == 0) {
        // Si no hay contenido, revisar si hay un error en el JSON
        FILE *err_check = fopen("resp.json", "r");
        char err_buffer[1024] = {0};
        fread(err_buffer, 1, sizeof(err_buffer) - 1, err_check);
        fclose(err_check);
        
        if (strstr(err_buffer, "\"error\"")) {
            return strdup("Error: La API de OpenAI devolvió un error. Verifica el archivo resp.json para más detalles.");
        }
        return strdup("Error: Respuesta vacía de la API. Posible error en el formato JSON.");
    }
    
    // Guardar la respuesta en el contexto
    ctx = fopen("context.txt", "a");
    if (ctx) {
        fprintf(ctx, "assistant\t%s\n", response);
        fclose(ctx);
    }
    
    return strdup(response);
}