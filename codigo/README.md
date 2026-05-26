# Código fuente — Simulador de Memoria Paginada (Proyecto II EdD)

Esta carpeta contiene el código C++ del simulador y los archivos de prueba.

## Estructura

```
codigo/
├── include/
│   ├── Page.h               Estructura de una página
│   ├── AVLTree.h            Plantilla del árbol AVL
│   ├── SplayTree.h          Plantilla del Splay Tree
│   ├── Cache.h              Caché LRU (lista + hash map)
│   ├── Proceso.h            Clase Process (nombre del archivo evita conflicto con <process.h> de Windows)
│   ├── MemoryManager.h      Núcleo del simulador
│   └── OperationParser.h    Lector de comandos
├── src/
│   └── main.cpp             Punto de entrada
├── input/
│   ├── test1.txt            Caso básico del enunciado
│   ├── test2_errores.txt    Casos de error
│   └── test3_lru.txt        Demostración del LRU
├── Makefile                 Compilación con make
├── build.bat                Compilación con g++ en Windows
└── simulador.exe            Ejecutable
```

## Requisitos

- Compilador C++17 o superior (probado con `g++ 15.2.0` de MSYS2/MinGW en Windows).

## Compilación

### Windows (PowerShell)

```powershell
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src\main.cpp -o simulador.exe
```

O con el script incluido:

```powershell
.\build.bat
```

### Linux / MSYS2 / macOS

```bash
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src/main.cpp -o simulador
```

O con `make`:

```bash
make
```

## Ejecución

### Modo interactivo (consola)

```powershell
.\simulador.exe
```

Aparece un prompt `>`. Escriba `HELP` para ver la lista de comandos o `EXIT` para terminar.

### Modo archivo

```powershell
.\simulador.exe input\test1.txt
```

### Modo mixto (archivo + interactivo)

```powershell
.\simulador.exe input\test1.txt -i
```

### Guardar la salida en un archivo

```powershell
.\simulador.exe input\test1.txt > salida.txt
```

## Comandos del simulador

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| Configuración | `MEMORIA_TOTAL n` | Memoria total del sistema en bytes |
| Configuración | `TAMANO_PAGINA n` | Tamaño de cada página en bytes |
| Configuración | `TAMANO_CACHE_GLOBAL n` | Capacidad del caché global |
| Crear proceso | `NEW_PROCESS id memoria` | Crea un proceso con su tabla de páginas y caché local |
| Finalizar proceso | `END_PROCESS id` | Libera todas las páginas, cachés y elimina el proceso |
| Acceso a memoria | `ACCESS id direccion` | Referencia una dirección (calcula VP y offset) |
| Memoria adicional | `ALLOCATE id memoria` | Asigna páginas adicionales a un proceso |
| Liberar memoria | `FREE id direccionInicial bytes` | Libera un rango de memoria |
| Diagnóstico | `STATUS` | Imprime el estado completo del sistema |
| Métricas | `METRICS` | Imprime hits, misses, reemplazos LRU y utilización |
| Ayuda | `HELP` | Muestra la lista de comandos |
| Salir | `EXIT` | Termina el simulador |

> Las líneas que empiezan con `#` en los archivos de entrada se ignoran (comentarios).

## Ejemplo de uso paso a paso

Ejecute el simulador en modo interactivo y escriba estos comandos uno a uno:

```
MEMORIA_TOTAL 65536
TAMANO_PAGINA 1024
TAMANO_CACHE_GLOBAL 8

NEW_PROCESS P1 4096
NEW_PROCESS P2 8192

ACCESS P1 100
ACCESS P1 2500
ACCESS P1 2500
ACCESS P2 7000

ALLOCATE P1 2048
FREE P2 2048 2048

STATUS
METRICS

END_PROCESS P1
END_PROCESS P2

EXIT
```

## Estructuras de datos

| Estructura | Implementación | Propósito |
|------------|----------------|-----------|
| **Árbol AVL** | `AVLTree<K,V>` propia (template) | Páginas físicas libres del sistema |
| **Splay Tree** | `SplayTree<K,V>` propia (template) | Tabla de páginas de cada proceso |
| **Lista doblemente enlazada** + **`unordered_map`** | Clase `Cache` | Caché LRU en O(1) |
| **`unordered_map`** | Dentro del `MemoryManager` | Tabla global de procesos |

> Ni `std::map` ni `std::set` se usan para reemplazar el AVL o el Splay Tree (cumple regla 1.1.17 del enunciado).

## Política del caché local

```
cache_local_max = max(1, floor(paginasAsignadas * 0.20))
```

Se actualiza dinámicamente cuando el proceso llama a `ALLOCATE` o `FREE`. La política de reemplazo es **LRU**.

## Documentación adicional

Para los detalles completos (análisis de complejidad, justificación de cada estructura, manejo de errores, casos de prueba), consultar el archivo `../documento-tecnico.pdf`.
