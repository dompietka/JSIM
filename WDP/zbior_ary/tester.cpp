// Author: Kajetan Ramsza
#include "bits/stdc++.h"

using std::ostream;
using std::vector;

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& v) {
    os << "{";
    typename vector<T>::const_iterator it;
    for (it = v.begin(); it != v.end(); it++) {
        if (it != v.begin())
            os << ", ";
        os << *it;
    }
    return os << "}";
}

void dbg_out() {
    std::cerr << '\n';
}
template<typename Head, typename... Tail>
void dbg_out(Head H, Tail... T) {
    std::cerr << ' ' << H;
    dbg_out(T...);
}

#ifdef DEBUG
#define dbg(...) std::cerr << "(" << #__VA_ARGS__ << "):", dbg_out(__VA_ARGS__)
#else
#define dbg(...)
#endif

namespace Solution {

extern "C" {
#include "zbior_ary.h"
};

}  // namespace Solution

using namespace std;

namespace BruteForce {

#define all(x) begin(x), end(x)
typedef vector<int> zbior_ary;

int q = 0;

vector<int> singleton(int x) {
    assert(q != 0);
    return {x};
}

vector<int> ciag_arytmetyczny(int a, int q_, int b) {
    assert(q == 0 || q == q_);
    q = q_;
    assert((a % q + q) % q == (b % q + q) % q);
    vector<int> result;
    for (int i = a; i <= b; i += q)
        result.push_back(i);
    return result;
}

vector<int> suma(vector<int> a, vector<int> b) {
    vector<int> result;
    set_union(all(a), all(b), back_inserter(result));
    return result;
}

vector<int> roznica(vector<int> a, vector<int> b) {
    vector<int> result;
    set_difference(all(a), all(b), back_inserter(result));
    return result;
}

vector<int> iloczyn(vector<int> a, vector<int> b) {
    vector<int> result;
    set_intersection(all(a), all(b), back_inserter(result));
    return result;
}

bool nalezy(vector<int> a, int x) {
    return find(all(a), x) != end(a);
}

unsigned moc(vector<int> a) {
    return (unsigned)a.size();
}

unsigned ary(vector<int> a) {
    unsigned result = 0;
    map<int, int> m;
    for (auto x : a) {
        int r = (x % q + q) % q;
        if (!m.count(r) || m[r] < x - q)
            result++;
        m[r] = x;
    }
    return result;
}

}  // namespace BruteForce

template<typename T, typename std::enable_if<std::is_same<T, BruteForce::zbior_ary>::value>::type* = nullptr>
T ciag_arytmetyczny(int a, int q, int b) {
    return BruteForce::ciag_arytmetyczny(a, q, b);
}

template<typename T, typename std::enable_if<std::is_same<T, Solution::zbior_ary>::value>::type* = nullptr>
T ciag_arytmetyczny(int a, int q, int b) {
    return Solution::ciag_arytmetyczny(a, q, b);
}

template<typename T, typename std::enable_if<std::is_same<T, BruteForce::zbior_ary>::value>::type* = nullptr>
T singleton(int x) {
    return BruteForce::singleton(x);
}

template<typename T, typename std::enable_if<std::is_same<T, Solution::zbior_ary>::value>::type* = nullptr>
T singleton(int x) {
    return Solution::singleton(x);
}

using BruteForce::ary;
using BruteForce::iloczyn;
using BruteForce::moc;
using BruteForce::nalezy;
using BruteForce::roznica;
using BruteForce::suma;

using Solution::ary;
using Solution::iloczyn;
using Solution::moc;
using Solution::nalezy;
using Solution::roznica;
using Solution::suma;

enum OperationType { NEW_SET, NEW_SINGLETON, SUM, DIFFERENCE, INTERSECTION };
enum ResultType { SIZE, ARY, CONTAINS };

struct Operation {
    OperationType type;
    int a, b;

    friend ostream& operator<<(ostream& os, const Operation& ops) {
        if (ops.type == NEW_SET) {
            return os << "NEW_SET " << ops.a << ' ' << ops.b;
        }
        if (ops.type == NEW_SINGLETON) {
            return os << "NEW_SINGLETON " << ops.a;
        }
        if (ops.type == SUM) {
            return os << "SUM " << ops.a << ' ' << ops.b;
        }
        if (ops.type == DIFFERENCE) {
            return os << "DIFFERENCE " << ops.a << ' ' << ops.b;
        }
        if (ops.type == INTERSECTION) {
            return os << "INTERSECTION " << ops.a << ' ' << ops.b;
        }
        return os;
    }

    friend istream& operator>>(istream& is, Operation& ops) {
        string type_string;
        is >> type_string;
        if (type_string == "NEW_SET") {
            ops.type = NEW_SET;
            is >> ops.a >> ops.b;
        } else if (type_string == "NEW_SINGLETON") {
            ops.type = NEW_SINGLETON;
            is >> ops.a;
        } else if (type_string == "SUM") {
            ops.type = SUM;
            is >> ops.a >> ops.b;
        } else if (type_string == "DIFFERENCE") {
            ops.type = DIFFERENCE;
            is >> ops.a >> ops.b;
        } else if (type_string == "INTERSECTION") {
            ops.type = INTERSECTION;
            is >> ops.a >> ops.b;
        } else {
            cerr << "Invalid operation type: " << type_string << '\n';
            exit(1);
        }
        return is;
    }
};

int X;

pair<int, vector<Operation>> generate_operations(unsigned input_size, int max_q, bool positive, int operations,
                                                 const vector<OperationType> allowed_ops) {
    assert(max_q >= 1);

    const auto random = [&](int a, int b) { return rand() % (b - a + 1) + a; };
    const int q = random(1, max_q);

    const int min_val = positive ? 0 : -input_size;
    const int max_val = input_size;

    const auto gen_new_set_op = [&]() {
        int start = random(min_val, max_val);
        int len = random(1, input_size);
        return Operation{NEW_SET, start, start + (len - 1) * q};
    };

    vector<Operation> ops;
    int vals_size = 0;

    while (operations--) {
        OperationType type = allowed_ops[random(0, (int)allowed_ops.size() - 1)];
        if (vals_size < 2)
            type = NEW_SET;

        if (type == NEW_SET) {
            vals_size++;
            ops.push_back(gen_new_set_op());
            continue;
        }

        if (type == NEW_SINGLETON) {
            vals_size++;
            ops.push_back(Operation{type, random(min_val, max_val), -1});
            continue;
        }

        int start_index = min(vals_size - 2, vals_size - vals_size / 10);

        int i = random(start_index, vals_size - 1), j;
        do {
            j = random(start_index, vals_size - 1);
        } while (i == j);

        ops.push_back(Operation{type, i, j});
    }

    X = random(min_val, max_val);

    return {q, ops};
}

void write_operations(const int q, const vector<Operation>& ops, const ResultType result_type) {
    std::cerr << ops.size() << ' ' << q << '\n';
    for (const auto& op : ops) {
        std::cerr << op << '\n';
    }
    if (result_type == SIZE) {
        std::cerr << "SIZE\n";
    } else if (result_type == ARY) {
        std::cerr << "ARY\n";
    } else if (result_type == CONTAINS) {
        std::cerr << "CONTAINS " << X << '\n';
    }
}

ResultType result_type_from_string(const string& s) {
    if (s == "SIZE") {
        return SIZE;
    } else if (s == "ARY") {
        return ARY;
    } else if (s == "CONTAINS") {
        return CONTAINS;
    }
    cerr << "Invalid result type: " << s << '\n';
    exit(1);
}

tuple<int, vector<Operation>, ResultType> read_operations() {
    int operations, q;
    cin >> operations >> q;
    vector<Operation> ops(operations);
    for (auto& op : ops) {
        cin >> op;
    }
    string result_type_string;
    cin >> result_type_string;
    ResultType result_type = result_type_from_string(result_type_string);
    if (result_type == CONTAINS) {
        cin >> X;
    }
    return {q, ops, result_type};
}

template<typename T>
unsigned gen(const int q, const vector<Operation>& ops, const ResultType result_type) {
    vector<T> vals;

    for (auto op : ops) {
        if (op.type == NEW_SET) {
            vals.push_back(ciag_arytmetyczny<T>(op.a, q, op.b));
        } else if (op.type == NEW_SINGLETON) {
            vals.push_back(singleton<T>(op.a));
        } else if (op.type == SUM) {
            vals.push_back(suma(vals[op.a], vals[op.b]));
        } else if (op.type == DIFFERENCE) {
            vals.push_back(roznica(vals[op.a], vals[op.b]));
        } else if (op.type == INTERSECTION) {
            vals.push_back(iloczyn(vals[op.a], vals[op.b]));
        }
    }

    switch (result_type) {
        case SIZE:
            return moc(vals.back());
        case ARY:
            return ary(vals.back());
        case CONTAINS:
            return nalezy(vals.back(), X);
    }
    assert(false);
    return -1;
}

int run_test(bool check_output, const int q, const vector<Operation>& ops, const ResultType result_type) {
    unsigned result_solution = gen<Solution::zbior_ary>(q, ops, result_type);

    if (!check_output) {
        cout << result_solution;
        return 0;
    }

    unsigned result_brute = gen<BruteForce::zbior_ary>(q, ops, result_type);
    cout << result_brute << ' ' << result_solution << '\n';

    return !(result_brute == result_solution);
}

int main(int, char** argv) {
    const string test_type = argv[1];
    if (test_type == "CUSTOM") {
        const string check_output = argv[2];

        const auto [q, ops, result_type] = read_operations();
        return run_test(check_output == "CHECK", q, ops, result_type);
    }

    const string test_function = argv[2];
    const string result_type_string = argv[3];
    int max_q = stoi(argv[4]);
    int input_size = stoi(argv[5]);
    bool positive = stoi(argv[6]);

    unsigned seed = static_cast<unsigned>(stoi(argv[7]));
    srand(seed);

    const ResultType result_type = result_type_from_string(result_type_string);

    int operations;
    if (test_type == "CONSTRUCTOR") {
        operations = 1;
    } else if (test_type == "UNIT") {
        operations = 3;
    } else if (test_type == "INTEGRATION") {
        operations = 10;
    } else if (test_type == "INTEGRATION_LARGE") {
        operations = 100;
    } else if (test_type == "STRESS") {
        operations = 10000;
    } else {
        cerr << "Unrecognized test_type: " << test_type << '\n';
        return 1;
    }

    vector<OperationType> allowed_ops;
    if (test_type != "UNIT") {
        allowed_ops.push_back(NEW_SET);
        allowed_ops.push_back(NEW_SINGLETON);
    }

    if (test_function == "SUM") {
        allowed_ops.push_back(SUM);
    } else if (test_function == "DIFFERENCE") {
        allowed_ops.push_back(DIFFERENCE);
    } else if (test_function == "INTERSECTION") {
        allowed_ops.push_back(INTERSECTION);
    } else if (test_function == "ALL") {
        allowed_ops.push_back(SUM);
        allowed_ops.push_back(DIFFERENCE);
        allowed_ops.push_back(INTERSECTION);
    }

    const auto [q, ops] = generate_operations(input_size, max_q, positive, operations, allowed_ops);

    write_operations(q, ops, result_type);

    return run_test(test_type != "STRESS", q, ops, result_type);
}