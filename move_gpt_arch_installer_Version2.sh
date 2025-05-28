#!/bin/bash

# Script para mover gpt_arch_installer, sus dependencias y carpetas asociadas a una carpeta destino

# Uso:
# ./move_gpt_arch_installer.sh /ruta/absoluta/destino

if [ -z "$1" ]; then
    echo "Uso: $0 /ruta/absoluta/destino"
    exit 1
fi

DEST="$1"

# Crear la carpeta de destino
mkdir -p "$DEST"

# Copiar el ejecutable principal
cp ./gpt_arch_installer "$DEST/"

# Determinar el nombre del bridge nativo generado por el build (ajusta si usas otro nombre)
BRIDGE_NATIVE=$(ls | grep 'bridge' | head -n 1)
if [ -f "$BRIDGE_NATIVE" ]; then
    cp "$BRIDGE_NATIVE" "$DEST/"
    echo "Bridge nativo copiado: $BRIDGE_NATIVE"
else
    echo "ADVERTENCIA: No se encontró bridge nativo. Por favor, verifique el nombre del archivo generado."
fi

# Copiar la carpeta api/ si existe
if [ -d "./api" ]; then
    cp -r ./api "$DEST/"
fi

# Copiar la carpeta modulos/arch_installer/ si existe
if [ -d "./modulos/arch_installer" ]; then
    mkdir -p "$DEST/modulos/"
    cp -r ./modulos/arch_installer "$DEST/modulos/"
fi

# Crear un instalador simple (como en el Makefile)
echo '#!/bin/bash' > "$DEST/install_arch.sh"
echo 'cd "$(dirname "$0")"' >> "$DEST/install_arch.sh"
echo './gpt_arch_installer' >> "$DEST/install_arch.sh"
chmod +x "$DEST/install_arch.sh"

echo "Copia completada en $DEST"
echo "Para instalar, ejecute: cd $DEST && ./install_arch.sh"