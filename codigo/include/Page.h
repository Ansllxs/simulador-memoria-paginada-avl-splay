#ifndef PAGE_H
#define PAGE_H

#include <string>

/**
 * Page representa una página de memoria (virtual o física).
 *
 * Es la unidad mínima que el simulador maneja. Una página física existe
 * en el "hotel" (memoria física) y puede estar libre o asignada a un proceso.
 * Una página virtual es la forma en que cada proceso ve sus propias páginas.
 */
struct Page {
    int virtualPage;       // Número de página virtual (cómo la ve el proceso)
    int physicalPage;      // Número de página física (cuarto real del hotel)
    std::string processId; // Proceso dueño ("" si está libre)
    bool valid;            // Bit de validez
    bool modified;         // Bit de modificación
    int references;        // Contador de cuántas veces ha sido referenciada
    long lastReference;    // Timestamp lógico del último acceso (LRU)

    Page()
        : virtualPage(-1),
          physicalPage(-1),
          processId(""),
          valid(false),
          modified(false),
          references(0),
          lastReference(0) {}

    Page(int virtPage, int physPage, const std::string& pid)
        : virtualPage(virtPage),
          physicalPage(physPage),
          processId(pid),
          valid(true),
          modified(false),
          references(0),
          lastReference(0) {}
};

#endif // PAGE_H
