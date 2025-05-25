#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // Para INT_MIN

// --- Constantes Configuráveis ---
#define MAX_SEQ_INPUT 100       // Comprimento máximo de uma sequência individual de entrada
#define NUM_SEQUENCES 50    // Número de sequências a alinhar
#define MAX_ALIGNED_LEN 2000 // Comprimento máximo estimado de uma sequência alinhada.
                             // Ajuste se necessário para dados muito divergentes.

// --- Scores para Needleman-Wunsch ---
#define MATCH_SCORE 1
#define MISMATCH_SCORE 0
#define GAP_PENALTY -2

// --- Estrutura para Informações da Sequência ---
typedef struct {
    char seq[MAX_SEQ_INPUT + 1];
    int len;
} SequenceInfo;

// --- Protótipos de Função ---
int max3(int a, int b, int c);
void needleman_wunsch(const char *seq1, const char *seq2, char *result_seq1, char *result_seq2);
int compare_sequences_by_length_desc(const void *a, const void *b);
void perform_progressive_msa(SequenceInfo sorted_initial_sequences[NUM_SEQUENCES],
                             int num_total_seqs,
                             char final_msa[NUM_SEQUENCES][MAX_ALIGNED_LEN + 1]);

// --- Implementação das Funções ---

int max3(int a, int b, int c) {
    int max_val = a;
    if (b > max_val) max_val = b;
    if (c > max_val) max_val = c;
    return max_val;
}

void needleman_wunsch(const char *seq1, const char *seq2, char *result_seq1, char *result_seq2) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);

    if (len1 == 0 && len2 == 0) {
        result_seq1[0] = '\0';
        result_seq2[0] = '\0';
        return;
    }
     if (len1 == 0) {
        result_seq1[0] = '\0'; // Empty string remains empty
        for(int j=0; j<len2; ++j) result_seq1[j] = '-'; // Fill seq1 with gaps
        result_seq1[len2] = '\0';
        strcpy(result_seq2, seq2);
        return;
    }
    if (len2 == 0) {
        result_seq2[0] = '\0'; // Empty string remains empty
        for(int i=0; i<len1; ++i) result_seq2[i] = '-'; // Fill seq2 with gaps
        result_seq2[len1] = '\0';
        strcpy(result_seq1, seq1);
        return;
    }


    int **dp = (int **)malloc((len1 + 1) * sizeof(int *));
    if (!dp) { perror("Falha ao alocar linhas da tabela DP"); exit(EXIT_FAILURE); }
    for (int i = 0; i <= len1; i++) {
        dp[i] = (int *)malloc((len2 + 1) * sizeof(int));
        if (!dp[i]) {
            for(int k=0; k<i; ++k) free(dp[k]);
            free(dp);
            perror("Falha ao alocar colunas da tabela DP"); exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i <= len1; i++) dp[i][0] = i * GAP_PENALTY;
    for (int j = 0; j <= len2; j++) dp[0][j] = j * GAP_PENALTY;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int score_diag = dp[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
            int score_up   = dp[i-1][j] + GAP_PENALTY;
            int score_left = dp[i][j-1] + GAP_PENALTY;
            dp[i][j] = max3(score_diag, score_up, score_left);
        }
    }

    char temp_aligned1[MAX_ALIGNED_LEN + 1];
    char temp_aligned2[MAX_ALIGNED_LEN + 1];
    int i = len1, j = len2;
    int k_trace = 0; 

    while (i > 0 || j > 0) {
        if (k_trace >= MAX_ALIGNED_LEN) {
            fprintf(stderr, "Erro: Sequencia alinhada excede MAX_ALIGNED_LEN (%d) durante traceback NW.\n", MAX_ALIGNED_LEN);
            for (int r = 0; r <= len1; r++) free(dp[r]);
            free(dp);
            exit(EXIT_FAILURE);
        }
        if (i > 0 && j > 0 && dp[i][j] == dp[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE)) {
            temp_aligned1[k_trace] = seq1[i-1];
            temp_aligned2[k_trace] = seq2[j-1];
            i--; j--;
        } else if (i > 0 && dp[i][j] == dp[i-1][j] + GAP_PENALTY) {
            temp_aligned1[k_trace] = seq1[i-1];
            temp_aligned2[k_trace] = '-';
            i--;
        } else if (j > 0 && dp[i][j] == dp[i][j-1] + GAP_PENALTY) { // Prioritize this if scores are equal
            temp_aligned1[k_trace] = '-';
            temp_aligned2[k_trace] = seq2[j-1];
            j--;
        } else if (i > 0) { // Only gaps in seq2 left
             temp_aligned1[k_trace] = seq1[i-1];
             temp_aligned2[k_trace] = '-';
             i--;
        } else { // Only gaps in seq1 left (j > 0)
             temp_aligned1[k_trace] = '-';
             temp_aligned2[k_trace] = seq2[j-1];
             j--;
        }
        k_trace++;
    }
    
    for (int l = 0; l < k_trace; l++) {
        result_seq1[l] = temp_aligned1[k_trace - 1 - l];
        result_seq2[l] = temp_aligned2[k_trace - 1 - l];
    }
    result_seq1[k_trace] = '\0';
    result_seq2[k_trace] = '\0';

    for (int r = 0; r <= len1; r++) free(dp[r]);
    free(dp);
}

int compare_sequences_by_length_desc(const void *a, const void *b) {
    const SequenceInfo *s1 = (const SequenceInfo *)a;
    const SequenceInfo *s2 = (const SequenceInfo *)b;
    return s2->len - s1->len;
}

void perform_progressive_msa(SequenceInfo sorted_initial_sequences[NUM_SEQUENCES],
                             int num_total_seqs,
                             char final_msa[NUM_SEQUENCES][MAX_ALIGNED_LEN + 1]) {
    if (num_total_seqs == 0) return;

    strcpy(final_msa[0], sorted_initial_sequences[0].seq);
    int num_seqs_in_msa = 1;
    int current_msa_overall_length = strlen(final_msa[0]);

    char aligned_ref_buffer[MAX_ALIGNED_LEN + 1];
    char aligned_new_seq_buffer[MAX_ALIGNED_LEN + 1];
    char temp_msa_storage[NUM_SEQUENCES][MAX_ALIGNED_LEN + 1]; // Para construir o próximo estado do MSA

    for (int k = 1; k < num_total_seqs; k++) {
        char *sequence_to_add = sorted_initial_sequences[k].seq;
        char profile_representative_for_nw[MAX_ALIGNED_LEN + 1];
        strcpy(profile_representative_for_nw, final_msa[0]); // P_old (cópia da primeira seq do MSA atual)

        needleman_wunsch(profile_representative_for_nw, sequence_to_add,
                         aligned_ref_buffer, aligned_new_seq_buffer);

        int new_iteration_msa_length = strlen(aligned_ref_buffer);
        if (new_iteration_msa_length > MAX_ALIGNED_LEN) {
            fprintf(stderr, "Erro: Comprimento do alinhamento %d excede MAX_ALIGNED_LEN %d no par %d.\n", new_iteration_msa_length, MAX_ALIGNED_LEN, k);
            exit(EXIT_FAILURE);
        }
        
        // Ajustar todas as sequências já no MSA (índices 0 a num_seqs_in_msa - 1)
        for (int msa_s_idx = 0; msa_s_idx < num_seqs_in_msa; msa_s_idx++) {
            char *current_seq_in_msa_old = final_msa[msa_s_idx]; // Sj_old
            char adjusted_s_j[MAX_ALIGNED_LEN + 1];
            
            int ptr_P_old = 0;  // Ponteiro para profile_representative_for_nw (P_old)
            int ptr_Sj_old = 0; // Ponteiro para current_seq_in_msa_old (Sj_old)

            for (int l_new = 0; l_new < new_iteration_msa_length; l_new++) {
                // aligned_ref_buffer é P'
                if (aligned_ref_buffer[l_new] != '-') {
                    // P'[l_new] é um caractere. Ele veio de P_old[ptr_P_old].
                    // Então, Sj_new pega seu caractere de Sj_old[ptr_Sj_old].
                    if (ptr_Sj_old < strlen(current_seq_in_msa_old)) {
                         adjusted_s_j[l_new] = current_seq_in_msa_old[ptr_Sj_old++];
                    } else { // Sj_old terminou, mas P_old (e P') continua; preencher Sj_new com gaps
                         adjusted_s_j[l_new] = '-';
                    }
                    if (ptr_P_old < strlen(profile_representative_for_nw)) { // Avança ptr_P_old se não ultrapassar
                        ptr_P_old++;
                    }
                } else {
                    // P'[l_new] é um gap ('-'). Isso significa que S_k (sequence_to_add) inseriu um caractere aqui.
                    // Portanto, Sj_new (e todas as outras sequências no MSA atual) também recebem um gap.
                    adjusted_s_j[l_new] = '-';
                    // ptr_P_old NÃO avança, pois o caractere em P_old[ptr_P_old] foi "empurrado".
                    // ptr_Sj_old NÃO avança pelo mesmo motivo.
                }
            }
            adjusted_s_j[new_iteration_msa_length] = '\0';
            strcpy(temp_msa_storage[msa_s_idx], adjusted_s_j);
        }

        // Adicionar a nova sequência alinhada (aligned_new_seq_buffer) ao armazenamento temporário
        strcpy(temp_msa_storage[num_seqs_in_msa], aligned_new_seq_buffer);

        // Atualizar o final_msa com o conteúdo do temp_msa_storage
        num_seqs_in_msa++; // Agora inclui a nova sequência
        for(int i=0; i < num_seqs_in_msa; ++i) {
            strcpy(final_msa[i], temp_msa_storage[i]);
        }
        current_msa_overall_length = new_iteration_msa_length;
    }

    // Garantir que todas as strings no MSA final tenham o mesmo comprimento (preenchendo com gaps finais se necessário)
    for(int i=0; i<num_seqs_in_msa; ++i) {
        int len = strlen(final_msa[i]);
        if (len < current_msa_overall_length) {
            for(int l_pad = len; l_pad < current_msa_overall_length; ++l_pad) {
                final_msa[i][l_pad] = '-';
            }
            final_msa[i][current_msa_overall_length] = '\0';
        }
    }
}

int main() {
    static SequenceInfo initial_sequences[NUM_SEQUENCES]; // Usar static para arrays grandes
    static char final_msa_result[NUM_SEQUENCES][MAX_ALIGNED_LEN + 1];
    FILE *inputFile, *outputFile;
    int sequences_read = 0;

    inputFile = fopen("input.txt", "r");
    if (inputFile == NULL) {
        perror("Erro ao abrir o arquivo de entrada (input.txt)");
        return 1;
    }

    char line_buffer[MAX_SEQ_INPUT + 3]; // +2 para \r\n, +1 para \0
    while (sequences_read < NUM_SEQUENCES && fgets(line_buffer, sizeof(line_buffer), inputFile) != NULL) {
        line_buffer[strcspn(line_buffer, "\r\n")] = 0; 
        if (strlen(line_buffer) == 0 && feof(inputFile) && sequences_read == 0){
            // Arquivo pode estar vazio ou conter apenas linhas vazias
            continue;
        }
        if (strlen(line_buffer) > MAX_SEQ_INPUT) {
            fprintf(stderr, "Aviso: Sequencia %d (0-indexed) excede MAX_SEQ_INPUT (%d) e sera truncada.\n", sequences_read, MAX_SEQ_INPUT);
            strncpy(initial_sequences[sequences_read].seq, line_buffer, MAX_SEQ_INPUT);
            initial_sequences[sequences_read].seq[MAX_SEQ_INPUT] = '\0';
        } else {
            strcpy(initial_sequences[sequences_read].seq, line_buffer);
        }
        initial_sequences[sequences_read].len = strlen(initial_sequences[sequences_read].seq);
        if (initial_sequences[sequences_read].len > 0) { // Somente adiciona se não for uma linha completamente vazia após remover \n
            sequences_read++;
        }
    }
    fclose(inputFile);

    if (sequences_read == 0) {
        fprintf(stderr, "Nenhuma sequencia valida lida do arquivo input.txt\n");
        return 1;
    }
    if (sequences_read < NUM_SEQUENCES) {
        printf("Aviso: Foram lidas %d sequencias, menos que as %d esperadas/solicitadas.\n", sequences_read, NUM_SEQUENCES);
        // Continua o processamento com as sequências lidas
    }
    
    qsort(initial_sequences, sequences_read, sizeof(SequenceInfo), compare_sequences_by_length_desc);

    perform_progressive_msa(initial_sequences, sequences_read, final_msa_result);

    outputFile = fopen("output.txt", "w");
    if (outputFile == NULL) {
        perror("Erro ao abrir o arquivo de saida (output.txt)");
        return 1;
    }

    fprintf(outputFile, "Alinhamento Multiplo de Sequencias Resultante:\n");
    int final_alignment_length = 0;
    if (sequences_read > 0 && strlen(final_msa_result[0]) > 0) { 
        final_alignment_length = strlen(final_msa_result[0]);
    }

    for (int i = 0; i < sequences_read; i++) {
        if (strlen(final_msa_result[i]) != final_alignment_length && final_alignment_length > 0) {
             // Isso é uma verificação de segurança, a lógica em perform_progressive_msa deve garantir isso
             fprintf(stderr, "Alerta: Comprimento inconsistente para a sequencia %d. Esperado %d, obtido %zu.\n", i, final_alignment_length, strlen(final_msa_result[i]));
             // Tentar padronizar, mas idealmente não deveria acontecer
             fprintf(outputFile, "%-*.*s\n", final_alignment_length, final_alignment_length, final_msa_result[i]);
        } else {
            fprintf(outputFile, "%s\n", final_msa_result[i]);
        }
    }
    fclose(outputFile);

    printf("Processamento concluido. %d sequencias foram processadas.\n", sequences_read);
    printf("Verifique 'output.txt' para os resultados.\n");
    
    // Usar 'static' para alocar estes arrays grandes fora da pilha da função main,
    // evitando estouro de pilha (stack overflow) se MAX_INPUT_LINES for grande.
    static char sequences[NUM_SEQUENCES][MAX_SEQ_INPUT + 1]; // +1 para o caractere nulo '\0'
    int lines_read = 0;
    long long total_score = 0;
    // Buffer para leitura de linha: MAX_SEQ_INPUT para conteúdo + 1 para \n (ou \r) + 1 para \r (ou \n) + 1 para \0
    char line_buffer2[MAX_SEQ_INPUT + 3];

    FILE *file_ptr = fopen("output.txt", "r");
    if (file_ptr == NULL) {
        // Se o arquivo não puder ser aberto, o score é 0, pois nenhuma operação é realizada.
        // O problema pede apenas o score como saída.
        printf("0\n");
        return 0;
    }

    // Lê as linhas do arquivo
    while (lines_read < NUM_SEQUENCES &&
           fgets(line_buffer2, sizeof(line_buffer2), file_ptr) != NULL) {
        
        // Remove caracteres de nova linha (\n ou \r\n) do final da string lida
        line_buffer2[strcspn(line_buffer2, "\r\n")] = 0;

        // Se a linha (após remover a nova linha) for maior que o permitido, trunca.
        if (strlen(line_buffer2) > MAX_SEQ_INPUT) {
            line_buffer2[MAX_SEQ_INPUT] = '\0';
            // Opcional: imprimir um aviso para stderr, se desejado.
            // fprintf(stderr, "Aviso: Linha %d foi truncada para %d caracteres.\n", lines_read + 1, MAX_SEQ_INPUT);
        }
        
        // Armazena a linha somente se não estiver vazia após o processamento
        if (strlen(line_buffer2) > 0) {
            strcpy(sequences[lines_read], line_buffer2);
            lines_read++;
        }
    }
    fclose(file_ptr);

    // A lógica de comparação "exceto a primeira" e "de cima para baixo" (linha i vs linha i+1)
    // requer que tenhamos pelo menos 3 linhas lidas no total para a primeira comparação ocorrer:
    // sequences[0] (primeira linha do arquivo - ignorada no início das comparações)
    // sequences[1] (segunda linha do arquivo - será a primeira string no par de comparação)
    // sequences[2] (terceira linha do arquivo - será a segunda string no primeiro par de comparação)
    if (lines_read < 3) {
        printf("0\n");
        return 0;
    }

    // Itera através das colunas (índice de caracteres)
    for (int col_idx = 0; col_idx < MAX_SEQ_INPUT; col_idx++) {
        // Itera através das linhas para fazer as comparações "de cima para baixo"
        // sequences[row_idx] será comparada com sequences[row_idx + 1]
        // A primeira linha do arquivo (sequences[0]) é ignorada na formação dos pares.
        // O loop de row_idx vai de 1 (segunda linha lida) até lines_read - 2
        // para garantir que sequences[row_idx + 1] seja um índice válido.
        for (int row_idx = 1; row_idx < (lines_read - 1); row_idx++) {
            
            // Verifica se o caractere na coluna atual existe em ambas as strings do par
            if (col_idx < strlen(sequences[row_idx]) && col_idx < strlen(sequences[row_idx + 1])) {
                char char1 = sequences[row_idx][col_idx];
                char char2 = sequences[row_idx + 1][col_idx];

                if (char1 == '-' || char2 == '-') {
                    total_score -= 2;
                } else if (char1 == char2) {
                    total_score += 1;
                } else {
                    // Caracteres diferentes (e nenhum é '-'), score += 0 (nenhuma mudança)
                }
            }
            // Se o col_idx estiver além do final de uma das strings do par,
            // nenhuma comparação é feita para este par nesta coluna.
            // O loop continua para a próxima coluna ou próximo par de linhas.
        }
    }

    printf("Pontuacao do Alinhamento obtida: %lld\n", total_score);

    return 0;
}