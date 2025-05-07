#ifndef EXECUTOR_CHAT_H
#define EXECUTOR_CHAT_H

// Función específica para extraer comandos en modo Chat
char* extract_command_chat(const char* text);

// Función específica para ejecutar comandos en modo Chat
char* run_command_chat(const char* cmd);

#endif /* EXECUTOR_CHAT_H */