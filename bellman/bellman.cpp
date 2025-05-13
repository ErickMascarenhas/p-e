#include <iostream>
#include <vector>
#include <tuple>
#include <fstream>
#include <cstring>

#define infinito 1e9 //1 * 10 ^ 9

using namespace std;

void helpfunction(){
    cout << endl << "Algoritmo de Caminhos Minimos (com pesos negativos): ./bellman -f <arquivo de entrada> [-o <arquivo de saida>] [-i <vertice inicial>] [-h]" << endl;
    cout << "-h\t\t : mostra o help" << endl;
    cout << "-o <arquivo>\t : redireciona a saida para o arquivo informado" << endl;
    cout << "-f <arquivo>\t : indica o ''arquivo'' que contem o grafo de entrada" << endl;
    cout << "-i <vertice>\t : indice do vertice inicial (de 0 ate n -1)" << endl;
    cout << "No arquivo de entrada coloque os numeros dos vertices, vizinho, peso" << endl;
    cout << "No arquivo os vertices devem ser colocados como o seu numero (de 1 ate n)" << endl;
    cout << "Esse algoritmo tratara o grafo no arquivo de entrada como direcionado, logo, o resultado pode diferir do Dijkstra" << endl;
}

void bellman(vector<vector<tuple<double, int>>> &adj, int s, ostream& saida){

    vector<double> dist(adj.size());

    for (unsigned int i = 0; i < dist.size(); ++i) dist[i] = infinito; //define tudo como "infinito"
    dist[s] = 0;

    for (unsigned int j = 1; j < dist.size(); ++j){
        for (unsigned int i = 0; i < dist.size(); ++i){
            for (tuple <double, int> vizinho : adj[i]){
                double peso = get<0>(vizinho);
                int indice_vizinho = get<1>(vizinho);
                if (dist[i] + peso < dist[indice_vizinho]) dist[indice_vizinho] = dist[i] + peso;
            }
        }
    }

    for (unsigned int i = 0; i < dist.size(); ++i){ //rodar tudo mais uma vez para ver se tem ciclo negativo
        for (tuple <double, int> vizinho :adj[i]){
            double peso = get<0>(vizinho);
            int indice_vizinho = get<1>(vizinho);

            if (dist[i] + peso < dist[indice_vizinho]){
                cout << "O grafo possui ciclo negativo, entao nao e possivel calcular o menor caminho entre os vertices" << endl;
                return;
            }
        }
    }

    for (unsigned int i = 0; i < dist.size(); ++i) saida << i + 1 << ":" << dist[i] << " ";
    saida << endl;
}


int main(int argc, char *argv[]){

    string inputfile;
    string outputfile;
    int indexstart = 0; //vertice 1 por padrao (indice 0 no vetor)

    for (int i = 1; i < argc; ++i){
        if (strcmp(argv[i], "-h") == 0) {
            helpfunction();
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) inputfile = argv[++i];
        else if (strcmp(argv[i],"-o") == 0 && i + 1 < argc) outputfile = argv[++i];
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) indexstart = atoi(argv[++i]);
    }


    if (inputfile.empty()){
        cerr << "Arquivo de entrada ausente, confira usando ./bellman -h" << endl;
        return 1;
    }
    
    ifstream input(inputfile);
    if (!input.is_open()){
        cerr << "Erro ao abrir o arquivo de entrada" << endl;
        return 1;
    }

    int n, m, vertice, vizinho; //n, m = quantidade de arestas e vertices.
    double peso; // peso entre vertice e vizinho
    input >> n >> m;

    vector<vector<tuple<double, int>>> adj(n);

    for (int i = 0; i < m; ++i){
        input >> vertice >> vizinho >> peso;
        adj[vertice - 1].push_back(make_tuple(peso, vizinho -1));
    }
    input.close();

    if (outputfile.empty()) bellman(adj, indexstart, cout); // se nao tiver arquivo de saida coloca no terminal
    else{ //se tiver arquivo de saida, escreve nele
        ofstream output(outputfile);
        if(!output.is_open()){
            cerr << "Erro ao abrir o arquivo de saida" << endl;
            return 1;
        }
        bellman(adj, indexstart, output);
        output.close();
    }

    return 0;
}