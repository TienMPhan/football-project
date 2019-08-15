//
// Created by minht on 8/15/2019.
//

#include <random>

int randint(int lower, int upper) {
    std::random_device rd;
    std::mt19937_64 rde(rd());
    std::uniform_int_distribution<int> distribution(lower, upper);
    int number = distribution(rde);
    return number;
}

double randDouble() {
    std::random_device rd;
    std::mt19937_64 rde(rd());
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    double r = distribution(rde);
    return r;
}
