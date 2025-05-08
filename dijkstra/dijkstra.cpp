#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <tuple>
#include <cstring>

#define infinito 1e9 // 1 * 10^9

using namespace std;

void helpfunction(){
    cout << endl << "Algoritmo de Caminhos Minimos: .\\dijkstra -f <arquivo de entrada> [-o <arquivo de saida>] [-i <vertice inicial>] [-h]" << endl << endl;
    cout << "-h\t\t : mostra o help" << endl;
    cout << "-o <arquivo>\t : redireciona a saida para o ''arquivo''" << endl;
    cout << "-f <arquivo>\t : indica o ''arquivo'' que contem o grafo de entrada" << endl;
    cout << "-i <vertice>\t : indice do vertice inicial" << endl << endl;
}

void dijkstra(vector<vector<tuple<double, int>>> &adj, int s, ostream& saida){
    vector<double> dist(adj.size());
    vector<int> pre(adj.size());
    for (unsigned int i = 0; i < adj.size(); ++i){
        dist[i] = infinito;
        pre[i] = -1;
    }
    dist[s] = 0.0; // distancia de s ate a eh 0 (0.0 porque eh double)
    priority_queue<tuple<double, int>, vector<tuple<double, int>>, greater<tuple<double, int>>> H;
    H.push({0.0, s}); // coloca alguem na heap para o algoritmo funcionar
    while (!H.empty()){
        tuple<double, int> atual = H.top(); // pega o vertice do topo da heap
        H.pop(); // remove o vertice da heap
        //separa os valores para facilitar o acesso depois
        double distancia = get<0>(atual);
        int vertice = get<1>(atual);
        if (distancia < dist[vertice]) continue; // se ja achou um caminho menor, ignora
        for (tuple<double, int> vizinho : adj[vertice]){  // vai iterar sobre todos os vizinho dos vertice
            double peso = get<0>(vizinho);
            int indicev = get<1>(vizinho);
            if (dist[indicev] > dist[vertice] + peso){
                dist[indicev] = dist[vertice] + peso; // atualiza a distancia
                pre[indicev] = vertice; // diz por onde veio ("quem eh pai daquele vertice")
                H.push({dist[indicev], indicev}); // coloca um novo elemento na heap já se preocupando com a prioridade
            }
        }
    }

    for (unsigned int i = 0; i < dist.size(); ++i) saida << i + 1 << ":" << dist[i] << " ";
}

int main(int argc, char *argv[]){
    string inputfile;
    string outputfile;
    int indexstart = 0; // vertice 1 por padrao (indice 0 no vetor)
    for (int i = 1; i < argc; ++i){
        if (strcmp(argv[i], "-h") == 0){ // se encontrar -h, executa helpfunction
            helpfunction();
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) inputfile = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) outputfile = argv[++i];
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) indexstart = atoi(argv[++i]);
    }

    if (inputfile.empty()){
        cerr << "Arquivo de entrada ausente, confira usando .\\dijkstra -h" << endl;
        return 1;
    }
    ifstream input(inputfile);
    if (!input.is_open()){
        cerr << "Erro ao abrir o arquivo de entrada" << endl;
        return 1;
    }

    int n, m, vertice, vizinho; // n,m = quantidade de vertices e arestas; vertice,vizinho = uso futuro
    double peso; // peso da aresta entre vertice,vizinho
    input >> n >> m;
    vector<vector<tuple<double, int>>> adj(n); // criou um vetor de n vetores de tuplas inicialmente vazias
    //[[(), (), ()...],[(), (), ()...],[()...],[()...]...], na verdade as tuplas não existem, mas está reservado para tuplas

    for (int i = 0; i < m; ++i){ // vai guardar tudo em input
        input >> vertice >> vizinho >> peso;
        adj[vertice - 1].push_back(make_tuple(peso, vizinho - 1));
        adj[vizinho - 1].push_back(make_tuple(peso, vertice - 1));
    }
    input.close();

    if (outputfile.empty()) dijkstra(adj, indexstart, cout); // se n tiver arquivo de saida, bota no terminal
    else{ // se tiver, escreve tudo no output
        ofstream output(outputfile);
        if (!output.is_open()){
            cerr << "Erro ao abrir o arquivo de saida" << endl;
            return 1;
        }
        dijkstra(adj, indexstart, output);
        output.close();
    }

    return 0;
}