/**
 * -------------------------------------------------------------------------- *
 *                     Copyright (c) by OopsieLogsy 2025                      *
 * -------------------------------------------------------------------------- *
 * @license https://github.com/TheAncientOwl/oopsie-logsy/blob/main/LICENSE
 *
 * @file generate_entries.tsx
 * @author Alexandru Delegeanu
 * @version 0.2
 * @description Generator tool for log entries.
 */

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

std::string generate_payload(int min_len, int max_len)
{
    static const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> len_dist(min_len, max_len);
    std::uniform_real_distribution<> space_prob(0.0, 1.0);
    std::uniform_int_distribution<> char_dist(0, chars.size() - 1);

    int length = len_dist(gen);
    std::string result;
    for (int i = 0; i < length; ++i)
    {
        if (space_prob(gen) < 0.1)
        {
            result += ' ';
        }
        else
        {
            result += chars[char_dist(gen)];
        }
    }

    std::string cleaned;
    bool last_space = true;
    for (char c : result)
    {
        if (c == ' ')
        {
            if (!last_space)
            {
                cleaned += ' ';
                last_space = true;
            }
        }
        else
        {
            cleaned += c;
            last_space = false;
        }
    }

    return cleaned;
}

void generate_and_write_log_entries(
    const std::string &filename, int count, int start_timestamp,
    int min_payload_len, int max_payload_len)
{
    std::vector<std::string> channels = {"Channel1", "Channel2", "Channel3", "Channel4"};
    std::vector<std::string> levels = {"info", "warn", "error", "debug"};

    std::ofstream outfile(filename);
    if (!outfile)
    {
        std::cerr << "Failed to open output file.\n";
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> channel_dist(0, channels.size() - 1);
    std::uniform_int_distribution<> level_dist(0, levels.size() - 1);

    for (int i = 0; i < count; ++i)
    {
        int timestamp = start_timestamp + i;
        std::string channel = channels[channel_dist(gen)];
        std::string level = levels[level_dist(gen)];
        std::string payload = generate_payload(min_payload_len, max_payload_len);

        outfile << timestamp << " " << channel << " " << level << " " << payload << "\n";
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: generate_entries <count> [min_payload_len] [max_payload_len]\n";
        return 1;
    }

    int count = std::stoi(argv[1]);
    int min_payload_len = (argc > 2) ? std::stoi(argv[2]) : 25;
    int max_payload_len = (argc > 3) ? std::stoi(argv[3]) : 50;

    if (min_payload_len > max_payload_len)
    {
        std::cerr << "Error: min_payload_len > max_payload_len\n";
        return 1;
    }

    std::string filename = "benchmark_logs." + std::to_string(count) + ".txt";
    generate_and_write_log_entries(filename, count, 1000000, min_payload_len, max_payload_len);

    return 0;
}
