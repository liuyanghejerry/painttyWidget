#ifndef BINARY_H
#define BINARY_H

/*
 * This simple binary literal converter comes from
 * http://stackoverflow.com/a/2611850/802650
 * with a little modify to work with Qt
 *
 * Usage: std::cout << bin<1000>::value << '\n';
 */


template<unsigned long N>
struct binL {
    enum { value = N%10 + (binL<N/10>::value <<1) };
};

template<>
struct binL<0> {
    enum { value = 0 };
};


#endif // BINARY_H
