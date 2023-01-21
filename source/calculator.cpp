#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <charconv>
#include <cassert>
#include <cmath>

namespace Calculator {
    namespace {
        // symbols that compose the language of the calculator
        enum class Symbol {
            null    = 0,
            v0      = '0',
            v1      = '1',
            v2      = '2',
            v3      = '3',
            v4      = '4',
            v5      = '5',
            v6      = '6',
            v7      = '7',
            v8      = '8',
            v9      = '9',
            plus    = '+',
            minus   = '-',
            mul     = '*',
            div     = '/',
            exp     = '^',
            mod     = '%',
            open    = '(',
            close   = ')',
            period  = '.',
            space   = ' ',
            newline = '\n',
            tab     = '\t',
        };

        // symbol classification into types
        enum class SymbolType {
            null = 0,
            val,
            opr,
            open,
            close,
            period,
            blank,
        };

        // operator symbol associativity
        enum class Associativity {
            left,
            right
        };

        // get the symbol's type
        inline SymbolType type(Symbol sym) {
            using enum Symbol;

            switch (sym) {
                case v0:
                case v1:
                case v2:
                case v3:
                case v4:
                case v5:
                case v6:
                case v7:
                case v8:
                case v9:
                    return SymbolType::val;
                case plus:
                case minus:
                case mul:
                case div:
                case exp:
                case mod:
                    return SymbolType::opr;
                case open:
                    return SymbolType::open;
                case close:
                    return SymbolType::close;
                case period:
                    return SymbolType::period;
                case space:
                case newline:
                case tab:
                    return SymbolType::blank;
                default:
                    abort();
            }
        }

        // get the operator symbol's precedence
        inline int prec(Symbol op) {
            using enum Symbol;

            switch (op) {
                case exp:
                    return 1;
                case mul:
                case div:
                case mod:
                    return 2;
                case plus:
                case minus:
                    return 3;
                default:
                    abort();
            }
        }

        // get the operator symbol's associativity
        inline Associativity associa(Symbol op) {
            using enum Symbol;

            switch (op) {
                case exp:
                    return Associativity::right;
                case plus:
                case minus:
                case mul:
                case div:
                case mod:
                    return Associativity::left;
                default:
                    abort();
            }
        }

        // An object that finds and stores the starting and ending addresses of "parts" of the expression.
        // A part, in this case, is either an operator (e.g. +, -, *) or a value (e.g. 15, 5.11321).
        // There are two constructors: one for values and one for operators.
        // In general, the point of this object is to avoid making string copies and to combine operators and values into one type (so that you can make a vector of that type).
        // Note: the ending address data member, `end`, points to one past the last char of the operator/value.
        struct Part {
            const char* start = nullptr;
            const char* end   = nullptr;
            Symbol symbol     = Symbol::null;

            // Value constructor; handles both integer and floating-point values.
            // This constructor modifies the value of the `end` argument: `end` will point to the last char of the value (NOT one-past-the-last).
            Part(const std::string& s, int start, int& end) {
                bool notFloatingPoint = true;

                while (end < s.size()) {
                    switch (type(static_cast<Symbol>(s[end]))) {
                        case SymbolType::val:
                            end++;
                            break;
                        case SymbolType::period:
                            if (notFloatingPoint) {
                                end++;
                                notFloatingPoint = false;
                                break;
                            }

                            abort(); // don't allow more than one period in a floating-point value
                        default:
                            goto loopEnd;
                    }
                }

            loopEnd:

                this->start  = s.data() + start * sizeof(char);
                this->end    = s.data() + end * sizeof(char);
                this->symbol = static_cast<Symbol>(*this->start);

                end--; // adjust to point to the last char of the value; this is required by the main loop in the `rpn()` function
            }

            // operator constructor; since operators are only ever a single character, there is no loop here
            Part(const std::string& s, int start)
                : start(s.data() + start * sizeof(char)),
                  end(s.data() + (start + 1) * sizeof(char)),
                  symbol(static_cast<Symbol>(*this->start)) {}

            // debug print function
            void print() const {
                assert(start && end);

                switch (type(symbol)) {
                    case SymbolType::val: {
                        double val;
                        std::from_chars(start, end, val);
                        std::cout << val << " ";
                        return;
                    }
                    case SymbolType::opr:
                    case SymbolType::open:
                    case SymbolType::close: {
                        std::cout << *start << " ";
                        return;
                    }
                    default:
                        abort();
                }
            }

            // computes the value between `start` and `end` and returns it
            double computeVal() {
                double result;
                std::from_chars(start, end, result);
                return result;
            }

            // stores the result between `start` and `end` in `result` out-argument
            void computeVal(double& result) {
                std::from_chars(start, end, result);
            }
        };

        std::ostream& operator<<(std::ostream& os, Symbol s) {
            os << static_cast<char>(s);
            return os;
        }

        inline void print(const std::vector<Part>& a) {
            for (const Part& part : a) {
                part.print();
            }
            std::cout << '\n';
        }

        inline void print(const std::string& a) {
            for (auto x : a) {
                std::cout << x;
            }
            std::cout << '\n';
        }

        template <typename T>
        inline void print(const std::vector<T>& a) {
            for (auto x : a) {
                std::cout << x << ' ';
            }
            std::cout << '\n';
        }

        inline void applyOperator(double& a, const double& b, Symbol opr) {
            switch (opr) {
                using enum Symbol;

                case plus:
                    a += b;
                    return;
                case minus:
                    a -= b;
                    return;
                case mul:
                    a *= b;
                    return;
                case div:
                    a /= b;
                    return;
                case mod:
                    a = fmod(a, b);
                    return;
                case exp:
                    a = pow(a, b);
                    return;
                default:
                    abort();
            }
        }
    } // namespace

    /*
    This function converts an infix expression to Reverse Polish Notation (postfix) via the shunting yard algorithm.

    This is an adaptation of the Wikipedia description of the shunting yard algorithm found here:

        https://en.wikipedia.org/wiki/Shunting_yard_algorithm

    The text within this Wikipedia article is licensed under Creative Commons Attribution-ShareAlike License 3.0:

        https://creativecommons.org/licenses/by-sa/3.0/

    One requirement listed by the "human-readable summary" of this license states:

        "If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original."

    Consequently, the code in this rpn() function is also licensed under Creative Commons Attribution-ShareAlike License 3.0.
    */
    void rpn(const std::string& s, std::vector<Part>& expr) {
        std::vector<Part> oprs; // operator stack

        for (int i = 0; i < s.size(); i++) {
            const Symbol& sym = static_cast<Symbol>(s[i]);

            switch (type(sym)) {
                case SymbolType::val:
                    expr.emplace_back(Part(s, i, i));
                    break;
                case SymbolType::opr:
                    while (!oprs.empty() && oprs.back().symbol != Symbol::open &&
                           (prec(oprs.back().symbol) < prec(sym) ||
                            (associa(sym) == Associativity::left) && prec(oprs.back().symbol) == prec(sym))) {
                        expr.emplace_back(oprs.back());
                        oprs.pop_back();
                    }
                    oprs.emplace_back(Part(s, i));
                    break;
                case SymbolType::open:
                    oprs.emplace_back(Part(s, i));
                    break;
                case SymbolType::close:
                    while (!oprs.empty() && oprs.back().symbol != Symbol::open) {
                        expr.emplace_back(oprs.back());
                        oprs.pop_back();
                    }
                    assert(!oprs.empty());
                    assert(oprs.back().symbol == Symbol::open);
                    oprs.pop_back();
                    break;
                case SymbolType::blank:
                    break;
                default:
                    abort();
            }
        }

        while (!oprs.empty()) {
            assert(oprs.back().symbol != Symbol::open);
            expr.emplace_back(oprs.back());
            oprs.pop_back();
        }
    }

    // `s` is a string that contains the infix expression to be evaluated
    double eval(const std::string& s) {
        std::vector<Part> expr; // will contain the RPN conversion of the string `s` infix expression

        rpn(s, expr); // convert `s` to Reverse Polish Notation (postfix) and store in `expr`

        std::vector<double> evld; // used to evaluate and store the RPN expression

        // sequentially evaluate the parts of the RPN expression and store the result as the first (and only) value in the `evld` vector
        for (Part& p : expr) {
            switch (type(p.symbol)) {
                case SymbolType::val:
                    evld.emplace_back(p.computeVal());
                    break;
                case SymbolType::opr:
                    applyOperator(evld[evld.size() - 2], evld.back(), p.symbol);
                    evld.pop_back();
                    expr.pop_back();
                    break;
                default:
                    abort();
            }
        }

        // `evld` should contain only one value: the result
        assert(evld.size() == 1);

        return evld.back();
    }
}; // namespace Calculator

int main() {
    std::cout.precision(100);

    {
        auto test = [&](std::string infixExpr, double expected, double error) {
            std::cout << "infix:\t\t";
            Calculator::print(infixExpr);

            std::cout << "rpn:\t\t";
            std::vector<Calculator::Part> rpnExpr;
            Calculator::rpn(infixExpr, rpnExpr);
            Calculator::print(rpnExpr);

            double r = Calculator::eval(infixExpr);
            std::cout << "result:\t\t" << r << '\n';

            assert(r >= expected - error && r <= expected + error);

            std::cout << "----\n";
        };

        double error = 0.0000000001;

        // https://www.wolframalpha.com/input?i=3+%2B+4+*+2+%2F+%281+-+5%29+%5E+2+%5E+3
        test("3 + 4 * 2 / (1 - 5) ^ 2 ^ 3", 3.0001220703125, error);

        // https://www.wolframalpha.com/input?i=3+%2B+4+*+2+%2F+%28+1+-+5+%29+%5E+2+%5E+3
        test("10 * 15 / 23 / (512 * 13 ^ 2 ^ 2 / 13 ^ 2) * 3213 + 1 * 2 - 11 + 10", 1.2421684059042963725237972729611525598147671726267043992796501157, error);

        // https://www.wolframalpha.com/input?i=10.321+*+15.12451+%2F+23.1231+%2F+%28512.5643+*+13.345+%5E+2.3123+%5E+2+%2F+13+%5E+2%29+*+3213.42+%2B+1+*+2+-+11+%2B+10
        test("10.321 * 15.12451 / 23.1231 / (512.5643 * 13.345 ^ 2.3123 ^ 2 / 13 ^ 2) * 3213.42 + 1 * 2 - 11 + 10", 1.0068815587795003943699518459476085786540847766316463805172, error);

        // https://www.wolframalpha.com/input?i=0+-+10.321+*+15.12451+%2F+23.1231+%2F+%28512.5643+*+13.345+%5E+2.3123+%5E+2+%2F+13+%5E+2%29+*+3213.42+%2B+1+*+2+-+11+%2B+10
        test("0 - 10.321 * 15.12451 / 23.1231 / (512.5643 * 13.345 ^ 2.3123 ^ 2 / 13 ^ 2) * 3213.42 + 1 * 2 - 11 + 10", 0.9931184412204996056300481540523914213459152233683536194827, error);

        // https://www.wolframalpha.com/input?i=542+%2F+122+%2B+%283+%2B+4%29+*+3+-+4+%5E+3+%5E+1.123
        test("542 / 122 + (3 + 4) * 3 - 4 ^ 3 ^ 1.123", -91.37456685970892539418662159436692907944381008665673239539, error);

        /*
        WolframAlpha seems to give division a higher priority compared to mod, which is different from:

            https://en.cppreference.com/w/cpp/language/operator_precedence

        So, the below WolframAlpha equation requires extra parentheses for the initial "542 % 15.515".

            https://www.wolframalpha.com/input?i=%28542+mod+15.515%29+%2F+%28122+mod+2+%5E+%281.5+%2F+1.25%29%29+%2B+%283+%2B+4+*+11.111111%29+*+3+-+4+%5E+3+%5E+1.123
        */
        test("542 % 15.515 / (122 % 2 ^ (1.5 / 1.25)) + (3 + 4 * 11.111111) * 3 - 4 ^ 3 ^ 1.123", 86.405052120668061919918225856697354813352563787666758760999, error);
    }
};