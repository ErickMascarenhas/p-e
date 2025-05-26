#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxcompentradasseq 100
#define maxsequencias 50
#define maxcompalinhado 2000

#define alfa 1
#define beta 0
#define delta -2

typedef struct{
    char seq[maxcompentradasseq + 1];
    int comp;
} infoseq;

infoseq seqsiniciais[maxsequencias];
char resmsafinal[maxsequencias][maxcompalinhado + 1];

int max3(int a, int b, int c) {return (b > a) ? ((c > b) ? c : b) : ((c > a) ? c : a);}

int pontuarparcaracteres(char c1, char c2){
    if (c1 == '-' || c2 == '-') return delta;
    if (c1 == c2) return alfa;
    return beta;
}

void needlemanwunsch(char *seq1, char *seq2, char *resseq1, char *resseq2){
    int comp1 = strlen(seq1), comp2 = strlen(seq2);
    if (comp1 == 0 && comp2 == 0){
        resseq1[0] = '\0';
        resseq2[0] = '\0';
        return;
    }
    if (comp1 == 0){
        for(int j = 0; j < comp2; ++j) resseq1[j] = '-';
        resseq1[comp2] = '\0';
        strcpy(resseq2, seq2);
        return;
    }
    if (comp2 == 0){
        for(int i = 0; i < comp1; ++i) resseq2[i] = '-';
        resseq2[comp1] = '\0';
        strcpy(resseq1, seq1);
        return;
    }
    int **dp = (int **)malloc((comp1 + 1) * sizeof(int *));
    if (!dp){
        perror("falha ao alocar linhas da tabela dp"); 
        exit(EXIT_FAILURE); 
    }
    for (int i = 0; i <= comp1; i++){
        dp[i] = (int *)malloc((comp2 + 1) * sizeof(int));
        if (!dp[i]){
            for(int k_free = 0; k_free < i; ++k_free) free(dp[k_free]);
            free(dp);
            perror("falha ao alocar colunas da tabela dp"); 
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i <= comp1; i++) dp[i][0] = i * delta;
    for (int j = 0; j <= comp2; j++) dp[0][j] = j * delta;
    for (int i = 1; i <= comp1; i++){
        for (int j = 1; j <= comp2; j++){
            int scorediag = dp[i-1][j-1] + pontuarparcaracteres(seq1[i-1], seq2[j-1]), scorecima = dp[i-1][j] + delta, scoreesq = dp[i][j-1] + delta;
            dp[i][j] = max3(scorediag, scorecima, scoreesq);
        }
    }
    char tempalin1[maxcompalinhado + 1], tempalin2[maxcompalinhado + 1];
    int i_tb = comp1, j_tb = comp2, idxrastro = 0; 
    while (i_tb > 0 || j_tb > 0){
        if (idxrastro >= maxcompalinhado){
            fprintf(stderr, "erro: sequencia alinhada excede maxcompalinhado (%d) durante traceback nw.\n", maxcompalinhado);
            for (int r = 0; r <= comp1; r++) free(dp[r]);
            free(dp);
            exit(EXIT_FAILURE);
        }
        if (i_tb > 0 && j_tb > 0 && dp[i_tb][j_tb] == dp[i_tb-1][j_tb-1] + pontuarparcaracteres(seq1[i_tb-1], seq2[j_tb-1])){
            tempalin1[idxrastro] = seq1[i_tb-1];
            tempalin2[idxrastro] = seq2[j_tb-1];
            i_tb--;
            j_tb--;
        }
        else if (i_tb > 0 && dp[i_tb][j_tb] == dp[i_tb-1][j_tb] + delta){
            tempalin1[idxrastro] = seq1[i_tb-1];
            tempalin2[idxrastro] = '-';
            i_tb--;
        }
        else if (j_tb > 0 && dp[i_tb][j_tb] == dp[i_tb][j_tb-1] + delta){
            tempalin1[idxrastro] = '-';
            tempalin2[idxrastro] = seq2[j_tb-1];
            j_tb--;
        }
        else if (i_tb > 0){
            tempalin1[idxrastro] = seq1[i_tb-1];
            tempalin2[idxrastro] = '-';
            i_tb--;
        }
        else if (j_tb > 0){
            tempalin1[idxrastro] = '-';
            tempalin2[idxrastro] = seq2[j_tb-1];
            j_tb--;
        }
        else break;
        idxrastro++;
    }
    for (int l = 0; l < idxrastro; l++){
        resseq1[l] = tempalin1[idxrastro - 1 - l];
        resseq2[l] = tempalin2[idxrastro - 1 - l];
    }
    resseq1[idxrastro] = '\0';
    resseq2[idxrastro] = '\0';
    for (int r = 0; r <= comp1; r++) free(dp[r]);
    free(dp);
}

int compseqcompdesc(void *a, void *b){
    infoseq *sinfo1 = (infoseq *)a, *sinfo2 = (infoseq *)b;
    return sinfo2->comp - sinfo1->comp;
}

void execmsaprog(infoseq seqsinord[maxsequencias], int numtotseqs, char msafinal[maxsequencias][maxcompalinhado + 1], int *compfinalalin){
    if (numtotseqs == 0){
        *compfinalalin = 0;
        return;
    }
    strcpy(msafinal[0], seqsinord[0].seq);
    int numseqsmsa = 1;
    *compfinalalin = strlen(msafinal[0]);
    char bufrefalin[maxcompalinhado + 1], bufnovaseqalin[maxcompalinhado + 1], tempmsaarm[maxsequencias][maxcompalinhado + 1];
    for (int k = 1; k < numtotseqs; k++){
        char *seqadic = seqsinord[k].seq, perfilrefnw[maxcompalinhado + 1];
        strcpy(perfilrefnw, msafinal[0]);
        needlemanwunsch(perfilrefnw, seqadic, bufrefalin, bufnovaseqalin);
        int novocompitermsa = strlen(bufrefalin);
        if (novocompitermsa > maxcompalinhado){
            fprintf(stderr, "erro: comprimento do alinhamento %d excede maxcompalinhado %d no par %d.\n", novocompitermsa, maxcompalinhado, k);
            exit(EXIT_FAILURE);
        }
        for (int idxseqmsa = 0; idxseqmsa < numseqsmsa; idxseqmsa++){
            char *seqatualmsaold = msafinal[idxseqmsa], seqajustj[maxcompalinhado + 1];
            int lenseqatualmsaold = strlen(seqatualmsaold), lenperfilrefnw = strlen(perfilrefnw), idxpold = 0, idxsjold = 0;
            for (int idxlnovo = 0; idxlnovo < novocompitermsa; idxlnovo++){
                if (bufrefalin[idxlnovo] != '-'){
                    if (idxpold < lenperfilrefnw){
                        if (idxsjold < lenseqatualmsaold) seqajustj[idxlnovo] = seqatualmsaold[idxsjold++];
                        else seqajustj[idxlnovo] = '-';
                        idxpold++;
                    }
                    else seqajustj[idxlnovo] = '-';
                }
                else seqajustj[idxlnovo] = '-';
            }
            seqajustj[novocompitermsa] = '\0';
            strcpy(tempmsaarm[idxseqmsa], seqajustj);
        }
        strcpy(tempmsaarm[numseqsmsa], bufnovaseqalin);
        numseqsmsa++; 
        for(int i = 0; i < numseqsmsa; ++i) strcpy(msafinal[i], tempmsaarm[i]);
        *compfinalalin = novocompitermsa;
    }
    for(int i = 0; i < numseqsmsa; ++i){
        int len = strlen(msafinal[i]);
        if (len < *compfinalalin) {
            for(int idxpad = len; idxpad < *compfinalalin; ++idxpad) msafinal[i][idxpad] = '-';
            msafinal[i][*compfinalalin] = '\0';
        }
    }
}

int calcularscorevertical(char msa[][maxcompalinhado + 1], int numseqs, int compalin){
    int scoretotal = 0;
    if (numseqs < 2) return 0;
    for (int col = 0; col < compalin; col++){
        for (int linha = 0; linha < (numseqs - 1); linha++){
            char c1 = msa[linha][col], c2 = msa[linha + 1][col];
            if (c1 == '-' || c2 == '-') scoretotal += delta;
            else if (c1 == c2) scoretotal += alfa;
        }
    }
    return scoretotal;
}

int main() {
    FILE *arqent, *arqsai;
    int seqslidas = 0, compfinalalin = 0;
    arqent = fopen("input.txt", "r");
    if (arqent == NULL){
        perror("erro ao abrir o arquivo de entrada (input.txt)");
        return 1;
    }
    char buflinha[maxcompentradasseq + 3];
    while (seqslidas < maxsequencias && fgets(buflinha, sizeof(buflinha), arqent) != NULL){
        buflinha[strcspn(buflinha, "\r\n")] = 0; 
        if (strlen(buflinha) == 0 && feof(arqent) && seqslidas == 0) continue;
        if (strlen(buflinha) > maxcompentradasseq){
            fprintf(stderr, "aviso: sequencia %d (0-indexed) excede maxcompentradasseq (%d) e sera truncada.\n", seqslidas, maxcompentradasseq);
            strncpy(seqsiniciais[seqslidas].seq, buflinha, maxcompentradasseq);
            seqsiniciais[seqslidas].seq[maxcompentradasseq] = '\0';
        }
        else strcpy(seqsiniciais[seqslidas].seq, buflinha);
        seqsiniciais[seqslidas].comp = strlen(seqsiniciais[seqslidas].seq);
        if (seqsiniciais[seqslidas].comp > 0) seqslidas++;
    }
    fclose(arqent);
    if (seqslidas == 0){
        fprintf(stderr, "nenhuma sequencia valida lida do arquivo input.txt\n");
        return 1;
    }
    if (seqslidas < maxsequencias && seqslidas > 0) printf("aviso: foram lidas %d sequencias. o maximo configurado e %d.\n", seqslidas, maxsequencias);
    else if (seqslidas == maxsequencias) printf("%d sequencias lidas com sucesso de input.txt.\n", seqslidas);
    qsort(seqsiniciais, seqslidas, sizeof(infoseq), compseqcompdesc);
    execmsaprog(seqsiniciais, seqslidas, resmsafinal, &compfinalalin);
    arqsai = fopen("output.txt", "w");
    if (arqsai == NULL){
        perror("erro ao abrir o arquivo de saida (output.txt)");
        return 1;
    }
    fprintf(arqsai, "Alinhamento Multiplo de Sequencias Resultante:\n");
    for (int i = 0; i < seqslidas; i++) fprintf(arqsai, "%s\n", resmsafinal[i]);
    int scoreverticalfinal = 0;
    if (seqslidas > 0 && compfinalalin > 0){
        scoreverticalfinal = calcularscorevertical(resmsafinal, seqslidas, compfinalalin);
        fprintf(arqsai, "\nPontuacao (calculo sequencial vertical): %d\n", scoreverticalfinal);
    }
    else if (seqslidas > 0) fprintf(arqsai, "\nNao e possivel calcular a pontuacao vertical com o alinhamento atual.\n");
    else fprintf(arqsai, "\nNenhuma sequencia alinhada para calcular pontuacao.\n");
    fclose(arqsai);
    printf("processamento concluido. %d sequencias foram processadas.\nverifique 'output.txt' para os resultados.\n", seqslidas);
    if (seqslidas > 0 && compfinalalin > 0) printf("pontuacao (calculo sequencial vertical): %d\n", scoreverticalfinal);
    return 0;
}