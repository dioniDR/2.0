/* =====================
   Modificación de openai.h
   ===================== */

   #ifndef OPENAI_H
   #define OPENAI_H
   
   // Función para enviar un prompt a la API de OpenAI
   // Ahora recibe un archivo de configuración en lugar de un archivo de rol
   char* send_prompt(const char* prompt, const char* config_file);
   
   #endif /* OPENAI_H */
   
   
   /* =====================
      Modificación de main.c (fragmento)
      ===================== */
   
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "api/openai.h"
   #include "common/includes/utils.h"
   #include "common/includes/context.h"
   #include "common/includes/config_manager.h"
   
   // Definiciones específicas para cada módulo
   #ifdef MODO_ARCH
   #include "modulos/arch/executor.h"
   #define MODULE_NAME "Asistente Arch Linux"
   #define CONFIG_FILE "modulos/arch/config.ini"
   #define extract_command extract_command_arch
   #define run_command run_command_arch
   #endif
   
   #ifdef MODO_CHAT
   #include "modulos/chat/executor.h"
   #define MODULE_NAME "Asistente Conversacional"
   #define CONFIG_FILE "modulos/chat/config.ini"
   #define extract_command extract_command_chat
   #define run_command run_command_chat
   #endif
   
   #ifdef MODO_CREATOR
   #include "modulos/creator/executor.h"
   #define MODULE_NAME "Generador de Estructuras"
   #define CONFIG_FILE "modulos/creator/config.ini"
   #define extract_command extract_command_creator
   #define run_command run_command_creator
   #endif
   
   // Definición predeterminada de CONFIG_FILE si no se definió un módulo
   #ifndef CONFIG_FILE
   #define CONFIG_FILE "default/config.ini"
   #endif
   
   #ifndef MODULE_NAME
   #define MODULE_NAME "Asistente GPT"
   #endif
   
   #ifndef extract_command
   #define extract_command extract_command_improved
   #endif
   
   #ifndef run_command
   #define run_command run_command_improved
   #endif
   
   // Función principal
   int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[]) {
      // Inicializar el contexto
       load_context();
       
       printf("=== %s ===\n", MODULE_NAME);
       printf("Escribe 'salir' para terminar.\n\n");
       
       char input[2048];
       
       while (1) {
           printf("> ");
           if (!fgets(input, sizeof(input), stdin)) {
               break;
           }
           
           // Eliminar el salto de línea final
           input[strcspn(input, "\n")] = 0;
           
           // Verificar si se debe salir
           if (strcmp(input, "salir") == 0 || 
               strcmp(input, "exit") == 0 || 
               strcmp(input, "quit") == 0) {
               break;
           }
           
           // Si está vacío, continuar
           if (strlen(input) == 0) {
               continue;
           }
           
           // Enviar prompt a la API
           printf("Consultando a OpenAI...\n");
           char* respuesta = send_prompt(input, CONFIG_FILE);
           
           // Mostrar la respuesta
           printf("\n--- Respuesta ---\n%s\n\n", respuesta);
           
           // Verificar si hay comandos en la respuesta
           char* comando = extract_command(respuesta);
           if (comando) {
               printf("¿Deseas ejecutar el comando detectado? [s/N]: ");
               char confirmar[10] = {0};
               fgets(confirmar, sizeof(confirmar), stdin);
               confirmar[strcspn(confirmar, "\n")] = 0;
               
               if (confirmar[0] == 's' || confirmar[0] == 'S') {
                   printf("\n=== Ejecutando comando ===\n");
                   char* resultado = run_command(comando);
                   printf("%s\n", resultado);
                   free(resultado);
               }
               
               free(comando);
           }
           
           free(respuesta);
       }
       
       printf("¡Hasta pronto!\n");
       return 0;
   }