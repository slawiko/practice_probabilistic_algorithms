#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE* input = fopen("matrix.in", "r");
    int n;

    fscanf(input, "%d", &n);

    int block_size = n / 4;
    if (n % 4 != 0) {
        block_size += 1;
    }

    char* block_buffer = (char*)malloc(block_size * sizeof(char));
    char* symbol_buffer = (char*)malloc(sizeof(char));

    int word_size = sizeof(long) * 8;
    int word_cnt = n / word_size;
    if (n % word_size != 0) {
        word_cnt += 1;
    }

    long** words = (long**)malloc(n * sizeof(long*));
    int w = 0;
    int word_cursor = word_size;

    for (int i = 0; i < n; ++i) {
        words[i] = (long*)malloc(word_cnt * sizeof(long));
        for (int j = 0; j < word_cnt; ++j) {
            words[i][j] = 0;
        }

        fscanf(input, "%s", block_buffer);
        for (int j = 0; j < block_size; ++j) {
            symbol_buffer[0] = block_buffer[j];

            words[i][w] <<= 4;
            words[i][w] += strtol(symbol_buffer, NULL, 16);
            word_cursor -= 4;

            if (word_cursor == 0) {
                w++;
                word_cursor = word_size;
            }
        }
    }

    fclose(input);

    return 0;
}