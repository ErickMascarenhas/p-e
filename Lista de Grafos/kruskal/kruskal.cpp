#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;

void helpfunction(){
    cout << endl << "Algoritmo de Arvore Geradora Minima: ./kruskal -f <arquivo de entrada> [-o <arquivo de saida>] [-s] [-h]" << endl;
    cout << "-h\t\t : mostra o help" << endl;
    cout << "-o <arquivo>\t : redireciona a saida para o arquivo informado" << endl;
    cout << "-f <arquivo>\t : indica o ''arquivo'' que contem o grafo de entrada" << endl;
    cout << "-s\t\t : mostra a solucao" << endl;
    cout << "No arquivo de entrada coloque os numeros dos vertices, vizinho, peso" << endl;
    cout << "No arquivo os vertices devem ser colocados como o seu numero (de 1 ate n)" << endl;
}

struct aresta{ // assim eh mais facil de tratar, porque ele percorre arestas ao inves de vertices
    int vertice, vizinho;
    double peso;
};

// fiz com bubblesort, mas se quiser pode mudar a vontade
void ordenar(vector<aresta>& arestas, int esquerda, int direita){
    
    if(esquerda >= direita) return;

    int i = esquerda, j = esquerda -1;
    double pivo = arestas[direita].peso;

    while (i < direita){
        if (arestas[i].peso < pivo){
            j++;
            swap(arestas[i], arestas[j]);
        }
        i++;
    }

    j++;
    arestas[i] = arestas[j];
    arestas[j].peso = pivo;

    ordenar(arestas, esquerda, j - 1);
    ordenar(arestas, j + 1, direita);
}

int raiz(int vertice, vector<int>& pais){ // pegar o ancestral de um descendente proprio
    if (vertice != pais[vertice]) pais[vertice] = raiz(pais[vertice], pais);
    return pais[vertice];
}

void unir(int vertice, int vizinho, vector<int>& pais, vector<int>& profundidades){
    vertice = raiz(vertice, pais);
    vizinho = raiz(vizinho, pais);
    if (vertice == vizinho) return; // se ja estao no mesmo conjunto, nao une, pra nao formar ciclo
    else if (profundidades[vizinho] < profundidades[vertice]) pais[vizinho] = vertice;
    else{
        pais[vertice] = vizinho;
        if (profundidades[vizinho] == profundidades[vertice]) profundidades[vizinho]++;
    }
}

int main(int argc, char *argv[]){
    string inputfile;
    string outputfile;
    int show = 0; // show = 0: mostra apenas custo da arvore

    for (int i = 1; i < argc; ++i){
        if (strcmp(argv[i], "-h") == 0){ // se encontrar -h, executa helpfunction
            helpfunction();
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) inputfile = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) outputfile = argv[++i];
        else if (strcmp(argv[i], "-s") == 0) show = 1; // show = 1: mostra solucao pra arvore geradora minima
    }

    if (inputfile.empty()){
        cerr << "Arquivo de entrada ausente, confira usando ./kruskal -h" << endl;
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
    vector<aresta> arestas; // arestas guardam vertice - vizinho - peso
    vector<int> pais(n), profundidades(n, 0); // cria vetor de pais e profundidades e inicializa em -1 e 0
    for (int i = 1; i <= n; ++i) pais[i] = i; // inicializa cada um como pai de si mesmo (1 a n)

    for (int i = 0; i < m; ++i){ // vai guardar tudo em input
        input >> vertice >> vizinho >> peso;
        arestas.push_back({vertice - 1, vizinho - 1, peso}); // faz duas vezes porque...
        arestas.push_back({vizinho - 1, vertice - 1, peso}); // ...nunca eh direcionado
    }
    input.close();

    ordenar(arestas, 0, arestas.size() - 1); // por causa desse ordenar, o algoritmo pode demorar pra rodar, mas ele eh necessario
    double custo = 0; // custo total da arvore geradora minima comeca em 0
    vector<aresta> arvore; // arvore geradora minima eh um vetor de arestas
    for (aresta& galho : arestas){
        if (raiz(galho.vizinho, pais) != raiz(galho.vertice, pais)){
            unir(galho.vizinho, galho.vertice, pais, profundidades);
            arvore.push_back(galho);
            custo += galho.peso;
        }
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
        if (show == 1) for (aresta& galho : arvore) output << "(" << galho.vertice + 1 << "," << galho.vizinho + 1 << ") " << endl;
        else output << custo << endl; // se nao for pra mostrar solucao, mostra so o custo
        output.close();
    }

    return 0;
}
