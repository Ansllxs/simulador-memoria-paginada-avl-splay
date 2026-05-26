#ifndef OPERATIONPARSER_H
#define OPERATIONPARSER_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include "MemoryManager.h"

/**
 * OperationParser
 *
 * Lee comandos línea por línea desde un std::istream y los ejecuta sobre el
 * MemoryManager. Acepta:
 *
 *   MEMORIA_TOTAL n
 *   TAMANO_PAGINA n
 *   TAMANO_CACHE_GLOBAL n
 *   NEW_PROCESS id memoria
 *   END_PROCESS id
 *   ACCESS id direccion
 *   ALLOCATE id memoria
 *   FREE id direccionInicial bytes
 *   STATUS
 *   METRICS
 *   EXIT  (sólo en modo interactivo)
 *
 * Las líneas vacías y las que comienzan con '#' se ignoran (comentarios).
 */
class OperationParser {
private:
    MemoryManager& manager;

    // Configuración acumulada (se aplica al detectar las 3 directivas).
    long pendingTotalMemory = -1;
    int pendingPageSize = -1;
    int pendingGlobalCache = -1;

    static std::string upper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        return s;
    }

    static std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string t;
        while (iss >> t) tokens.push_back(t);
        return tokens;
    }

    void tryAutoConfigure() {
        if (manager.isConfigured()) return;
        if (pendingTotalMemory > 0 && pendingPageSize > 0 && pendingGlobalCache >= 0) {
            manager.configure(pendingTotalMemory, pendingPageSize, pendingGlobalCache);
        }
    }

public:
    OperationParser(MemoryManager& mgr) : manager(mgr) {}

    /** Procesa una sola línea. Retorna false si se solicitó EXIT. */
    bool processLine(const std::string& rawLine) {
        std::string line = rawLine;
        // Quitar comentarios estilo '#'
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        // Trim
        while (!line.empty() && isspace((unsigned char)line.front())) line.erase(line.begin());
        while (!line.empty() && isspace((unsigned char)line.back())) line.pop_back();
        if (line.empty()) return true;

        auto tokens = tokenize(line);
        if (tokens.empty()) return true;

        std::string cmd = upper(tokens[0]);

        try {
            if (cmd == "MEMORIA_TOTAL") {
                if (tokens.size() != 2) { std::cout << "[ERROR] Uso: MEMORIA_TOTAL n\n"; return true; }
                pendingTotalMemory = std::stol(tokens[1]);
                tryAutoConfigure();
            } else if (cmd == "TAMANO_PAGINA") {
                if (tokens.size() != 2) { std::cout << "[ERROR] Uso: TAMANO_PAGINA n\n"; return true; }
                pendingPageSize = std::stoi(tokens[1]);
                tryAutoConfigure();
            } else if (cmd == "TAMANO_CACHE_GLOBAL") {
                if (tokens.size() != 2) { std::cout << "[ERROR] Uso: TAMANO_CACHE_GLOBAL n\n"; return true; }
                pendingGlobalCache = std::stoi(tokens[1]);
                tryAutoConfigure();
            } else if (cmd == "NEW_PROCESS") {
                if (tokens.size() != 3) { std::cout << "[ERROR] Uso: NEW_PROCESS id memoria\n"; return true; }
                manager.newProcess(tokens[1], std::stoi(tokens[2]));
            } else if (cmd == "END_PROCESS") {
                if (tokens.size() != 2) { std::cout << "[ERROR] Uso: END_PROCESS id\n"; return true; }
                manager.endProcess(tokens[1]);
            } else if (cmd == "ACCESS") {
                if (tokens.size() != 3) { std::cout << "[ERROR] Uso: ACCESS id direccion\n"; return true; }
                manager.access(tokens[1], std::stol(tokens[2]));
            } else if (cmd == "ALLOCATE") {
                if (tokens.size() != 3) { std::cout << "[ERROR] Uso: ALLOCATE id memoria\n"; return true; }
                manager.allocate(tokens[1], std::stoi(tokens[2]));
            } else if (cmd == "FREE") {
                if (tokens.size() != 4) { std::cout << "[ERROR] Uso: FREE id direccionInicial bytes\n"; return true; }
                manager.freeMemory(tokens[1], std::stol(tokens[2]), std::stoi(tokens[3]));
            } else if (cmd == "STATUS") {
                manager.printStatus();
            } else if (cmd == "METRICS") {
                manager.printMetrics();
            } else if (cmd == "EXIT" || cmd == "QUIT" || cmd == "SALIR") {
                return false;
            } else if (cmd == "HELP" || cmd == "AYUDA") {
                printHelp();
            } else {
                std::cout << "[ERROR] Operacion desconocida: " << tokens[0] << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Argumento numerico invalido en la linea: '" << rawLine << "'\n";
        }
        return true;
    }

    /** Procesa todas las líneas de un stream. */
    void processStream(std::istream& in) {
        std::string line;
        while (std::getline(in, line)) {
            if (!processLine(line)) break;
        }
    }

    /** Abre un archivo y procesa todas sus líneas. */
    bool processFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "[ERROR] No se pudo abrir el archivo: " << path << "\n";
            return false;
        }
        std::cout << ">>> Leyendo operaciones desde: " << path << "\n\n";
        processStream(file);
        return true;
    }

    static void printHelp() {
        std::cout <<
            "\nComandos disponibles:\n"
            "  MEMORIA_TOTAL n               Configura la memoria total (bytes)\n"
            "  TAMANO_PAGINA n               Configura el tamano de pagina (bytes)\n"
            "  TAMANO_CACHE_GLOBAL n         Configura el tamano del cache global\n"
            "  NEW_PROCESS id memoria        Crea un proceso con la memoria solicitada\n"
            "  END_PROCESS id                Finaliza un proceso\n"
            "  ACCESS id direccion           Referencia una direccion de memoria\n"
            "  ALLOCATE id memoria           Solicita memoria adicional\n"
            "  FREE id dirInicial bytes      Libera memoria de un rango\n"
            "  STATUS                        Muestra el estado del sistema\n"
            "  METRICS                       Muestra las metricas\n"
            "  HELP                          Muestra esta ayuda\n"
            "  EXIT                          Termina el simulador\n";
    }
};

#endif // OPERATIONPARSER_H
