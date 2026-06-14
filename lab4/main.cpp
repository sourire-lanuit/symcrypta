#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

const std::string posl = "011100010100000001101011111100110100001011100110010011001001110011011011001100010011101011011000000111010011010000101011101100010111010111110111101110110110010000101001001101000100111110111000001000111010111111000100001111101011110000101111001000111010001000101111101100101000101110001111010010010111000111101000011000110010001000000101010100111100010000011000010111101010000110001100110101010101001110101111110110000111011000110001101000101000110010111110101000010110111001011000000001100010000000101101011011001011001001100101100000111100001000001101111001111111100011000001000011010010100101101111100000001111101111100000001011010001100100111001010101011001011101100100011100101001110101101001000001011101001110101101100010101110000000110111101101110011010011010110111011111011010110000100110001111111000000001100010101110100011010100110111101000100001111000011011101111111011011111001101110110111111110110000101010001001000101011101111000110101001100101010111001111111110011001110100001111011001101010101110010001001101101010101001110010101001000110110111010010000010000010100110110000010101001111101001010111010011110100001010001010101111011111111100010011001111011000000010100111110011101111001010110110111101111000100110011010100110010100011010101001111001000000010111101111110001010011101111100101100100110101100011110010001111001101010110001011111011100000110110011101010000111001111110000101010100001000100110010011011000001010011101010111000111010100110101011111100001010011100100101010010111110011011010001110101100000111111110110010101110010101001101001101100111110101111011001100101010100111111011111000010101010010110100110010110011010111111111110000011011011111110111110100110011100000110100110010100010000100110101101000011010000011101001000110011011100001011100111100001111010001100101001100011100010110011101101111011001001001101000011010100110001000000100101010101100101101100011000110110000101010110010000010100110000000100001000010001011110011000110000011100100100011100111111101101000000010111111000010011001000001111111111110";
std::vector<uint32_t> z;

inline uint32_t step_L1(uint32_t& state) {
    uint32_t out = state & 1;
    uint32_t new_bit = (state ^ (state >> 1) ^ (state >> 4) ^ (state >> 6)) & 1;
    state = (state >> 1) | (new_bit << 29);
    return out;
}

inline uint32_t step_L2(uint32_t& state) {
    uint32_t out = state & 1;
    uint32_t new_bit = (state ^ (state >> 3)) & 1;
    state = (state >> 1) | (new_bit << 30);
    return out;
}

inline uint32_t step_L3(uint32_t& state) {
    uint32_t out = state & 1;
    uint32_t new_bit = (state ^ (state >> 1) ^ (state >> 2) ^ (state >> 3) ^ (state >> 5) ^ (state >> 7)) & 1;
    state = (state >> 1) | (new_bit << 31);
    return out;
}

int main() {
    for (char c : posl) z.push_back(c - '0');
    
    std::vector<uint32_t> L1_cand;
    std::vector<uint32_t> L2_cand;

    std::cout << "Length of poslidovnist: " << z.size() << " bytes.\n";

    std::cout << "Finding L1\n";
    for (uint32_t start_state = 1; start_state < (1U << 30); ++start_state) {
        uint32_t state = start_state;
        int matches = 0;

        for (int i = 0; i < 256; ++i) {
            if (step_L1(state) == z[i]) matches++;
        }
        if (matches < 155) continue; 
        
        for (int i = 256; i < z.size(); ++i) {
            if (step_L1(state) == z[i]) matches++;
        }
    
        if (matches > z.size() * 0.68) {
            std::cout << "Found L1: " << start_state << " (matches: " << matches << " with " << z.size() << ")\n";
            L1_cand.push_back(start_state);
        }
    }

    std::cout << "Finding L2\n";
    for (uint32_t start_state = 1; start_state < (1ULL << 31); ++start_state) {
        uint32_t state = start_state;
        int matches = 0;
        
        for (int i = 0; i < 256; ++i) {
            if (step_L2(state) == z[i]) matches++;
        }
        if (matches < 155) continue;
        
        for (int i = 256; i < z.size(); ++i) {
            if (step_L2(state) == z[i]) matches++;
        }
        
        if (matches > z.size() * 0.68) {
            std::cout << "Found L2: " << start_state << " (matches: " << matches << " with " << z.size() << ")\n" ;
            L2_cand.push_back(start_state);
        }
    }

    std::cout << "Finding L3\n";
    if (L1_cand.empty() || L2_cand.empty()) {
        std::cout << "Error no L1 or L2 found\n";
        return 1;
    }

    for (uint32_t c1 : L1_cand) {
        for (uint32_t c2 : L2_cand) {
            for (uint32_t start_state = 1; start_state != 0; ++start_state) {
                uint32_t s1 = c1;
                uint32_t s2 = c2;
                uint32_t s3 = start_state;
                bool valid = true;
                
                for (int i = 0; i < z.size(); ++i) {
                    uint32_t x = step_L1(s1);
                    uint32_t y = step_L2(s2);
                    uint32_t s = step_L3(s3);
                    
                    uint32_t z_out = (s * x) ^ ((1 ^ s) * y); 
                    
                    if (z_out != z[i]) {
                        valid = false;
                        break;
                    }
                }
                
                if (valid) {
                    std::cout << "Key found\n";
                    std::cout << "L1: " << c1 << std::endl;
                    std::cout << "L2: " << c2 << std::endl;
                    std::cout << "L3: " << start_state << std::endl;
                   
                    return 0;
                }
            }
        }
    }

    std::cout << "No key found" << std::endl;

    return 0;
}