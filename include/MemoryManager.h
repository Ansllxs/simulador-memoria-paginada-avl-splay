#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <string>
#include <unordered_map>
#include <iostream>
#include "AVLTree.h"
#include "Page.h"
#include "Proceso.h"
#include "Cache.h"

/**
 * Estructura con las métricas del sistema.
 */
struct Metrics {
    long totalAccesses = 0;
    long localHits = 0;
    long localMisses = 0;
    long globalHits = 0;
    long globalMisses = 0;

    long splayHits = 0;       // estaba en el Splay (pero no en cachés)
    long pageFaults = 0;      // dirección inválida / página no existe

    long lruEvictions = 0;    // veces que se hizo reemplazo LRU
    long errors = 0;          // errores detectados
};

/**
 * MemoryManager
 *
 * Núcleo del simulador. Orquesta:
 *   - AVL de páginas físicas libres
 *   - Tabla global de procesos (unordered_map id -> Process)
 *   - Caché global (LRU)
 *   - Métricas y manejo de errores
 */
class MemoryManager {
private:
    long totalMemory;
    int pageSize;
    int totalPages;
    int globalCacheCapacity;
    long timeStamp; // reloj lógico para LRU y referencias

    AVLTree<int, Page> freePages;                        // páginas físicas libres
    std::unordered_map<std::string, Process> processes;  // procesos vivos
    Cache globalCache;
    Metrics metrics;
    bool configured;

    /** Toma la página física libre con menor número y la quita del AVL. */
    bool takeFreePage(int& outPhysical) {
        Page tmp;
        int key;
        if (!freePages.minimo(key, tmp)) return false;
        freePages.eliminar(key);
        outPhysical = key;
        return true;
    }

    void returnFreePage(int physical) {
        Page p;
        p.physicalPage = physical;
        p.valid = false;
        freePages.insertar(physical, p);
    }

    void log(const std::string& msg) {
        std::cout << msg << "\n";
    }

    void error(const std::string& msg) {
        std::cout << "[ERROR] " << msg << "\n";
        ++metrics.errors;
    }

public:
    MemoryManager()
        : totalMemory(0), pageSize(0), totalPages(0),
          globalCacheCapacity(0), timeStamp(0),
          globalCache(0, "Global"),
          configured(false) {}

    bool isConfigured() const { return configured; }
    long getTimeStamp() const { return timeStamp; }
    const Metrics& getMetrics() const { return metrics; }

    /** Configura el sistema. Solo se puede hacer cuando aún no hay procesos. */
    bool configure(long totalMem, int pSize, int globalCacheCap) {
        if (!processes.empty()) {
            error("No se puede reconfigurar mientras existan procesos.");
            return false;
        }
        if (totalMem <= 0 || pSize <= 0 || totalMem % pSize != 0) {
            error("Parametros invalidos: MEMORIA_TOTAL debe ser multiplo de TAMANO_PAGINA y ambos > 0.");
            return false;
        }
        totalMemory = totalMem;
        pageSize = pSize;
        totalPages = (int)(totalMem / pSize);
        globalCacheCapacity = std::max(0, globalCacheCap);

        // Reinicializar
        while (!freePages.vacio()) {
            int k; Page v;
            if (!freePages.minimo(k, v)) break;
            freePages.eliminar(k);
        }
        for (int i = 0; i < totalPages; ++i) {
            Page p;
            p.physicalPage = i;
            p.valid = false;
            freePages.insertar(i, p);
        }
        globalCache.clear();
        globalCache.setCapacity(globalCacheCapacity);
        configured = true;

        std::cout << "Sistema configurado: " << totalMemory << " bytes, "
                  << "tamano pagina=" << pageSize << ", "
                  << "paginas fisicas=" << totalPages << ", "
                  << "cache global=" << globalCacheCapacity << "\n";
        return true;
    }

    bool requireConfigured() {
        if (!configured) {
            error("El sistema no esta configurado. Use MEMORIA_TOTAL, TAMANO_PAGINA y TAMANO_CACHE_GLOBAL primero.");
            return false;
        }
        return true;
    }

    // ===================== OPERACIONES PRINCIPALES =====================

    /** NEW_PROCESS id memoriaInicial */
    void newProcess(const std::string& pid, int memory) {
        if (!requireConfigured()) return;
        if (processes.count(pid)) {
            error("Proceso duplicado: " + pid);
            return;
        }
        if (memory <= 0) {
            error("Memoria inicial invalida para " + pid);
            return;
        }
        int pagesNeeded = (memory + pageSize - 1) / pageSize; // techo
        if (pagesNeeded > freePages.tamano()) {
            error("Memoria insuficiente para " + pid +
                  " (requiere " + std::to_string(pagesNeeded) +
                  " paginas, libres " + std::to_string(freePages.tamano()) + ")");
            return;
        }

        Process proc(pid, memory);
        for (int i = 0; i < pagesNeeded; ++i) {
            int phys = -1;
            takeFreePage(phys);
            Page pg(proc.getNextVirtualPage(), phys, pid);
            pg.lastReference = ++timeStamp;
            proc.addPage(pg);
        }
        processes.emplace(pid, std::move(proc));

        std::cout << "NEW_PROCESS " << pid << " OK: "
                  << pagesNeeded << " paginas asignadas. "
                  << "Cache local max = "
                  << Process::computeLocalCacheSize(pagesNeeded) << "\n";
    }

    /** END_PROCESS id */
    void endProcess(const std::string& pid) {
        if (!requireConfigured()) return;
        auto it = processes.find(pid);
        if (it == processes.end()) {
            error("Proceso inexistente: " + pid);
            return;
        }
        std::vector<int> physicals;
        it->second.collectAndClearAllPhysicalPages(physicals);
        for (int phys : physicals) returnFreePage(phys);
        globalCache.removeByProcess(pid);
        processes.erase(it);

        std::cout << "END_PROCESS " << pid << " OK: "
                  << physicals.size() << " paginas liberadas.\n";
    }

    /** ACCESS id direccion */
    void access(const std::string& pid, long address) {
        if (!requireConfigured()) return;
        auto it = processes.find(pid);
        if (it == processes.end()) {
            error("Proceso inexistente: " + pid);
            return;
        }
        if (address < 0) {
            error("Direccion invalida: " + std::to_string(address));
            return;
        }
        int virtualPage = (int)(address / pageSize);
        int offset = (int)(address % pageSize);

        ++metrics.totalAccesses;
        ++timeStamp;

        std::cout << "ACCESS " << pid << " " << address
                  << " -> VP=" << virtualPage << " offset=" << offset << "\n";

        std::string localKey = Cache::makeKey(pid, virtualPage);
        std::string globalKey = localKey;

        Process& proc = it->second;
        CacheEntry hitEntry;

        // 1. Caché local
        if (proc.getLocalCache().get(localKey, hitEntry)) {
            ++metrics.localHits;
            // También refrescar en caché global
            CacheEntry victim;
            hitEntry.lastReference = timeStamp;
            globalCache.put(hitEntry, victim);
            // Actualizar Splay (para que la pagina suba a la raiz)
            Page* pg = proc.findPage(virtualPage);
            if (pg) {
                pg->lastReference = timeStamp;
                ++pg->references;
            }
            std::cout << "  Resultado: HIT en cache LOCAL. Pagina fisica = "
                      << hitEntry.physicalPage << "\n";
            return;
        }
        ++metrics.localMisses;

        // 2. Caché global
        if (globalCache.get(globalKey, hitEntry)) {
            ++metrics.globalHits;
            // Promover al caché local
            hitEntry.lastReference = timeStamp;
            CacheEntry victim;
            bool evicted = proc.getLocalCache().put(hitEntry, victim);
            if (evicted) ++metrics.lruEvictions;
            // Splay
            Page* pg = proc.findPage(virtualPage);
            if (pg) {
                pg->lastReference = timeStamp;
                ++pg->references;
            }
            std::cout << "  Resultado: MISS local, HIT en cache GLOBAL. Pagina fisica = "
                      << hitEntry.physicalPage << "\n";
            return;
        }
        ++metrics.globalMisses;

        // 3. Tabla de páginas (Splay)
        Page* pg = proc.findPage(virtualPage);
        if (!pg) {
            ++metrics.pageFaults;
            error("Pagina virtual " + std::to_string(virtualPage) +
                  " no asignada al proceso " + pid);
            return;
        }

        ++metrics.splayHits;
        pg->lastReference = timeStamp;
        ++pg->references;

        // Insertar en ambos cachés
        CacheEntry newEntry(pid, virtualPage, pg->physicalPage, timeStamp);
        CacheEntry victim;
        bool ev1 = proc.getLocalCache().put(newEntry, victim);
        bool ev2 = globalCache.put(newEntry, victim);
        if (ev1) ++metrics.lruEvictions;
        if (ev2) ++metrics.lruEvictions;

        std::cout << "  Resultado: MISS en ambos caches, HIT en Splay Tree. Pagina fisica = "
                  << pg->physicalPage << "\n";
    }

    /** ALLOCATE id memoriaAdicional */
    void allocate(const std::string& pid, int extra) {
        if (!requireConfigured()) return;
        auto it = processes.find(pid);
        if (it == processes.end()) {
            error("Proceso inexistente: " + pid);
            return;
        }
        if (extra <= 0) {
            error("Memoria adicional invalida: " + std::to_string(extra));
            return;
        }
        Process& proc = it->second;
        int pagesNeeded = (extra + pageSize - 1) / pageSize;
        if (pagesNeeded > freePages.tamano()) {
            error("Memoria insuficiente para ALLOCATE en " + pid);
            return;
        }
        for (int i = 0; i < pagesNeeded; ++i) {
            int phys = -1;
            takeFreePage(phys);
            Page pg(proc.getNextVirtualPage(), phys, pid);
            pg.lastReference = ++timeStamp;
            proc.addPage(pg);
        }
        proc.addRequestedMemory(extra);
        std::cout << "ALLOCATE " << pid << " OK: " << pagesNeeded
                  << " paginas adicionales. Total ahora: " << proc.getPageCount()
                  << ". Cache local max = "
                  << Process::computeLocalCacheSize(proc.getPageCount()) << "\n";
    }

    /** FREE id direccionInicial cantidadBytes */
    void freeMemory(const std::string& pid, long startAddress, int bytes) {
        if (!requireConfigured()) return;
        auto it = processes.find(pid);
        if (it == processes.end()) {
            error("Proceso inexistente: " + pid);
            return;
        }
        if (startAddress < 0 || bytes <= 0) {
            error("Parametros invalidos en FREE");
            return;
        }
        int startPage = (int)(startAddress / pageSize);
        int endPage = (int)((startAddress + bytes - 1) / pageSize);

        Process& proc = it->second;
        int freed = 0;
        for (int vp = startPage; vp <= endPage; ++vp) {
            Page* pg = proc.findPage(vp);
            if (!pg) {
                error("FREE: pagina virtual " + std::to_string(vp) +
                      " no asignada a " + pid);
                continue;
            }
            int phys = pg->physicalPage;
            proc.removePage(vp);
            returnFreePage(phys);
            globalCache.remove(Cache::makeKey(pid, vp));
            ++freed;
        }
        std::cout << "FREE " << pid << " OK: " << freed
                  << " paginas liberadas. Cache local max = "
                  << Process::computeLocalCacheSize(proc.getPageCount()) << "\n";
    }

    // ===================== DIAGNOSTICOS =====================

    void printStatus(std::ostream& out = std::cout) {
        if (!requireConfigured()) return;
        out << "\n========== ESTADO DEL SISTEMA ==========\n";
        out << "Memoria total: " << totalMemory << " bytes\n";
        out << "Tamano pagina: " << pageSize << " bytes\n";
        out << "Total paginas fisicas: " << totalPages << "\n";
        out << "Paginas libres: " << freePages.tamano() << "\n";
        out << "Paginas ocupadas: " << (totalPages - freePages.tamano()) << "\n";
        out << "Utilizacion: "
            << ((totalPages - freePages.tamano()) * 100.0 / totalPages) << "%\n";
        out << "Procesos activos: " << processes.size() << "\n";

        for (auto& kv : processes) {
            const Process& p = kv.second;
            out << "\n--- Proceso " << p.getId() << " ---\n";
            out << "  Memoria solicitada: " << p.getRequestedMemory() << " bytes\n";
            out << "  Paginas asignadas: " << p.getPageCount() << "\n";
            out << "  Cache local capacidad: "
                << Process::computeLocalCacheSize(p.getPageCount()) << "\n";
            out << "  Tabla de paginas (in-order del Splay):\n";
            p.forEachPage([&](const int& vp, const Page& pg) {
                out << "    VP=" << vp << " -> PP=" << pg.physicalPage
                    << " refs=" << pg.references
                    << " ultRef=" << pg.lastReference << "\n";
            });
            p.getLocalCache().display(out);
        }

        out << "\n--- Cache Global ---\n";
        globalCache.display(out);
        out << "========================================\n";
    }

    void printMetrics(std::ostream& out = std::cout) {
        out << "\n========== METRICAS ==========\n";
        out << "Total accesos:           " << metrics.totalAccesses << "\n";
        out << "Hits en cache local:     " << metrics.localHits << "\n";
        out << "Misses en cache local:   " << metrics.localMisses << "\n";
        out << "Hits en cache global:    " << metrics.globalHits << "\n";
        out << "Misses en cache global:  " << metrics.globalMisses << "\n";
        out << "Hits en Splay (proceso): " << metrics.splayHits << "\n";
        out << "Page faults:             " << metrics.pageFaults << "\n";
        out << "Reemplazos LRU:          " << metrics.lruEvictions << "\n";
        out << "Errores detectados:      " << metrics.errors << "\n";
        if (configured) {
            int occupied = totalPages - freePages.tamano();
            out << "Paginas asignadas:       " << occupied << "/" << totalPages << "\n";
            out << "Paginas libres:          " << freePages.tamano() << "\n";
            out << "Utilizacion memoria:     "
                << (occupied * 100.0 / totalPages) << "%\n";
        }
        if (metrics.totalAccesses > 0) {
            double hitRateLocal = (metrics.localHits * 100.0) / metrics.totalAccesses;
            double hitRateGlobal = (metrics.globalHits * 100.0) / metrics.totalAccesses;
            out << "Hit-rate local:          " << hitRateLocal << "%\n";
            out << "Hit-rate global:         " << hitRateGlobal << "%\n";
        }
        out << "===============================\n";
    }
};

#endif // MEMORYMANAGER_H
