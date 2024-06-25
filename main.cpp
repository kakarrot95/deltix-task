#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>
#include <chrono>
#include <algorithm>
#include <limits>
#include <iomanip>

struct MarketData
{
    std::chrono::system_clock::time_point timestamp;
    long double price;
};

struct UserData
{
    std::string user_id;
    std::string currency;
    std::chrono::system_clock::time_point timestamp;
    long double delta;
};

struct BarData
{
    long double min_balance = std::numeric_limits<long double>::infinity();
    long double max_balance = -std::numeric_limits<long double>::infinity();
    long double sum_balance = 0.0;
    int count = 0;
};

std::unordered_map<std::string, std::vector<MarketData>> market_prices;

const std::unordered_map<std::string, int> bars = {
    {"1h", 3600},
    {"1d", 86400},
    {"30d", 2592000}};

std::chrono::system_clock::time_point parse_timestamp(const std::string &timestamp)
{
    std::time_t time = std::stoll(timestamp);
    return std::chrono::system_clock::from_time_t(time);
}

using OutputData = std::unordered_map<std::string, std::map<std::time_t, std::unordered_map<std::string, BarData>>>;

/*
**  Reads CSV file provided as input to the function
**  And writes the data to the global market_prices
*/
void read_market_data(const std::string &filename)
{
    std::ifstream file(filename);
    std::string line, symbol, timestamp, price;

    std::getline(file, line);

    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::getline(ss, symbol, ',');
        std::getline(ss, timestamp, ',');
        std::getline(ss, price, ',');

        market_prices[symbol].push_back({parse_timestamp(timestamp), std::stold(price)});
    }
}

/*
** Reads CSV file provided as input to the function
** And writes the data to the user_data provided by refrence
** as the second argument to the function
*/
void read_user_data(const std::string &filename, std::vector<UserData> &user_data)
{
    std::ifstream file(filename);
    std::string line, user_id, currency, timestamp, delta;

    std::getline(file, line);

    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::getline(ss, user_id, ',');
        std::getline(ss, currency, ',');
        std::getline(ss, timestamp, ',');
        std::getline(ss, delta, ',');

        user_data.push_back({user_id, currency, parse_timestamp(timestamp), std::stold(delta)});
    }
}

/*
** Returns the price of the symbol in USD
** at the given timestamp.
** As the data is sorted by timestamp
** we can use binary search to find the price.
*/
long double get_price(const std::string &symbol, const std::chrono::system_clock::time_point &timestamp)
{
    const auto &prices = market_prices[symbol];
    auto it = std::upper_bound(prices.begin(), prices.end(), timestamp,
                               [](const std::chrono::system_clock::time_point &time, const MarketData &data)
                               {
                                   return time < data.timestamp;
                               });

    if (it == prices.begin())
        return prices.front().price;
    --it;
    return it->price;
}

OutputData process(const std::vector<UserData> &user_data)
{
    OutputData output_data;

    std::unordered_map<std::string, std::unordered_map<std::string, long double>> balances;
    /* balances[id][X] = amount of currency, X
    ** that the user with user_id = id , has till now .
    */

    std::unordered_map<std::string, long double> balances_usd;
    /* balances_usd[id] = total amount of USD
    ** that the user with user_id = id , has till now .
    */

    for (const auto &row : user_data)
    {
        std::string user_id = row.user_id;
        std::string currency = row.currency;
        auto timestamp = row.timestamp;
        long double delta = row.delta;

        // Update balance
        long double old_balance;
        long double new_balance;

        {
            old_balance = balances[user_id][currency];
            new_balance = old_balance + delta;
            balances[user_id][currency] = new_balance;
            /*  updating the ammount of currency
            **  that the user with user_id has.
            */
        }

        // Update USD balance
        long double old_balance_usd = (currency != "USD") ? old_balance * get_price(currency + "USD", timestamp) : old_balance;
        long double new_balance_usd = (currency != "USD") ? new_balance * get_price(currency + "USD", timestamp) : new_balance;

        {
            balances_usd[user_id] += (new_balance_usd - old_balance_usd);
        }

        for (const auto &[bar_name, bar_length] : bars)
        {
            auto bar_start = std::chrono::system_clock::to_time_t(timestamp) / bar_length * bar_length;

            auto &bar_data = output_data[bar_name][bar_start][user_id];

            bar_data.min_balance = std::min(bar_data.min_balance, balances_usd[user_id]);
            bar_data.max_balance = std::max(bar_data.max_balance, balances_usd[user_id]);
            bar_data.sum_balance += balances_usd[user_id];
            bar_data.count += 1;
        }
    }
    return output_data;
}

/* Makes the output .csv files
 */
void write_output_files(const OutputData &output_data)
{
    for (const auto &[bar_name, user_data] : output_data)
    {
        std::ofstream file("bars-" + bar_name + ".csv");
        file << "user_id,minimum_balance,maximum_balance,average_balance,start_timestamp\n";

        for (const auto &[start_timestamp, user_bars] : user_data)
        {
            for (const auto &[user_id, data] : user_bars)
            {
                long double avg_balance;
                {
                    avg_balance = data.sum_balance / data.count;
                }
                file << user_id << ',' << data.min_balance << ',' << data.max_balance << ','
                     << std::fixed << std::setprecision(4) << avg_balance << ',' << start_timestamp << '\n';
            }
        }
    }
}

int main()
{
    auto start = std::chrono::high_resolution_clock::now();

    read_market_data("market_data.csv");

    std::vector<UserData> user_data;
    read_user_data("user_data.csv", user_data);

    auto output_data = process(user_data);

    write_output_files(output_data);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long double> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << "s\n";

    return 0;
}
