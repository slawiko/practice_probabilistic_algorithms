#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

typedef unsigned long word_type;
const int WORD_SIZE = sizeof(word_type) * 8;

int ITERATION_CNT = 40;
const int N_MAX = 4000;
const int WORD_CNT_MAX = N_MAX / WORD_SIZE;

void write_v(word_type* v, int word_cnt) {
    int cursor = WORD_SIZE;

    for (int i = 0; i < word_cnt; i++) {
        while(cursor) {
            printf("%lu ", (v[i] >> (cursor - 1)) & 1);
            cursor -= 1;
        }
        cursor = WORD_SIZE;
        printf(" ");
    }
    printf("\n");
}

void write_m(word_type** m, int word_cnt, int n) {
    for (int i = 0; i < n; i++) {
        write_v(m[i], word_cnt);
    }
}

void read_matrix(FILE* input, int word_cnt, int block_size, int n, word_type** M) {
    char* block_buffer = (char*) malloc((block_size + 1) * sizeof(char));
    char* symbol_buffer = (char*) malloc(2 * sizeof(char));
    symbol_buffer[1] = '\0';

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

        if (n % 4 != 0) {
            M[i][w] >>= 4 - (n % 4);
        }

        w = 0;
        word_cursor = WORD_SIZE;
    }

    free(block_buffer);
    free(symbol_buffer);
}

word_type mul_v_v(word_type* v1, word_type* v2, int word_cnt) {
    word_type result = 0;
    int counter = 0;
    for (int j = 0; j < word_cnt; ++j) {
        result ^= v1[j] & v2[j];
    }
    while (result) {
        counter += result & 1;
        result >>= 1;
    }
    result += counter % 2;

    return result;
}

void mul_m_v(word_type** M, word_type* v, int word_cnt, int n, word_type* Mv) {
    int w = 0;
    int word_cursor = WORD_SIZE;

    for (int i = 0; i < n; ++i) {
        Mv[i] = 0;
        Mv[w] <<= 1;
        Mv[w] += mul_v_v(M[i], v, word_cnt);
        word_cursor -= 1;

        if (word_cursor == 0) {
            w++;
            word_cursor = WORD_SIZE;
        }
    }
}

int calculate_position(word_type a, word_type b, int left_offset) {
    int position = 0;
    word_type diff = a ^ b;
    if (diff == 0) {
        return -1;
    }

    while (diff) {
        diff <<= 1;
        position += 1;
    }

    return left_offset + position;
}

int seek_col(word_type* Ai, word_type** B, word_type* Ci, int word_cnt, int n) {
    word_type* Bj = (word_type*) malloc(word_cnt * sizeof(word_type));
    word_type* ABi = (word_type*) malloc(word_cnt * sizeof(word_type));
    int w = 0, result_w = 0;
    int cursor = WORD_SIZE, result_cursor = WORD_SIZE;
    Bj[w] = 0;
    ABi[result_w] = 0;

    int last_word_shift = 0;
    int bit_position_shift = 0;

    for (int j = 0; j < n; ++j) {
        bit_position_shift = WORD_SIZE - (j % WORD_SIZE) - 1;
        for (int i = 0; i < n; ++i) {
            Bj[w] <<= 1;
            if (w + 1 == word_cnt) {
                last_word_shift = WORD_SIZE - n % WORD_SIZE;
            } else {
                last_word_shift = 0;
            }
            Bj[w] += ((B[i][j / WORD_SIZE] << last_word_shift) >> bit_position_shift) & 1;
            cursor -= 1;

            if (cursor == 0 && w + 1 < word_cnt) {
                w++;
                Bj[w] = 0;
                cursor = WORD_SIZE;
            }
        }

        w = 0;
        cursor = WORD_SIZE;

        ABi[result_w] <<= 1;
        ABi[result_w] += mul_v_v(Ai, Bj, word_cnt);
        result_cursor -= 1;

        if (result_cursor == 0 && result_w + 1 < word_cnt) {
            result_w++;
            ABi[result_w] = 0;
            result_cursor = WORD_SIZE;
        }
    }

    int position = 0;

    for (int i = 0; i < word_cnt; ++i) {
        position = calculate_position(ABi[i], Ci[i], i * WORD_SIZE);
        if (position != -1) {
            if (i + 1 == word_cnt) {
                position -= WORD_SIZE - n % WORD_SIZE;
            }
            return position;
        }
    }

    return -1;
}

int main() {
    srand(time(NULL));

    FILE* input = fopen("element.in", "r");
    FILE* output = fopen("element.out", "w");

    word_type** A = (word_type**) malloc(N_MAX * sizeof(word_type*));
    word_type** B = (word_type**) malloc(N_MAX * sizeof(word_type*));
    word_type** C = (word_type**) malloc(N_MAX * sizeof(word_type*));
    for (int i = 0; i < N_MAX; ++i) {
        A[i] = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));
        B[i] = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));
        C[i] = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));
    }
    word_type* r = (word_type*) malloc(WORD_CNT_MAX * sizeof(word_type));

    word_type* Br = (word_type*) malloc(N_MAX * sizeof(word_type));
    word_type* ABr = (word_type*) malloc(N_MAX * sizeof(word_type));
    word_type* Cr = (word_type*) malloc(N_MAX * sizeof(word_type));

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

        read_matrix(input, word_cnt, block_size, n, A);
        read_matrix(input, word_cnt, block_size, n, B);
        read_matrix(input, word_cnt, block_size, n, C);

        int col, row;

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
                row = calculate_position(ABr[j], Cr[j], j * WORD_SIZE);
                if (row != -1) {
                    fprintf(output, "Yes\n");

                    if (j + 1 == word_cnt) {
                        row -= WORD_SIZE - n % WORD_SIZE;
                    }

                    col = seek_col(A[row - 1], B, C[row - 1], word_cnt, n);
                    fprintf(output, "%d %d\n", row, col);
                    next = true;
                    break;
                }
            }
            if (next) {
                break;
            }
        }

        if (!next) {
            fprintf(output, "No\n");
        }

        fscanf(input, "%d", &n);
    }

    for (int k = 0; k < n; ++k) {
        free(A[k]);
        free(B[k]);
        free(C[k]);
    }
    free(A);
    free(B);
    free(C);
    free(r);

    free(Br);
    free(ABr);
    free(Cr);

    fclose(input);
    fclose(output);

    return 0;
}