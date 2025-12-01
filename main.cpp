#include <iostream>
#include <vector>
#include <stdexcept>
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

//El código fue hecho con la librería SFML y compilado en mingw

//Constantes para la visualización
const float NODE_RADIUS = 25.f;
const float VERTICAL_SPACING = 80.f;

class SegmentTree {
public:
    enum State { DEFAULT, CONTAINED, OUTSIDE, PARTIAL };

private:
    int ledge;    // límite izquierdo del intervalo
    int redge;    // límite derecho del intervalo
    int val;      // valor agregado (en este caso, suma del rango)
    SegmentTree* left;   // hijo izquierdo
    SegmentTree* right;  // hijo derecho

    // Atributos para la visualización
    float x, y;
    mutable State state;

    // Recalcula el valor del nodo a partir de sus hijos.
    void recalc() {
        if (!left && !right) return; // nodo hoja, nada que recalcular
        int leftVal  = (left  ? left->val  : 0);
        int rightVal = (right ? right->val : 0);
        val = leftVal + rightVal;
    }

    // Construye el árbol de manera recursiva.
    void build(int l, int r, const std::vector<int>& arr) {
        ledge = l;
        redge = r;
        state = DEFAULT;

        if (l == r) { // nodo hoja
            val = arr[l];
            left = right = nullptr;
            return;
        }

        int mid = (l + r) / 2;
        left  = new SegmentTree(l, mid, arr);
        right = new SegmentTree(mid+1, r, arr);
        recalc();
    }

public:
    // Constructor: crea el árbol a partir de un vector.
    SegmentTree(const std::vector<int>& arr)
        : ledge(0), redge(0), val(0), left(nullptr), right(nullptr), state(DEFAULT)
    {
        if (arr.empty()) {
            throw std::invalid_argument("El arreglo no puede estar vacío");
        }
        build(0, static_cast<int>(arr.size()) - 1, arr);
    }

    // Constructor para nodos hijos:
    SegmentTree(int l, int r, const std::vector<int>& array)
        : ledge(l), redge(r), state(DEFAULT) 
    {
        build(l, r, array);
    }


    // Devuelve la suma en el rango [l, r] (0-indexado, inclusivo). O(log(n))
    int query(int l, int r) const {
        if (l > r) std::swap(l, r);

        // Caso 1: no hay intersección entre [l, r] y [ledge, redge] (El nodo no está en la consulta)
        if (r < ledge || redge < l) {
            state = OUTSIDE;
            return 0; // elemento neutro de la suma
        }

        // Caso 2: el intervalo del nodo está completamente contenido en [l, r]
        if (l <= ledge && redge <= r) {
            state = CONTAINED;
            return val;
        }

        // Caso 3: intersección parcial, se consulta en ambos hijos (El nodo esta parcialmente en la consulta)
        state = PARTIAL;
        int leftAns  = (left  ? left->query(l, r)  : 0);
        int rightAns = (right ? right->query(l, r) : 0);
        return leftAns + rightAns;
    }

    // Actualiza el valor en la posición idx a newValue.
    void update(int idx, int newValue) {
        if (idx < ledge || idx > redge) {
            state = OUTSIDE;
            return; // fuera del rango de este nodo
        }

        if (ledge == redge) { // nodo hoja
            state = CONTAINED;
            val = newValue;
            return;
        }

        //Contiene a idx dentro de su intervalo
        state = PARTIAL;
        if (left && idx <= left->redge) {
            left->update(idx, newValue);
        } else if (right) {
            right->update(idx, newValue);
        }

        recalc();
    }


    // Suma un delta a todos los elementos en el rango [ql, qr]. O(n)
    void updateRange(int ql, int qr, int delta) {
        if (ql > qr) std::swap(ql, qr);

        // No hay intersección
        if (qr < ledge || ql > redge) {
            state = OUTSIDE;
            return;
        }

        // Nodo hoja contenido en el rango
        if (ledge == redge) {
            state = CONTAINED;
            val += delta;
            return;
        }
        


        state = PARTIAL;

        // Intersección parcial: se baja a ambos hijos
        if (left)  left->updateRange(ql, qr, delta);
        if (right) right->updateRange(ql, qr, delta);

        recalc();
    }

    // Funciones para la visualización

    void setPosition(float px, float py, float width) {
        x = px;
        y = py;
        if (left && right) {
            left->setPosition(x - width / 4, y + VERTICAL_SPACING, width / 2);
            right->setPosition(x + width / 4, y + VERTICAL_SPACING, width / 2);
        } else if (left) {
             left->setPosition(x, y + VERTICAL_SPACING, width / 2);
        }
    }

    void resetVisuals() const {
        state = DEFAULT;
        if (left) left->resetVisuals();
        if (right) right->resetVisuals();
    }

    void draw(sf::RenderWindow& window, sf::Font& font) const {

        if (left) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, y), sf::Color::Black),
                sf::Vertex(sf::Vector2f(left->x, left->y), sf::Color::Black)
            };
            window.draw(line, 2, sf::Lines);
            left->draw(window, font);
        }
        if (right) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, y), sf::Color::Black),
                sf::Vertex(sf::Vector2f(right->x, right->y), sf::Color::Black)
            };
            window.draw(line, 2, sf::Lines);
            right->draw(window, font);
        }


        sf::CircleShape circle(NODE_RADIUS);
        circle.setOrigin(NODE_RADIUS, NODE_RADIUS);
        circle.setPosition(x, y);
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color::Black);

        switch (state) {
            case CONTAINED: circle.setFillColor(sf::Color::Green); break;
            case OUTSIDE:   circle.setFillColor(sf::Color::Red); break;
            case PARTIAL:   circle.setFillColor(sf::Color::Yellow); break;
            default:        circle.setFillColor(sf::Color::White); break;
        }

        window.draw(circle);


        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(val));
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::Black);
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        text.setPosition(x, y);
        window.draw(text);
    }
    
    void getLeaves(std::vector<int>& leaves) const {
        if (ledge == redge) {
            leaves.push_back(val);
            return;
        }
        if (left) left->getLeaves(leaves);
        if (right) right->getLeaves(leaves);
    }
};

struct Operation {
    std::string name;
    std::function<void(SegmentTree&)> func;
};

int main() {

    sf::RenderWindow window(sf::VideoMode(1200, 800), "Segment Tree Visualization");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/consola.ttf")) {
             std::cerr << "Error loading font" << std::endl;
             return -1;
        }
    }


    std::vector<int> datos = {2, 1, 3, 4, 5, 7, 8, 9};


    SegmentTree st(datos);
    st.setPosition(600, 50, 1000);

    std::string lastResult = "";


    std::vector<Operation> ops;
    

    ops.push_back({"Estado Inicial", [&](SegmentTree& t){ 
        t.resetVisuals(); 
        lastResult = "";
    }});


    ops.push_back({"Consulta [2, 5]", [&](SegmentTree& t){
        t.resetVisuals();
        int res = t.query(2, 5);
        lastResult = "Resultado: " + std::to_string(res);
        std::cout << "2) Consulta suma en [2, 5] = " << res << "\n";
    }});


    ops.push_back({"Actualizacion T[3] = 10", [&](SegmentTree& t){
        t.resetVisuals();
        t.update(3, 10);
        lastResult = "";
        std::cout << "3) Actualizacion puntual: datos[3] = 10\n";
    }});


    ops.push_back({"Consulta [2, 5] (Post-Update)", [&](SegmentTree& t){
        t.resetVisuals();
        int res = t.query(2, 5);
        lastResult = "Resultado: " + std::to_string(res);
        std::cout << "   Nueva suma en [2, 5] = " << res << "\n";
    }});


    ops.push_back({"Actualizacion Rango [1, 4] += 3", [&](SegmentTree& t){
        t.resetVisuals();
        t.updateRange(1, 4, 3);
        lastResult = "";
        std::cout << "4) Actualizacion por rango: se suma 3 a [1, 4]\n";
    }});


    ops.push_back({"Consulta [2, 5]", [&](SegmentTree& t){
        t.resetVisuals();
        int res = t.query(2, 5);
        lastResult = "Resultado: " + std::to_string(res);
        std::cout << "   Suma final en [2, 5] = " << res << "\n";
    }});
    

    ops.push_back({"Estado final", [&](SegmentTree& t){
        t.resetVisuals();
        lastResult = "";
    }});

    int currentOp = 0;

    ops[currentOp].func(st);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Right) {
                    if (currentOp < ops.size() - 1) {
                        currentOp++;
                        ops[currentOp].func(st);
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                     if (currentOp < ops.size() - 1) {
                        currentOp++;
                        ops[currentOp].func(st);
                    }
                }
            }
        }

        window.clear(sf::Color(240, 240, 240));


        st.draw(window, font);


        std::vector<int> leaves;
        st.getLeaves(leaves);
        float startX = 600 - (leaves.size() * 50) / 2.0f;
        float startY = 700;
        
        for (size_t i = 0; i < leaves.size(); ++i) {
            sf::RectangleShape rect(sf::Vector2f(50, 50));
            rect.setPosition(startX + i * 50, startY);
            rect.setFillColor(sf::Color::White);
            rect.setOutlineThickness(1);
            rect.setOutlineColor(sf::Color::Black);
            window.draw(rect);

            sf::Text valText;
            valText.setFont(font);
            valText.setString(std::to_string(leaves[i]));
            valText.setCharacterSize(20);
            valText.setFillColor(sf::Color::Black);
            sf::FloatRect textRect = valText.getLocalBounds();
            valText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
            valText.setPosition(startX + i * 50 + 25, startY + 25);
            window.draw(valText);
            
            // Index
            sf::Text idxText;
            idxText.setFont(font);
            idxText.setString(std::to_string(i));
            idxText.setCharacterSize(12);
            idxText.setFillColor(sf::Color::Black);
            sf::FloatRect idxRect = idxText.getLocalBounds();
            idxText.setOrigin(idxRect.left + idxRect.width/2.0f, idxRect.top + idxRect.height/2.0f);
            idxText.setPosition(startX + i * 50 + 25, startY + 60);
            window.draw(idxText);
        }

        // Draw Info
        sf::Text infoText;
        infoText.setFont(font);
        infoText.setString("Operacion " + std::to_string(currentOp) + ": " + ops[currentOp].name + "\n" + lastResult + "\nPress Space/Click for Next");
        infoText.setCharacterSize(24);
        infoText.setFillColor(sf::Color::Black);
        infoText.setPosition(10, 10);
        window.draw(infoText);

        window.display();
    }

    return 0;
}