#ifndef AVLTREE_H
#define AVLTREE_H

#include <algorithm>
#include <functional>
#include <iostream>

/**
 * AVLTree<K, V>
 *
 * Árbol Binario de Búsqueda autobalanceado.
 * Mantiene la diferencia de alturas entre subárboles izquierdo y derecho
 * en {-1, 0, 1}, garantizando operaciones en O(log n).
 *
 * En este simulador se usa para administrar las páginas físicas libres:
 *   Clave (K)  = número de página física
 *   Valor (V)  = información de la página (objeto Page)
 */
template <typename K, typename V>
class AVLTree {
private:
    struct Node {
        K key;
        V value;
        int height;
        Node* left;
        Node* right;

        Node(const K& k, const V& v)
            : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
    };

    Node* root;
    int nodeCount;

    int height(Node* n) const { return n ? n->height : 0; }

    int balanceFactor(Node* n) const {
        return n ? height(n->left) - height(n->right) : 0;
    }

    void updateHeight(Node* n) {
        if (n) n->height = 1 + std::max(height(n->left), height(n->right));
    }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        x->right = y;
        y->left = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        y->left = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    Node* balance(Node* n) {
        updateHeight(n);
        int bf = balanceFactor(n);

        // Caso izquierda-izquierda
        if (bf > 1 && balanceFactor(n->left) >= 0)
            return rotateRight(n);

        // Caso izquierda-derecha
        if (bf > 1 && balanceFactor(n->left) < 0) {
            n->left = rotateLeft(n->left);
            return rotateRight(n);
        }

        // Caso derecha-derecha
        if (bf < -1 && balanceFactor(n->right) <= 0)
            return rotateLeft(n);

        // Caso derecha-izquierda
        if (bf < -1 && balanceFactor(n->right) > 0) {
            n->right = rotateRight(n->right);
            return rotateLeft(n);
        }

        return n;
    }

    Node* insertNode(Node* n, const K& key, const V& value, bool& inserted) {
        if (!n) {
            inserted = true;
            ++nodeCount;
            return new Node(key, value);
        }
        if (key < n->key) {
            n->left = insertNode(n->left, key, value, inserted);
        } else if (n->key < key) {
            n->right = insertNode(n->right, key, value, inserted);
        } else {
            n->value = value; // Ya existe, actualiza
            inserted = false;
            return n;
        }
        return balance(n);
    }

    Node* findMin(Node* n) const {
        while (n && n->left) n = n->left;
        return n;
    }

    Node* removeNode(Node* n, const K& key, bool& removed) {
        if (!n) {
            removed = false;
            return nullptr;
        }
        if (key < n->key) {
            n->left = removeNode(n->left, key, removed);
        } else if (n->key < key) {
            n->right = removeNode(n->right, key, removed);
        } else {
            removed = true;
            --nodeCount;
            if (!n->left || !n->right) {
                Node* tmp = n->left ? n->left : n->right;
                delete n;
                return tmp;
            }
            Node* succ = findMin(n->right);
            n->key = succ->key;
            n->value = succ->value;
            bool dummy;
            n->right = removeNode(n->right, succ->key, dummy);
        }
        return balance(n);
    }

    Node* searchNode(Node* n, const K& key) const {
        if (!n) return nullptr;
        if (key < n->key) return searchNode(n->left, key);
        if (n->key < key) return searchNode(n->right, key);
        return n;
    }

    void inOrderTraversal(Node* n, std::function<void(const K&, const V&)> visit) const {
        if (!n) return;
        inOrderTraversal(n->left, visit);
        visit(n->key, n->value);
        inOrderTraversal(n->right, visit);
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

public:
    AVLTree() : root(nullptr), nodeCount(0) {}
    ~AVLTree() { destroy(root); }

    // Sin copia (para evitar errores con punteros crudos)
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    // Sí permite mover (necesario para guardar el árbol dentro de unordered_map)
    AVLTree(AVLTree&& other) noexcept
        : root(other.root), nodeCount(other.nodeCount) {
        other.root = nullptr;
        other.nodeCount = 0;
    }
    AVLTree& operator=(AVLTree&& other) noexcept {
        if (this != &other) {
            destroy(root);
            root = other.root;
            nodeCount = other.nodeCount;
            other.root = nullptr;
            other.nodeCount = 0;
        }
        return *this;
    }

    /** Inserta una clave-valor. Si ya existe, actualiza el valor. */
    void insertar(const K& key, const V& value) {
        bool inserted;
        root = insertNode(root, key, value, inserted);
    }

    /** Elimina una clave. Retorna true si existía. */
    bool eliminar(const K& key) {
        bool removed;
        root = removeNode(root, key, removed);
        return removed;
    }

    /** Busca una clave. Retorna puntero al valor o nullptr si no existe. */
    V* buscar(const K& key) {
        Node* n = searchNode(root, key);
        return n ? &n->value : nullptr;
    }

    const V* buscar(const K& key) const {
        Node* n = searchNode(root, key);
        return n ? &n->value : nullptr;
    }

    bool contiene(const K& key) const {
        return searchNode(root, key) != nullptr;
    }

    /** Recorrido in-orden: visita las claves en orden ascendente. */
    void inOrden(std::function<void(const K&, const V&)> visit) const {
        inOrderTraversal(root, visit);
    }

    /** Retorna la clave mínima (útil para "asignar la página libre más baja"). */
    bool minimo(K& outKey, V& outValue) const {
        Node* n = findMin(root);
        if (!n) return false;
        outKey = n->key;
        outValue = n->value;
        return true;
    }

    int altura() const { return height(root); }
    int tamano() const { return nodeCount; }
    bool vacio() const { return root == nullptr; }
};

#endif // AVLTREE_H
