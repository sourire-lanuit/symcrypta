#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

const std::string alph_space = " אבגדהוזחטיךכלםמןנסעףפץצקרשüû‎‏ÿ";

const std::string alph_nospace = "אבגדהוזחטיךכלםמןנסעףפץצקרשüû‎‏ÿ";

bool contains_char(char c, const std::string& alphabet) {
    return alphabet.find(c) != std::string::npos;
}

std::string read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) std::cerr << "file ne vidkryvsya\n";

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return text;
}

std::string clean_text(const std::string& text) {
    std::string result;
    for (unsigned char c : text) {
        c = tolower(c);
        if (contains_char(c, alph_space)) result += c;
    }

    return result;
}

std::string clean_txt_wo_space(const std::string& text) {
    std::string result;
    for (unsigned char c : text) {
        c = tolower(c);
        if (contains_char(c, alph_nospace)) result += c;
    }

    return result;
}

std::unordered_map<std::string, long long> letter_freq(const std::string& text) {
    std::unordered_map<std::string, long long> freq;
    for (char c : text) {
        std::string s(1, c);
        freq[s]++;
    }

    return freq;
}

std::unordered_map<std::string, long long> bigram_intersect(const std::string& text) {
    std::unordered_map<std::string, long long> freq;
    for (size_t i = 0; i + 1 < text.size(); i++) freq[text.substr(i, 2)]++;

    return freq;
}

std::unordered_map<std::string, long long> bigram_no_intersect(const std::string& text) {
    std::unordered_map<std::string, long long> freq;
    for (size_t i = 0; i + 1 < text.size(); i += 2) freq[text.substr(i, 2)]++;

    return freq;
}

std::unordered_map<std::string, long long> trigram(const std::string& text) {
    std::unordered_map<std::string, long long> freq;
    for (size_t i = 0; i + 2 < text.size(); i++) freq[text.substr(i, 3)]++;

    return freq;
}

double entropy(const std::unordered_map<std::string, long long>& freq, long long total) {
    double H = 0.0;
    for (auto& pair : freq) {
        double p = (double)pair.second / total;
        H -= p * log2(p);
    }

    return H;
}

double entropy_estime(const std::string& text, int n) {
    std::unordered_map<std::string, std::unordered_map<char, long long>> contexts;
    for (size_t i = 0; i + n - 1 < text.size(); i++) {
        std::string context = text.substr(i, n - 1);
        char next_char = text[i + n - 1];
        contexts[context][next_char]++;
    }

    long long total = 0;
    long long correct = 0;

    for (auto& context : contexts) {
        long long max_freq = 0;
        long long sum = 0;

        for (auto& letter : context.second) {
            sum += letter.second;
            max_freq = std::max(max_freq, letter.second);
        }

        total += sum;
        correct += max_freq;
    }

    double q = total == 0 ? 0.000001 : (double)correct / total;

    return -log2(q);
}

void print_most_freq_letters(const std::unordered_map<std::string, long long>& freq) {
    std::vector<std::pair<std::string, long long>> data(freq.begin(), freq.end());
    sort(data.begin(), data.end(), [](auto& a, auto& b) {
        return a.second > b.second;
    });

    std::cout << "\n10 most frequent symbols:\n";

    for (int i = 0; i < 10 && i < data.size(); i++) std::cout << data[i].first << " : " << data[i].second << std::endl;
}

void bigram_print(const std::unordered_map<std::string, long long>& bigrams, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) return;
    out << std::setw(4) << " ";
    for (char c : alph_nospace) out << std::setw(8) << c;
    out << "\n";

    for (char row : alph_nospace) {
        out << std::setw(4) << row;
        for (char col : alph_nospace) {
            std::string bigr;
            bigr += row;
            bigr += col;
            auto it = bigrams.find(bigr);
            long long count = (it == bigrams.end()) ? 0 : it->second;
            out << std::setw(8) << count;
        }

        out << "\n";
    }

    out.close();
}

int main() {
    std::string orig_text = read_file("text_potok_mysli_arestovicha.txt");
    std::string text = clean_text(orig_text);
    std::string text_wo_space = clean_txt_wo_space(orig_text);
    auto letters = letter_freq(text);
    auto bigrams = bigram_intersect(text);
    auto trigrams = trigram(text);

    long long total_letters = text.size();
    long long total_bigrams = text.size() - 1;
    long long total_trigrams = text.size() - 2;

    double H1 = entropy(letters, total_letters);
    double H2 = entropy(bigrams, total_bigrams) / 2.0;
    double H3 = entropy(trigrams, total_trigrams) / 3.0;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "---- W i t h    s p a c e s ----\n";
    std::cout << "H1 = " << H1 << std::endl;
    std::cout << "H2 = " << H2 << std::endl;
    std::cout << "H3 = "<< H3 << std::endl;

    auto letters_wo_space = letter_freq(text_wo_space);
    auto bigrams_wo_space = bigram_intersect(text_wo_space);
    auto trigrams_wo_space = trigram(text_wo_space);

    double H1WoS = entropy(letters_wo_space, text_wo_space.size());
    double H2WoS = entropy(bigrams_wo_space, text_wo_space.size() - 1) / 2.0;
    double H3WoS = entropy(trigrams_wo_space,text_wo_space.size() - 2) / 3.0;

    std::cout << "\n----Withoutspaces----\n";
    std::cout << "H1 = " << H1WoS << std::endl;
    std:: cout << "H2 = " << H2WoS << std::endl;
    std::cout << "H3 = " << H3WoS << std::endl;

    std::cout << "\n---- Entropy guess ----\n";

    std::cout << "H(10) = " << entropy_estime(text, 10) <<std::endl;
    std::cout << "H(20) = " << entropy_estime(text, 20) << std::endl;
    std::cout << "H(30) = " << entropy_estime(text, 30) << std::endl;

    double H0 = log2(32.0);
    std::cout << "\n---- Reducant rusnya ----\n";
    std::cout << "R(H1) = " << 1 - H1 / H0 << std::endl;
    std::cout << "R(H2) = " << 1 - H2 / H0 << std::endl;
    std::cout << "R(H3) = " << 1 - H3 / H0 << std::endl;

    print_most_freq_letters(letters);
    bigram_print(bigrams_wo_space, "bigram_wo_space_table.txt");
    std::cout << "\nTable is saved to bigram_wo_space_table.txt\n";
    bigram_print(bigrams, "bigram_space_table.txt");
    std::cout << "Table is saved to bigram_space_table.txt\n";

    return 0;
}