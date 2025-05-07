all: gpt_arch gpt_chat gpt_creator

gpt_arch:
	gcc -DMODO_ARCH -o gpt_arch main.c chat.c context.c common_utils.c executor_arch.c -Wall

gpt_chat:
	gcc -DMODO_CHAT -o gpt_chat main.c chat.c context.c common_utils.c executor_chat.c -Wall

gpt_creator:
	gcc -DMODO_CREATOR -o gpt_creator main.c chat.c context.c common_utils.c executor_creator.c -Wall

clean:
	rm -f gpt_arch gpt_chat gpt_creator req.json out.txt resp.json context.txt

test_api:
	@echo "Probando conexión a la API de OpenAI..."
	@curl -s https://api.openai.com/v1/models -H "Authorization: Bearer $$(cat config.txt | grep API_KEY | cut -d= -f2)" | grep -q "gpt-3.5-turbo" && echo "✓ API Key válida y conexión establecida correctamente." || echo "✗ Error al conectar con la API. Verifica tu API Key y conexión a internet."

help:
	@echo "Comandos disponibles:"
	@echo "  make          - Compila todos los programas"
	@echo "  make gpt_arch - Compila el asistente para Arch Linux"
	@echo "  make gpt_chat - Compila el asistente conversacional general"
	@echo "  make gpt_creator - Compila el generador de estructuras de proyecto"
	@echo "  make clean    - Elimina archivos compilados y temporales"
	@echo "  make test_api - Verifica si la API key es válida"
	@echo "  make help     - Muestra esta ayuda"