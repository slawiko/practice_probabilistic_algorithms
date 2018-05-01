import random

rounds_cnt = 20
rounds_cnt_crm = 20

rand = random.randint


def main():
    # input_file = open("primality.in", "r")
    # output_file = open("primality.out", "w")
    input_file = open("carmichael.in", "r")
    output_file = open("carmichael.out", "w")

    n = int(input_file.readline(), 10)
    lines = input_file.readlines()
    out_lines = []
    out_lines_append = out_lines.append

    for line in lines:
        # if is_probably_prime(int(line)):
        if is_probably_carmichael(int(line, 10)):
            out_lines_append('Yes\n')
        else:
            out_lines_append('No\n')

    output_file.writelines(out_lines)


def is_probably_carmichael(n):
    if n < 561 or is_probably_prime(n):
        return False

    n_1 = n - 1

    rounds_cnt_crm = 20
    while rounds_cnt_crm:
        a = rand(2, n_1)
        if gcd(a, n) == 1:
            rounds_cnt_crm -= 1
            if pow(a, n_1, n) != 1:
                return False

    return True


def gcd(a, b):
    return a if not b else gcd(b, a % b)


def is_probably_prime(m):
    if m == 2 or m == 3:
        return True
    if m < 2 or m % 2 == 0:
        return False

    m_1 = m - 1
    s = 0
    rest = 0

    while m_1 % 2 == 0:
        s += 1
        m_1 //= 2

    t = m_1
    for _ in xrange(rounds_cnt):
        if not is_compositeness_witness(m, s, t):
            return False

    return True


def is_compositeness_witness(m, s, t):
    a = rand(2, m - 2)

    if m % a == 0:
        return False
    x = pow(a, t, m)

    if x == 1 or x == m - 1:
        return True

    for k in xrange(s - 1):
        x = x * x % m
        if x == 1:
            return False
        if x == m - 1:
            return True

    return False


main()
