#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <array>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <vector>
typedef std::vector<std::string> str_arr;

namespace fs = std::filesystem;

// Setup popen/pclose names based on the operating system
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#include <windows.h>
fs::path getExecutablePath()
{
    wchar_t buffer[MAX_PATH];

    DWORD length = GetModuleFileNameW(
        nullptr,
        buffer,
        MAX_PATH
    );

    if (length == 0) {
        throw std::runtime_error("Could not determine executable path");
    }

    return fs::path(std::wstring(buffer, length));
}
#endif

// Runs shell commands and retrieves the output
std::string but_i_dont_have_ay_other_funcs_with(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    // Open a pipe to run the command in read mode
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd, "r"), pclose);
    
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    // Read the output chunk by chunk into the string
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    // Remove trailing newline if present so it doesn't break command concatenation
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

#ifdef __APPLE__
#include <mach-o/dyld.h>

fs::path getExecutablePath()
{
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    std::string buffer(size, '\0');

    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        throw std::runtime_error("Could not determine executable path");
    }

    return fs::weakly_canonical(fs::path(buffer));
}
int alterarDescricaoMacOS(const std::string& caminhoAppBundle, const std::string& novaDescricao) {
    // Valida se o caminho termina com .app para evitar alterações incorretas
    if (caminhoAppBundle.find(".app") == std::string::npos) {
        std::cerr << "[macOS] Erro: O caminho deve apontar para um diretório .app\n";
        return -1; // FIXED: Added missing return statement
    }

    // Define a chave NSHumanReadableCopyright
    std::string comando = "defaults write " + caminhoAppBundle + "/Contents/Info NSHumanReadableCopyright \"" + novaDescricao + "\"";
    
    return std::system(comando.c_str());
}
#endif

#ifdef __linux__
#include <unistd.h>
int alterarDescricaoLinux(const std::string& caminhoDesktop, const std::string& novaDescricao) {
    std::ifstream arquivoEntrada(caminhoDesktop);
    if (!arquivoEntrada.is_open()) {
        return 1;
    }

    std::vector<std::string> linhas;
    std::string linha;
    bool chaveEncontrada = false;

    // Lê o arquivo linha por linha
    while (std::getline(arquivoEntrada, linha)) {
        // Se encontrar a linha de comentário/descrição, substitui seu valor
        if (linha.rfind("Comment=", 0) == 0) { 
            linha = "Comment=" + novaDescricao;
            chaveEncontrada = true;
        }
        linhas.push_back(linha);
    }
    arquivoEntrada.close();

    // Se a chave não existia no arquivo, adiciona ela ao final do bloco
    if (!chaveEncontrada) {
        linhas.push_back("Comment=" + novaDescricao);
    }

    // Grava as alterações de volta no arquivo
    std::ofstream arquivoSaida(caminhoDesktop);
    for (const auto& l : linhas) {
        arquivoSaida << l << "\n";
    }
    arquivoSaida.close();
    return 0;
}
fs::path getExecutablePath()
{
    char buffer[4096];

    ssize_t length = readlink(
        "/proc/self/exe",
        buffer,
        sizeof(buffer) - 1
    );

    if (length == -1) {
        throw std::runtime_error("Could not determine executable path");
    }

    buffer[length] = '\0';
    return fs::path(buffer);
}
#endif
std::string join_with_spaces(const std::vector<std::string>& words) {
    if (words.empty()) {
        return "";
    }

    // Performance Optimization: Calculate total length to allocate memory once
    size_t total_length = 0;
    for (const auto& word : words) {
        total_length += word.length() + 1; // +1 for the space
    }

    std::string result;
    result.reserve(total_length);

    // Append the first element
    result += words[0];

    // Append subsequent elements preceded by a space
    for (size_t i = 1; i < words.size(); ++i) {
        result += " ";
        result += words[i];
    }

    return result;
}
int check_app_and_dowload(const std::string& app) {
#ifndef _WIN32
    // Suppress output for clean console
    std::string ok = "which " + app + " > /dev/null 2>&1";
#else
    // FIXED: Typo 's   td::string'
    std::string ok = "winget list " + app + " >nul 2>&1"; 
#endif

    int isthere = std::system(ok.c_str());
    if (isthere != 0) {
#ifdef __linux__
        // Grab the first available package manager
        std::string temp = but_i_dont_have_ay_other_funcs_with("which apt dnf pacman zypper apk xbps-install 2>/dev/null | head -n 1");
        
        // Extract the binary name (e.g., "apt") and append " install "
        std::string instaler = fs::path(temp).filename().string() + " install ";
#elif _WIN32
        // FIXED: Missing semicolon and added " install "
        std::string instaler = "winget install "; 
#else 
        std::string instaler = "brew install ";
#endif
        return std::system((instaler + app).c_str());
    }
    
    // FIXED: Return 0 if the application is already installed
    return 0; 
}
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>

int check_lib_and_download(const std::string& lib) {
    std::string command = "";

    #if defined(_WIN32) || defined(_WIN64)
        // No Windows (ex: Chocolatey ou vcpkg)
        command = "choco install " + lib + " -y";

    #elif defined(__APPLE__) || defined(__MACH__)
        // No macOS (Homebrew)
        command = "brew install " + lib;

    #elif defined(__linux__)
        if (std::filesystem::exists("/usr/bin/pacman")) {
            // Arch Linux: usa o pacman com --needed para garantir que os headers venham juntos
            // Nota: Se a dependência no JSON for "sdl2", o pacman vai instalar o pacote correto do Arch.
            command = "sudo pacman -S --needed --noconfirm " + lib;
        } 
        else if (std::filesystem::exists("/usr/bin/apt")) {
            // Debian / Ubuntu (exige sufixo -dev)
            // Dica: Se no JSON estiver escrito "sdl2", você pode mapear automaticamente para "libsdl2-dev" se quiser!
            std::string apt_lib = (lib == "sdl2") ? "libsdl2-dev" : lib;
            command = "sudo apt update && sudo apt install -y " + apt_lib;
        }
        else if (std::filesystem::exists("/usr/bin/dnf")) {
            // Fedora (exige sufixo -devel)
            std::string dnf_lib = (lib == "sdl2") ? "SDL2-devel" : lib;
            command = "sudo dnf install -y " + dnf_lib;
        }
    #endif

    if (!command.empty()) {
        std::cout << "[Build System] Verificando/Instalando dependencia: " << lib << "\n";
        return std::system(command.c_str());
    }
    
    return 0;
}


std::string exec_command(const std::string& cmd) {
    std::string result = "";
    std::array<char, 128> buffer;
    
    // popen abre um pipe para rodar o comando (funciona no Linux, Mac e Windows com MSYS2/MinGW)
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    pclose(pipe);
    
    // Remove quebras de linha no final da string, se houver
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

// Função inteligente que pega as flags de include e link de uma biblioteca
std::string get_pkg_flags(const std::string& lib_name) {
    std::string flags = "";

    #if defined(_WIN32) || defined(_WIN64)
        // No Windows, se você usar ferramentas como MSYS2/MinGW, o pkg-config existe.
        // Se estiver usando vcpkg, geralmente as flags vêm de variáveis de ambiente.
        std::string cmd = "pkg-config --cflags --libs " + lib_name + " 2>nul";
        flags = exec_command(cmd);
        

        }
    #else
        // No Linux e macOS, o pkg-config é padrão
        std::string cmd = "pkg-config --cflags --libs " + lib_name + " 2>/dev/null";
        flags = exec_command(cmd);

    #endif

    return flags;
}
bool mergeStaticLibraries(const std::string& output_lib, const std::vector<std::string>& input_libs) {
    if (input_libs.empty()) return false;
    std::cout<< "final_lib : " + output_lib + " libs_given : " + join_with_spaces(input_libs);
#if defined(_WIN32) && defined(_MSC_VER)
    // Windows with MSVC: use lib.exe to combine static libraries
    std::string cmd = "lib /OUT:" + output_lib;
    for (const auto& lib : input_libs) {
        // FIX: Check 'lib', not 'cmd'. Also accept .lib files for Windows.
        if (lib.ends_with(".a") || lib.ends_with(".lib")) { 
            cmd += " " + lib;
        }
    }
    return std::system(cmd.c_str()) == 0;
#else
    std::string temp_dir = "ar_merge_temp_dir";
    
    // FIX: Force clean the directory BEFORE starting in case a previous run crashed
    std::system(("rm -rf " + temp_dir).c_str());
    if (std::system(("mkdir -p " + temp_dir).c_str()) != 0) return false;

    // Unix-like systems (Linux, macOS): extract object files safely and pack them
    for (size_t i = 0; i < input_libs.size(); ++i) {
        if (input_libs[i].ends_with(".a")) {
            std::string sub_dir = temp_dir + "/lib_" + std::to_string(i);
            
            std::string extract_cmd = "mkdir -p " + sub_dir + " && cd " + sub_dir + " && ar x ../../" + input_libs[i];
            
            if (std::system(extract_cmd.c_str()) != 0) {
                // Cleanup on failure
                std::system(("rm -rf " + temp_dir).c_str());
                return false;
            }
        }
    }

    // Pack all extracted object files into the final archive
    std::string pack_cmd = "ar crs lib" + output_lib + ".a " + temp_dir + "/lib_*/*.o";
    int pack_result = std::system(pack_cmd.c_str());

    // Cleanup temporary workspace after success
    std::system(("rm -rf " + temp_dir).c_str());

    return pack_result == 0;}
#endif
