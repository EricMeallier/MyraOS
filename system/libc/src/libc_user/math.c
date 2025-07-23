#include <math.h>

int abs(int x) {
    return x < 0 ? -x : x;
}

double pow(double base, int exp) {
    double result = 1.0;
    int negative = 0;

    if (exp < 0) {
        negative = 1;
        exp = -exp;
    }

    while (exp--) {
        result *= base;
    }

    return negative ? 1.0 / result : result;
}

double sqrt(double x) {
    if (x < 0) {
        return -1; // Not handling NaN
    }

    double guess = x / 2.0;
    for (int i = 0; i < 20; i++) {
        guess = 0.5 * (guess + x / guess);
    }

    return guess;
}

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}
