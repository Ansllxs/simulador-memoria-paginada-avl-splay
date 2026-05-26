# Simulador de Gestión de Memoria Paginada con AVL y Splay Trees

Proyecto II del curso de **Estructuras de Datos** — TEC, I Semestre 2026.

Simulador en C++ de un sistema operativo simplificado que administra memoria por paginación, procesos, cachés (local y global) y políticas de reemplazo, utilizando **Árboles AVL** y **Splay Trees** como estructuras centrales.

## Estructura del proyecto

```
Proyecto II/
├── include/          # Archivos .h con las clases (AVL, Splay, Page, Process, etc.)
├── src/              # Archivos .cpp con las implementaciones
├── input/            # Archivos de prueba con operaciones
├── docs/             # Informe técnico, evidencias y diagramas
├── tests/            # Casos de prueba adicionales
├── .gitignore
└── README.md
```

## Compilación

Con g++ (MinGW o Linux):

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o simulador
```

## Ejecución

Modo interactivo (consola):

```bash
./simulador
```

Modo archivo:

```bash
./simulador input/test1.txt
```

## Operaciones soportadas

| Comando | Descripción |
|---------|-------------|
| `MEMORIA_TOTAL n` | Define la memoria total del sistema (bytes) |
| `TAMANO_PAGINA n` | Define el tamaño de cada página (bytes) |
| `TAMANO_CACHE_GLOBAL n` | Define el tamaño del caché global |
| `NEW_PROCESS id mem` | Crea un proceso con la memoria solicitada |
| `END_PROCESS id` | Finaliza un proceso y libera sus recursos |
| `ACCESS id dir` | Referencia una dirección de memoria del proceso |
| `ALLOCATE id mem` | Solicita memoria adicional para un proceso |
| `FREE id dirInicial bytes` | Libera memoria de un proceso |
| `STATUS` | Muestra el estado completo del sistema |
| `METRICS` | Muestra las estadísticas |
| `EXIT` | Termina el simulador |

## Estructuras de datos utilizadas

- **AVL Tree**: administra las páginas físicas libres del sistema.
- **Splay Tree**: tabla de páginas de cada proceso (autoajustable a los accesos frecuentes).
- **Lista doblemente enlazada + `unordered_map`**: implementación O(1) del caché LRU.

## Autora

Angie Mariela — Estudiante de Ingeniería en Computación, TEC.
