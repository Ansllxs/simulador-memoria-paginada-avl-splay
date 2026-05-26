# Proyecto II — Simulador de Gestión de Memoria Paginada con AVL y Splay Trees

> Curso de **Estructuras de Datos** — Instituto Tecnológico de Costa Rica (TEC)
> I Semestre 2026  
> **Autora:** Angie Mariela Alpizar Porras

Simulador en **C++** de un sistema operativo simplificado que administra memoria por **paginación**, procesos, cachés (local y global) con política **LRU**, y utiliza **Árboles AVL** y **Splay Trees** como estructuras centrales.

---

## Estructura de la entrega

```
Proyecto II/
│
├── 📁 codigo/                     ← Código fuente del simulador
│   ├── include/                   archivos .h (AVLTree, SplayTree, MemoryManager, etc.)
│   ├── src/                       archivo .cpp (main.cpp)
│   ├── input/                     archivos de prueba (test1, test2_errores, test3_lru)
│   ├── simulador.exe              ejecutable ya compilado para Windows
│   ├── Makefile                   compilación con make
│   ├── build.bat                  compilación con g++ en Windows
│   └── README.md                  guía detallada del código (comandos, compilación, ejemplos)
│
├── 📄 documento-tecnico.pdf       ← Informe técnico completo (12 secciones)
├── 📄 evidencias-ejecucion.pdf    ← 14 capturas demostrando todas las funcionalidades
├── 📄 EdD-Pry2-ISem26.pdf         ← Enunciado original del proyecto (referencia)
├── .gitignore
└── README.md                      ← este archivo
```

## Ejecución rápida

```powershell
cd codigo
.\simulador.exe                                  # modo interactivo
.\simulador.exe input\test1.txt                  # modo archivo
.\simulador.exe input\test3_lru.txt              # ver reemplazo LRU
```

Para los detalles completos del simulador (comandos, sintaxis, análisis de complejidad, política del caché, manejo de errores), ver:

- [`codigo/README.md`](codigo/README.md) — guía detallada del código y de los comandos
- [`documento-tecnico.pdf`](documento-tecnico.pdf) — informe técnico

## Entregables (según el PDF del enunciado, sección 1.1.13)

| # | Entregable | Ubicación |
|---|-----------|-----------|
| 1 | Código fuente en C++ | `codigo/` |
| 2 | `README.md` | raíz + `codigo/README.md` |
| 3 | Archivo de entrada de prueba | `codigo/input/` (3 archivos) |
| 4 | Informe técnico | `documento-tecnico.pdf` |
| 5 | Evidencias de ejecución | `evidencias-ejecucion.pdf` |
| 6 | Análisis de complejidad | sección 11 de `documento-tecnico.pdf` |
| 7 | Explicación uso AVL/Splay | secciones 5 y 6 de `documento-tecnico.pdf` |

## Compilación (si es necesario recompilar)

Desde la carpeta `codigo/`:

```powershell
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src\main.cpp -o simulador.exe
```
