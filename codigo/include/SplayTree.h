#ifndef SPLAYTREE_H
#define SPLAYTREE_H

#include <functional>
#include <iostream>

/**
 * SplayTree<K, V>
 *
 * Árbol Binario de Búsqueda autoajustable. Cada operación (insertar, buscar,
 * eliminar) hace "splay" sobre el nodo accedido, llevándolo a la raíz mediante
 * rotaciones Zig, Zig-Zig y Zig-Zag.
 *
 * Esto hace que los elementos consultados frecuentemente queden cerca de la
 * raíz, logrando O(log n) amortizado.
 *
 * En este simulador se usa como TABLA DE PÁGINAS de cada proceso:
 *   Clave (K)  = número de página virtual
 *   Valor (V)  = página asignada (objeto Page)
 */
template <typename K, typename V>
class SplayTree {
private:
    struct Node {
        K key;
        V value;
        Node* left;
        Node* right;
        Node* parent;

        Node(const K& k, const V& v)
            : key(k), value(v), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root;
    int nodeCount;

    /** Rotación Zig hacia la derecha (sube el hijo izquierdo). */
    void rotacionZigDerecha(Node* x) {
        Node* p = x->parent;
        Node* B = x->right;

        x->parent = p->parent;
        if (p->parent) {
            if (p->parent->left == p) p->parent->left = x;
            else p->parent->right = x;
        }

        x->right = p;
        p->parent = x;

        p->left = B;
        if (B) B->parent = p;
    }

    /** Rotación Zig hacia la izquierda (sube el hijo derecho). */
    void rotacionZigIzquierda(Node* x) {
        Node* p = x->parent;
        Node* B = x->left;

        x->parent = p->parent;
        if (p->parent) {
            if (p->parent->left == p) p->parent->left = x;
            else p->parent->right = x;
        }

        x->left = p;
        p->parent = x;

        p->right = B;
        if (B) B->parent = p;
    }

    /**
     * splay(x): aplica rotaciones para llevar x hasta la raíz.
     *  - Zig:     x es hijo de la raíz                       (1 rotación)
     *  - Zig-Zig: x y su padre son hijos del mismo lado      (2 rotaciones iguales)
     *  - Zig-Zag: x y su padre son hijos de lados opuestos   (2 rotaciones distintas)
     */
    void splay(Node* x) {
        while (x->parent) {
            Node* p = x->parent;
            Node* g = p->parent;
            if (!g) {
                // Caso ZIG (padre es la raíz)
                if (p->left == x) rotacionZigDerecha(x);
                else rotacionZigIzquierda(x);
            } else if (g->left == p && p->left == x) {
                // Caso ZIG-ZIG izquierda
                rotacionZigDerecha(p);
                rotacionZigDerecha(x);
            } else if (g->right == p && p->right == x) {
                // Caso ZIG-ZIG derecha
                rotacionZigIzquierda(p);
                rotacionZigIzquierda(x);
            } else if (g->left == p && p->right == x) {
                // Caso ZIG-ZAG izquierda-derecha
                rotacionZigIzquierda(x);
                rotacionZigDerecha(x);
            } else {
                // Caso ZIG-ZAG derecha-izquierda
                rotacionZigDerecha(x);
                rotacionZigIzquierda(x);
            }
        }
        root = x;
    }

    Node* findNode(const K& key) const {
        Node* cur = root;
        Node* last = nullptr;
        while (cur) {
            last = cur;
            if (key < cur->key) cur = cur->left;
            else if (cur->key < key) cur = cur->right;
            else return cur;
        }
        return last; // Si no la encuentra, retorna el último visitado (para hacer splay igual)
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    void inOrderTraversal(Node* n, std::function<void(const K&, const V&)> visit) const {
        if (!n) return;
        inOrderTraversal(n->left, visit);
        visit(n->key, n->value);
        inOrderTraversal(n->right, visit);
    }

public:
    SplayTree() : root(nullptr), nodeCount(0) {}
    ~SplayTree() { destroy(root); }

    SplayTree(const SplayTree&) = delete;
    SplayTree& operator=(const SplayTree&) = delete;

    // Sí permite mover (necesario para guardar el árbol dentro de Process)
    SplayTree(SplayTree&& other) noexcept
        : root(other.root), nodeCount(other.nodeCount) {
        other.root = nullptr;
        other.nodeCount = 0;
    }
    SplayTree& operator=(SplayTree&& other) noexcept {
        if (this != &other) {
            destroy(root);
            root = other.root;
            nodeCount = other.nodeCount;
            other.root = nullptr;
            other.nodeCount = 0;
        }
        return *this;
    }

    /** Inserta una clave-valor y hace splay del nuevo nodo a la raíz. */
    void insertar(const K& key, const V& value) {
        if (!root) {
            root = new Node(key, value);
            ++nodeCount;
            return;
        }
        Node* cur = root;
        Node* parent = nullptr;
        while (cur) {
            parent = cur;
            if (key < cur->key) cur = cur->left;
            else if (cur->key < key) cur = cur->right;
            else {
                cur->value = value;
                splay(cur);
                return;
            }
        }
        Node* newNode = new Node(key, value);
        newNode->parent = parent;
        if (key < parent->key) parent->left = newNode;
        else parent->right = newNode;
        ++nodeCount;
        splay(newNode);
    }

    /**
     * Busca una clave y hace splay sobre ella (o sobre el último nodo visitado
     * si no se encuentra). Retorna puntero al valor o nullptr si no existe.
     */
    V* buscar(const K& key) {
        if (!root) return nullptr;
        Node* n = findNode(key);
        splay(n);
        if (n->key == key) return &n->value;
        return nullptr;
    }

    /** Indica si la clave existe (también hace splay). */
    bool contiene(const K& key) {
        return buscar(key) != nullptr;
    }

    /**
     * Elimina una clave. Estrategia clásica:
     *  1. Hacer splay(key) -> queda en la raíz.
     *  2. Reemplazar la raíz uniendo subárbol izquierdo con el derecho.
     */
    bool eliminar(const K& key) {
        if (!root) return false;
        Node* n = findNode(key);
        splay(n);
        if (n->key != key) return false;

        Node* L = root->left;
        Node* R = root->right;
        if (L) L->parent = nullptr;
        if (R) R->parent = nullptr;
        delete root;
        --nodeCount;

        if (!L) {
            root = R;
        } else {
            // Hacer splay del máximo de L para que su hijo derecho sea null
            Node* maxL = L;
            while (maxL->right) maxL = maxL->right;
            root = L;
            splay(maxL);
            root->right = R;
            if (R) R->parent = root;
        }
        return true;
    }

    /** Recorrido in-orden (no modifica el árbol). */
    void recorrido(std::function<void(const K&, const V&)> visit) const {
        inOrderTraversal(root, visit);
    }

    int tamano() const { return nodeCount; }
    bool vacio() const { return root == nullptr; }

    /** Acceso de solo lectura a la raíz (la página más recientemente usada). */
    bool raiz(K& outKey, V& outValue) const {
        if (!root) return false;
        outKey = root->key;
        outValue = root->value;
        return true;
    }
};

#endif // SPLAYTREE_H
