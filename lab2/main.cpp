#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <locale>
#include <codecvt>

const std::wstring alphabet = L"абвгдежзийклмнопрстуфхцчшщъыьэюя";

std::map<wchar_t, double> rus_letter_freq = {
    {L'а', 0.031138}, {L'б', 0.027416}, {L'в', 0.018856}, {L'г', 0.022454}, {L'д',  0.023074}, {L'е', 0.027540},
    {L'ж', 0.022206}, {L'з', 0.026920}, {L'й', 0.029773}, {L'к', 0.039821}, {L'л', 0.023198}, {L'м', 0.041062},
    {L'н', 0.033867}, {L'о', 0.049870}, {L'п', 0.032502}, {L'р', 0.039945}, {L'ы',0.032750}, {L'ь', 0.039697},
    {L'с', 0.039201}, {L'т', 0.034115}, {L'у', 0.040938}, {L'ч', 0.028781}, {L'й', 0.029773}, {L'х',  0.031758},
    {L'и', 0.027416}, {L'ш', 0.029525}, {L'ю',  0.033122}, {L'ц', 0.031014}, {L'щ',  0.024191}, {L'э', 0.028408},
    {L'ф', 0.029525}, {L'ъ', 0.036100}, {L'я', 0.023818} 
};

int index(wchar_t c) {
    size_t pos = alphabet.find(c);
    return (pos != std::wstring::npos) ? (int)pos : -1;
}

std::wstring clean_text(const std::wstring& text) {
    std::wstring res = L"";
    for (wchar_t c : text) {
        if (index(c) != -1) {
            res += c;
        }
    }
    return res;
}

std::wstring encrypt(const std::wstring& plaintext, const std::wstring& key) {
    std::wstring ciphertext = L"";
    int m = key.length();
    int N = alphabet.length();
    
    for (size_t i = 0; i < plaintext.length(); ++i) {
        int pi = index(plaintext[i]);
        int ki = index(key[i % m]);
        int ci = (pi + ki) % N;
        ciphertext += alphabet[ci];
    }
    return ciphertext;
}

std::wstring decrypt(const std::wstring& ciphertext, const std::wstring& key) {
    std::wstring plaintext = L"";
    int m = key.length();
    int N = alphabet.length();
    
    for (size_t i = 0; i < ciphertext.length(); ++i) {
        int ci = index(ciphertext[i]);
        int ki = index(key[i % m]);
        int pi = (ci - ki + N) % N; 
        plaintext += alphabet[pi];
    }
    return plaintext;
}

double coincidence(const std::wstring& text) {
    int N = text.length();
    if (N <= 1) return 0.0;
    
   std::map<wchar_t, int> freq;
    for (wchar_t c : text) {
        freq[c]++;
    }
    
    double ic = 0.0;
    for (auto const& pair : freq) {
        int count = pair.second;
        ic += count * (count - 1);
    }
    return ic / (N * (N - 1));
}

int estimate_key_length(const std::wstring& cipher_text, int max_r = 30) {
    double random = 1.0 / alphabet.length();
    int best_r = 1;
    double best = 0.0;
    
    for (int r = 2; r <= max_r; ++r) {
        double ic_sum = 0.0;
        for (int part = 0; part < r; ++part) {
            std::wstring block = L"";
            for (size_t i = part; i < cipher_text.length(); i += r) {
                block += cipher_text[i];
            }
            if (block.length() > 1) {
                ic_sum += coincidence(block);
            }
        }
        double avg_ic = ic_sum / r;
        if (avg_ic > random * 1.5 && avg_ic > best) {
            best = avg_ic;
            best_r = r;
        }
    }
    return best_r;
}

std::wstring guess_key(const std::wstring& cipher_text, int key_length) {
    std::wstring key = L"";
    int N = alphabet.length();
    
    for (int j = 0; j < key_length; ++j) {
        std::wstring block = L"";
        for (size_t i = j; i < cipher_text.length(); i += key_length) {
            block += cipher_text[i];
        }
        if (block.empty()) {
            key += L'?';
            continue;
        }
        
       std::map<wchar_t, int> freq;
        for (wchar_t c : block) freq[c]++;
        
        wchar_t most_common = block[0];
        int max_count = 0;
        for (auto const& pair : freq) {
            if (pair.second > max_count) {
                max_count = pair.second;
                most_common = pair.first;
            }
        }
        
        int k = (index(most_common) - index(L'о') + N) % N;
        key += alphabet[k];
    }
    return key;
}

std::wstring guess_key_M(const std::wstring& cipher_text, int key_length) {
    std::wstring key = L"";
    int N = alphabet.length();
    
    for (int j = 0; j < key_length; ++j) {
        std::wstring block = L"";
        for (size_t i = j; i < cipher_text.length(); i += key_length) {
            block += cipher_text[i];
        }
        if (block.empty()) {
            key += L'?';
            continue;
        }
        
        std::map<wchar_t, int> freq;
        for (wchar_t c : block) freq[c]++;
        
        double best_score = -1.0;
        int best_g = 0;
        
        for (int g = 0; g < N; ++g) {
            double score = 0.0;
            for (auto const& pair : freq) {
                wchar_t c = pair.first;
                int count = pair.second;
                
                int plain_idx = (index(c) - g + N) % N;
                wchar_t plain_char = alphabet[plain_idx];
                
                double prob = 0.0;
                if (rus_letter_freq.find(plain_char) != rus_letter_freq.end()) {
                    prob = rus_letter_freq[plain_char];
                }
                
                score += ((double)count / block.length()) * prob;
            }
            if (score > best_score) {
                best_score = score;
                best_g = g;
            }
        }
        key += alphabet[best_g];
    }
    return key;
}

std::wstring read_file(const std::string& filename) {
    std::wifstream wif(filename);
    wif.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstring content = L"";
    std::wstring line;
    while (getline(wif, line)) {
        content += line;
    }
    return content;
}

int main() {
    setlocale(LC_ALL, "");
    std::wstring raw_text = read_file("text.txt");
    
    std::wstring plain_text = clean_text(raw_text);
    std::wcout << L"Довжина вiдкритого тексту: " << plain_text.length() << L"\n";
    std::wcout << L"Вiдкритий текст: iндекс = " << coincidence(plain_text) << L"\n\n";
    
    std::vector<std::pair<int, std::wstring>> sample_keys = {
        {2, L"аб"},
        {3, L"абв"},
        {4, L"абвг"},
        {5, L"абвгд"},
        {13, L"майтринадцать"}
    };
    
    std::wcout << L"Зашифрованi тексти та iндекси вiдповiдностi:\n\n";
    for (auto const& item : sample_keys) {
        int r = item.first;
        std::wstring key = item.second;
        
        std::wstring cipher_text = encrypt(plain_text, key);
        double ic_value = coincidence(cipher_text);
        
        std::wcout << L"Ключ довжини " << r << L" = '" << key << L"':\n";
        std::wcout << cipher_text.substr(0, 50) << L"\n";
        std::wcout << L"iндекс = " << ic_value << L"\n\n";
    }

    std::wstring cipher_data = encrypt(plain_text, L"майтринадцать");

    int est_r = estimate_key_length(cipher_data, 30);
    std::wcout << L"Оцiнена довжина ключа: " << est_r << L"\n";

    std::wstring key_guess1 = guess_key(cipher_data, est_r);
    std::wcout << L"Первинна оцiнка ключа за частотами: " << key_guess1 << L"\n";

    std::wstring key_guess2 = guess_key_M(cipher_data, est_r);
    std::wcout << L"Уточнений ключ за методом M(g): " << key_guess2 << L"\n";

    std::wstring decrypted_text = decrypt(cipher_data, key_guess2);
    std::wcout << L"Дешифрований текст:\n" << decrypted_text.substr(0, 100) << L"\n";
    std::wcout << L"iндекс дешифрованого тексту: " << coincidence(decrypted_text) << L"\n";

    return 0;
}