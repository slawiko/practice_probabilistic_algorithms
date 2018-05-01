#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned int word_type;
const int WORD_SIZE = sizeof(word_type) * 8;

int ITERATION_CNT = 50;

int bit_count(int v) {
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    return (((v + (v >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void read_matrix(FILE* input, int word_cnt, int block_size, int n, word_type** M_compressed) {
    char* block_buffer = (char*) malloc((block_size + 1) * sizeof(char));
    char* symbol_buffer = (char*) malloc(2 * sizeof(char));
    symbol_buffer[1] = '\0';
    int shift = n % 4 != 0 ? 4 - (n % 4) : 0;

    int w = 0;
    int word_cursor = WORD_SIZE;

    for (int i = 0; i < n; ++i) {
        M_compressed[i][w] = 0;

        fscanf(input, "%s", block_buffer);
        for (int j = 0; j < block_size; ++j) {
            symbol_buffer[0] = block_buffer[j];

            M_compressed[i][w] <<= 4;
            M_compressed[i][w] += strtol(symbol_buffer, NULL, 16);
            word_cursor -= 4;

            if (word_cursor == 0 && w + 1 < word_cnt) {
                w++;
                M_compressed[i][w] = 0;
                word_cursor = WORD_SIZE;
            }
        }

        M_compressed[i][w] >>= shift;

        w = 0;
        word_cursor = WORD_SIZE;
    }

    free(block_buffer);
    free(symbol_buffer);
}

void mul_m_v(word_type** M_compressed, const word_type* v_compressed, int word_cnt, int n, word_type* Mv_compressed) {
    int w = 0;
    Mv_compressed[w] = 0;
    int word_cursor = WORD_SIZE;
    word_type tmp = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < word_cnt; ++j) {
            tmp ^= M_compressed[i][j] & v_compressed[j];
        }

        Mv_compressed[w] <<= 1;
        Mv_compressed[w] += bit_count(tmp) & 1;
        word_cursor -= 1;

        if (word_cursor == 0) {
            w++;
            Mv_compressed[w] = 0;
            word_cursor = WORD_SIZE;
        }

        tmp = 0;
    }
}

void generate_r_compressed(word_type* r_compressed, int word_cnt) {
    for (int j = 0; j < word_cnt; ++j) {
        for (int k = 0; k < WORD_SIZE; ++k) {
            r_compressed[j] <<= 1;
            r_compressed[j] += rand() & 1;
        }
    }
}

int main() {
    srand(time(NULL));

    FILE* input = fopen("matrix.in", "r");
    FILE* output = fopen("matrix.out", "w");
    int n;

    fscanf(input, "%d", &n);

    int block_size = n / 4;
    block_size += n % 4 != 0 ? 1 : 0;
    int word_cnt = n / WORD_SIZE;
    word_cnt += n % WORD_SIZE != 0 ? 1 : 0;

    word_type** A_compressed = (word_type**) malloc(n * sizeof(word_type*));
    word_type** B_compressed = (word_type**) malloc(n * sizeof(word_type*));
    word_type** C_compressed = (word_type**) malloc(n * sizeof(word_type*));
    for (int i = 0; i < n; ++i) {
        A_compressed[i] = (word_type*) malloc(word_cnt * sizeof(word_type));
        B_compressed[i] = (word_type*) malloc(word_cnt * sizeof(word_type));
        C_compressed[i] = (word_type*) malloc(word_cnt * sizeof(word_type));
    }

    word_type* Br = (word_type*) malloc(word_cnt * sizeof(word_type));
    word_type* ABr = (word_type*) malloc(word_cnt * sizeof(word_type));
    word_type* Cr = (word_type*) malloc(word_cnt * sizeof(word_type));

    word_type* r_compressed = (word_type*) malloc(word_cnt * sizeof(word_type));

    read_matrix(input, word_cnt, block_size, n, A_compressed);
    read_matrix(input, word_cnt, block_size, n, B_compressed);
    read_matrix(input, word_cnt, block_size, n, C_compressed);

    for (int i = 0; i < ITERATION_CNT; ++i) {
        generate_r_compressed(r_compressed, word_cnt);

        mul_m_v(B_compressed, r_compressed, word_cnt, n, Br);
        mul_m_v(A_compressed, Br, word_cnt, n, ABr);
        mul_m_v(C_compressed, r_compressed, word_cnt, n, Cr);

        for (int j = 0; j < word_cnt; ++j) {
            if (ABr[j] != Cr[j]) {
                fprintf(output, "NO");

                goto exit;
            }
        }
    }

    fprintf(output, "YES");

    exit:
    for (int k = 0; k < n; ++k) {
        free(A_compressed[k]);
        free(B_compressed[k]);
        free(C_compressed[k]);
    }
    free(A_compressed);
    free(B_compressed);
    free(C_compressed);

    free(Br);
    free(ABr);
    free(Cr);

    free(r_compressed);

    fclose(input);
    fclose(output);

    return 0;
}