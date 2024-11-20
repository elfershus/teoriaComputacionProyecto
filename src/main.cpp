#include <iostream>
#include <string>
#include <regex>
#include <stack>
#include <cmath>
#include <stdexcept>

class Calculator {
private:
    // Regex patterns
    const std::regex NUMBER_PATTERN{"\\d+(\\.\\d+)?"};  // Matches integers and decimals
    const std::regex OPERATOR_PATTERN{"[+\\-*/^]"};     // Matches basic operators
    const std::regex PARENTHESIS_PATTERN{"[(){}]"};     // Matches parentheses and braces
    const std::regex WHITESPACE_PATTERN{"\\s+"};        // Matches whitespace
    const std::regex VALID_EXPRESSION{
        "^[\\s]*([(){}]|\\d+(\\.\\d+)?|[+\\-*/^])+[\\s]*$"  // Matches valid expression structure
    };

    bool isOperator(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
    }

    int getPrecedence(char op) {
        if (op == '^') return 3;
        if (op == '*' || op == '/') return 2;
        if (op == '+' || op == '-') return 1;
        return 0;
    }

    double applyOperation(double a, double b, char op) {
        switch (op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': 
                if (b == 0) throw std::runtime_error("Division by zero");
                return a / b;
            case '^': return std::pow(a, b);
            default: throw std::runtime_error("Invalid operator");
        }
    }

    bool isMatchingPair(char opening, char closing) {
        return (opening == '(' && closing == ')') ||
               (opening == '{' && closing == '}');
    }

public:
    void validateExpression(const std::string& expression) {
        // Check if expression matches valid pattern
        if (!std::regex_match(expression, VALID_EXPRESSION)) {
            throw std::invalid_argument("Invalid expression format");
        }

        // Check brackets balance
        std::stack<char> brackets;
        for (char c : expression) {
            if (c == '(' || c == '{') {
                brackets.push(c);
            } else if (c == ')' || c == '}') {
                if (brackets.empty() || !isMatchingPair(brackets.top(), c)) {
                    throw std::invalid_argument("Mismatched brackets");
                }
                brackets.pop();
            }
        }
        if (!brackets.empty()) {
            throw std::invalid_argument("Unclosed brackets");
        }
    }

    double evaluate(std::string expression) {
        validateExpression(expression);

        std::stack<double> values;
        std::stack<char> operators;

        for (size_t i = 0; i < expression.length(); i++) {
            char c = expression[i];

            if (std::isspace(c)) continue;

            if (c == '(' || c == '{') {
                operators.push(c);
            }
            else if (std::isdigit(c)) {
                std::string num;
                while (i < expression.length() && 
                      (std::isdigit(expression[i]) || expression[i] == '.')) {
                    num += expression[i++];
                }
                i--;
                values.push(std::stod(num));
            }
            else if (c == ')' || c == '}') {
                while (!operators.empty() && operators.top() != '(' && operators.top() != '{') {
                    double val2 = values.top(); values.pop();
                    double val1 = values.top(); values.pop();
                    char op = operators.top(); operators.pop();
                    values.push(applyOperation(val1, val2, op));
                }
                if (!operators.empty()) operators.pop();
            }
            else if (isOperator(c)) {
                while (!operators.empty() && operators.top() != '(' && operators.top() != '{' &&
                       getPrecedence(operators.top()) >= getPrecedence(c)) {
                    double val2 = values.top(); values.pop();
                    double val1 = values.top(); values.pop();
                    char op = operators.top(); operators.pop();
                    values.push(applyOperation(val1, val2, op));
                }
                operators.push(c);
            }
        }

        while (!operators.empty()) {
            double val2 = values.top(); values.pop();
            double val1 = values.top(); values.pop();
            char op = operators.top(); operators.pop();
            values.push(applyOperation(val1, val2, op));
        }

        return values.top();
    }
};

int main() {
    Calculator calc;

    while (true) {
        std::cout << "\nEnter an expression (or 'q' to quit): ";
        std::string expression;
        std::getline(std::cin, expression);

        if (expression == "q" || expression == "Q") {
            break;
        }

        try {
            double result = calc.evaluate(expression);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}