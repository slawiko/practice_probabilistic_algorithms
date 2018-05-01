#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

typedef unsigned int word_type;
const int WORD_SIZE = sizeof(word_type) * 8;
const int RAND_SIZE = 16;
const int N_MAX = 4000;
const int WORD_CNT_MAX = N_MAX / WORD_SIZE + (N_MAX % WORD_SIZE ? 1 : 0);
const int ITERATION_CNT = 20;

int bit_count(int v) {
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    return (((v + (v >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void compress(const int* v, word_type* v_compressed, int n) {
    int w = 0;
    v_compressed[w] = 0;
    int word_cursor = WORD_SIZE;
    for (int i = 0; i < n; ++i) {
        v_compressed[w] <<= 1;
        v_compressed[w] += v[i];
        word_cursor -= 1;

        if (word_cursor == 0) {
            w++;
            v_compressed[w] = 0;
            word_cursor = WORD_SIZE;
        }
    }
}

void read_matrix(FILE* input, int word_cnt, int block_size, int n, word_type** M_compressed, int** M) {
    char* block_buffer = (char*) malloc((block_size + 1) * sizeof(char));
    char* symbol_buffer = (char*) malloc(2 * sizeof(char));
    symbol_buffer[1] = '\0';
    int shift = n % 4 != 0 ? 4 - (n % 4) : 0;

    int w = 0;
    int word_cursor = WORD_SIZE;
    word_type tmp;
    int index = 0;

    for (int i = 0; i < n; ++i) {
        M_compressed[i][w] = 0;

        fscanf(input, "%s", block_buffer);
        for (int j = 0; j < block_size; ++j) {
            symbol_buffer[0] = block_buffer[j];

            tmp = (word_type)strtol(symbol_buffer, NULL, 16);

            M_compressed[i][w] <<= 4;
            M_compressed[i][w] += tmp;
            word_cursor -= 4;

            if (word_cursor == 0 && w + 1 < word_cnt) {
                w++;
                M_compressed[i][w] = 0;
                word_cursor = WORD_SIZE;
            }

            for (int k = 0; k < 4; ++k) {
                index = (4 * j) + k;
                if (index >= n) {
                    break;
                }
                M[i][index] = (tmp >> (3 - k)) & 1;
            }
        }

        M_compressed[i][w] >>= shift;

        w = 0;
        word_cursor = WORD_SIZE;
    }

    free(block_buffer);
    free(symbol_buffer);
}

void mul_m_v(word_type** M_compressed, const word_type* v_compressed, int word_cnt, int n, int* Mv) {
    word_type tmp = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < word_cnt; ++j) {
            tmp ^= M_compressed[i][j] & v_compressed[j];
        }

        Mv[i] = bit_count(tmp) & 1;
        tmp = 0;
    }
}

int calculate_position(const int* v1, const int* v2, int n) {
    for (int i = 0; i < n; ++i) {
        if (v1[i] != v2[i]) {
            return i;
        }
    }

    return -1;
}

int seek_col(const int* Ai, int** B, int* Ci, int n) {
    int* ABi = (int*) malloc(n * sizeof(int));

    for (int j = 0; j < n; ++j) {
        ABi[j] = 0;
        for (int k = 0; k < n; ++k) {
            ABi[j] ^= B[k][j] & Ai[k];
        }
    }

    int result = calculate_position(ABi, Ci, n);

    free(ABi);
    return result;
}

void generate_r_compressed(word_type* r_compressed, int word_cnt) {
    for (int j = 0; j < word_cnt; ++j) {
        r_compressed[j] = (rand() << RAND_SIZE + 1) |
                          (rand() << 2) |
                          (rand() & 3);
    }
}

int main() {
    srand(time(NULL));

    FILE* input = fopen("element.in", "r");
    FILE* output = fopen("element.out", "w");

    word_type** A_compressed = (word_type**) malloc(N_MAX * sizeof(word_type*));
    word_type** B_compressed = (word_type**) malloc(N_MAX * sizeof(word_type*));
    word_type** C_compressed = (word_type**) malloc(N_MAX * sizeof(word_type*));
    int** A = (int**) malloc(N_MAX * sizeof(int*));
    int** B = (int**) malloc(N_MAX * sizeof(int*));
    int** C = (int**) malloc(N_MAX * sizeof(int*));
    for (int i = 0; i < N_MAX; ++i) {
        A_compressed[i] = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));
        B_compressed[i] = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));
        C_compressed[i] = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));
        A[i] = (int*) malloc(N_MAX * sizeof(int));
        B[i] = (int*) malloc(N_MAX * sizeof(int));
        C[i] = (int*) malloc(N_MAX * sizeof(int));
    }

    word_type* Br_compressed = (word_type*) malloc(N_MAX * sizeof(word_type));
    int* Br = (int*) malloc(N_MAX * sizeof(int));
    int* ABr = (int*) malloc(N_MAX * sizeof(int));
    int* Cr = (int*) malloc(N_MAX * sizeof(int));

    word_type* r_compressed = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));

    bool next;
    int word_cnt;
    int block_size;

    int n;
    fscanf(input, "%d", &n);

    while (n) {
        next = false;
        block_size = n / 4;
        block_size += n % 4 != 0 ? 1 : 0;
        word_cnt = n / WORD_SIZE;
        word_cnt += n % WORD_SIZE != 0 ? 1 : 0;

        read_matrix(input, word_cnt, block_size, n, A_compressed, A);
        read_matrix(input, word_cnt, block_size, n, B_compressed, B);
        read_matrix(input, word_cnt, block_size, n, C_compressed, C);

        int col, row;

        for (int i = 0; i < ITERATION_CNT; ++i) {
            generate_r_compressed(r_compressed, word_cnt);

            mul_m_v(B_compressed, r_compressed, word_cnt, n, Br);
            compress(Br, Br_compressed, n);
            mul_m_v(A_compressed, Br_compressed, word_cnt, n, ABr);
            mul_m_v(C_compressed, r_compressed, word_cnt, n, Cr);

            row = calculate_position(ABr, Cr, n);
            if (row != -1) {
                fprintf(output, "Yes\n");

                col = seek_col(A[row], B, C[row], n);
                fprintf(output, "%d %d\n", row + 1, col + 1);
                next = true;
                break;
            }
        }

        if (!next) {
            fprintf(output, "No\n");
        }

        fscanf(input, "%d", &n);
    }

    for (int k = 0; k < n; ++k) {
        free(A_compressed[k]);
        free(B_compressed[k]);
        free(C_compressed[k]);
        free(A[k]);
        free(B[k]);
        free(C[k]);
    }
    free(A_compressed);
    free(B_compressed);
    free(C_compressed);
    free(A);
    free(B);
    free(C);

    free(Br_compressed);
    free(Br);
    free(ABr);
    free(Cr);

    free(r_compressed);

    fclose(input);
    fclose(output);

    return 0;
}