# Simulador de Gestión de Memoria Paginada con AVL y Splay Trees

> **Proyecto II** — Curso de Estructuras de Datos
> Instituto Tecnológico de Costa Rica (TEC) — I Semestre 2026

Simulador en **C++** de un sistema operativo simplificado que administra memoria por **paginación**, procesos, cachés (local y global) y políticas de reemplazo, utilizando **Árboles AVL** y **Splay Trees** como estructuras centrales.

---

## Tabla de contenidos

1. [Descripción general](#descripción-general)
2. [Requisitos](#requisitos)
3. [Estructura del proyecto](#estructura-del-proyecto)
4. [Compilación](#compilación)
5. [Ejecución](#ejecución)
6. [Comandos del simulador](#comandos-del-simulador)
7. [Ejemplo de uso paso a paso](#ejemplo-de-uso-paso-a-paso)
8. [Estructuras de datos utilizadas](#estructuras-de-datos-utilizadas)
9. [Política del caché local](#política-del-caché-local)
10. [Manejo de errores](#manejo-de-errores)
11. [Análisis de complejidad](#análisis-de-complejidad)
12. [Archivos de prueba incluidos](#archivos-de-prueba-incluidos)
13. [Autora](#autora)

---

## Descripción general

El programa simula un administrador de memoria que:

- Reserva páginas físicas a procesos que las solicitan.
- Lleva una **tabla de páginas** por proceso (en un **Splay Tree**) para que los accesos frecuentes sean rápidos.
- Mantiene un **AVL** de páginas físicas libres para asignaciones balanceadas en O(log n).
- Implementa un **caché local** por proceso y un **caché global** compartido, ambos con política **LRU**.
- Reporta hits, misses, errores y métricas de utilización.

---

## Requisitos

- **Compilador C++17 o superior** (probado con `g++ 15.2.0` de MSYS2/MinGW en Windows).
- Opcional: `make` (Linux/MSYS2) o `g++` directo.

---

## Estructura del proyecto

```
Proyecto II/
├── include/                 Archivos de cabecera (.h)
│   ├── Page.h               Estructura de una página de memoria
│   ├── AVLTree.h            Árbol AVL genérico (template)
│   ├── SplayTree.h          Árbol Splay genérico (template)
│   ├── Cache.h              Caché con política LRU
│   ├── Proceso.h            Clase Process (renombrada para evitar conflicto con <process.h> de Windows)
│   ├── MemoryManager.h      Núcleo del simulador
│   └── OperationParser.h    Lector de comandos (consola y archivo)
├── src/
│   └── main.cpp             Punto de entrada
├── input/                   Archivos de prueba
│   ├── test1.txt            Caso básico del enunciado
│   ├── test2_errores.txt    Casos de manejo de errores
│   └── test3_lru.txt        Demostración del reemplazo LRU
├── docs/                    Informe técnico y evidencias
├── tests/                   Pruebas adicionales
├── Makefile                 Compilación con make (Linux/MSYS2)
├── build.bat                Compilación con g++ en Windows
├── .gitignore
└── README.md
```

---

## Compilación

### Windows (PowerShell)

```powershell
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src\main.cpp -o simulador.exe
```

O usando el script incluido:

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

---

## Ejecución

El simulador tiene **dos modos** de operación.

### Modo interactivo (consola)

Lee comandos línea por línea desde el teclado.

```powershell
.\simulador.exe
```

Aparece un prompt `>` donde puede escribir comandos. Escriba `HELP` para ver la lista o `EXIT` para terminar.

### Modo archivo

Lee y ejecuta automáticamente las operaciones contenidas en un archivo de texto.

```powershell
.\simulador.exe input\test1.txt
```

Al terminar muestra las métricas finales.

### Modo mixto (archivo + interactivo)

Ejecuta el archivo y luego entra en modo interactivo:

```powershell
.\simulador.exe input\test1.txt -i
```

### Guardar la salida (para evidencias)

```powershell
.\simulador.exe input\test1.txt > docs\evidencia_test1.txt
```

---

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

> Las líneas que empiezan con `#` en archivos de entrada se ignoran (son comentarios).

---

## Ejemplo de uso paso a paso

Después de compilar, ejecute el simulador en modo interactivo y escriba los siguientes comandos uno a uno:

```
MEMORIA_TOTAL 65536
TAMANO_PAGINA 1024
TAMANO_CACHE_GLOBAL 8

NEW_PROCESS P1 4096
NEW_PROCESS P2 8192

ACCESS P1 100          # Primer acceso de P1, página virtual 0
ACCESS P1 2500         # Página virtual 2, MISS en cachés
ACCESS P1 2500         # Segundo acceso a la misma página: HIT local
ACCESS P2 7000         # P2 accede a su página virtual 6

ALLOCATE P1 2048       # P1 pide 2 páginas adicionales
FREE P2 2048 2048      # P2 libera 2 páginas

STATUS
METRICS

END_PROCESS P1
END_PROCESS P2

EXIT
```

Salida esperada (extracto):

```
NEW_PROCESS P1 OK: 4 paginas asignadas. Cache local max = 1
ACCESS P1 2500 -> VP=2 offset=452
  Resultado: MISS en ambos caches, HIT en Splay Tree. Pagina fisica = 2
ACCESS P1 2500 -> VP=2 offset=452
  Resultado: HIT en cache LOCAL. Pagina fisica = 2
```

---

## Estructuras de datos utilizadas

| Estructura | Implementación | Propósito en el simulador |
|------------|----------------|---------------------------|
| **Árbol AVL** | Plantilla `AVLTree<K, V>` (`include/AVLTree.h`) | Administra las **páginas físicas libres**. La clave es el número de página física. Garantiza inserción, búsqueda y eliminación en **O(log n)** gracias a su auto-balanceo. |
| **Splay Tree** | Plantilla `SplayTree<K, V>` (`include/SplayTree.h`) | Implementa la **tabla de páginas** de cada proceso. La clave es el número de página virtual. Cada acceso aplica *splay* sobre la página, llevándola a la raíz para que los accesos repetidos sean más rápidos. |
| **Lista doblemente enlazada** + **`unordered_map`** | Clase `Cache` (`include/Cache.h`) | Implementación **O(1)** del caché LRU. El frente de la lista es la entrada más reciente, el final es la víctima en caso de desalojo. |
| **`unordered_map`** | Dentro de `MemoryManager` | Tabla global de procesos (`id` → `Process`). Búsqueda en O(1) promedio. |

> **Importante:** ni `std::map` ni `std::set` se usan para reemplazar el AVL o el Splay (cumple regla 1.1.17 del enunciado).

### Justificación de las estructuras

- **¿Por qué AVL para páginas libres?** Las páginas libres se buscan, insertan y eliminan constantemente. El AVL garantiza altura logarítmica incluso en el peor caso (cosa que un BST normal no asegura), y nos da fácilmente el mínimo (siempre asignamos la página libre más baja para mantener la memoria compacta).

- **¿Por qué Splay para la tabla de páginas?** Los procesos reales tienen *localidad temporal*: acceden a las mismas páginas varias veces seguidas. El Splay Tree, al subir la página recién consultada a la raíz, hace que esos accesos repetidos sean prácticamente O(1) amortizado, sin necesidad de balanceo explícito.

---

## Política del caché local

El tamaño del caché local de cada proceso se calcula como:

```
cache_local_max = max(1, floor(paginasAsignadas * 0.20))
```

- Ejemplo: proceso con 10 páginas → caché local de 2 entradas.
- Ejemplo: proceso con 4 páginas → caché local de 1 entrada (porque `floor(0.8) = 0`, pero forzamos mínimo 1).

Esta política se actualiza dinámicamente cuando el proceso llama a `ALLOCATE` o `FREE`.

**Política de reemplazo:** LRU (Least Recently Used) — la entrada menos recientemente accedida se desaloja cuando se llena el caché.

---

## Manejo de errores

El simulador detecta y reporta los siguientes errores con el prefijo `[ERROR]`:

| Error | Cuándo ocurre |
|-------|---------------|
| Proceso inexistente | Se opera sobre un id que no fue creado |
| Proceso duplicado | `NEW_PROCESS` con un id ya existente |
| Memoria insuficiente | No hay suficientes páginas libres |
| Dirección inválida | `ACCESS` con dirección negativa o página no asignada |
| Liberación inválida | `FREE` sobre páginas no asignadas al proceso |
| Tamaño de página inválido | Configuración con `MEMORIA_TOTAL` no múltiplo de `TAMANO_PAGINA` |
| Reconfiguración bloqueada | Intento de reconfigurar mientras existen procesos |
| Operación desconocida | Comando no reconocido |
| Argumento numérico inválido | Argumento no numérico donde se espera número |

Todos los errores incrementan la métrica `errors` que aparece en `METRICS`.

---

## Análisis de complejidad

| Operación | Complejidad | Estructura involucrada |
|-----------|-------------|------------------------|
| `NEW_PROCESS` con k páginas | O(k · log n) | AVL (extraer k mínimos) + Splay (insertar k páginas) |
| `END_PROCESS` con k páginas | O(k · log n) | Splay (recorrido) + AVL (devolver k) + recorrido caché global |
| `ACCESS` | O(log m) amortizado | Caché local O(1) → Caché global O(1) → Splay O(log m) amortizado |
| `ALLOCATE` con k páginas | O(k · log n) | Igual que `NEW_PROCESS` |
| `FREE` con k páginas | O(k · log m) | Para cada página: Splay O(log m) + AVL O(log n) |
| `STATUS` | O(n + p · k) | Recorrido in-order de AVL y de cada Splay |
| `METRICS` | O(1) | Solo lectura de contadores |
| Operación de caché LRU | O(1) | Lista + hash map |

> **n** = total de páginas físicas, **m** = páginas asignadas a un proceso, **k** = páginas afectadas por la operación, **p** = procesos activos.

---

## Archivos de prueba incluidos

| Archivo | Qué demuestra |
|---------|---------------|
| `input/test1.txt` | Caso básico del enunciado: creación, accesos, allocate, free, end_process |
| `input/test2_errores.txt` | Cada uno de los errores que el sistema debe detectar |
| `input/test3_lru.txt` | Llenado de cachés y reemplazo LRU con verificación de víctimas |

Para correrlos:

```powershell
.\simulador.exe input\test1.txt
.\simulador.exe input\test2_errores.txt
.\simulador.exe input\test3_lru.txt
```

---

## Autora

**Angie Mariela**
Estudiante de Ingeniería en Computación
Instituto Tecnológico de Costa Rica (TEC)
III Semestre — I Semestre 2026

---

## Licencia

Proyecto académico realizado con fines educativos para el curso de Estructuras de Datos del TEC.
