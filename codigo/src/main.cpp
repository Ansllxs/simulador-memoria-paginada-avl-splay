/**
 * Simulador de Gestión de Memoria Paginada con AVL y Splay Trees.
 * Proyecto II - Estructuras de Datos - TEC, I Semestre 2026.
 *
 * Uso:
 *   ./simulador                  Modo interactivo
 *   ./simulador <archivo>        Procesa operaciones desde un archivo
 *   ./simulador <archivo> -i     Procesa el archivo y luego entra a interactivo
 */

#include <iostream>
#include <string>
#include "MemoryManager.h"
#include "OperationParser.h"

static void banner() {
    std::cout <<
        "===============================================================\n"
        " Simulador de Memoria Paginada (AVL + Splay) - Proyecto II EdD \n"
        " TEC, I Semestre 2026                                          \n"
        "===============================================================\n"
        "Escriba HELP para ver la lista de comandos.\n";
}

static void runInteractive(OperationParser& parser) {
    std::string line;
    std::cout << "\nModo interactivo activado. Escriba EXIT para salir.\n";
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, line)) break;
        if (!parser.processLine(line)) break;
    }
    std::cout << "\nFinalizando simulador.\n";
}

int main(int argc, char** argv) {
    banner();

    MemoryManager manager;
    OperationParser parser(manager);

    bool runInteractiveAfter = false;
    std::string filePath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--interactive") {
            runInteractiveAfter = true;
        } else if (arg == "-h" || arg == "--help") {
            OperationParser::printHelp();
            return 0;
        } else {
            filePath = arg;
        }
    }

    if (!filePath.empty()) {
        parser.processFile(filePath);
        // Mostrar estado y métricas finales tras procesar el archivo
        std::cout << "\n>>> Estado final tras procesar el archivo:\n";
        manager.printMetrics();
        if (!runInteractiveAfter) return 0;
    }

    runInteractive(parser);

    std::cout << "\n>>> Metricas finales:\n";
    manager.printMetrics();
    return 0;
}
