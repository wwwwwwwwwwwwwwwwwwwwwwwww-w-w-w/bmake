//build.cpp

#include <cstdlib>
#include <fstream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include "nlohmann/json.hpp"
#include <vector>
#include <fstream>
#include <functional>
typedef std::vector<std::string> str_arr;
namespace fs = std::filesystem; 
const std::map<std::string,std::string> code_and_extension {
    {".c++" ,"c++"},
    {".rs","rust"},
    {".cpp","c++"},
    {".c","c"},
    {".a","a"},
    {".o","o"}
};

extern nlohmann::json config_file;
std::string remove_i_flags(const std::string& flags) {
    std::stringstream ss(flags);
    std::string token;
    std::string result = "";

    while (ss >> token) {
        // Verifica se o token NÃO começa com "-I"
        if (token.rfind("-I", 0) != 0) {
            if (!result.empty()) {
                result += " ";
            }
            result += token;
        }
    }
    return result;
}
std::string build(fs::path file, std::string pkg_args) {
    str_arr afiles;
std::string ext = file.extension().string();
auto it = code_and_extension.find(ext);

if (it != code_and_extension.end()) {
    
    // Verifica se a linguagem está permitida no config.json
    bool is_lang_allowed = !config_file.contains("languages") || 
                           config_file["languages"].empty() || 
                           std::find(config_file["languages"].begin(), config_file["languages"].end(), it->second) != config_file["languages"].end();
    
    // Roda se for permitido OU se já for um objeto/biblioteca estática
    if (is_lang_allowed || it->second == "a" || it->second == "o") {
        
        // 1. Garante que o diretório de hashes existe
        // ... (o resto do seu código continua exatamente igual daqui pra baixo){
        
        // 1. Garante que o diretório de hashes existe
        fs::create_directories("hash");

        // 2. Lê o arquivo fonte e calcula o novo hash
        std::hash<std::string> hasher;
        std::ifstream source_file(file);
        std::stringstream buffer;
        buffer << source_file.rdbuf();
        std::string content = buffer.str();
        
        size_t new_hash = hasher(content);
        size_t old_hash = 0;

        // 3. Lê o hash antigo evitando colisão de arquivos com mesmo nome em subpastas
        std::string safe_hash_name = file.string();
        std::replace(safe_hash_name.begin(), safe_hash_name.end(), '/', '_');
        std::replace(safe_hash_name.begin(), safe_hash_name.end(), '\\', '_');
        fs::path hash_path = fs::path("hash") / (safe_hash_name + ".hash");

        std::ifstream hash_in(hash_path);
        if (hash_in.is_open()) {
            hash_in >> old_hash;
            hash_in.close();
        }

        std::string stem_str = file.stem().string();
        std::string out_a_file = stem_str + ".a";
        std::string out_o_file = stem_str + ".o";

        // 4. Verifica se precisa recompilar (Hash diferente ou arquivo .a ausente)
        if (new_hash != old_hash || (!fs::exists(out_a_file) && code_and_extension.at(ext) != "a")) {
            
            std::string lang = code_and_extension.at(ext);

            if (lang == "c++") {
                std::string compile_tags = config_file.contains("compile_tags") && config_file["compile_tags"].contains("c++") 
                                           ? std::string(config_file["compile_tags"]["c++"]) : "";
                
                std::string comand = "g++ -c " + compile_tags + " " + pkg_args + " -o " + out_o_file + " ../" + file.string();
                std::cout << "\ncompling a c++ file with: " << comand << "\n\n";
                system(comand.c_str());

                std::string temp = "ar rcs " + out_a_file + " " + out_o_file;
                system(temp.c_str());
                afiles.push_back(out_a_file);
            }

            else if (lang == "c") {
                std::string compile_tags = config_file.contains("compile_tags") && config_file["compile_tags"].contains("c") 
                                           ? std::string(config_file["compile_tags"]["c"]) : "";
                
                std::string comand = "gcc -c " + compile_tags + " " + pkg_args + " -o " + out_o_file + " ../" + file.string();
                std::cout << "\ncompling a c file with: " << comand << "\n\n";
                system(comand.c_str());

                std::string temp = "ar rcs " + out_a_file + " " + out_o_file;
                system(temp.c_str());
                afiles.push_back(out_a_file);
            }
            else if (lang == "a") {
                afiles.push_back("../" + file.string());
            }
            else if (lang == "o") {
                std::string temp = "ar rcs " + out_a_file + " ../" + out_o_file;
                system(temp.c_str());
                afiles.push_back( out_a_file);
            }
            else if (lang == "rust") {
                std::string compile_tags = config_file.contains("compile_tags") && config_file["compile_tags"].contains("rust") 
                                           ? std::string(config_file["compile_tags"]["rust"]) : "";
                #ifdef _WIN32
                    std::string cmd = "rustc --crate-type=staticlib --target=x86_64-pc-windows-gnu ../" + file.string() + " --crate-name=" + stem_str + " -o " + out_a_file + " " + compile_tags + " " + remove_i_flags(pkg_args);
                #else
                    std::string cmd = "rustc --crate-type=staticlib ../" + file.string() + " --crate-name=" + stem_str + " -o " + out_a_file + " " + compile_tags + " " + remove_i_flags(pkg_args);
                #endif
                system(cmd.c_str());
                afiles.push_back(out_a_file);
            }
            else if (lang == "zig") {
                std::string compile_tags = config_file.contains("compile_tags") && config_file["compile_tags"].contains("zig") 
                                           ? std::string(config_file["compile_tags"]["zig"]) : "";
                #ifdef _WIN32
                    std::string cmd = "zig build-lib ../" + file.string() + " -target x86_64-windows-gnu -femit-bin=" + out_a_file + " " + compile_tags;
                #else
                    std::string cmd = "zig build-lib ../" + file.string() + " -femit-bin=" + out_a_file + " " + compile_tags;
                #endif

                std::cout << "\n[DEBUG COMPILACAO] " << cmd << "\n\n";
                system(cmd.c_str());
                afiles.push_back(out_a_file);
            }
            // 5. Atualiza o arquivo de hash
            std::ofstream hash_out(hash_path);
            hash_out << new_hash;
            
        } else {
            // 6. Cache hit
            if (code_and_extension.at(ext) == "a") {
                afiles.push_back("../" + file.string());
            } else {
                afiles.push_back(out_a_file);
            }
        }
    }

   
}  return afiles.empty() ? "" : afiles[0];}
