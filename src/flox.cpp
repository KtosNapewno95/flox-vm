#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <map>
#include <algorithm>
#include <sys/stat.h>
#include <chrono>   // Do precyzyjnego pomiaru czasu wydajności
#include <windows.h>

using Instruction = uint32_t;

enum Opcode : uint8_t {
    OP_LOADI, OP_ADD, OP_SUB, OP_MUL, OP_DIV, 
    OP_JMP, OP_JMP_LT, OP_JMP_GE, OP_PRINT, OP_HALT, 
    OP_PRINT_STR, OP_MOVE,
    OP_ARR_NEW,  // Alokacja tablicy
    OP_ARR_SET,  // Tablica[indeks] = wartość
    OP_ARR_GET,  // wartość = Tablica[indeks]
    OP_ARR_ADD   // Wektorowe dodawanie dwóch tablic (Szybkość sprzętowa!)
};

#define GET_OPCODE(i) static_cast<Opcode>((i) & 0xFF)
#define GET_REG_A(i)  (((i) >> 8) & 0xFF)
#define GET_REG_B(i)  (((i) >> 16) & 0xFF)
#define GET_REG_C(i)  (((i) >> 24) & 0xFF)
// NOWE: Pobieramy 16 bitów (bity z pozycji B i C) i rzutujemy na int16_t
#define GET_OFFSET(i) static_cast<int16_t>(((i) >> 16) & 0xFFFF)

constexpr Instruction make_ins(Opcode op, uint8_t a, uint8_t b, uint8_t c) {
    return static_cast<Instruction>(op) | (static_cast<Instruction>(a) << 8) | (static_cast<Instruction>(b) << 16) | (static_cast<Instruction>(c) << 24);
}

// NOWE: Specjalny konstruktor dla instrukcji skokowych upakowujący 16-bitowy offset
constexpr Instruction make_jmp_ins(Opcode op, uint8_t a, int16_t offset) {
    return static_cast<Instruction>(op) | (static_cast<Instruction>(a) << 8) | ((static_cast<Instruction>(offset) & 0xFFFF) << 16);
}


time_t get_file_mtime(const std::string& path) {
    struct stat res; return (stat(path.c_str(), &res) == 0) ? res.st_mtime : 0;
}

bool is_number(const std::string& s) {
    if (s.empty()) return false;
    size_t st = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    return std::all_of(s.begin() + st, s.end(), ::isdigit);
}

std::string trim(const std::string& s) {
    size_t f = s.find_first_not_of(" \t\r\n"), l = s.find_last_not_of(" \t\r\n");
    return (f == std::string::npos) ? "" : s.substr(f, (l - f + 1));
}

struct Compiler {
    std::map<std::string, uint8_t> vars; 
    std::vector<std::string> string_pool; 
    uint8_t next_reg = 0;

    uint8_t get_reg(const std::string& n) { 
        if (vars.find(n) == vars.end()) vars[n] = next_reg++; 
        return vars[n]; 
    }

    std::vector<Instruction> compile(const std::string& src) {
        std::vector<Instruction> bc; std::istringstream stream(src); std::string line; int line_num = 0;
        std::vector<std::string> blocks; std::vector<size_t> l_starts, l_exits, if_jumps, else_jumps;
        std::vector<uint8_t> f_regs; std::vector<std::string> f_steps;

        while (std::getline(stream, line)) {
            line_num++; size_t comment = line.find('#'); if (comment != std::string::npos) line = line.substr(0, comment);
            line = trim(line); if (line.empty()) continue;

            if (line == "}") {
                if (blocks.empty()) { std::cerr << "[Błąd] Klamra '}' w linii " << line_num << "\n"; exit(1); }
                std::string type = blocks.back(); blocks.pop_back();
                if (type == "while") {
                    bc.push_back(make_jmp_ins(OP_JMP, 0, static_cast<int16_t>(l_starts.back() - bc.size() - 1))); l_starts.pop_back();
                    size_t e = l_exits.back(); l_exits.pop_back();
                    bc[e] = make_jmp_ins(OP_JMP_GE, GET_REG_A(bc[e]), static_cast<int16_t>(bc.size() - e - 1));
                } else if (type == "for") {
                    bc.push_back(make_ins(OP_ADD, f_regs.back(), f_regs.back(), get_reg(f_steps.back()))); f_regs.pop_back(); f_steps.pop_back();
                    bc.push_back(make_jmp_ins(OP_JMP, 0, static_cast<int16_t>(l_starts.back() - bc.size() - 1))); l_starts.pop_back();
                    size_t e = l_exits.back(); l_exits.pop_back();
                    bc[e] = make_jmp_ins(OP_JMP_GE, GET_REG_A(bc[e]), static_cast<int16_t>(bc.size() - e - 1));
                } else if (type == "if") {
                    size_t idx = if_jumps.back(); if_jumps.pop_back();
                    bc[idx] = make_jmp_ins(OP_JMP_GE, GET_REG_A(bc[idx]), static_cast<int16_t>(bc.size() - idx - 1));
                } else if (type == "else") {
                    size_t idx = else_jumps.back(); else_jumps.pop_back();
                    bc[idx] = make_jmp_ins(OP_JMP, 0, static_cast<int16_t>(bc.size() - idx - 1));
                }
                continue;
            }


            if (!(line.rfind("while", 0) == 0 || line.rfind("for", 0) == 0 || line.rfind("if", 0) == 0 || line.rfind("else", 0) == 0 || line == "{")) {
                if (line.back() != ';') { std::cerr << "[Błąd] Brak średnika ';' w linii " << line_num << "\n"; exit(1); }
                line.pop_back(); line = trim(line);
            }

            std::istringstream ls(line); std::string tok; ls >> tok; if (tok == "{") continue;

            if (tok.rfind("print", 0) == 0) {
                size_t s = line.find('('), e = line.find(')');
                if (s != std::string::npos && e != std::string::npos && e > s) {
                    std::string cnt = trim(line.substr(s + 1, e - s - 1));
                    if (cnt.size() >= 2 && cnt.front() == '"' && cnt.back() == '"') {
                        bc.push_back(make_ins(OP_PRINT_STR, string_pool.size(), 0, 0)); string_pool.push_back(cnt.substr(1, cnt.size() - 2));
                    } else { bc.push_back(make_ins(OP_PRINT, get_reg(cnt), 0, 0)); }
                }
            } else if (tok == "while") {
                std::string v1, op, v2; ls >> v1 >> op >> v2; blocks.push_back("while"); l_starts.push_back(bc.size());
                bc.push_back(make_ins(OP_JMP_GE, get_reg(v1), get_reg(v2), 0)); l_exits.push_back(bc.size() - 1);
            } else if (tok == "for") {
                std::string v, eq, sv, to_t, lv, k_t, kv; ls >> v >> eq >> sv >> to_t >> lv >> k_t >> kv; blocks.push_back("for");
                bc.push_back(make_ins(OP_LOADI, get_reg(v), 0, std::stoi(sv))); bc.push_back(make_ins(OP_LOADI, get_reg("__limit_" + v), 0, std::stoi(lv)));
                bc.push_back(make_ins(OP_LOADI, get_reg("__step_" + v), 0, std::stoi(kv))); l_starts.push_back(bc.size());
                f_regs.push_back(get_reg(v)); f_steps.push_back("__step_" + v);
                bc.push_back(make_ins(OP_JMP_GE, get_reg(v), get_reg("__limit_" + v), 0)); l_exits.push_back(bc.size() - 1);
            } else if (tok == "if") {
                std::string v1, op, v2; ls >> v1 >> op >> v2; blocks.push_back("if");
                bc.push_back(make_ins(OP_JMP_GE, get_reg(v1), get_reg(v2), 0)); if_jumps.push_back(bc.size() - 1);
            } else if (tok == "else") {
                blocks.push_back("else"); size_t if_idx = if_jumps.back(); if_jumps.pop_back();
                bc.push_back(make_ins(OP_JMP, 0, 0, 0)); else_jumps.push_back(bc.size() - 1);
                bc[if_idx] = make_ins(OP_JMP_GE, GET_REG_A(bc[if_idx]), GET_REG_B(bc[if_idx]), static_cast<uint8_t>(bc.size() - if_idx - 1));
            }

            else if (tok == "array") { // array T = size
                std::string name, eq, size_var; ls >> name >> eq >> size_var;
                bc.push_back(make_ins(OP_ARR_NEW, get_reg(name), get_reg(size_var), 0));
            }
            else if (tok.rfind("vec_add", 0) == 0) {
                size_t s = line.find('('), c = line.find(','), e = line.find(')');
                std::string t1 = trim(line.substr(s + 1, c - s - 1)), t2 = trim(line.substr(c + 1, e - c - 1));
                bc.push_back(make_ins(OP_ARR_ADD, get_reg(t1), get_reg(t2), 0));
            }
            else if (tok.rfind("set", 0) == 0) {
                size_t s = line.find('('), c1 = line.find(','), c2 = line.find(',', c1 + 1), e = line.find(')');
                std::string tab = trim(line.substr(s + 1, c1 - s - 1)), idx = trim(line.substr(c1 + 1, c2 - c1 - 1)), val = trim(line.substr(c2 + 1, e - c2 - 1));
                bc.push_back(make_ins(OP_ARR_SET, get_reg(tab), get_reg(idx), get_reg(val)));
            }
            else if (tok.rfind("get", 0) == 0) {
                size_t s = line.find('('), c1 = line.find(','), c2 = line.find(',', c1 + 1), e = line.find(')');
                std::string var = trim(line.substr(s + 1, c1 - s - 1)), tab = trim(line.substr(c1 + 1, c2 - c1 - 1)), idx = trim(line.substr(c2 + 1, e - c2 - 1));
                bc.push_back(make_ins(OP_ARR_GET, get_reg(var), get_reg(tab), get_reg(idx)));
            }
            else {
                std::string dest = tok, eq, a1; ls >> eq >> a1; if (eq != "=") continue; std::string op, a2;
                if (ls >> op >> a2) {
                    Opcode o = (op == "+") ? OP_ADD : (op == "-") ? OP_SUB : (op == "*") ? OP_MUL : OP_DIV;
                    bc.push_back(make_ins(o, get_reg(dest), get_reg(a1), get_reg(a2)));
                } else {
                    if (a1.find_first_not_of("0123456789-+") != std::string::npos) bc.push_back(make_ins(OP_MOVE, get_reg(dest), get_reg(a1), 0));
                    else bc.push_back(make_ins(OP_LOADI, get_reg(dest), 0, std::stoi(a1)));
                }
            }
        }
        bc.push_back(make_ins(OP_HALT, 0, 0, 0)); return bc;
    }
};

void save_compiled_file(const std::string& path, const std::vector<Instruction>& bc, const std::vector<std::string>& sp) {
    std::ofstream f(path, std::ios::binary); if (!f.is_open()) return;
    size_t b_sz = bc.size(); f.write(reinterpret_cast<const char*>(&b_sz), sizeof(b_sz));
    f.write(reinterpret_cast<const char*>(bc.data()), b_sz * sizeof(Instruction));
    size_t s_sz = sp.size(); f.write(reinterpret_cast<const char*>(&s_sz), sizeof(s_sz));
    for (const auto& str : sp) {
        size_t len = str.size(); f.write(reinterpret_cast<const char*>(&len), sizeof(len)); f.write(str.data(), len);
    }
}

void load_compiled_file(const std::string& path, std::vector<Instruction>& bc, std::vector<std::string>& sp) {
    std::ifstream f(path, std::ios::binary); if (!f.is_open()) return;
    size_t b_sz = 0; f.read(reinterpret_cast<char*>(&b_sz), sizeof(b_sz)); bc.resize(b_sz);
    f.read(reinterpret_cast<char*>(bc.data()), b_sz * sizeof(Instruction));
    size_t s_sz = 0; f.read(reinterpret_cast<char*>(&s_sz), sizeof(s_sz)); sp.resize(s_sz);
    for (size_t i = 0; i < s_sz; ++i) {
        size_t len = 0; f.read(reinterpret_cast<char*>(&len), sizeof(len)); sp[i].resize(len);
    if (len > 0) f.read(&sp[i][0], len);
    }
}

// faster virtual machine
void run_vm(const std::vector<Instruction>& bytecode, const std::vector<std::string>& string_pool) {
    if (bytecode.empty()) return;


    int64_t R[256] = {0}; 


    std::vector<int64_t> arrays[256];


    const Instruction* pc = bytecode.data();

    // computed goto
    static const void* dispatch_table[] = {
        &&do_loadi, &&do_add, &&do_sub, &&do_mul, &&do_div, 
        &&do_jmp, &&do_jmp_lt, &&do_jmp_ge, &&do_print, &&do_halt,
        &&do_print_str, &&do_move, &&do_arr_new, &&do_arr_set, &&do_arr_get, &&do_arr_add
    };

    #define DISPATCH() goto *dispatch_table[GET_OPCODE(*pc++)]
    
    // start
    DISPATCH();

do_loadi:  { Instruction i = pc[-1]; R[GET_REG_A(i)] = GET_REG_C(i); DISPATCH(); }
do_add:    { Instruction i = pc[-1]; R[GET_REG_A(i)] = R[GET_REG_B(i)] + R[GET_REG_C(i)]; DISPATCH(); }
do_sub:    { Instruction i = pc[-1]; R[GET_REG_A(i)] = R[GET_REG_B(i)] - R[GET_REG_C(i)]; DISPATCH(); }
do_mul:    { Instruction i = pc[-1]; R[GET_REG_A(i)] = R[GET_REG_B(i)] * R[GET_REG_C(i)]; DISPATCH(); } 
do_div:    { 
    Instruction i = pc[-1]; 
    if (R[GET_REG_C(i)] != 0) R[GET_REG_A(i)] = R[GET_REG_B(i)] / R[GET_REG_C(i)]; 
    DISPATCH(); 
}
do_jmp:    { Instruction i = pc[-1]; pc += GET_OFFSET(i); DISPATCH(); }
do_jmp_lt: { Instruction i = pc[-1]; if (R[GET_REG_A(i)] < R[GET_REG_B(i)]) pc += GET_OFFSET(i); DISPATCH(); }
do_jmp_ge: { Instruction i = pc[-1]; if (R[GET_REG_A(i)] >= R[GET_REG_B(i)]) pc += GET_OFFSET(i); DISPATCH(); } 
do_print:  { Instruction i = pc[-1]; std::cout << R[GET_REG_A(i)] << "\n"; DISPATCH(); }
do_print_str: { Instruction i = pc[-1]; std::cout << string_pool[GET_REG_A(i)] << "\n"; DISPATCH(); }
do_move:   { Instruction i = pc[-1]; R[GET_REG_A(i)] = R[GET_REG_B(i)]; DISPATCH(); }


do_arr_new: {
    Instruction i = pc[-1];
    int64_t size = R[GET_REG_B(i)];
    arrays[GET_REG_A(i)].assign(size, 0);
    DISPATCH();
}
do_arr_set: {
    Instruction i = pc[-1];
    int64_t idx = R[GET_REG_B(i)];
    int64_t val = R[GET_REG_C(i)];
    arrays[GET_REG_A(i)][idx] = val;
    DISPATCH();
}
do_arr_get: {
    Instruction i = pc[-1];
    int64_t idx = R[GET_REG_C(i)];
    R[GET_REG_A(i)] = arrays[GET_REG_B(i)][idx];
    DISPATCH();
}
do_arr_add: {
    Instruction i = pc[-1];
    auto& target_array = arrays[GET_REG_A(i)];
    auto& source_array = arrays[GET_REG_B(i)];
    size_t size = target_array.size();
    

    for (size_t k = 0; k < size; ++k) {
        target_array[k] += source_array[k];
    }
    DISPATCH();
}

do_halt:   { return; }

#undef DISPATCH
}


int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    if (argc < 2) { std::cout << "Użycie: " << argv[0] << " <plik.flox>\n"; return 1; }
    std::string flox_path = argv[1]; 
    if (flox_path.size() < 5 || flox_path.substr(flox_path.size() - 5) != ".flox") return 1;
    std::string floxc_path = flox_path + "c"; 
    std::vector<Instruction> program; std::vector<std::string> strings;
    time_t f_time = get_file_mtime(flox_path), c_time = get_file_mtime(floxc_path);
    
    if (c_time != 0 && c_time >= f_time) {
        std::cout << "[Flox] Ładowanie skompilowanego pliku binarnego: " << floxc_path << "\n";
        load_compiled_file(floxc_path, program, strings);
    } else {
        std::cout << "[Flox] Plik tekstowy zmieniony lub brak .floxc. Kompilacja...\n";
        std::ifstream file(flox_path); if (!file.is_open()) return 1;
        std::stringstream buffer; buffer << file.rdbuf();
        Compiler compiler; program = compiler.compile(buffer.str());
        strings = compiler.string_pool; save_compiled_file(floxc_path, program, strings);
    }


    auto start_time = std::chrono::high_resolution_clock::now();
    
    run_vm(program, strings); 
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    
    std::cout << "\n[Flox VM Performance]: Kod wykonany w " << elapsed << " mikrosekund!\n";
    return 0;
}
