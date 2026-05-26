#ifndef PROCESO_H
#define PROCESO_H

// Nota: el archivo se llama "Proceso.h" (no "Process.h") para evitar un
// conflicto con el header <process.h> del CRT de Windows. La clase interna
// se sigue llamando Process por compatibilidad con el enunciado del proyecto.


#include <string>
#include <algorithm>
#include <vector>
#include <functional>
#include "Page.h"
#include "SplayTree.h"
#include "Cache.h"

/**
 * Process: representa un proceso del sistema.
 *
 * Tiene su propia tabla de páginas (Splay Tree por número de página virtual)
 * y su propio caché local (LRU).
 *
 * El tamaño del caché local es 20% de las páginas asignadas, mínimo 1.
 */
class Process {
private:
    std::string id;
    int requestedMemory;        // bytes solicitados acumulados
    int pageCount;              // cantidad de páginas asignadas
    int nextVirtualPage;        // próximo número de página virtual a usar
    SplayTree<int, Page> pageTable;
    Cache localCache;

public:
    Process() : id(""), requestedMemory(0), pageCount(0), nextVirtualPage(0),
                localCache(0, "Local") {}

    Process(const std::string& pid, int memory)
        : id(pid), requestedMemory(memory), pageCount(0), nextVirtualPage(0),
          localCache(0, "Local " + pid) {}

    const std::string& getId() const { return id; }
    int getRequestedMemory() const { return requestedMemory; }
    int getPageCount() const { return pageCount; }
    int getNextVirtualPage() const { return nextVirtualPage; }

    /** Política decidida: 20% de las páginas, mínimo 1. */
    static int computeLocalCacheSize(int pages) {
        if (pages <= 0) return 0;
        int cap = (int)(pages * 0.20);
        return std::max(1, cap);
    }

    /** Agrega una página a la tabla del proceso y actualiza el tamaño del caché local. */
    void addPage(const Page& page) {
        pageTable.insertar(page.virtualPage, page);
        ++pageCount;
        nextVirtualPage = std::max(nextVirtualPage, page.virtualPage + 1);
        localCache.setCapacity(computeLocalCacheSize(pageCount));
    }

    /** Elimina una página por su número virtual. Devuelve true si existía. */
    bool removePage(int virtualPage) {
        Page* p = pageTable.buscar(virtualPage);
        if (!p) return false;
        bool ok = pageTable.eliminar(virtualPage);
        if (ok) {
            --pageCount;
            localCache.remove(Cache::makeKey(id, virtualPage));
            localCache.setCapacity(computeLocalCacheSize(pageCount));
        }
        return ok;
    }

    /** Busca una página en el Splay Tree (hace splay). */
    Page* findPage(int virtualPage) {
        return pageTable.buscar(virtualPage);
    }

    /** Aumenta la memoria solicitada cuando hay ALLOCATE. */
    void addRequestedMemory(int extra) { requestedMemory += extra; }

    /** Recorre las páginas asignadas (in-order del Splay). */
    void forEachPage(std::function<void(const int&, const Page&)> visit) const {
        pageTable.recorrido(visit);
    }

    Cache& getLocalCache() { return localCache; }
    const Cache& getLocalCache() const { return localCache; }

    /** Útil cuando se hace END_PROCESS: vacía la tabla devolviendo las páginas físicas. */
    void collectAndClearAllPhysicalPages(std::vector<int>& outPhysical) {
        pageTable.recorrido([&](const int&, const Page& pg) {
            outPhysical.push_back(pg.physicalPage);
        });
        // Eliminar todas las claves
        std::vector<int> keys;
        pageTable.recorrido([&](const int& k, const Page&) { keys.push_back(k); });
        for (int k : keys) pageTable.eliminar(k);
        pageCount = 0;
        localCache.clear();
        localCache.setCapacity(0);
    }
};

#endif // PROCESO_H
