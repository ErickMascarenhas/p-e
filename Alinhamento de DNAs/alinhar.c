#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define maxcompentradasseq 100
#define maxsequencias 50
#define maxcompalinhado 2000 // Max possible aligned length (should be >= maxcompentradasseq)

#define alfa 1
#define beta 0
#define delta -2
#define INFINITY_NEG -100000000 // A very small number for penalizing forbidden gaps

typedef struct{
    char seq[maxcompentradasseq + 1];
    int comp;
    int original_index; // To keep track of original order if needed, though not used in current output
} infoseq;

infoseq seqsiniciais[maxsequencias];
char resmsafinal[maxsequencias][maxcompalinhado + 1];

int max2(int a, int b) { return (a > b) ? a : b; }
int max3(int a, int b, int c) {return (b > a) ? ((c > b) ? c : b) : ((c > a) ? c : a);}

int pontuarparcaracteres(char c1, char c2){
    if (c1 == '-' || c2 == '-') return delta;
    if (c1 == c2) return alfa;
    return beta;
}

// Modified Needleman-Wunsch:
// if seq1_is_master_no_gaps is true, seq1 will not receive any gaps.
void needlemanwunsch(char *seq1, char *seq2, char *resseq1, char *resseq2, int seq1_is_master_no_gaps){
    int comp1 = strlen(seq1), comp2 = strlen(seq2);

    // Handle empty sequence cases first
    if (comp1 == 0 && comp2 == 0){
        resseq1[0] = '\0';
        resseq2[0] = '\0';
        return;
    }
    if (comp1 == 0){ // seq1 (master or not) is empty
        resseq1[0] = '\0'; // Aligned seq1 is empty
        // resseq2 is seq2, and resseq1 should be padded with gaps to match length of resseq2
        // However, if seq1_is_master_no_gaps, this implies the target length is 0.
        // The calling context (execmsaprog) will handle final length consistency.
        // For now, standard NW if one is empty:
        for(int j = 0; j < comp2; ++j) resseq1[j] = '-';
        resseq1[comp2] = '\0';
        strcpy(resseq2, seq2);
        return;
    }
    if (comp2 == 0){ // seq2 is empty
        strcpy(resseq1, seq1); // seq1 remains as is
        for(int i = 0; i < comp1; ++i) resseq2[i] = '-'; // seq2 is all gaps
        resseq2[comp1] = '\0';
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

    // Initialize DP table
    dp[0][0] = 0;
    for (int i = 1; i <= comp1; i++) {
        // Gap in seq2 (or seq1 aligned to nothing)
        dp[i][0] = dp[i-1][0] + delta;
    }
    for (int j = 1; j <= comp2; j++) {
        if (seq1_is_master_no_gaps) {
            // If seq1 is master, it cannot align with initial gaps.
            // This means aligning an empty prefix of seq1 with a prefix of seq2 is infinitely penalized.
            dp[0][j] = INFINITY_NEG;
        } else {
            // Gap in seq1 (or seq2 aligned to nothing)
            dp[0][j] = dp[0][j-1] + delta;
        }
    }


    for (int i = 1; i <= comp1; i++){
        for (int j = 1; j <= comp2; j++){
            int scorediag = dp[i-1][j-1] + pontuarparcaracteres(seq1[i-1], seq2[j-1]);
            int scorecima = dp[i-1][j] + delta; // Gap in seq2 (seq1[i-1] aligns with '-')
            int scoreesq;

            if (seq1_is_master_no_gaps) {
                scoreesq = INFINITY_NEG; // Penalize gap in seq1 heavily
            } else {
                scoreesq = dp[i][j-1] + delta; // Gap in seq1 (seq2[j-1] aligns with '-')
            }
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

        int current_score = dp[i_tb][j_tb];
        int diag_prev_score = (i_tb > 0 && j_tb > 0) ? dp[i_tb-1][j_tb-1] : INFINITY_NEG;
        int up_prev_score = (i_tb > 0) ? dp[i_tb-1][j_tb] : INFINITY_NEG;
        int left_prev_score = (j_tb > 0) ? dp[i_tb][j_tb-1] : INFINITY_NEG;

        if (i_tb > 0 && j_tb > 0 && current_score == diag_prev_score + pontuarparcaracteres(seq1[i_tb-1], seq2[j_tb-1])){
            tempalin1[idxrastro] = seq1[i_tb-1];
            tempalin2[idxrastro] = seq2[j_tb-1];
            i_tb--; j_tb--;
        } else if (i_tb > 0 && current_score == up_prev_score + delta){
            tempalin1[idxrastro] = seq1[i_tb-1];
            tempalin2[idxrastro] = '-';
            i_tb--;
        } else if (j_tb > 0 && !seq1_is_master_no_gaps && current_score == left_prev_score + delta){ // Allow gap in seq1 only if not master
            tempalin1[idxrastro] = '-';
            tempalin2[idxrastro] = seq2[j_tb-1];
            j_tb--;
        } else if (j_tb > 0 && seq1_is_master_no_gaps && current_score == left_prev_score + delta) {
             // This path should have been disfavored by INFINITY_NEG during DP calculation.
             // If it's chosen, it means other paths were even worse (e.g. all INFINITY_NEG).
             // This might happen if seq2 is much longer and forces gaps in seq1, which is disallowed.
             // However, the primary choice should be gap in seq2 if seq1 must be consumed.
             // If i_tb is 0, but j_tb > 0, and seq1 is master:
             // This state dp[0][j_tb] should be INFINITY_NEG.
             // The logic should prioritize consuming seq1 if i_tb > 0.
             // If i_tb is exhausted, and j_tb > 0, seq1 (master) gets padded with '-', but seq1 is already fully output.
             // This branch means gap in seq1. If seq1_is_master_no_gaps, this is an issue or means seq2 is longer and has to be gapped.
             // The DP calculation with INFINITY_NEG for scoreesq should prevent this path.
             // If we still reach here for a master, it's likely a fallback for an impossible alignment under constraints.
             // Force consuming seq2 with a gap if i_tb is stuck but > 0 (should be covered by 'gap in seq2' path)
             // Or if i_tb = 0, then remaining seq2 chars align with gaps.
            if (i_tb > 0) { // Should have chosen gap in seq2
                 tempalin1[idxrastro] = seq1[i_tb-1];
                 tempalin2[idxrastro] = '-'; // Implicitly, seq1 character must be output
                 i_tb--;
            } else { // i_tb == 0, seq1 exhausted, remaining seq2 chars align with gaps in seq1 (which is bad for master)
                     // This means the master sequence is shorter than what seq2 implies it should align to.
                     // This specific traceback part for master might need refinement if an issue appears.
                     // For now, assume DP calculation prevents this faulty path choice.
                 tempalin1[idxrastro] = '-'; // Should not happen for master
                 tempalin2[idxrastro] = seq2[j_tb-1];
                 j_tb--;
            }
        }
        // Fallbacks if primary choices don't match (e.g. due to INFINITY_NEG making scores equal)
        else if (i_tb > 0) { // Prioritize consuming seq1 (master or not)
            tempalin1[idxrastro] = seq1[i_tb-1];
            tempalin2[idxrastro] = (j_tb > 0) ? seq2[j_tb-1] : '-'; // This is not right, should be gap in seq2
            tempalin2[idxrastro] = '-';
            i_tb--;
            // if (j_tb > 0 && i_tb was already > 0) j_tb--; // if it was a forced match
        } else if (j_tb > 0) { // Then consume seq2
            tempalin1[idxrastro] = '-';
            tempalin2[idxrastro] = seq2[j_tb-1];
            j_tb--;
        }
        else break; // Both i_tb and j_tb are 0
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


// Comparison function for qsort, sorts by sequence length descending
int compseqcompdesc(const void *a, const void *b){
    infoseq *sinfo1 = (infoseq *)a;
    infoseq *sinfo2 = (infoseq *)b;
    return sinfo2->comp - sinfo1->comp;
}

// Revised MSA execution function
void execmsaprog(infoseq allseqs[maxsequencias], int numtotseqs, char msafinal[maxsequencias][maxcompalinhado + 1], int *compfinalalin){
    if (numtotseqs == 0){
        *compfinalalin = 0;
        return;
    }

    // Sort sequences by length, descending. This makes allseqs[0] one of the longest.
    qsort(allseqs, numtotseqs, sizeof(infoseq), compseqcompdesc);

    int max_len = 0;
    if (numtotseqs > 0) {
        max_len = allseqs[0].comp; // Length of the longest sequence(s)
    }
    *compfinalalin = max_len;

    if (max_len == 0 && numtotseqs > 0) { // All sequences are empty
        for (int i = 0; i < numtotseqs; i++) {
            msafinal[i][0] = '\0';
        }
        return;
    }
    if (max_len > maxcompalinhado) {
        fprintf(stderr, "Erro: Comprimento maximo da sequencia (%d) excede maxcompalinhado (%d).\n", max_len, maxcompalinhado);
        exit(EXIT_FAILURE);
    }


    char master_ref_seq[maxcompentradasseq + 1];
    // Use the first sequence (which is one of the longest) as the master reference for aligning shorter sequences.
    // If allseqs[0] is empty and max_len is 0, this strcpy is fine.
    if (max_len > 0) {
       strcpy(master_ref_seq, allseqs[0].seq);
    } else {
       master_ref_seq[0] = '\0';
    }


    char temp_aligned_master_buffer[maxcompalinhado + 1]; // Buffer for the master part of NW result (should be identical to master)

    for (int i = 0; i < numtotseqs; i++) {
        if (allseqs[i].comp == max_len) {
            // This is one of the longest sequences, copy it directly without gaps.
            strcpy(msafinal[i], allseqs[i].seq);
            // Ensure it's null-terminated at max_len, though strcpy should do it.
            msafinal[i][max_len] = '\0';
        } else {
            // This sequence is shorter than the max_len.
            // Align it to the master_ref_seq. master_ref_seq (seq1) must not get gaps.
            if (max_len > 0) { // Only align if there's a master reference to align to
                needlemanwunsch(master_ref_seq, allseqs[i].seq, temp_aligned_master_buffer, msafinal[i], 1); // 1 means seq1 (master_ref_seq) is master
                // Post-check: ensure msafinal[i] is exactly max_len.
                // needlemanwunsch should ensure this if master doesn't get gaps and has length max_len.
                int current_aligned_len = strlen(msafinal[i]);
                if (current_aligned_len != max_len) {
                    // This might happen if the shorter sequence is empty, NW might return empty for it.
                    // Or if NW logic for master had an issue. For safety, pad.
                    //fprintf(stderr, "Aviso: Sequencia %d (original: '%s') alinhada para '%s' (comp %d), esperado %d. Re-ajustando padding.\n", i, allseqs[i].seq, msafinal[i], current_aligned_len, max_len);
                    for (int k = current_aligned_len; k < max_len; k++) {
                        msafinal[i][k] = '-';
                    }
                    msafinal[i][max_len] = '\0';
                }
            } else { // max_len is 0, all sequences are empty
                 msafinal[i][0] = '\0';
            }
        }
         // Final check for safety, ensuring all strings are not too long and null terminated properly
        if (strlen(msafinal[i]) > max_len && max_len <= maxcompalinhado) {
            msafinal[i][max_len] = '\0'; // Truncate if something went very wrong
        } else if (strlen(msafinal[i]) > maxcompalinhado) {
             msafinal[i][maxcompalinhado] = '\0';
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

    char buflinha[maxcompentradasseq + 3]; // +2 for \r\n, +1 for \0
    while (seqslidas < maxsequencias && fgets(buflinha, sizeof(buflinha), arqent) != NULL){
        buflinha[strcspn(buflinha, "\r\n")] = 0;
        if (strlen(buflinha) == 0 && feof(arqent) && seqslidas == 0 && !ferror(arqent)) continue; // Skip empty lines unless it's the only content leading to 0 sequences

        if (strlen(buflinha) > maxcompentradasseq){
            fprintf(stderr, "aviso: sequencia %d (0-indexed) excede maxcompentradasseq (%d) e sera truncada.\n", seqslidas, maxcompentradasseq);
            strncpy(seqsiniciais[seqslidas].seq, buflinha, maxcompentradasseq);
            seqsiniciais[seqslidas].seq[maxcompentradasseq] = '\0';
        } else {
            strcpy(seqsiniciais[seqslidas].seq, buflinha);
        }
        seqsiniciais[seqslidas].comp = strlen(seqsiniciais[seqslidas].seq);
        seqsiniciais[seqslidas].original_index = seqslidas; // Store original index

        // Only increment if the sequence (even if truncated) is not effectively empty *after* processing
        // Or if it was an empty line but we want to count it (policy: only count non-empty after processing)
        if (seqsiniciais[seqslidas].comp > 0 || strlen(buflinha) > 0) { // Count if original line had content, or resulting seq has content
             seqslidas++;
        } else if (feof(arqent) && seqslidas == 0) { // Handle file with only one empty line
            // allow one empty sequence if it's the only thing
            if (strlen(buflinha) == 0) seqslidas++;
        }

    }
    fclose(arqent);

    if (seqslidas == 0){
        fprintf(stderr, "nenhuma sequencia valida lida do arquivo input.txt\n");
        // Still create an empty output file for consistency
        arqsai = fopen("output.txt", "w");
        if (arqsai == NULL) {
            perror("erro ao abrir o arquivo de saida (output.txt)");
            return 1;
        }
        fprintf(arqsai, "Alinhamento Multiplo de Sequencias Resultante:\n");
        fprintf(arqsai, "\nNenhuma sequencia alinhada para calcular pontuacao.\n");
        fclose(arqsai);
        printf("processamento concluido. 0 sequencias foram processadas.\nverifique 'output.txt' para os resultados.\n");
        return 0; // Not an error, just no input
    }

    if (seqslidas < maxsequencias) { // Removed "seqslidas > 0" as it's implied by not exiting above
        printf("aviso: foram lidas %d sequencias. o maximo configurado e %d.\n", seqslidas, maxsequencias);
    } else if (seqslidas == maxsequencias) {
        printf("%d sequencias lidas com sucesso de input.txt.\n", seqslidas);
    }

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
    if (seqslidas > 0 && compfinalalin > 0) printf("pontuacao (calculo sequencial vertical): %d\n", scoreverticalfinal);    return 0;
}