#!/bin/bash
mkdir -p gpt_modular_executor/roles && cd gpt_modular_executor

echo "API_KEY=sk-reemplaza-tu-clave" > config.txt
touch context.txt

# roles
echo 'system\nEres un asistente especializado en instalación de Arch Linux. Responde con un paso a la vez en bash.' > roles/arch.txt
echo 'system\nEres un asistente conversacional. Puedes responder preguntas técnicas o generales.' > roles/chat.txt
echo 'system\nEres un generador de estructuras de proyecto. Responde con comandos bash listos para crear carpetas y archivos.' > roles/creator.txt

# Headers
cat > chat.h << 'EOF'
#ifndef CHAT_H
#define CHAT_H
void load_api_key();
char* send_prompt(const char* prompt, const char* role_file);
#endif
EOF

cat > context.h << 'EOF'
#ifndef CONTEXT_H
#define CONTEXT_H
void append_to_context(const char* cmd, const char* output);
void load_context();
#endif
EOF

cat > executor_arch.h << 'EOF'
#ifndef EXECUTOR_ARCH_H
#define EXECUTOR_ARCH_H
char* extract_command(const char* text);
char* run_command(const char* cmd);
#endif
EOF

cat > executor_chat.h << 'EOF'
#ifndef EXECUTOR_CHAT_H
#define EXECUTOR_CHAT_H
char* extract_command_chat(const char* text);
char* run_command_chat(const char* cmd);
#endif
EOF

cat > executor_creator.h << 'EOF'
#ifndef EXECUTOR_CREATOR_H
#define EXECUTOR_CREATOR_H
char* extract_command_creator(const char* text);
char* run_command_creator(const char* cmd);
#endif
EOF

# chat.c
cat > chat.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char api_key[256];

void load_api_key() {
    FILE *f = fopen("config.txt", "r");
    if (!f) {
        fprintf(stderr, "Falta config.txt\n");
        exit(1);
    }
    while (fgets(api_key, sizeof(api_key), f)) {
        if (strncmp(api_key, "API_KEY=", 8) == 0) {
            memmove(api_key, api_key + 8, strlen(api_key) - 7);
            break;
        }
    }
    fclose(f);
}

char* send_prompt(const char *prompt, const char *role_file) {
    FILE *ctx = fopen("context.txt", "a");
    if (ctx) {
        fprintf(ctx, "user\t%s\n", prompt);
        fclose(ctx);
    }

    FILE *rfile = fopen("req.json", "w");
    fprintf(rfile, "{ \"model\": \"gpt-3.5-turbo\", \"messages\": [");

    FILE *rolef = fopen(role_file, "r");
    if (rolef) {
        char role[16], content[2000];
        fgets(role, sizeof(role), rolef);
        fgets(content, sizeof(content), rolef);
        role[strcspn(role, "\n")] = 0;
        content[strcspn(content, "\n")] = 0;
        fprintf(rfile, "{\"role\": \"%s\", \"content\": \"%s\"},", role, content);
        fclose(rolef);
    }

    FILE *ctxin = fopen("context.txt", "r");
    char line[2048];
    while (fgets(line, sizeof(line), ctxin)) {
        char role[16], content[2000];
        sscanf(line, "%15[^	]	%[^
]", role, content);
        fprintf(rfile, "{\"role\": \"%s\", \"content\": \"%s\"},", role, content);
    }
    fclose(ctxin);
    fseek(rfile, -1, SEEK_CUR);
    fprintf(rfile, "] }");
    fclose(rfile);

    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "curl -s https://api.openai.com/v1/chat/completions "
        "-H \"Authorization: Bearer %s\" "
        "-H \"Content-Type: application/json\" "
        "--data @req.json | jq -r '.choices[0].message.content' > out.txt", api_key);
    system(cmd);

    FILE *r = fopen("out.txt", "r");
    if (!r) return NULL;
    static char response[4096] = {0};
    fread(response, 1, sizeof(response) - 1, r);
    fclose(r);
    return strdup(response);
}
EOF

# context.c
cat > context.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

void append_to_context(const char* cmd, const char* output) {
    FILE *f = fopen("context.txt", "a");
    fprintf(f, "user\t%s\n", cmd);
    fprintf(f, "assistant\t%s\n", output);
    fclose(f);
}

void load_context() {
    FILE *f = fopen("context.txt", "a");
    if (f) fclose(f);
}
EOF

# main.c (unificado, se adapta con #define MODO_ARCH / CHAT / CREATOR)
cat > main.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chat.h"
#include "context.h"

#ifdef MODO_ARCH
#include "executor_arch.h"
#define ROLE_FILE "roles/arch.txt"
#define extract_command extract_command
#define run_command run_command
#endif

#ifdef MODO_CHAT
#include "executor_chat.h"
#define ROLE_FILE "roles/chat.txt"
#define extract_command extract_command_chat
#define run_command run_command_chat
#endif

#ifdef MODO_CREATOR
#include "executor_creator.h"
#define ROLE_FILE "roles/creator.txt"
#define extract_command extract_command_creator
#define run_command run_command_creator
#endif

int main() {
    load_api_key();
    load_context();
    char input[2048];

    printf("Modo GPT iniciado (%s)\n", ROLE_FILE);

    while (1) {
        printf("\n> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "salir") == 0) break;

        char* respuesta = send_prompt(input, ROLE_FILE);
        printf("[GPT]: %s\n", respuesta);

        char* comando = extract_command(respuesta);
        if (comando) {
            printf("[Shell] Ejecutando: %s\n", comando);
            char* salida = run_command(comando);
            printf("[Salida]:\n%s\n", salida);
            append_to_context(comando, salida);
            free(salida);
            free(comando);
        } else {
            printf("⚠️  No se detectó comando ejecutable.\n");
        }
        free(respuesta);
    }
    return 0;
}
EOF

# Ejecutores (todos iguales para demo, se pueden personalizar después)
for tipo in arch chat creator; do
cat > executor_${tipo}.c << EOF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* extract_command_${tipo}(const char *text) {
    const char *start = strstr(text, "```bash");
    if (!start) return NULL;
    start = strchr(start, '\n');
    if (!start) return NULL;
    start++;
    const char *end = strstr(start, "```");
    if (!end) return NULL;
    size_t len = end - start;
    char *cmd = malloc(len + 1);
    strncpy(cmd, start, len);
    cmd[len] = 0;
    return cmd;
}

char* run_command_${tipo}(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return strdup("Error al ejecutar");
    char *out = malloc(4096); out[0] = 0;
    while (fgets(out + strlen(out), 1024, fp));
    pclose(fp);
    return out;
}
EOF
done

# Makefile maestro
cat > Makefile << 'EOF'
all: gpt_arch gpt_chat gpt_creator

gpt_arch:
	gcc -DMODO_ARCH -o gpt_arch main.c chat.c context.c executor_arch.c -Wall

gpt_chat:
	gcc -DMODO_CHAT -o gpt_chat main.c chat.c context.c executor_chat.c -Wall

gpt_creator:
	gcc -DMODO_CREATOR -o gpt_creator main.c chat.c context.c executor_creator.c -Wall

clean:
	rm -f gpt_arch gpt_chat gpt_creator req.json out.txt resp.json context.txt
EOF
