#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

struct Registro {
    int id;
    int edad;
    string nombre;
    string apellido;
    string cui;
    // Otros campos que puedas tener en tus registros
};

Registro getRegistroPorID(const string& nombreArchivo, int targetID) {
    Registro registroEncontrado;
    
    ifstream file(nombreArchivo);
    if (!file) {
        cerr << "Error al abrir el archivo." << std::endl;
        return registroEncontrado;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        Registro registro;
        if (iss >> registro.id >> registro.edad >>registro.nombre >> registro.apellido >> registro.cui) {
            if (registro.id == targetID) {
                registroEncontrado = registro;
                break;
            }
        }
    }

    return registroEncontrado;
}

vector<int> getValueIDS(const string& nombreArchivo) {
    vector<int> ids;
    
    ifstream file(nombreArchivo);
    if (!file) {
        cerr << "Error al abrir el archivo." << endl;
        return ids;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        Registro registro;
        if (iss >> registro.id >> registro.edad >>registro.nombre >> registro.apellido >> registro.cui) {
            ids.push_back(registro.id);
        }
    }

    return ids;
}
    
void print_vectorIDS(vector <int>& vect) {
    cout << "[";
    for (size_t i = 0; i < vect.size(); i++) {
        cout << vect[i];
        if (i != vect.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    string nombreArchivo = "testregistro.txt";
    vector<int> ids = getValueIDS(nombreArchivo);

    cout << "IDs en el archivo:" << endl;
    print_vectorIDS(ids);
    //string nombreArchivo = "testregistro.txt";
    
    int idBuscado = 5;
    Registro registro = getRegistroPorID(nombreArchivo, idBuscado);

    if (registro.id != 0) {
        cout << "Registro encontrado:" << endl;
        cout << "ID: " << registro.id << endl;
        cout << "edad: " << registro.edad << endl;
        cout << "Nombre: " << registro.nombre << endl;
        cout << "Apellido: " << registro.apellido << endl;
        cout << "CUI: " << registro.cui << endl;
    } else {
        cout << "Registro no encontrado." << endl;
    }
    return 0;
}