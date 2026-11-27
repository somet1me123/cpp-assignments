#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Константы для типов токенов
constexpr char kQuit = 'q';
constexpr char kPrint = ';';
constexpr char kNumber = '8';
constexpr char kName = 'a';
constexpr char kLet = 'L';

const std::string kPrompt = "> ";
const std::string kResult = "= ";
const std::string kDeclKey = "let";
const std::string kQuitKey = "quit";

// ---------- Токен ----------
struct Token
{
    char kind;
    long long value;
    std::string name;

    Token(char ch) : kind{ch}, value{0} {}
    Token(char ch, long long val) : kind{ch}, value{val} {}
    Token(char ch, std::string id) : kind{ch}, value{0}, name{id} {}
};

// ---------- Переменная ----------
struct Variable
{
    std::string name;
    long long value;
};

// ---------- Поток токенов ----------
class TokenStream
{
private:
    bool full = false;
    Token buffer{'\0'};

public:
    void putback(Token t)
    {
        if (full)
        {
            throw std::runtime_error("Буфер токенов уже заполнен");
        }
        buffer = t;
        full = true;
    }

    void ignore(char c)
    {
        if (full && buffer.kind == c)
        {
            full = false;
            return;
        }
        full = false;

        char ch;
        while (std::cin >> ch)
        {
            if (ch == c)
                return;
        }
    }

    Token get()
    {
        if (full)
        {
            full = false;
            return buffer;
        }

        char ch;
        if (!(std::cin >> ch))
        {
            return Token{kQuit};
        }

        switch (ch)
        {
        case '(':
        case ')':
        case '=':
        case '&':
        case '|':
        case '^':
        case '~':
        case '-':
        case '+':
        case kPrint:
            return Token{ch};

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            std::cin.putback(ch);
            long long val;
            std::cin >> val;
            return Token{kNumber, val};
        }

        default:
            if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_')
            {
                std::string s;
                s += ch;
                while (std::cin.get(ch) &&
                       (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_'))
                {
                    s += ch;
                }
                if (std::cin)
                {
                    std::cin.putback(ch);
                }

                if (s == kDeclKey)
                    return Token{kLet};
                if (s == kQuitKey)
                    return Token{kQuit};
                return Token{kName, s};
            }
            throw std::runtime_error("Некорректный символ во вводе");
        }
    }
};

// ---------- Калькулятор ----------
class BitwiseCalculator
{
private:
    TokenStream ts;
    std::vector<Variable> vars;

    bool has_var(const std::string &name)
    {
        for (const Variable &v : vars)
        {
            if (v.name == name)
                return true;
        }
        return false;
    }

    long long get_var(const std::string &name)
    {
        for (const Variable &v : vars)
        {
            if (v.name == name)
                return v.value;
        }
        throw std::runtime_error("Неизвестная переменная: " + name);
    }

    void add_var(const std::string &name, long long value)
    {
        if (has_var(name))
        {
            throw std::runtime_error("Переменная '" + name + "' уже объявлена");
        }
        vars.push_back(Variable{name, value});
    }

    // Приоритет 1 (самый высокий): числа, переменные, скобки, ~, -, +
    long long primary()
    {
        Token t = ts.get();
        switch (t.kind)
        {
        case '(':
        {
            long long d = expr_or();
            t = ts.get();
            if (t.kind != ')')
            {
                throw std::runtime_error("Ожидалась ')'");
            }
            return d;
        }
        case kNumber:
            return t.value;
        case kName:
            return get_var(t.name);
        case '~':
            return ~primary();
        case '-':
            return -primary();
        case '+':
            return +primary();
        default:
            throw std::runtime_error("Ожидалось число, переменная или '('");
        }
    }

    // Приоритет 2: побитовое И (&)
    long long expr_and()
    {
        long long left = primary();
        while (true)
        {
            Token t = ts.get();
            if (t.kind == '&')
            {
                left &= primary();
            }
            else
            {
                ts.putback(t);
                return left;
            }
        }
    }

    // Приоритет 3: побитовое исключающее ИЛИ (^)
    long long expr_xor()
    {
        long long left = expr_and();
        while (true)
        {
            Token t = ts.get();
            if (t.kind == '^')
            {
                left ^= expr_and();
            }
            else
            {
                ts.putback(t);
                return left;
            }
        }
    }

    // Приоритет 4 (самый низкий): побитовое ИЛИ (|)
    long long expr_or()
    {
        long long left = expr_xor();
        while (true)
        {
            Token t = ts.get();
            if (t.kind == '|')
            {
                left |= expr_xor();
            }
            else
            {
                ts.putback(t);
                return left;
            }
        }
    }

    // Объявление переменной: let x = ...
    long long declaration()
    {
        Token t = ts.get();
        if (t.kind != kName)
        {
            throw std::runtime_error("Ожидалось имя переменной после 'let'");
        }

        std::string var_name = t.name;
        if (has_var(var_name))
        {
            throw std::runtime_error("Переменная '" + var_name + "' уже объявлена");
        }

        t = ts.get();
        if (t.kind != '=')
        {
            throw std::runtime_error("Ожидался знак '=' в объявлении " + var_name);
        }

        long long val = expr_or();
        add_var(var_name, val);
        return val;
    }

    // Инструкция: либо объявление, либо выражение
    long long statement()
    {
        Token t = ts.get();
        if (t.kind == kLet)
        {
            return declaration();
        }
        ts.putback(t);
        return expr_or();
    }

public:
    BitwiseCalculator()
    {
        add_var("true", 1);
        add_var("false", 0);
    }

    void run()
    {
        while (true)
        {
            try
            {
                std::cout << kPrompt;
                Token t = ts.get();
                while (t.kind == kPrint)
                {
                    t = ts.get();
                }
                if (t.kind == kQuit)
                {
                    return;
                }

                ts.putback(t);
                long long val = statement();

                Token next = ts.get();
                if (next.kind == kPrint)
                {
                    std::cout << kResult << val << '\n';
                }
                else
                {
                    ts.putback(next);
                    throw std::runtime_error("Ожидалась ';' в конце выражения");
                }
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Ошибка: " << e.what() << '\n';
                ts.ignore(kPrint);
            }
        }
    }
};

int main()
{
    BitwiseCalculator calc;
    calc.run();
    return 0;
}