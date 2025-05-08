#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <tuple>
#include <cstring>

using namespace std;

void helpfunction(){
    cout << endl << "Algoritmo de Arvore Geradora Minima: .\\prim -f <arquivo de entrada> [-o <arquivo de saida>] [-i <vertice inicial>] [-h]" << endl;
    cout << "-h\t\t : mostra o help" << endl;
    cout << "-o <arquivo>\t : redireciona a saida para o arquivo informado" << endl;
    cout << "-f <arquivo>\t : indica o ''arquivo'' que contem o grafo de entrada" << endl;
    cout << "-s\t\t : mostra a solucao" << endl;
    cout << "-i <vertice>\t : indice do vertice inicial (de 0 ate n - 1)" << endl;
    cout << "No arquivo de entrada coloque os numeros dos vertices, vizinho, peso" << endl;
    cout << "No arquivo os vertices devem ser colocados como o seu numero (de 1 ate n)" << endl;
}

struct aresta{ // assim eh mais facil de tratar, porque ele percorre arestas ao inves de vertices
    int vertice, vizinho;
    double peso;
    bool operator<(const aresta& outro) const{
        return peso > outro.peso; // priority queue exige um operador pra comparacao, e a prioridade deve ser do menor peso
    }
};

int main(int argc, char *argv[]){
    string inputfile;
    string outputfile;
    int indexstart = 0, show = 0; // vertice 1 por padrao (indice 0 no vetor), show = 0: mostra apenas custo da arvore
    for (int i = 1; i < argc; ++i){
        if (strcmp(argv[i], "-h") == 0){ // se encontrar -h, executa helpfunction
            helpfunction();
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) inputfile = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) outputfile = argv[++i];
        else if (strcmp(argv[i], "-s") == 0) show = 1; // show = 1: mostra solucao pra arvore geradora minima
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) indexstart = atoi(argv[++i]);
    }

    if (inputfile.empty()){
        cerr << "Arquivo de entrada ausente, confira usando .\\prim -h" << endl;
        return 1;
    }
    ifstream input(inputfile);
    if (!input.is_open()){
        cerr << "Erro ao abrir o arquivo de entrada" << endl;
        return 1;
    }

    int n, m, vertice, vizinho; // n, m = quantidade de vertices e arestas; vertice, vizinho = uso futuro
    double peso;  // peso da aresta entre vertice - vizinho
    input >> n >> m;

    vector<vector<tuple<double, int>>> adj(n); // criou um vetor de n vetores de tuplas inicialmente vazias
    for (int i = 0; i < m; ++i){ // vai guardar tudo em input
        input >> vertice >> vizinho >> peso;
        adj[vertice - 1].push_back(make_tuple(peso, vizinho - 1)); // ...nunca eh direcionado
        adj[vizinho - 1].push_back(make_tuple(peso, vertice - 1)); // faz duas vezes porque...
    }
    input.close();

    vector<bool> visitados(n, false);
    visitados[indexstart] = true;
    vector<aresta> arvore;
    double custo = 0;
    priority_queue<aresta> fila;
    for (tuple<double, int> vizinho : adj[indexstart]) fila.push({indexstart, get<1>(vizinho), get<0>(vizinho)});
    while (!fila.empty()){
        aresta atual = fila.top();
        fila.pop();
        if (visitados[atual.vizinho]) continue; // se ja visitado, skippa
        visitados[atual.vizinho] = true; // se nao, marca que visitou agora
        custo += atual.peso;
        arvore.push_back(atual);

        for (tuple<double, int> vizinho : adj[atual.vizinho]) if (!visitados[get<1>(vizinho)]) fila.push({atual.vizinho, get<1>(vizinho), get<0>(vizinho)});
    }

    if (outputfile.empty()){ // se n tiver arquivo de saida, bota no terminal
        if (show == 1) for (aresta& galho : arvore) cout << "(" << galho.vertice + 1 << "," << galho.vizinho + 1 << ") ";
        else cout << custo << endl; // se nao for pra mostrar solucao, mostra so o custo
    }
    else{ // se tiver, escreve tudo no output
        ofstream output(outputfile);
        if (!output.is_open()){
            cerr << "Erro ao abrir o arquivo de saida" << endl;
            return 1;
        }
        if (show == 1) for (aresta& galho : arvore) output << "(" << galho.vertice + 1 << "," << galho.vizinho + 1 << ") ";
        else output << custo << endl; // se nao for pra mostrar solucao, mostra so o custo
        output.close();
    }

    return 0;
}
