#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int iteration_cnt = 50;

void read_matrix(FILE* input, int word_size, int block_size, int n, unsigned long** M) {
    char* block_buffer = (char*)malloc((block_size + 1) * sizeof(char));
    char* symbol_buffer = (char*)malloc(2 * sizeof(char));
    symbol_buffer[1] = '\0';

    int w = 0;
    int word_cursor = word_size;

    for (int i = 0; i < n; ++i) {
        M[i][w] = 0;

        fscanf(input, "%s", block_buffer);
        for (int j = 0; j < block_size; ++j) {
            symbol_buffer[0] = block_buffer[j];

            M[i][w] <<= 4;
            M[i][w] += strtol(symbol_buffer, NULL, 16);
            word_cursor -= 4;

            if (word_cursor == 0) {
                w++;
                word_cursor = word_size;
            }
        }

        w = 0;
        word_cursor = word_size;
    }

    free(block_buffer);
    free(symbol_buffer);
}

void mul_m_v(unsigned long** M, unsigned long* r, int word_cnt, int word_size, int n, unsigned long* Mr) {
    int w = 0;
    int word_cursor = word_size;
    int counter = 0;

    for (int i = 0; i < n; ++i) {
        Mr[i] = 0;
        for (int j = 0; j < word_cnt; ++j) {
            Mr[i] ^= M[i][j] & r[j];
        }
        while(Mr[i]) {
            counter += Mr[i] & 1;
            Mr[i] >>= 1;
        }
        Mr[w] <<= 1;
        Mr[w] += counter % 2;
        word_cursor -= 1;

        if (word_cursor == 0) {
            w++;
            word_cursor = word_size;
        }

        counter = 0;
    }

    if (n % 4 != 0) {
        Mr[w] <<= 4 - (n % 4);
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

    int word_size = sizeof(unsigned long) * 8;
    int word_cnt = n / word_size;
    if (n % word_size != 0) {
        word_cnt += 1;
    }

    unsigned long** A = (unsigned long**)malloc(n * sizeof(unsigned long*));
    for (int i = 0; i < n; ++i) {
        A[i] = (unsigned long *) malloc(word_cnt * sizeof(unsigned long));
    }

    unsigned long** B = (unsigned long**)malloc(n * sizeof(unsigned long*));
    for (int i = 0; i < n; ++i) {
        B[i] = (unsigned long*) malloc(word_cnt * sizeof(unsigned long));
    }

    unsigned long** C = (unsigned long**)malloc(n * sizeof(unsigned long*));
    for (int i = 0; i < n; ++i) {
        C[i] = (unsigned long*) malloc(word_cnt * sizeof(unsigned long));
    }

    read_matrix(input, word_size, block_size, n, A);
    read_matrix(input, word_size, block_size, n, B);
    read_matrix(input, word_size, block_size, n, C);

    unsigned long* Br = (unsigned long*)malloc(n * sizeof(unsigned long));
    unsigned long* ABr = (unsigned long*)malloc(n * sizeof(unsigned long));
    unsigned long* Cr = (unsigned long*)malloc(n * sizeof(unsigned long));

    FILE* output = fopen("matrix.out", "w");

    unsigned long* r = (unsigned long*)malloc(word_cnt * sizeof(unsigned long));

    for (int i = 0; i < iteration_cnt; ++i) {
        for (int j = 0; j < word_cnt; ++j) {
            r[j] = rand();
            r[j] <<= sizeof(int) * 8;
            r[j] += rand();
        }

        mul_m_v(B, r, word_cnt, word_size, n, Br);
        mul_m_v(A, Br, word_cnt, word_size, n, ABr);
        mul_m_v(C, r, word_cnt, word_size, n, Cr);

        for (int j = 0; j < word_cnt; ++j) {
            if (ABr[j] != Cr[j]) {
                fprintf(output, "NO");

                free(r);
                for (int k = 0; k < n; ++k) {
                    free(A[k]);
                }
                free(A);
                for (int k = 0; k < n; ++k) {
                    free(B[k]);
                }
                free(B);
                for (int k = 0; k < n; ++k) {
                    free(C[k]);
                }
                free(C);

                free(Br);
                free(ABr);
                free(Cr);

                fclose(input);
                fclose(output);

                return 0;
            }
        }
    }

    fprintf(output, "YES");

    free(r);
    for (int k = 0; k < n; ++k) {
        free(A[k]);
    }
    free(A);
    for (int k = 0; k < n; ++k) {
        free(B[k]);
    }
    free(B);
    for (int k = 0; k < n; ++k) {
        free(C[k]);
    }
    free(C);

    free(Br);
    free(ABr);
    free(Cr);

    fclose(input);
    fclose(output);

    return 0;
}