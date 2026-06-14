#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <windows.h>

const std::wstring alph1 = L"абвгдежзийклмнопрстуфхцчшщыьэюя";
const std::wstring alph2 = L"абвгдежзийклмнопрстуфхцчшщьыэюя"; 
const int M = 31;
const int M2 = 31 * 31; 
const std::vector<std::wstring> rusoriz = {L"ст", L"но", L"то", L"на", L"ен"};

std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Не відкриваєцця\n";
        exit(1);
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    text.erase(remove(text.begin(), text.end(), '\n'), text.end());
    text.erase(remove(text.begin(), text.end(), '\r'), text.end());
    return utf8_to_wstring(text);
}

int char_to_idx(wchar_t c, const std::wstring& alph) {
    return alph.find(c);
}

wchar_t idx_to_char(int idx, const std::wstring& alph) {
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

std::vector<std::pair<std::wstring, int>> noperetyn_bigrams(const std::wstring& text, int top_n) {
    std::unordered_map<std::wstring, int> freq;
    for (size_t i = 0; i + 1 < text.size(); i += 2) {
        freq[text.substr(i, 2)]++;
    }
    
    std::vector<std::pair<std::wstring, int>> sorted_freq(freq.begin(), freq.end());
    sort(sorted_freq.begin(), sorted_freq.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    if (sorted_freq.size() > top_n) sorted_freq.resize(top_n);
    return sorted_freq;
}

int bigram_to_int(const std::wstring& b, const std::wstring& alph) {
    int x1 = char_to_idx(b[0], alph);
    int x2 = char_to_idx(b[1], alph);
    if (x1 == -1 || x2 == -1) return -1;
    return x1 * M + x2;
}

std::wstring decrypt(const std::wstring& text, int a, int b_key, const std::wstring& alph) {
    std::wstring decrypted = L"";
    int a_inv = mod_inverse(a, M2);
    if (a_inv == -1) return L"";

    for (size_t i = 0; i + 1 < text.size(); i += 2) {
        int y = bigram_to_int(text.substr(i, 2), alph);
        if (y == -1) continue;
        int x = (1LL * a_inv * (y - b_key % M2 + M2)) % M2;
        decrypted += idx_to_char(x / M, alph);
        decrypted += idx_to_char(x % M, alph);
    }
    return decrypted;
}

bool is_valid(const std::wstring& text) {
    if (text.empty()) return false;
    
    int good_chars = 0;
    std::wstring popular = L"оеаинтсрвл"; 
    
    for (size_t i = 0; i < text.size(); i++) {
        if (popular.find(text[i]) != std::wstring::npos) good_chars++;
        if (i >= 2 && text[i] == text[i-1] && text[i] == text[i-2]) {
            return false; 
        }
    }

    std::vector<std::wstring> forbidden = {L"аь", L"еь", L"иь", L"оь", L"уь", L"ыь", L"эь", L"юь", L"яь", L"ьь", L"йь", L"ъь"};
    for (const auto& fb : forbidden) {
        if (text.find(fb) != std::wstring::npos) return false;
    }

    return (double)good_chars / text.size() > 0.55;
}

void attack_cipher(const std::wstring& ciphertext, const std::wstring& alph, const std::string& alph_name) {
    std::cout << "\nАтака з використанням алфавіту: " << alph_name << std::endl;
    
    auto top_bigrams = noperetyn_bigrams(ciphertext, 5);
    
    std::cout << "Топ-5 біграм шифротексту:\n";
    for (auto& p : top_bigrams) {
        std::cout << wstring_to_utf8(p.first) << " (" << p.second << ")\n";
    }

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

                        std::wstring decrypted = decrypt(ciphertext.substr(0, 500), a, b, alph);
                        if (!decrypted.empty() && is_valid(decrypted)) {
                            std::cout << "\nКлюч найшли\n";
                            std::cout << "Ключ (a, b): (" << a << ", " << b << ")\n";
                            std::cout << "Співставлення: " << wstring_to_utf8(rusoriz[i]) << "  to " << wstring_to_utf8(top_bigrams[k].first)  << ", " << wstring_to_utf8(rusoriz[j]) << " to " << wstring_to_utf8(top_bigrams[l].first) << "\n";
                            
                            std::wstring full_text = decrypt(ciphertext, a, b, alph);
                            std::cout << "\nДешифрований текст:\n" << wstring_to_utf8(full_text.substr(0, 500)) << std::endl;
                
                            return; 
                        }
                    }
                }
            }
        }
    }
    std::cout << "\nКлюч не знайдено для цього алфавіту.\n";
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    std::wstring ciphertext = read_file("04.txt");
    
    attack_cipher(ciphertext, alph1, "Варіант 1");
    attack_cipher(ciphertext, alph2, "Варіант 2");

    return 0;
}