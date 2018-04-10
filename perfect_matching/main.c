#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

int MODULE = 10004983;


void write_v(long long* v, int n) {
    for (int i = 0; i < n; i++) {
        printf("%lld ", v[i]);
    }
    printf("\n");
}

void write_m(long long** m, int n) {
    for (int i = 0; i < n; i++) {
        write_v(m[i], n);
    }
    printf("\n");
}

long long binpow(long long a, long long n) {
    long long res = 1;
    while (n) {
        if (n & 1) {
            res *= a;
            res %= MODULE;
        }
        a *= a;
        a %= MODULE;
        n >>= 1;
    }
    return res;
}

long long inverse(long long a) {
    return binpow(a, MODULE - 2) % MODULE;
}

void swap_rows(long long** M, int i, int j) {
    long long* tmp;
    tmp = M[i];
    M[i] = M[j];
    M[j] = tmp;
}

int main() {
    srand(time(NULL));

    FILE* output = fopen("matching.out", "w");
    FILE* input = fopen("matching.in", "r");
    int n, m;

    fscanf(input, "%d %d", &n, &m);

    long long** B = (long long**) malloc(n * sizeof(long long*));
    long long** E = (long long**) malloc(n * sizeof(long long*));
    for (int i = 0; i < n; ++i) {
        B[i] = (long long*) malloc(n * sizeof(long long));
        E[i] = (long long*) malloc(n * sizeof(long long));
        for (int j = 0; j < n; ++j) {
            B[i][j] = 0;
            E[i][j] = 0;
        }
        E[i][i] = 1;
    }

    int v, w;
    long long tmp;
    int** V = (int**) malloc(m * sizeof(int*));
    for (int i = 0; i < m; ++i) {
        V[i] = (int*) malloc(2 * sizeof(int));

        fscanf(input, "%d %d", &v, &w);
        --v;
        --w;

        if (v > w) {
            tmp = v;
            v = w;
            w = tmp;
        }
        V[i][0] = v;
        V[i][1] = w;

        tmp = ((long long) rand() << 17 | (long long) rand() << 2 | (long long) rand() & 3) % MODULE;
        B[v][w] = tmp;
        B[w][v] = -tmp;
    }

    bool isInvertable = true;

    for (int k = 0; k < n; ++k) {
        if (B[k][k] == 0) {
            isInvertable = false;
            for (int j = k; j < n; ++j) {
                if (B[j][k] != 0) {
                    swap_rows(B, k, j);
                    swap_rows(E, k, j);
                    isInvertable = true;
                    break;
                }
            }
        }

        if (!isInvertable) {
            for (int i = 0; i < m; ++i) {
                fprintf(output, "NO\n");
            }

            goto exit;
        }
        tmp = inverse(B[k][k]);
        for (int j = 0; j < n; ++j) {
            B[k][j] *= tmp;
            B[k][j] %= MODULE;
            E[k][j] *= tmp;
            E[k][j] %= MODULE;
        }

        for (int i = k + 1; i < n; ++i) {
            if (B[i][k] != 0) {
                tmp = B[i][k];
                for (int j = 0; j < n; ++j) {
                    B[i][j] -= (B[k][j] * tmp) % MODULE;
                    B[i][j] %= MODULE;
                    E[i][j] -= (E[k][j] * tmp) % MODULE;
                    E[i][j] %= MODULE;
                }
            }
        }
    }

    for (int k = n - 1; k >= 0; --k) {
        for (int i = 0; i < k; ++i) {
            if (B[i][k] != 0) {
                tmp = B[i][k];
                for (int j = 0; j < n; ++j) {
                    B[i][j] -= (B[k][j] * tmp) % MODULE;
                    B[i][j] %= MODULE;
                    E[i][j] -= (E[k][j] * tmp) % MODULE;
                    E[i][j] %= MODULE;
                }
            }
        }
    }

    for (int i = 0; i < m; ++i) {
        if (E[V[i][1]][V[i][0]] != 0) {
            fprintf(output, "YES\n");
        } else {
            fprintf(output, "NO\n");
        }
    }

    exit:
    fclose(input);
    fclose(output);

    for (int i = 0; i < n; ++i) {
        free(B[i]);
        free(E[i]);
    }
    free(B);
    free(E);
    for (int i = 0; i < m; ++i) {
        free(V[i]);
    }
    free(V);

    return 0;
}
