#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned long word_type;
const int WORD_SIZE = sizeof(word_type) * 8;

int ITERATION_CNT = 50;

void read_matrix(FILE* input, int word_cnt, int block_size, int n, word_type** M) {
    char* block_buffer = (char*) malloc((block_size + 1) * sizeof(char));
    char* symbol_buffer = (char*) malloc(2 * sizeof(char));
    symbol_buffer[1] = '\0';
    int shift = n % 4 != 0 ? 4 - (n % 4) : 0;

    int w = 0;
    int word_cursor = WORD_SIZE;

    for (int i = 0; i < n; ++i) {
        M[i][w] = 0;

        fscanf(input, "%s", block_buffer);
        for (int j = 0; j < block_size; ++j) {
            symbol_buffer[0] = block_buffer[j];

            M[i][w] <<= 4;
            M[i][w] += strtol(symbol_buffer, NULL, 16);
            word_cursor -= 4;

            if (word_cursor == 0 && w + 1 < word_cnt) {
                w++;
                M[i][w] = 0;
                word_cursor = WORD_SIZE;
            }
        }

        M[i][w] >>= shift;

        w = 0;
        word_cursor = WORD_SIZE;
    }

    free(block_buffer);
    free(symbol_buffer);
}

void mul_m_v(word_type** M, word_type* r, int word_cnt, int n, word_type* Mr) {
    int w = 0;
    int word_cursor = WORD_SIZE;
    int counter = 0;

    for (int i = 0; i < n; ++i) {
        Mr[i] = 0;
        for (int j = 0; j < word_cnt; ++j) {
            Mr[i] ^= M[i][j] & r[j];
        }
        while (Mr[i]) {
            counter += Mr[i] & 1;
            Mr[i] >>= 1;
        }
        Mr[w] <<= 1;
        Mr[w] += counter % 2;
        word_cursor -= 1;

        if (word_cursor == 0) {
            w++;
            word_cursor = WORD_SIZE;
        }

        counter = 0;
    }
}

int main() {
    srand(time(NULL));

    FILE* input = fopen("matrix.in", "r");
    int n;

    fscanf(input, "%d", &n);

    int block_size = n / 4;
    if (n % 4 != 0) {
        block_size += 1;
    }

    int word_cnt = n / WORD_SIZE;
    if (n % WORD_SIZE != 0) {
        word_cnt += 1;
    }

    word_type** A = (word_type**) malloc(n * sizeof(word_type*));
    word_type** B = (word_type**) malloc(n * sizeof(word_type*));
    word_type** C = (word_type**) malloc(n * sizeof(word_type*));
    for (int i = 0; i < n; ++i) {
        A[i] = (word_type*) malloc(word_cnt * sizeof(word_type));
        B[i] = (word_type*) malloc(word_cnt * sizeof(word_type));
        C[i] = (word_type*) malloc(word_cnt * sizeof(word_type));
    }

    read_matrix(input, word_cnt, block_size, n, A);
    read_matrix(input, word_cnt, block_size, n, B);
    read_matrix(input, word_cnt, block_size, n, C);

    word_type* Br = (word_type*) malloc(n * sizeof(word_type));
    word_type* ABr = (word_type*) malloc(n * sizeof(word_type));
    word_type* Cr = (word_type*) malloc(n * sizeof(word_type));

    FILE* output = fopen("matrix.out", "w");

    word_type* r = (word_type*) malloc(word_cnt * sizeof(word_type));

    for (int i = 0; i < ITERATION_CNT; ++i) {
        for (int j = 0; j < word_cnt; ++j) {
            for (int k = 0; k < WORD_SIZE; ++k) {
                r[j] <<= 1;
                r[j] += rand() % 2;
            }
        }

        mul_m_v(B, r, word_cnt, n, Br);
        mul_m_v(A, Br, word_cnt, n, ABr);
        mul_m_v(C, r, word_cnt, n, Cr);

        for (int j = 0; j < word_cnt; ++j) {
            if (ABr[j] != Cr[j]) {
                fprintf(output, "NO");

                goto exit;
            }
        }
    }

    fprintf(output, "YES");

    exit:
    free(r);
    for (int k = 0; k < n; ++k) {
        free(A[k]);
        free(B[k]);
        free(C[k]);
    }
    free(A);
    free(B);
    free(C);

    free(Br);
    free(ABr);
    free(Cr);

    fclose(input);
    fclose(output);

    return 0;
}