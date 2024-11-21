#include <iostream>
#include <string>
#include <regex>
#include <stack>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <sstream>

class Calculator {
private:
    const std::regex NUMBER_PATTERN{"\\d+(\\.\\d+)?"};
    const std::regex OPERATOR_PATTERN{"[+\\-*/^]"};
    const std::regex PARENTHESIS_PATTERN{"[(){}]"};
    const std::regex WHITESPACE_PATTERN{"\\s+"};
    const std::regex VALID_EXPRESSION{
        "^[\\s]*([(){}]|\\d+(\\.\\d+)?|[+\\-*/^])+[\\s]*$"
    };

    std::vector<std::string> steps;

    std::string replaceInExpression(const std::string& expr, size_t start, size_t length, const std::string& replacement) {
        std::string result = expr;
        result.replace(start, length, replacement);
        return result;
    }

    void addStep(const std::string& step) {
        // Only add the step if it's different from the last one
        if (steps.empty() || steps.back() != step) {
            steps.push_back(step);
        }
    }

    bool isOperator(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
    }

    int getPrecedence(char op) {
        if (op == '^') return 3;
        if (op == '*' || op == '/') return 2;
        if (op == '+' || op == '-') return 1;
        return 0;
    }

    std::string formatNumber(double num) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << num;
        std::string str = ss.str();
        // Remove trailing zeros and decimal point if not needed
        if (str.find('.') != std::string::npos) {
            str = str.substr(0, str.find_last_not_of('0') + 1);
            if (str.back() == '.') {
                str = str.substr(0, str.size() - 1);
            }
        }
        return str;
    }

    std::string evaluateSubExpression(const std::string& expr, size_t start, size_t end) {
        std::string subExpr = expr.substr(start, end - start + 1);
        std::stack<double> values;
        std::stack<char> ops;
        std::string currentNum;

        for (size_t i = 0; i < subExpr.length(); i++) {
            char c = subExpr[i];
            if (std::isdigit(c) || c == '.') {
                currentNum += c;
            } else {
                if (!currentNum.empty()) {
                    values.push(std::stod(currentNum));
                    currentNum.clear();
                }
                if (isOperator(c)) {
                    while (!ops.empty() && getPrecedence(ops.top()) >= getPrecedence(c)) {
                        double b = values.top(); values.pop();
                        double a = values.top(); values.pop();
                        char op = ops.top(); ops.pop();
                        values.push(applyOperation(a, b, op));
                        
                        // Add intermediate step
                        std::string step = formatNumber(a) + " " + op + " " + formatNumber(b) + " = " + formatNumber(values.top());
                        addStep(step);
                    }
                    ops.push(c);
                }
            }
        }

        if (!currentNum.empty()) {
            values.push(std::stod(currentNum));
        }

        while (!ops.empty()) {
            double b = values.top(); values.pop();
            double a = values.top(); values.pop();
            char op = ops.top(); ops.pop();
            values.push(applyOperation(a, b, op));
            
            // Add intermediate step
            std::string step = formatNumber(a) + " " + op + " " + formatNumber(b) + " = " + formatNumber(values.top());
            addStep(step);
        }

        return formatNumber(values.top());
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
    void printRegexMatches(const std::string& expression) {
        std::cout << "\nRegex Pattern Matches:" << std::endl;
        
        std::cout << "Numbers found:" << std::endl;
        auto numbers_begin = std::sregex_iterator(expression.begin(), expression.end(), NUMBER_PATTERN);
        auto numbers_end = std::sregex_iterator();
        for (std::sregex_iterator i = numbers_begin; i != numbers_end; ++i) {
            std::cout << "  - " << i->str() << std::endl;
        }

        std::cout << "Operators found:" << std::endl;
        auto operators_begin = std::sregex_iterator(expression.begin(), expression.end(), OPERATOR_PATTERN);
        auto operators_end = std::sregex_iterator();
        for (std::sregex_iterator i = operators_begin; i != operators_end; ++i) {
            std::cout << "  - " << i->str() << std::endl;
        }

        std::cout << "Parentheses/Braces found:" << std::endl;
        auto parens_begin = std::sregex_iterator(expression.begin(), expression.end(), PARENTHESIS_PATTERN);
        auto parens_end = std::sregex_iterator();
        for (std::sregex_iterator i = parens_begin; i != parens_end; ++i) {
            std::cout << " " << i->str() << " ";
        }
        std::cout << std::endl;
    }

    void validateExpression(const std::string& expression) {
        if (!std::regex_match(expression, VALID_EXPRESSION)) {
            throw std::invalid_argument("Invalid expression format");
        }

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

    double evaluate(const std::string& expression) {
        steps.clear();
        std::string currentExpr = expression;
        addStep(currentExpr);
        validateExpression(expression);
        printRegexMatches(expression);

        // Keep evaluating until we have a single number
        while (true) {
            // Find innermost parentheses
            size_t openPos = currentExpr.find_last_of("({");
            if (openPos == std::string::npos) {
                // No more parentheses, evaluate the remaining expression
                if (std::regex_search(currentExpr, OPERATOR_PATTERN)) {
                    std::string result = evaluateSubExpression(currentExpr, 0, currentExpr.length() - 1);
                    addStep(result);
                }
                break;
            }

            // Find matching closing parenthesis
            size_t closePos = currentExpr.find_first_of(")}", openPos);
            if (closePos == std::string::npos) {
                throw std::invalid_argument("Mismatched parentheses");
            }

            // Evaluate the subexpression
            std::string subExprResult = evaluateSubExpression(currentExpr, openPos + 1, closePos - 1);
            
            // Replace the parentheses and their contents with the result
            currentExpr = replaceInExpression(currentExpr, openPos, closePos - openPos + 1, subExprResult);
            addStep(currentExpr);
        }

        return std::stod(steps.back());
    }

    void printSteps() {
        std::cout << "\nEvaluation Steps:" << std::endl;
        for (size_t i = 0; i < steps.size(); i++) {
            std::cout << i + 1 << ". " << steps[i] << std::endl;
        }
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
            std::cout << "\nExpression: " << expression << std::endl;
            double result = calc.evaluate(expression);
            calc.printSteps();
            std::cout << "\nResult: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}