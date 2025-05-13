#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <cstring>

using namespace std;

void helpfunction(){
    cout << endl << "Algoritmo das Componentes Fortemente Conexas: ./kosaraju -f <arquivo de entrada> [-o <arquivo de saida>] [-h]" << endl;
    cout << "-h\t\t : mostra o help" << endl;
    cout << "-o <arquivo>\t : redireciona a saida para o arquivo informado" << endl;
    cout << "-f <arquivo>\t : indica o ''arquivo'' que contem o grafo de entrada" << endl;
    cout << "Arquivo de entrada deve ter numero de vertices e arcos, e a lista de adjacencia" << endl;
}

void ordenar(vector<int>& componente){ // precisa de um sort para os resultados coincidirem com os do bat1
    int n = componente.size();
    for (int i = 0; i < n - 1; ++i) for (int j = 0; j < n - i - 1; ++j) if (componente[j] > componente[j + 1]) swap(componente[j], componente[j + 1]);
}
void ordenartudo(vector<vector<int>>& componentes) {
    int n = componentes.size();
    for (int i = 0; i < n - 1; ++i) for (int j = 0; j < n - i - 1; ++j) if (componentes[j][0] > componentes[j + 1][0]) swap(componentes[j], componentes[j + 1]);
}

void dfs1(int adj, vector<vector<int>>& graph, vector<bool>& visitados, stack<int>& pilha){
    visitados[adj] = true;
    for (int v : graph[adj]) if (!visitados[v]) dfs1(v, graph, visitados, pilha);
    pilha.push(adj);
}
void dfs2(int adj, vector<vector<int>>& inverso, vector<bool>& visitados, vector<int>& componente){
    visitados[adj] = true;
    componente.push_back(adj);
    for (int v : inverso[adj]) if (!visitados[v]) dfs2(v, inverso, visitados, componente);
}

int main(int argc, char *argv[]){
    string inputfile;
    string outputfile;
    for (int i = 1; i < argc; ++i){
        if (strcmp(argv[i], "-h") == 0){
            helpfunction();
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) inputfile = argv[++i];
        else if (strcmp(argv[i],"-o") == 0 && i + 1 < argc) outputfile = argv[++i];
    }

    if (inputfile.empty()){
        cerr << "Arquivo de entrada ausente, confira usando ./kosaraju -h" << endl;
        return 1;
    }
    ifstream input(inputfile);
    if (!input.is_open()){
        cerr << "Erro ao abrir o arquivo de entrada" << endl;
        return 1;
    }

    int n, m, vertice, vizinho; // n, m = quantidade de arestas e vertices.
    input >> n >> m;

    vector<vector<int>> grafo(n); // precisa dele
    vector<vector<int>> inverso(n); // e do transposto dele

    for (int i = 0; i < m; ++i){
        input >> vertice >> vizinho;
        grafo[vertice - 1].push_back(vizinho - 1);
        inverso[vizinho - 1].push_back(vertice - 1);
    }
    input.close();

    stack<int> pilha;
    vector<bool> visitados(n, false); // para inicializar como false
    for (int i = 0; i < n; i++) if (!visitados[i]) dfs1(i, grafo, visitados, pilha); // dfs1 empilha vertices que finalizar primeiro
    for (int i = 0; i < n; i++) visitados[i] = false; // reseta visitados para usar no dfs2
    vector<vector<int>> componentes;
    while(!pilha.empty()){ // aqui faz o dfs2 ate esvaziar a pilha
        int vizinho = pilha.top();
        pilha.pop();
        if (!visitados[vizinho]){
            vector<int> componente;
            dfs2(vizinho, inverso, visitados, componente);
            ordenar(componente); // ordena em ordem crescente
            componentes.push_back(componente);
        }
    }

    ordenartudo(componentes);

    if (outputfile.empty()){
        for (vector<int> componente : componentes){
            for (int v : componente) cout << v + 1 << " ";
            cout << endl;
        }
    }
    else{
        ofstream output(outputfile);
        if(!output.is_open()){
            cerr << "Erro ao abrir o arquivo de saida" << endl;
            return 1;
        }
        for (vector<int> componente : componentes){
            for (int v : componente) output << v + 1 << " ";
            output << endl;
        }
        output.close();
    }

    return 0;
}