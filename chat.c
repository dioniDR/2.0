#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "common_utils.h"

// Función para escapar caracteres especiales en JSON
char* escape_json(const char* input) {
    if (!input) return NULL;
    
    size_t input_len = strlen(input);
    // Reservamos espacio extra para posibles escapes (4x debería ser suficiente en el peor caso)
    char* escaped = malloc(input_len * 4 + 1);
    if (!escaped) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < input_len; i++) {
        switch (input[i]) {
            case '\\': 
                escaped[j++] = '\\';
                escaped[j++] = '\\';
                break;
            case '"': 
                escaped[j++] = '\\';
                escaped[j++] = '"';
                break;
            case '\b': 
                escaped[j++] = '\\';
                escaped[j++] = 'b';
                break;
            case '\f': 
                escaped[j++] = '\\';
                escaped[j++] = 'f';
                break;
            case '\n': 
                escaped[j++] = '\\';
                escaped[j++] = 'n';
                break;
            case '\r': 
                escaped[j++] = '\\';
                escaped[j++] = 'r';
                break;
            case '\t': 
                escaped[j++] = '\\';
                escaped[j++] = 't';
                break;
            default:
                if ((unsigned char)input[i] < 32) {
                    // Controlar caracteres de control
                    sprintf(escaped + j, "\\u%04x", input[i]);
                    j += 6;
                } else {
                    escaped[j++] = input[i];
                }
                break;
        }
    }
    escaped[j] = '\0';
    return escaped;
}

char* send_prompt(const char *prompt, const char *role_file) {
    // Verificar que el archivo de rol existe
    if (access(role_file, F_OK) != 0) {
        fprintf(stderr, "Error: El archivo de rol %s no existe.\n", role_file);
        return strdup("Error: Archivo de rol no encontrado.");
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
    
    // Comenzar a escribir el JSON
    fprintf(rfile, "{\n  \"model\": \"gpt-3.5-turbo\",\n  \"messages\": [\n");

    // Agregar el rol del sistema
    FILE *rolef = fopen(role_file, "r");
    if (rolef) {
        char role[16] = {0}, content[2000] = {0};
        if (fgets(role, sizeof(role), rolef) && fgets(content, sizeof(content), rolef)) {
            role[strcspn(role, "\r\n")] = 0;
            content[strcspn(content, "\r\n")] = 0;
            
            char* escaped_content = escape_json(content);
            if (escaped_content) {
                fprintf(rfile, "    {\"role\": \"%s\", \"content\": \"%s\"},\n", 
                        role, escaped_content);
                free(escaped_content);
            }
        }
        fclose(rolef);
    }

    // Agregar el contexto previo
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
    
    // Mostrar el JSON para depuración (opcional)
    //system("cat req.json");

    // Enviar la solicitud
    char cmd[1024];
    // Leer la clave API desde config.txt
    char *api_key = read_api_key();
    if (!api_key) {
        return strdup("Error: No se pudo leer la clave API desde config.txt.");
    }

    snprintf(cmd, sizeof(cmd),
        "curl -s -w \"\\nHTTP_STATUS:%%{http_code}\" https://api.openai.com/v1/chat/completions "
        "-H \"Authorization: Bearer %s\" "
        "-H \"Content-Type: application/json\" "
        "--data @req.json > resp.json", api_key);

    free(api_key);
    
    // Ejecutar la solicitud
    printf("Enviando solicitud a OpenAI...\n");
    system(cmd);
    
    // Mostrar la respuesta para depuración (opcional)
    //system("cat resp.json");
    
    // Verificar código de estado HTTP
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
    
    // Extraer el contenido de la respuesta con jq
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

// Function to read the API key from config.txt
extern char* read_api_key();