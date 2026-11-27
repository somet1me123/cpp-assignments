#include <iostream>
#include <ctime>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include <algorithm>
#include <random>
#include <map>

constexpr int bot_win = 2;
constexpr int user_win = 1;

struct num
{
    std::array<int, 4> full{};

    num() = default;

    num(int a)
    {
        update(a);
    }

    void update(int a)
    {
        for (int i{0}; i < 4; ++i)
        {
            full[i] = a / static_cast<int>(pow(10, full.size() - 1 - i)) % 10;
        }
    }

    bool validate()
    {
        for (int i{0}; i < 4; ++i)
        {
            for (int j{i + 1}; j < 4; ++j)
            {
                if (full[i] == full[j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    std::string toString()
    {
        std::string s;
        for (int i{0}; i < 4; ++i)
        {
            s += std::to_string(full[i]);
        }
        return s;
    }

    bool operator==(const num &other) const
    {
        for (int i{0}; i < 4; ++i)
        {
            if (full[i] != other.full[i])
            {
                return false;
            }
        }
        return true;
    }
};

int get_random_number()
{
    constexpr unsigned short temp = 9999 + 1;
    return rand() % temp;
}

void countCoeffs(std::array<int, 2> &coeffs, num refNum, num compNum);
// алгоритм Кнута
struct solver
{
    std::vector<num> candidates;
    num lastGuess;

    solver()
    {
        for (int i = 1023; i <= 9876; ++i)
        {
            num temp(i);
            if (temp.validate())
            {
                candidates.push_back(temp);
            }
        }

        lastGuess = num(1234);
    }

    std::string do_prediction()
    {
        if (candidates.empty())
            return "0000";

        if (candidates.size() <= 2)
        {
            lastGuess = candidates.front();
            return lastGuess.toString();
        }

        if (candidates.size() == 4536)
        {
            lastGuess = num(1234);
            return lastGuess.toString();
        }

        int best_max_group = 10000;
        num best_candidate = candidates.front();

        size_t search_limit = std::min(candidates.size(), size_t(60));

        for (size_t i = 0; i < search_limit; ++i)
        {
            const auto &guess = candidates[i];
            std::map<std::pair<int, int>, int> score_counts;
            std::array<int, 2> temp_coeffs;

            for (const auto &target : candidates)
            {
                countCoeffs(temp_coeffs, guess, target);
                score_counts[{temp_coeffs[0], temp_coeffs[1]}]++;
            }

            int current_max = 0;
            for (const auto &pair : score_counts)
            {
                if (pair.second > current_max)
                {
                    current_max = pair.second;
                }
            }

            if (current_max < best_max_group)
            {
                best_max_group = current_max;
                best_candidate = guess;
            }
        }

        lastGuess = best_candidate;
        return lastGuess.toString();
    }

    void filter_candidates(int bulls, int cows)
    {
        std::vector<num> next_candidates;
        std::array<int, 2> sim_coeffs;

        for (const auto &candidate : candidates)
        {
            countCoeffs(sim_coeffs, lastGuess, candidate);

            if (sim_coeffs[0] == bulls && sim_coeffs[1] == cows)
            {
                next_candidates.push_back(candidate);
            }
        }

        candidates = std::move(next_candidates);
    }
};

void countCoeffs(std::array<int, 2> &coeffs, num refNum, num compNum)
{
    coeffs[0] = 0;
    coeffs[1] = 0;

    for (int i{0}; i < 4; ++i)
    {
        for (int j{0}; j < 4; ++j)
        {
            if (refNum.full[i] == compNum.full[j])
            {
                if (i == j)
                {
                    ++coeffs[0];
                }
                else
                {
                    ++coeffs[1];
                }
            }
        }
    }
}

void game()
{
    int gameIsOver{0}; // 0 - in process, 1 - end game
    std::array<int, 2> coeffs{0, 0};

    solver bot;

    num botNum{get_random_number()};
    while (!botNum.validate())
    {
        botNum.update(get_random_number());
    }

    std::cout << "Bulls and cows game." << std::endl;

    int moveNum = 1;

    while (!gameIsOver)
    {
        std::cout << "Move " << moveNum << ". Your move: ";

        unsigned int userGuessNumRaw{0};

        if (!(std::cin >> userGuessNumRaw))
        {
            std::cout << "Not valide number. Please Again!" << std::endl;
            std::cin.clear();
            while (std::cin.get() != '\n')
                ;
            continue;
        }

        if (userGuessNumRaw > 9999 || userGuessNumRaw < 1000)
        {
            std::cout << "Not correct number, must be in range[1000, 9999]. Please again!\n";
            continue;
        }

        num userGuessNum{static_cast<int>(userGuessNumRaw)};

        countCoeffs(coeffs, botNum, userGuessNum);

        ++moveNum;

        if (coeffs[0] == 4)
        {
            gameIsOver = user_win;
            std::cout << "User Win!!!" << std::endl;
        }

        if (gameIsOver != user_win)
        {
            std::cout << "Bulls: " << coeffs[0] << " Cows: " << coeffs[1] << std::endl;
        }

        std::cout << "My move. Is it " << bot.do_prediction() << "? (y/n): ";
        while (true)
        {
            std::string t;
            std::cin >> t;

            if (t == "y")
            {
                gameIsOver = bot_win;
                std::cout << "Bot win!!!" << std::endl;
                return;
            }
            if (t == "n")
            {
                break;
            }
            std::cout << "Invalid message, please again. (y/n): ";
        }

        if (gameIsOver != bot_win)
        {
            if (gameIsOver == user_win)
            {
                std::cout << "User win!!!\n";
                return;
            }

            std::array<int, 2> botCoeffs;
            std::cout << "Bulls: ";
            std::cin >> botCoeffs[0];
            std::cout << "Cows: ";
            std::cin >> botCoeffs[1];

            bot.filter_candidates(botCoeffs[0], botCoeffs[1]);

            if (bot.candidates.empty())
            {
                std::cout << "Error: Looks like you made a mistake in your bull/cow counts somewhere!" << std::endl;
                return;
            }

            ++moveNum;
        }
    }
}

int main()
{
    srand(static_cast<unsigned int>(time(0)));

    game();

    return 0;
}