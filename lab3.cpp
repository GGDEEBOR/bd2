#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <map>
#include <unordered_map>
#include <list>
#include <sstream>
#include <vector>
using namespace std;

const int PAGE_SIZE = 4096;  // Tamaño constante de la página
const int MAX_RECORD_LENGTH = 100;


class GestorAlmacenamiento {
private:
    fstream archivo;   // Objeto de flujo de archivo
    int numTotalPags;       // Número total de páginas en el archivo
    int posPag;             // Posición actual de la página

public:
    GestorAlmacenamiento(const string& nombreArchivo) {
        archivo.open(nombreArchivo, ios::in | ios::out | ios::binary);  // Apertura del archivo en modo lectura/escritura en formato binario
        archivo.read(reinterpret_cast<char*>(&numTotalPags), sizeof(int));  // Lectura del número total de páginas desde el archivo
        posPag = -1;  // Inicialización de la posición de la página 
    }

    ~GestorAlmacenamiento() {
        archivo.close();  // Cierre del archivo
    }

    int getNumTotalPags() const {
        return numTotalPags;  // total de páginas
    }

    void leerBloque(int numPag, char* buffer) {
        if (numPag >= numTotalPags) {
            cout << "Error: El archivo no tiene tantas páginas." << endl;  // Mensaje de error si el número de página solicitado es mayor al número total de páginas
            return;
        }

        archivo.seekg(sizeof(int) + numPag * PAGE_SIZE);  // Desplazamiento en el archivo a la posición correspondiente al bloque deseado
        archivo.read(buffer, PAGE_SIZE);  // Lectura del bloque desde el archivo al búfer
        posPag = numPag;  // Actualiza la posición de la página actual
    }

    void escribirBloque(int numPag, const char* data) {
        if (numPag >= numTotalPags) {
            cout << "Error: El archivo no tiene tantas páginas." << endl;  // Mensaje de error si el número de página solicitado es mayor al número total de páginas
            return;
        }

        archivo.seekp(sizeof(int) + numPag * PAGE_SIZE);  // Desplazamiento en el archivo a la posición correspondiente al bloque deseado
        archivo.write(data, PAGE_SIZE);  // Escribe los datos en el archivo desde el búfer proporcionado
        posPag = numPag;  // Actualiza la posición de la página actual
    }

    void eliminarBloque(int numPag) {
        if (numPag >= numTotalPags) {
            cout << "Error: El archivo no tiene tantas páginas." << endl;  // Mensaje de error si el número de página solicitado es mayor al número total de páginas
            return;
        }

        // Desplazamiento en el archivo a la posición correspondiente al bloque deseado
        archivo.seekp(sizeof(int) + numPag * PAGE_SIZE);

        // Llenar el bloque con bytes nulos
        char nullBuffer[PAGE_SIZE] = {0};
        archivo.write(nullBuffer, PAGE_SIZE);

        // Actualizar la posición de la página actual si es necesario
        if (posPag == numPag) {
            posPag = -1;
        }
        cout << "Bloque eliminado: " << numPag << endl;
    }
};

class Pagina {
public:
    int page_id;
    bool dirty;
    int pin_count;

    Pagina(int id) {
        page_id = id;
        dirty = false;
        pin_count = 0;
    }
};

class EstrategiasReemplazo {
public:
    virtual void AddPage(Pagina* page) = 0;
    virtual void RemovePage(Pagina* page) = 0;
    virtual Pagina* ChoosePageToEvict() = 0;
};

class LRU : public EstrategiasReemplazo {
private:
    list<Pagina*> page_list;

public:
    void AddPage(Pagina* page) {
        page_list.push_front(page);
    }

    void RemovePage(Pagina* page) {
        page_list.remove(page);
    }

    Pagina* ChoosePageToEvict() {
        if (!page_list.empty()) {
            Pagina* evict_page = page_list.back();
            page_list.pop_back();
            return evict_page;
        }
        return nullptr;
    }
};

class MRU : public EstrategiasReemplazo {
private:
    list<Pagina*> page_list;

public:
    void AddPage(Pagina* page) {
        page_list.push_back(page);
    }

    void RemovePage(Pagina* page) {
        page_list.remove(page);
    }

    Pagina* ChoosePageToEvict() {
        if (!page_list.empty()) {
            Pagina* evict_page = page_list.back();
            page_list.pop_back();
            return evict_page;
        }
        return nullptr;
    }
};

class Clock : public EstrategiasReemplazo {
private:
    list<Pagina*> page_list;
    typename list<Pagina*>::iterator hand;

public:
    Clock() : hand(page_list.begin()) {}

    void AddPage(Pagina* page) {
        page_list.push_back(page);
    }

    void RemovePage(Pagina* page) {
        page_list.remove(page);
        if (hand == page_list.end())
            hand = page_list.begin();
    }

    Pagina* ChoosePageToEvict() {
        while (!page_list.empty()) {
            if (hand == page_list.end())
                hand = page_list.begin();

            Pagina* current_page = *hand;
            if (current_page->pin_count == 0) {
                ++hand;
                return current_page;
            }

            ++hand;
        }
        return nullptr;
    }
};

class BufferPoolManager {
private:
    int num_frames;
    EstrategiasReemplazo* replacer;
    unordered_map<int, Pagina*> page_table;
    GestorAlmacenamiento gestor;

public:
    BufferPoolManager(int num_frames, const string& archivo) : num_frames(num_frames), gestor(archivo) {
        replacer = new LRU();  //usa LRU como estrategia de reemplazo
    }

    ~BufferPoolManager() {
        for (auto& pair : page_table) {
            delete pair.second;
        }
        delete replacer;
    }

    Pagina* FetchPage(int page_id) {
        if (page_table.count(page_id) > 0) {
            Pagina* page = page_table[page_id];
            replacer->RemovePage(page);
            replacer->AddPage(page);
            return page;
        }

        if (page_table.size() >= num_frames) {
            Pagina* evict_page = replacer->ChoosePageToEvict();
            if (evict_page != nullptr) {
                page_table.erase(evict_page->page_id);
                if (evict_page->dirty) {
                    // Escribir la página en disco
                    char buffer[PAGE_SIZE];
                    // Obtener el contenido de la página desde el objeto Pagina
                    memcpy(buffer, evict_page, PAGE_SIZE);
                    gestor.escribirBloque(evict_page->page_id, buffer);
                }
                delete evict_page;
            }
        }

        char buffer[PAGE_SIZE];
        gestor.leerBloque(page_id, buffer);
        Pagina* page = new Pagina(page_id);
        memcpy(page, buffer, PAGE_SIZE);
        page_table[page_id] = page;
        replacer->AddPage(page);
        return page;
    }

    void UnpinPage(int page_id, bool is_dirty) {
        if (page_table.count(page_id) > 0) {
            Pagina* page = page_table[page_id];
            page->pin_count--;

            if (page->pin_count == 0) {
                replacer->AddPage(page);
            }

            if (is_dirty) {
                page->dirty = true;
            }
        }
    }

    bool FlushPage(int page_id) {
        if (page_table.count(page_id) > 0) {
            Pagina* page = page_table[page_id];
            char buffer[PAGE_SIZE];
            memcpy(buffer, page, PAGE_SIZE);
            gestor.escribirBloque(page_id, buffer);
            return true;
        }
        return false;
    }

    void FlushAllPages() {
        for (auto& pair : page_table) {
            Pagina* page = pair.second;
            if (page->dirty) {
                char buffer[PAGE_SIZE];
                memcpy(buffer, page, PAGE_SIZE);
                gestor.escribirBloque(page->page_id, buffer);
                page->dirty = false;
            }
        }
    }

    bool DeletePage(int page_id) {
        if (page_table.count(page_id) > 0) {
            Pagina* page = page_table[page_id];
            if (page->pin_count > 0) {
                cout << "La página está fijada y no se puede eliminar." << endl;
                return false;
            }

            page_table.erase(page_id);
            replacer->RemovePage(page);
            delete page;
            gestor.eliminarBloque(page_id);

            return true;
        }
        return false;
    }
};

class RegistroLongitudVariable {
private:
    string data;

public:
    RegistroLongitudVariable(const string& datos) : data(datos) {}

    int getTamanio() const {
        return static_cast<int>(data.length()) + sizeof(int);
    }

    const string& getDatos() const {
        return data;
    }
};

class RegistroLongitudFija {
private:
    char data[MAX_RECORD_LENGTH];

public:
    RegistroLongitudFija(const string& datos) {
        strncpy(data, datos.c_str(), MAX_RECORD_LENGTH);
    }

    const char* getDatos() const {
        return data;
    }
};

vector<string> dividir(const string& linea, char delimitador) {
    vector<string> campos;
    istringstream iss(linea);
    string campo;
    while (getline(iss, campo, delimitador)) {
        campos.push_back(campo);
    }
    return campos;
}

string generarPaginaLongitudVariable(const vector<RegistroLongitudVariable>& registros) {
    string pagina;
    int tamanioPagina = PAGE_SIZE;

    for (const auto& registro : registros) {
        int tamanioRegistro = registro.getTamanio();

        if (tamanioPagina - tamanioRegistro < sizeof(int)) {
            pagina.append(sizeof(int), '\0');  // Espacio para el tamaño de la siguiente página
            tamanioPagina = PAGE_SIZE;
        }

        pagina.append(reinterpret_cast<const char*>(&tamanioRegistro), sizeof(int));
        pagina.append(registro.getDatos());

        tamanioPagina -= tamanioRegistro;
    }

    return pagina;
}

string generarPaginaLongitudFija(const vector<RegistroLongitudFija>& registros) {
    string pagina;
    int tamanioPagina = PAGE_SIZE;

    for (const auto& registro : registros) {
        int tamanioRegistro = sizeof(RegistroLongitudFija);

        if (tamanioPagina - tamanioRegistro < sizeof(int)) {
            pagina.append(sizeof(int), '\0');  // Espacio para el tamaño de la siguiente página
            tamanioPagina = PAGE_SIZE;
        }

        pagina.append(reinterpret_cast<const char*>(&registro), sizeof(RegistroLongitudFija));

        tamanioPagina -= tamanioRegistro;
    }

    return pagina;
}

void escribirDatosEnArchivo(const string& nombreArchivo, const string& datos) {
    ofstream archivo(nombreArchivo);
    if (archivo.is_open()) {
        archivo << datos;
        archivo.close();
        cout << "Archivo generado: " << nombreArchivo << endl;
    } else {
        cout << "Error al abrir el archivo: " << nombreArchivo << endl;
    }
}

int main() {
    GestorAlmacenamiento gestor("archivo.txt");
    char buffer[PAGE_SIZE];

    gestor.leerBloque(0, buffer);
    cout << "Contenido del primer bloque: " << buffer << endl;

    gestor.leerBloque(1, buffer);
    cout << "Contenido del segundo bloque: " << buffer << endl;

    BufferPoolManager buffer_pool(10, "archivo.txt");

    Pagina* p1 = buffer_pool.FetchPage(1);
    if (p1 != nullptr) {
        buffer_pool.UnpinPage(1, true);
    }

    Pagina* p2 = buffer_pool.FetchPage(2);
    if (p2 != nullptr) {
        buffer_pool.UnpinPage(2, false);
    }

    buffer_pool.FlushAllPages();
    buffer_pool.DeletePage(1);

    ifstream archivoCSV("titanic.csv");
    string linea;

    vector<RegistroLongitudVariable> registrosLongitudVariable;
    vector<RegistroLongitudFija> registrosLongitudFija;

    getline(archivoCSV, linea); 

    while (getline(archivoCSV, linea)) {
        vector<string> campos = dividir(linea, ',');

        // Para registros de longitud variable
        RegistroLongitudVariable registroLongitudVariable(campos[0]); // Suponiendo que el primer campo es el dato relevante
        registrosLongitudVariable.push_back(registroLongitudVariable);

        // Para registros de longitud fija
        RegistroLongitudFija registroLongitudFija(campos[0]); // Suponiendo que el primer campo es el dato relevante
        registrosLongitudFija.push_back(registroLongitudFija);
    }

    archivoCSV.close();
    string paginaLongitudFija = generarPaginaLongitudFija(registrosLongitudFija);
    escribirDatosEnArchivo("Longfija.txt", paginaLongitudFija);

    string paginaLongitudVariable = generarPaginaLongitudVariable(registrosLongitudVariable);
    escribirDatosEnArchivo("Longvariable.txt", paginaLongitudVariable);


    return 0;
}
