import serial_comms.constant as constant
from math import gcd
import random


def generate_keys():
    # Generate two distinct numbers p, q
    p = generate_prime_number()
    q = generate_prime_number()
    while True:
        if constant.LOWER_N_BOUND < (p * q) < constant.UPPER_N_BOUND:
            break
        q = generate_prime_number()

    # Computer n = pq. n is in range [2^24, 2^32)
    n = p * q

    # Compute fi(n) = (p-1)(q-1)
    fi_n = (p - 1) * (q - 1)

    # Choose an integer e such that 1 < e < fi(n) AND e and fi(n) are coprime
    e = generate_e(fi_n)

    # Generate d such that d*e = 1 (mod fi_n)
    d = extended_euclidean(e, fi_n) % fi_n

    # Private key = (n, d)
    private_key = (n, d)

    # Public key = (n, e)
    public_key = (n, e)

    return {'private': private_key, 'public': public_key}


# Generate a random prime number
def generate_prime_number():
    while True:
        # Generate a number which is a multiple of 6
        x = random.randint(constant.LOWER_PRIME_BOUND // 6, constant.UPPER_PRIME_BOUND // 6)
        x *= 6

        # Check it's the numbers on either side are prime, since all prime numbers above 3 are 6n+-1 for some integer n
        if is_prime(x - 1):
            return x - 1
        if is_prime(x + 1):
            return x + 1


# Taken from https://stackoverflow.com/questions/15285534/isprime-function-for-python-language
def is_prime(n):
    # Quick checks for if n is a prime:
    if n == 2 or n == 3: return True
    if n < 2 or n % 2 == 0: return False
    if n < 9: return True
    if n % 3 == 0: return False

    # Not so quick checks:
    # Check up to sqrt(n)
    r = int(n ** 0.5)
    # All prime numbers p are divisible by some 6n +- 1 for some integer n
    f = 5
    while f <= r:
        if n % f == 0: return False
        if n % (f + 2) == 0: return False
        f += 6
    return True


def generate_e(fi_n):
    while True:
        # Generate a random number less than fi(n)
        e = random.randint(1, fi_n)

        # Make sure e and fi(n) are coprime
        if gcd(e, fi_n) == 1:
            return e

# based pseudo code from https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
def extended_euclidean(a, b):
    s = 0
    old_s = 1
    t = 1
    old_t = 0
    r = b
    old_r = a
    while r != 0:
        quotient = old_r // r
        (old_r, r) = (r, old_r - quotient * r)
        (old_s, s) = (s, old_s - quotient * s)
        (old_t, t) = (t, old_t - quotient * t)
    return old_s

# based on pseudo code from https://en.wikipedia.org/wiki/Modular_exponentiation
def mod_exp(base, exp, mod):
    res = 1
    while exp > 0:
        if exp & 1 == 1:
            res = (res * base) % mod
        base = (base * base) % mod
        exp >>= 1
    return res

if __name__ == '__main__':
    keys = generate_keys()
    print('private', keys['private'])
    print('public', keys['public'])
