This is a relatively simple calculator:
- It should correctly evaluate an infix string expression like "`10 * 15 / 23 / (512 * 13 ^ 2 ^ 2 / 13 ^ 2) * 3213 + 1 * 2 - 11 + 10`" as ~`1.242`.
    - To evaluate a string expression, `s`, call `Calculator::eval(const std::string& s)` and store its return value.
    - In order to evaluate infix string expressions, they are first converted to Reverse Polish Notation (postfix) expressions via an adaptation of the [Wikipedia description of the shunting yard algorithm](https://en.wikipedia.org/wiki/Shunting_yard_algorithm).
- It doesn't handle functions like sin, cos, etc.
- Symbols only consist of single chars, e.g. `*`, `+`, `%`.
- It doesn't handle the unary negation operator, e.g. `-10`; must use `0 - 10` or equivalent.
- You cannot have more than one operator between a pair of values, e.g. `5 -- 10` is invalid.
- Whitespace does not affect the result.
- It's entirely possible to create an infinite loop or some other undesirable behavior via malformed input.