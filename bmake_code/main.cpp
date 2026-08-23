//main.cpp
#include <iostream>
#include <string>
int check_app_and_dowload(const std::string& app);
#include "nlohmann/json.hpp"
using json = nlohmann::json;
#include <filesystem>
#include <thread>
#include <vector>
#include <fstream>
typedef std::vector<std::string> str_arr;

#include <future>
bool mergeStaticLibraries(const std::string& output_lib, const std::vector<std::string>& input_libs);
std::string apenas_libs(const std::string& pkg_args) {
    std::stringstream ss(pkg_args);
    std::string token;
    std::string result = "";

    while (ss >> token) {
        // Se o token NÃO começa com "-I", nós mantemos
        if (token.rfind("-I", 0) != 0) {
            if (!result.empty()) {
                result += " ";
            }
            result += token;
        }
    }
    return result;
}

bool has_item(str_arr vec,std::string item){
    return std::find(vec.begin(), vec.end(), item) != vec.end();
}
json config_file;
std::string get_pkg_flags(const std::string& lib_name);
int check_lib_and_download(const std::string& lib);
str_arr executarEmThreads(const std::vector<std::filesystem::path>& caminhos, std::string pkg_args, std::string (*func)(std::filesystem::path, std::string)) {
    std::vector<std::future<std::string>> futuros;
    futuros.reserve(caminhos.size());

    // Dispara uma thread própria para cada path do vetor usando uma lambda
    for (const auto& path : caminhos) {
        futuros.emplace_back(std::async(std::launch::async, [func, path, pkg_args]() {
            return func(path, pkg_args);
        }));
    }

    str_arr resultados;
    resultados.reserve(caminhos.size());

    // Código principal coleta as strings retornadas pelas threads
    for (auto& f : futuros) {
        resultados.push_back(f.get()); 
    }

    return resultados;
}
// Create the 'fs' alias for the namespace

namespace fs = std::filesystem; 
fs::path getExecutablePath();
void add_two_vectors(std::vector<fs::path> &ok, const std::vector<fs::path> &ok1) {
    ok.insert(ok.end(), ok1.begin(), ok1.end());
}
std::string build(fs::path code_files,std::string pkg_args);

std::vector<fs::path> getFilesWithExtension(std::string_view extension)
{
    std::vector<fs::path> matchingFiles;
    const fs::path dirPath = fs::current_path();

    if (!fs::is_directory(dirPath)) {
        return matchingFiles;
    }

    for (fs::recursive_directory_iterator it(dirPath), end;
         it != end;
         ++it)
    {
        std::error_code ec;

        if (it->is_directory(ec)) {
            if (it->path().filename() == "temp") {
                it.disable_recursion_pending();
            }

            continue;
        }

        if (it->is_regular_file(ec) &&
            it->path().extension().string() == extension)
        {
            matchingFiles.push_back(
                fs::relative(it->path(), dirPath, ec)
            );
        }
    }

    return matchingFiles;
}

std::vector<fs::path> getChildrenFolders()
{
    std::vector<fs::path> folders;
    const fs::path dirPath = fs::current_path();

    if (!fs::is_directory(dirPath)) {
        return folders;
    }

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        std::error_code ec;

        if (entry.is_directory(ec) &&
            entry.path().filename() != "temp")
        {
            folders.push_back(entry.path().filename());
        }
    }

    return folders;
}
bool copy_from_child_to_current(const std::string& child_dir_name,
                                  const std::string& file_name)
{
    try {
        fs::path current_dir = fs::current_path();
        fs::path source      = current_dir / child_dir_name / file_name;
        fs::path destination = current_dir / file_name;

        if (!fs::exists(source) || !fs::is_regular_file(source)) {
            std::cerr << "Source file not found or not a regular file: "
                      << source << '\n';
            return false;
        }

        // Ensure destination's parent exists (it should, but if current_dir is odd it helps)
        if (!fs::exists(destination.parent_path())) {
            std::cerr << "Destination parent directory missing: "
                      << destination.parent_path() << '\n';
            return false;
        }

        fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        return false;
    }
}
void run_config_commands(const nlohmann::json& config, const std::string& key) {
    if (config.contains(key)) {
        const auto& target = config[key];

        if (target.is_array()) {
            for (const auto& command : target) {
                if (command.is_string()) {
                    system(command.get<std::string>().c_str());
                }
            }
        } 
        else if (target.is_string()) {
            system(target.get<std::string>().c_str());
        }
    }
}

int main(int argc, char* argv[]){
    check_app_and_dowload("rustc");
    check_app_and_dowload("g++");
    #ifdef _WIN32
    system("rustup target add x86_64-pc-windows-gnu");
    #endif
        
    str_arr cli_arguments;
    if(argc < 2){
        goto no_arguments;
    }
    {
    int temp {1};
    std::string dir = argv[1];
    if(fs::is_directory(dir)){
            temp = 2;
            try{
            fs::current_path(dir);
        }catch(const fs::filesystem_error& e){
            std::cout<<"error when changing the directory:" << e.what();
            return 1;
        }
        }

        for(; temp < argc;temp ++){
            cli_arguments.push_back(argv[temp]);
        }
    }

    no_arguments:

    // FIX 1: Safely open and parse config.json
    std::ifstream NO_text_config_file("config.json");


    try {
        config_file = json::parse(NO_text_config_file);
    } catch (const json::parse_error& e) {
         config_file = {

         };
    }
    run_config_commands(config_file,"prebuild");
            if(has_item(cli_arguments,"-rebuild") or (config_file.contains("rebuild"))){
                fs::remove_all("temp");
    
    }
    std::string out_name;
    if (config_file.contains("name") && config_file["name"].is_string()) {
        out_name = config_file["name"].get<std::string>();
    } else {
        out_name = fs::current_path().filename().string();
    }
    // 1. Entra na pasta temp ou prepara o ambiente
    if(config_file.contains("files") && config_file["files"].is_array()){
        for (auto folder : getChildrenFolders()){
            if(std::find(
        config_file["files"].begin(),
    config_file["files"].end(),
    folder.string()
) == config_file["files"].end()){
        continue;     
}   
            std::string config;
            if(config_file.contains(folder.string())){
                config = config_file.at(folder.string()).dump(4);

            }
            else{
                json temporary = config_file;
                temporary["name"] = folder.string() + temporary["name"].get<std::string>();
                config = temporary.dump(4);
            }

            
    fs::path filePath = folder / "config.json";

    std::ofstream file(filePath);

    if (!file) {
        throw std::runtime_error("Could not open file: " + filePath.string());
    }

    file << config;
    file.close();
std::string exec_path = fs::absolute(getExecutablePath()).string();
    
    // 2. Monta o comando com ASPAS em volta dos caminhos para evitar bugs com espaços
    std::string cmd = "\"" + exec_path + "\" \"" + folder.string() + "\" -makelib";        
    
    std::cout << "\n[BUILD MODULE] Entrando na sub-pasta: " << folder.string() << "...\n";
    system(cmd.c_str());
}
    }
        auto code_files {getFilesWithExtension(".cpp")};
    add_two_vectors(code_files,getFilesWithExtension(".c++"));
    add_two_vectors(code_files,getFilesWithExtension(".c"));
    add_two_vectors(code_files,getFilesWithExtension(".rs"));
    add_two_vectors(code_files,getFilesWithExtension(".a"));
    add_two_vectors(code_files,getFilesWithExtension(".o"));
    fs::path target_path = fs::current_path() /= "temp";
    fs::create_directories(target_path);
    fs::current_path(target_path);

    std::string pkg_args = "";

    // 2. PRIMEIRO: Baixa e configura as dependências (isso roda o sudo pacman antes de tudo)
    if (config_file.contains("dependencies") && config_file["dependencies"].is_array()) {
        for (std::string lib : config_file["dependencies"]) {
            // Executa o pacman/apt/brew para garantir que o pacote existe no sistema
            check_lib_and_download(lib);

            // Pega as flags do pkg-config (ex: -I/usr/include/SDL2 -lSDL2)
            auto pkg_flags = get_pkg_flags(lib);
            if (!pkg_flags.empty()) {
                pkg_args += pkg_flags + " ";
            }
        }
    }

    // 3. SEGUNDO: Só agora que os headers estão garantidos, roda a compilação paralela
    auto allo_files = executarEmThreads(code_files, pkg_args,build);
    std::string all_files{};
    
    // FIX 3: Ignore empty strings returned by the cache
    for (const auto& jo : allo_files){
        if (!jo.empty() ) {
            all_files += jo + " ";
        }
    }
    std::string link_tags = "";
    if (config_file.contains("link_tags")) {
        link_tags = config_file["link_tags"].get<std::string>();
    }
bool has_build_worked {true};
    // O -o e o out_name ficam juntos e isolados. O pkg_args vai no final.
    // Exemplo de como o comando final deve ser montado:
    if(has_item(cli_arguments,"-makelib") or (config_file.contains("makelib"))){
        if(mergeStaticLibraries(out_name,allo_files)){
        fs::current_path("../");
        copy_from_child_to_current("temp","lib" + out_name + ".a");
    }
    else{
        has_build_worked = false;
    }
    }
    if(!has_item(cli_arguments,"-makelib") and !(config_file.contains("makelib"))){
std::string cmd = "g++ -Wl,--start-group " + all_files +
                   " -Wl,--end-group -o " + out_name + " " + apenas_libs(pkg_args) + " " + link_tags;
   std::cout << "comands are :" + cmd;
                   std::system(cmd.c_str());
    fs::current_path("../");
    if(!copy_from_child_to_current("temp",out_name)){
        has_build_worked = false;
    }
    }
    if(has_build_worked){
    run_config_commands(config_file,"afterbuild");
    return 0; }
    else{
        return 1;
    }

}
