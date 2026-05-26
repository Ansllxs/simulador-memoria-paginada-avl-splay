#ifndef CACHE_H
#define CACHE_H

#include <list>
#include <unordered_map>
#include <string>
#include <iostream>

/**
 * CacheEntry: una entrada del caché.
 * Guarda la información mínima para responder un acceso sin tocar el Splay Tree.
 */
struct CacheEntry {
    std::string processId;
    int virtualPage;
    int physicalPage;
    long lastReference;

    CacheEntry() : processId(""), virtualPage(-1), physicalPage(-1), lastReference(0) {}
    CacheEntry(const std::string& pid, int vp, int pp, long ts)
        : processId(pid), virtualPage(vp), physicalPage(pp), lastReference(ts) {}
};

/**
 * Cache con política LRU (Least Recently Used).
 *
 * Implementación en O(1) por operación:
 *   - std::list<CacheEntry>: orden de uso (frente = más reciente, final = más viejo)
 *   - std::unordered_map<string, iterator>: para localizar la entrada en la lista
 *
 * Cuando se accede a una entrada (hit), se mueve al frente.
 * Cuando se inserta y el caché está lleno, se desaloja la del final (LRU).
 *
 * Las claves son strings para soportar tanto el caché local (clave = virtualPage)
 * como el caché global (clave = processId + ":" + virtualPage).
 */
class Cache {
private:
    int capacity;
    std::list<CacheEntry> entries; // frente = más reciente
    std::unordered_map<std::string, std::list<CacheEntry>::iterator> index;
    std::string name; // sólo para impresiones (ej. "Local P1", "Global")

public:
    Cache(int cap = 0, const std::string& nombre = "Cache")
        : capacity(cap), name(nombre) {}

    void setCapacity(int cap) {
        capacity = cap;
        while ((int)entries.size() > capacity) {
            auto last = entries.back();
            index.erase(makeKey(last.processId, last.virtualPage));
            entries.pop_back();
        }
    }

    void setName(const std::string& n) { name = n; }
    const std::string& getName() const { return name; }
    int getCapacity() const { return capacity; }
    int size() const { return (int)entries.size(); }
    bool isFull() const { return (int)entries.size() >= capacity; }
    bool isEmpty() const { return entries.empty(); }

    /** Construye una clave única a partir de proceso + página virtual. */
    static std::string makeKey(const std::string& pid, int virtualPage) {
        return pid + ":" + std::to_string(virtualPage);
    }

    /**
     * Busca una entrada. Si la encuentra (HIT), la mueve al frente y retorna true.
     * outEntry recibe una copia de la entrada encontrada.
     */
    bool get(const std::string& key, CacheEntry& outEntry) {
        auto it = index.find(key);
        if (it == index.end()) return false;
        // Mover al frente
        entries.splice(entries.begin(), entries, it->second);
        outEntry = *it->second;
        return true;
    }

    bool contains(const std::string& key) const {
        return index.find(key) != index.end();
    }

    /**
     * Inserta o actualiza una entrada y la pone al frente.
     * Si el caché se llena, desaloja la del final (víctima LRU).
     * Retorna true si hubo desalojo, y carga la víctima en outVictim.
     */
    bool put(const CacheEntry& entry, CacheEntry& outVictim) {
        if (capacity <= 0) return false; // caché deshabilitado

        std::string key = makeKey(entry.processId, entry.virtualPage);
        auto it = index.find(key);

        if (it != index.end()) {
            // Ya existe: actualizar y mover al frente
            *it->second = entry;
            entries.splice(entries.begin(), entries, it->second);
            return false;
        }

        bool evicted = false;
        if ((int)entries.size() >= capacity) {
            // Sacar el LRU (el del final)
            outVictim = entries.back();
            index.erase(makeKey(outVictim.processId, outVictim.virtualPage));
            entries.pop_back();
            evicted = true;
        }

        entries.push_front(entry);
        index[key] = entries.begin();
        return evicted;
    }

    /** Elimina una entrada específica. */
    bool remove(const std::string& key) {
        auto it = index.find(key);
        if (it == index.end()) return false;
        entries.erase(it->second);
        index.erase(it);
        return true;
    }

    /** Elimina todas las entradas pertenecientes a un proceso (útil en END_PROCESS). */
    int removeByProcess(const std::string& pid) {
        int removed = 0;
        for (auto it = entries.begin(); it != entries.end(); ) {
            if (it->processId == pid) {
                index.erase(makeKey(it->processId, it->virtualPage));
                it = entries.erase(it);
                ++removed;
            } else {
                ++it;
            }
        }
        return removed;
    }

    void clear() {
        entries.clear();
        index.clear();
    }

    /** Imprime el contenido del caché del más reciente al menos reciente. */
    void display(std::ostream& out = std::cout) const {
        out << "  [" << name << "] (" << entries.size() << "/" << capacity << "): ";
        if (entries.empty()) {
            out << "(vacio)\n";
            return;
        }
        out << "\n";
        int i = 1;
        for (const auto& e : entries) {
            out << "    " << i++ << ". Proceso=" << e.processId
                << " VP=" << e.virtualPage
                << " PP=" << e.physicalPage
                << " t=" << e.lastReference << "\n";
        }
    }
};

#endif // CACHE_H
