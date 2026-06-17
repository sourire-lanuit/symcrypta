#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <cstdint>

double quantile(double t_prob) {
    double low = 0.0, high = 10.0, mid = 0.0;
    for (int i = 0; i < 100; ++i) {
        mid = low + (high - low) / 2.0;
        double curr_t = 0.5 * std::erfc(mid / std::sqrt(2.0));
        if (curr_t > t_prob) {
            low = mid; 
        } else {
            high = mid;
        }
    }
    return mid;
}

void parameters(uint32_t degree, uint32_t& N_out, uint32_t& C_out) {
    double p1 = 0.25; 
    double p2 = 0.50;
    
    double t_alpha = 2.576; 
    
    double M = std::pow(2.0, degree);
    double beta = 1.0 / M;
    double t_beta = quantile(beta);

    double sqrt_N = (-t_alpha * std::sqrt(p1 * (1.0 - p1)) - t_beta * std::sqrt(p2 * (1.0 - p2))) / (p1 - p2);
    double N_calc = std::ceil(sqrt_N * sqrt_N);
    
    double C_calc = N_calc * p1 + t_alpha * std::sqrt(N_calc * p1 * (1.0 - p1));

    N_out = static_cast<uint32_t>(N_calc);
    C_out = static_cast<uint32_t>(std::ceil(C_calc));
}

inline void step_L1(uint32_t& state) {
    uint32_t t = ((state >> 0) ^ (state >> 1) ^ (state >> 4) ^ (state >> 6)) & 1;
    state >>= 1;
    state |= (t << 29);
}

inline void step_L2(uint32_t& state) {
    uint32_t t = ((state >> 0) ^ (state >> 3)) & 1;
    state >>= 1;
    state |= (t << 30);
}

inline void step_L3(uint32_t& state) {
    uint32_t t = ((state >> 0) ^ (state >> 1) ^ (state >> 2) ^ (state >> 3) ^ (state >> 5) ^ (state >> 7)) & 1;
    state >>= 1;
    state |= (t << 31);
}

bool geffe_combine(uint32_t x, uint32_t y, uint32_t s) {
    return (s & 1) ? (x & 1) : (y & 1);
}

bool verify_full_key(uint32_t L1_init, uint32_t L2_init, uint32_t L3_init, const std::vector<uint8_t>& z) {
    uint32_t state1 = L1_init;
    uint32_t state2 = L2_init;
    uint32_t state3 = L3_init;

    for (size_t i = 0; i < z.size(); ++i) {
        bool out = geffe_combine(state1, state2, state3);
        if (out != z[i]) return false;
        
        step_L1(state1);
        step_L2(state2);
        step_L3(state3);
    }
    return true;
}

int main() {
    std::string z_str = "01110001010000000110101111110011010000101110011001001100100111001101101100110001001110101101100000011101001101000010101110110001011101011111011110111011011001000010100100110100010011111011100000100011101011111100010000111110101111000010111100100011101000100010111110110010100010111000111101001001011100011110100001100011001000100000010101010011110001000001100001011110101000011000110011010101010100111010111111011000011101100011000110100010100011001011111010100001011011100101100000000110001000000010110101101100101100100110010110000011110000100000110111100111111110001100000100001101001010010110111110000000111110111110000000101101000110010011100101010011001011101100100011100101001110101101001000001011101001110101101100010101110000000110111101101110011010011010110111011111011010110000100110001111111000000001100010101110100011010100110111101000100001111000011011101111111011011111001101110110111111110110000101010001001000101011101111000110101001100101010111001111111110011001110100001111011001101010101110010001001101101010101001110010101001000110110111010010000010000010100110110000010101001111101001010111010011110100001010001010101111011111111100010011001111011000000010100111110011101111001010110110111101111000100110011010100110010100011010101001111001000000010111101111110001010011101111100101100100110101100011110010001111001101010110001011111011100000110110011101010000111001111110000101010100001000100110010011011000001010011101010111000111010100110101011111100001010011100100101010010111110011011010001110101100000111111110110010101110010101001101001101100111110101111011001100101010100111111011111000010101010010110100110010110011010111111111110000011011011111110111110100110011100000110100110010100010000100110101101000011010000011101001000110011011100001011100111100001111010001100101001100011100010110011101101111011001001001101000011010100110001000000100101010101100101101100011000110110000101010110010000010100110000000100001000010001011110011000110000011100100100011100111111101101000000010111111000010011001000001111111111110";
    std::vector<uint8_t> z;
    z.reserve(z_str.size());
    for (char c : z_str) {
    z.push_back(c == '1' ? 1 : 0);
    }

    uint32_t N_L1, C_L1;
    uint32_t N_L2, C_L2;

    parameters(30, N_L1, C_L1);
    parameters(31, N_L2, C_L2);

    std::cout << "Parametrs" << std::endl;
    std::cout << "L1: N = " << N_L1 << ", C = " << C_L1 << std::endl;
    std::cout << "L2: N = " << N_L2 << ", C = " << C_L2 << std::endl;

    std::vector<uint32_t> L1_cand;
    std::vector<uint32_t> L2_cand;

    std::cout << "Finding L1...\n";
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t init = 1; init < (1U << 30); ++init) {
        uint32_t state = init;
        uint32_t mismatches = 0;
        bool valid = true;

        for (uint32_t i = 0; i < N_L1; ++i) {
            if ((state & 1) != z[i]) {
                mismatches++;
                if (mismatches > C_L1) {
                    valid = false;
                    break; 
                }
            }
            step_L1(state);
        }

        if (valid) {
            #pragma omp critical
            L1_cand.push_back(init);
        }
    }

    std::cout << "L1 candidates found" << L1_cand.size() << std::endl;;

    std::cout << "Finding L2...\n";
    #pragma omp parallel for schedule(dynamic)
    for (uint32_t init = 1; init < (1U << 31); ++init) {
        uint32_t state = init;
        uint32_t mismatches = 0;
        bool valid = true;

        for (uint32_t i = 0; i < N_L2; ++i) {
            if ((state & 1) != z[i]) {
                mismatches++;
                if (mismatches > C_L2) {
                    valid = false;
                    break; 
                }
            }
            step_L2(state);
        }

        if (valid) {
            #pragma omp critical
            L2_cand.push_back(init);
        }
    }

    std::cout << "L2 candidates found" << L2_cand.size() << std::endl;

    std::cout << "Finding L3...\n";
    
    for (uint32_t l1_val : L1_cand) {
        for (uint32_t l2_val : L2_cand) {
            
            uint32_t base_L3 = 0;
            std::vector<int> unknown_bits;
            
            uint32_t state1 = l1_val;
            uint32_t state2 = l2_val;
            
            for (int i = 0; i < 32; ++i) {
                uint32_t x = state1 & 1;
                uint32_t y = state2 & 1;

                if (x != y) {
                    uint32_t s = (z[i] == x) ? 1 : 0;
                    base_L3 |= (s << i);
                } else {
                    unknown_bits.push_back(i);
                }

                step_L1(state1);
                step_L2(state2);
            }
            
            size_t comb = 1ULL << unknown_bits.size();
            for (size_t k = 0; k < comb; ++k) {
                uint32_t cand_L3 = base_L3;
                
                for (size_t i = 0; i < unknown_bits.size(); ++i) {
                    if ((k >> i) & 1) {
                        cand_L3 |= (1U << unknown_bits[i]);
                    }
                }

                if (verify_full_key(l1_val, l2_val, cand_L3, z)) {
                    std::cout << "Key found" << std::endl;
                    std::cout << "L1: " << l1_val << std::endl;
                    std::cout << "L2: " << l2_val << std::endl;
                    std::cout << "L3: " << cand_L3 << std::endl;
                    return 0;
                }
            }
        }
    }

    std::cout << "Key not found." << std::endl;
    return 0;
}