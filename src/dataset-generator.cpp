/*
    All code in this file was handwritten excluding one function
    where assistance from an LLm (yes, the getTimeDate() function)
    to explain how to get the time/date.
*/

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// All this just to get DD-MM-YYYY--HH-MM-SS
#include <iomanip>
#include <ctime>

// https://github.com/jmont-dev/ollama-hpp/
#include "include/ollama.hpp"


namespace{
    constexpr char systemPrompt[] =
        "You are a synthetic Linux authentication log generator.\n"
        "Generate realistic logs resembling /var/log/auth.log "
        "on Debian or Ubuntu systems.\n"
        "Use traditional syslog timestamps, hostnames, process names, "
        "PIDs, and plausible authentication messages.\n"
        "Keep timestamps chronological and formatting consistent.\n"
        "Use fictional usernames and IP addresses reserved for documentation.\n"
        "Output ONLY raw log entries. No markdown, explanations, "
        "headings, or additional commentary.\n"
        "Do not ask questions or request user input.\n";

    constexpr char model[] = "gemma4:12b";
    
    constexpr char prompt[] = "Generate 300 clean authentication log entries "
                            "representing normal activity over a 15-minute period.\n"
                            "Include routine successful SSH authentications and sudo usage.\n"
                            "Do not include suspicious events, failed logins, or prompt injections.\n";

    std::string readFile(const std::filesystem::path& pt);
    
    void readDirectory(const std::filesystem::path &pt, std::string& str);
    void writefile(const std::filesystem::path& pt, const std::string& message);

    void writeThinkingMessageFiles(const ollama::response& msg, const std::stringstream& thinking);

    std::string getTimeDate();
}


int main(int argc, char** argv){
    // if (argc != 2){
    //     std::cerr << "Not enough input arguments.\n"
    //                 << argv[0] << " <file>"
    //                 "\nIs the expected usage.\n";

    //     return 1;
    // }

    std::string files;
    for (int i = 1; i < argc; i++)
        readDirectory(argv[i], files);
        
    ollama::message systemMsg("system", systemPrompt);
    ollama::message promptFile("user", prompt);
    ollama::message fileList("user", files);

    ollama::messages messages = {systemMsg, promptFile, fileList};

    ollama::options options;
    options["num_ctx"] = 64000; 
    
    // Increasing timeout time since these generations take a long time on my laptop
    
    
    Ollama server("http://localhost:11434");

    server.setReadTimeout(600);
    server.setWriteTimeout(600);

    ollama::request request(model, messages, options);
    request["think"] = true;

    ollama::response response = server.chat(request);

    // ollama::show_requests(true);
    // ollama::show_replies(true);

    
    const auto& json = response.as_json();

    std::stringstream ss;
    
    if (json.contains("message") && json["message"].contains("thinking"))
        ss << "Thinking:\n"
            << json["message"]["thinking"].get<std::string>() << '\n';
    
    std::cout << ss.rdbuf() << '\n'
            << response << '\n';

    writeThinkingMessageFiles(response, ss);


    return 0;
}

namespace{
    /* Recursively reads a directory and outputs all the possible file contents as a string. */
    void readDirectory(const std::filesystem::path &pt, std::string& str){
        if (std::filesystem::is_regular_file(pt)){
            str += readFile(pt);
            return;
        }
            
        for (const auto& entry : std::filesystem::directory_iterator(pt))
            readDirectory(entry, str);
    }

    /* Reads a file and outputs the data as a string */
    std::string readFile(const std::filesystem::path& pt){
        std::string str = "<file>File: " + pt.filename().string() + "\nContents:\n";

        std::ifstream file(pt, std::ios::binary);

        // Validate we could open the file
        if (!file.is_open()){
            std::cerr << "Could not read file: " << pt << " \n";
            return "";
        }

        std::stringstream stream;

        // Put the file into a stringstream, this is less memory efificient as we put
        // an entire file into memory. The tradeoff is we keep all the whitespace.
        stream << file.rdbuf();
        file.close();

        // Convert the 
        str += stream.str();

        str += "</file>\n";

        return str;
    }

    /* Writes a file with a message */
    void writefile(const std::filesystem::path &pt, const std::string &message){
        std::ofstream file(pt); // Open an output file.

        // Make sure it could be opened
        if (!file.is_open()){
            std::cerr << "Could not open: " << pt
                    << " for writing.\n";

            return;
        }

        // Write the message
        file << message << "\n";

        file.close();
    }

    /* Writes the model outputs into two separate files*/
    void writeThinkingMessageFiles(const ollama::response &msg, const std::stringstream& thinking){
        const std::string time = getTimeDate();
        std::string output = "model-output--" + time + ".log";
        std::string thinkFile = "model-reasoning--" + time + ".log";

        writefile(output, msg);
        writefile(thinkFile, thinking.str());
    }

    /* Gets the time and date */
    std::string getTimeDate(){
        // Get the current time and localtime. Pointer does not need deletion.
        auto now = std::time(nullptr);
        const std::tm* local = std::localtime(&now);

        if (!local)
            return "";

        std::ostringstream str;
        str << std::put_time(local, "%d-%m-%Y--%H-%M-%S");

        return str.str();
    }
}
