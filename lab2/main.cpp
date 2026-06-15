#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <locale>
#include <codecvt>
#include <iomanip>

const std::wstring alphabet = L"абвгдежзийклмнопрстуфхцчшщъыьэюя";

std::map<wchar_t, double> rus_letter_freq = {
    {L'а', 0.0801}, {L'б', 0.0159}, {L'в', 0.0454}, {L'г', 0.0170},
    {L'д', 0.0298}, {L'е', 0.0845}, {L'ж', 0.0094}, {L'з', 0.0165},
    {L'и', 0.0735}, {L'й', 0.0121}, {L'к', 0.0349}, {L'л', 0.0440},
    {L'м', 0.0321}, {L'н', 0.0670}, {L'о', 0.1097}, {L'п', 0.0281},
    {L'р', 0.0473}, {L'с', 0.0547}, {L'т', 0.0626}, {L'у', 0.0262},
    {L'ф', 0.0026}, {L'х', 0.0097}, {L'ц', 0.0048}, {L'ч', 0.0144},
    {L'ш', 0.0073}, {L'щ', 0.0036}, {L'ъ', 0.0004}, {L'ы', 0.0190},
    {L'ь', 0.0174}, {L'э', 0.0032}, {L'ю', 0.0064}, {L'я', 0.0201}
};

int index(wchar_t c) {
    size_t pos = alphabet.find(c);
    return (pos != std::wstring::npos) ? (int)pos : -1;
}

std::wstring clean_text(const std::wstring& text) {
    std::wstring res = L"";
    for (wchar_t c : text) {
        if (index(c) != -1) res += c;
    }
    return res;
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
    for (wchar_t c : text) freq[c]++;
    double ic = 0.0;
    for (auto const& pair : freq) {
        int count = pair.second;
        ic += count * (count - 1);
    }
    return ic / ((double)N * (N - 1));
}

double avg_ic_r(const std::wstring& cipher_text, int r) {
    double ic_sum = 0.0;
    int valid_blocks = 0;
    for (int part = 0; part < r; ++part) {
        std::wstring block = L"";
        for (size_t i = part; i < cipher_text.length(); i += r)
            block += cipher_text[i];
        if (block.length() > 1) {
            ic_sum += coincidence(block);
            valid_blocks++;
        }
    }
    return (valid_blocks > 0) ? ic_sum / valid_blocks : 0.0;
}

int estimate_key_length(const std::wstring& cipher_text, int max_r = 30) {
    double estim_ic = 0.0553;
    double random_ic = 1.0 / alphabet.length();

    std::wcout << L"\nТаблиця середнiх iндексiв вiдповiдности для r = 2 to " << max_r << L":\n";
    std::wcout << std::fixed << std::setprecision(5);
    std::wcout << L" r  |  avg IC  | вiдхилення вiд теорiї\n";
    std::wcout << L"----|----------|----------------------\n";

    int best_r = 2;
    double best = 1e9;

    for (int r = 2; r <= max_r; ++r) {
        double avg_ic = avg_ic_r(cipher_text, r);
        double diff = std::abs(avg_ic - estim_ic);

        std::wcout << std::setw(3) << r << L" | " << avg_ic << L"  | " << diff;
        if (diff < best) {
            best = diff;
            best_r = r;
        }
        if (avg_ic > estim_ic * 0.85)
            std::wcout << L"  <-- кандидат";
        std::wcout << L"\n";
    }
    return best_r;
}

std::wstring guess_key(const std::wstring& cipher_text, int key_length) {
    std::wstring key = L"";
    int N = alphabet.length();
    int o_idx = index(L'о'); 

    for (int j = 0; j < key_length; ++j) {
        std::wstring block = L"";
        for (size_t i = j; i < cipher_text.length(); i += key_length)
            block += cipher_text[i];

        if (block.empty()) { key += L'а'; continue; }

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
    
        int k = (index(most_common) - o_idx + N) % N;
        key += alphabet[k];
    }
    return key;
}

std::wstring guess_key_M(const std::wstring& cipher_text, int key_length) {
    std::wstring key = L"";
    int N = alphabet.length();

    for (int j = 0; j < key_length; ++j) {
        std::wstring block = L"";
        for (size_t i = j; i < cipher_text.length(); i += key_length)
            block += cipher_text[i];

        if (block.empty()) { key += L'а'; continue; }

        std::map<wchar_t, int> freq;
        for (wchar_t c : block) freq[c]++;
        int len = block.length();

        double best_score = -1.0;
        int best_g = 0;

        for (int g = 0; g < N; ++g) {
            double score = 0.0;
            for (auto const& [c, count] : freq) {
                int plain_idx = (index(c) - g + N) % N;
                wchar_t plain_char = alphabet[plain_idx];
                auto it = rus_letter_freq.find(plain_char);
                if (it != rus_letter_freq.end())
                    score += it->second * ((double)count / len);
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
    while (getline(wif, line)) content += line;
    return content;
}

int main() {
    setlocale(LC_ALL, "");
    std::wcout << std::fixed << std::setprecision(5);

    std::wstring raw_text = read_file("text.txt");
    std::wstring plain_text = clean_text(raw_text);
    std::wcout << L"Довжина вiдкритого тексту: " << plain_text.length() << L"\n";
    std::wcout << L"IC вiдкритого тексту: " << coincidence(plain_text) << L"\n\n";

    std::vector<std::pair<int, std::wstring>> sample_keys = {
        {2,  L"аб"},
        {3,  L"абв"},
        {4,  L"абвг"},
        {5,  L"абвгд"},
        {13, L"майтринадцать"}
    };

    std::wcout << L"Зашифрованi тексти та iндекси вiдповiдности:\n\n";
    for (auto const& [r, key] : sample_keys) {
        double ic_value = coincidence(plain_text);
        std::wcout << L"Ключ довжини " << r << L" = '" << key << L"':\n";
        std::wcout << plain_text.substr(0, 60) << L"\n";
        std::wcout << L"IC = " << ic_value << L"\n\n";
    }

    int est_r = estimate_key_length(plain_text, 30);
    std::wcout << L"\nОцiнена довжина ключа: " << est_r << L"\n";

    std::wstring key1 = guess_key(plain_text, est_r);
    std::wcout << L"\nМетод 1 (найчастiша лiтера'о'): " << key1 << L"\n";

    std::wstring key2 = guess_key_M(plain_text, est_r);
    std::wcout << L"Метод 2 (функцiя M_i(g)): " << key2 << L"\n";

    std::wstring dec1 = decrypt(plain_text, key1);
    std::wstring dec2 = decrypt(plain_text, key2);

    std::wcout << L"\nДешифрований текст (метод 1):\n";
    std::wcout << dec1.substr(0, 130) << L"\n";
    std::wcout << L"IC: " << coincidence(dec1) << L"\n";

    std::wcout << L"\nДешифрований текст (метод 2):\n";
    std::wcout << dec2.substr(0, 130) << L"\n";
    std::wcout << L"IC: " << coincidence(dec2) << L"\n";

    std::wstring dec_true = decrypt(plain_text, L"майтринадцать");
    std::wcout << L"\nДешифрування 'правильним ключем':\n";
    std::wcout << dec_true.substr(0, 130) << L"\n";
    std::wcout << L"IC: " << coincidence(dec_true) << L"\n";

    return 0;
}