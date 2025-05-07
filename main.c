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
   
   #include "common/includes/config_manager.h"
   
   // Dentro del bloque #ifdef MODO_ARCH
   #ifdef MODO_ARCH
   #include "modulos/arch/executor.h"
   #define MODULE_NAME "Asistente Arch Linux"
   #define CONFIG_FILE "modulos/arch/config.ini"
   #define extract_command extract_command_arch
   #define run_command run_command_arch
   #endif
   
   // Dentro del bloque #ifdef MODO_CHAT
   #ifdef MODO_CHAT
   #include "modulos/chat/executor.h"
   #define MODULE_NAME "Asistente Conversacional"
   #define CONFIG_FILE "modulos/chat/config.ini"
   #define extract_command extract_command_chat
   #define run_command run_command_chat
   #endif
   
   // Dentro del bloque #ifdef MODO_CREATOR
   #ifdef MODO_CREATOR
   #include "modulos/creator/executor.h"
   #define MODULE_NAME "Generador de Estructuras"
   #define CONFIG_FILE "modulos/creator/config.ini"
   #define extract_command extract_command_creator
   #define run_command run_command_creator
   #endif
   
   // Asegúrate de que 'input' esté declarado e inicializado antes de su uso
   const char* input = "example input"; // Reemplaza con la fuente de entrada real
   
   // Add a fallback definition for CONFIG_FILE if no mode is active
   #ifndef CONFIG_FILE
   #define CONFIG_FILE "default/config.ini" // Provide a default path
   #endif
   
   // Move the `send_prompt` call into a function to ensure it is executed at runtime
   void execute_prompt() {
       const char* input = "example input"; // Replace with actual input source
       char* respuesta = send_prompt(input, CONFIG_FILE);
       printf("Respuesta: %s\n", respuesta);
       free(respuesta);
   }