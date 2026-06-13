#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <windows.h>

const std::string alph1 = "абвгдежзийклмнопрстуфхцчшщыьэюя";
const std::string alph2 = "абвгдежзийклмнопрстуфхцчшщьыэюя"; 
const int M = 31;
const int M2 = 31 * 31; 
const std::vector<std::string> rusoriz = {"ст", "но", "то", "на", "ен"};

std::string read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) std::cerr << "file ne vidkryvsya\n";

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return text;
}

int char_to_idx(char c, const std::string& alph) {
    return alph.find(c);
}

char idx_to_char(int idx, const std::string& alph) {
    return alph[idx];
}

int ext_gcd(int a, int b, int &x, int &y) {
    if (a == 0) {
        x = 0; y = 1;
        return b;
    }
    int x1, y1;
    int d = ext_gcd(b % a, a, x1, y1);
    x = y1 - (b / a) * x1;
    y = x1;
    return d;
}

int mod_inverse(int a, int m) {
    int x, y;
    int g = ext_gcd(a, m, x, y);
    if (g != 1) return -1;
    return (x % m + m) % m;
}

std::vector<int> solve_congr(int a, int b, int m) {
    std::vector<int> sol;
    a = (a % m + m) % m;
    b = (b % m + m) % m;
    
    int x, y;
    int g = ext_gcd(a, m, x, y);
    
    if (b % g != 0) return sol;
    
    int a1 = a / g;
    int b1 = b / g;
    int m1 = m / g;
    
    int x0 = (1LL * b1 * mod_inverse(a1, m1)) % m1;
    for (int i = 0; i < g; i++) {
        sol.push_back((x0 + i * m1) % m);
    }
    return sol;
}

std::vector<std::pair<std::string, int>> noperetyn_bigrams(const std::string& text, int top_n) {
    std::unordered_map<std::string, int> freq;
    for (size_t i = 0; i + 1 < text.size(); i += 2) {
        freq[text.substr(i, 2)]++;
    }
    
    std::vector<std::pair<std::string, int>> sorted_freq(freq.begin(), freq.end());
    sort(sorted_freq.begin(), sorted_freq.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    if (sorted_freq.size() > top_n) sorted_freq.resize(top_n);
    return sorted_freq;
}

int bigram_to_int(const std::string& b, const std::string& alph) {
    int x1 = char_to_idx(b[0], alph);
    int x2 = char_to_idx(b[1], alph);
    return x1 * M + x2;
}

std::string decrypt(const std::string& text, int a, int b_key, const std::string& alph) {
    std::string decrypted = "";
    int a_inv = mod_inverse(a, M2);
    if (a_inv == -1) return "";

    for (size_t i = 0; i + 1 < text.size(); i += 2) {
        int y = bigram_to_int(text.substr(i, 2), alph);
        int x = (1LL * a_inv * (y - b_key % M2 + M2)) % M2;
        decrypted += idx_to_char(x / M, alph);
        decrypted += idx_to_char(x % M, alph);
    }
    return decrypted;
}

bool is_valid(const std::string& text) {
    int good_chars = 0;
    std::string popular = "оеаинтсрвл"; 
    for (char c : text) {
        if (popular.find(c) != std::string::npos) good_chars++;
    }

    return (double)good_chars / text.size() > 0.60;
}

void attack_cipher(const std::string& ciphertext, const std::string& alph, const std::string& alph_name) {
    std::cout << "\nAtack using alphabet: " << alph_name << std::endl;
    
    auto top_bigrams = noperetyn_bigrams(ciphertext, 5);
    
    std::cout << "Top 5 bigrams in the ciphertext: \n";
    for (auto& p : top_bigrams) std::cout << p.first << " (" << p.second << ")\n";

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i == j) continue;
            
            for (int k = 0; k < 5; k++) {
                for (int l = 0; l < 5; l++) {
                    if (k == l) continue;

                    int x1 = bigram_to_int(rusoriz[i], alph);
                    int x2 = bigram_to_int(rusoriz[j], alph);
                    int y1 = bigram_to_int(top_bigrams[k].first, alph);
                    int y2 = bigram_to_int(top_bigrams[l].first, alph);

                    int dX = (x1 - x2 + M2) % M2;
                    int dY = (y1 - y2 + M2) % M2;

                    std::vector<int> possible_a = solve_congr(dX, dY, M2);

                    for (int a : possible_a) {
                        if (ext_gcd(a, M, *new int, *new int) != 1) continue;
                        int b = (y1 - 1LL * a * x1) % M2;
                        b = (b + M2) % M2;

                        std::string decrypted = decrypt(ciphertext.substr(0, 500), a, b, alph);
                        
                        if (!decrypted.empty() && is_valid(decrypted)) {
                            std::cout << "\nPossible key found\n";
                            std::cout << "Key (a, b): (" << a << ", " << b << ")\n";
                            std::cout << "Finding similiarity: " << rusoriz[i] << " to " << top_bigrams[k].first  << ", " << rusoriz[j] << " to " << top_bigrams[l].first << "\n";
                            std::cout << "Text fragment: " << decrypted.substr(0, 150) << std::endl;
                        }
                    }
                }
            }
        }
    }
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    std::string ciphertext = read_file("04.txt");
    ciphertext.erase(remove(ciphertext.begin(), ciphertext.end(), '\n'), ciphertext.end());
    ciphertext.erase(remove(ciphertext.begin(), ciphertext.end(), '\r'), ciphertext.end());
    attack_cipher(ciphertext, alph1, "Var 1");
    attack_cipher(ciphertext, alph2, "Var 2");

    return 0;
}