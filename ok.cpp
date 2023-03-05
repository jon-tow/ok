// `ok` is a command line tool that uses the OpenAI API to generate shell commands
// and other command line utilities.
//
// NOTE: Make sure your OpenAI auth key is in your environment.
//
//    export OPENAI_API_KEY=sk-***********
//
// Dependencies:
//
//    - json-c: https://github.com/json-c/json-c
//      Install (mac): `brew install json-c`
//
//    - libcurl: http://curl.haxx.se/libcurl/c
//
// Build:
//
//    mkdir build; cd build; cmake ..
//    make; chmod 777 ok
//
// Optionally, copy it to somewhere in your PATH:
//
//    cp ok /usr/local/bin
//
// Example:
//
//    ok how do i delete a branch in git

#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <json-c/json.h>
#include <string>

const std::string URL = "https://api.openai.com/v1/completions";
const std::string MODEL = "code-davinci-002";
const std::string INSTRUCTION = "You're an expert programming assistant. You can help me write "
                                "shell commands and use command line tools. For example:\n\n"
                                "Me: grep for the word \"hello\" in the file \"hello.txt\"\n"
                                "Ok: grep \"hello\" hello.txt\n"
                                "\n###\n"
                                "Me: how can i get the first 10 lines of a file?\n"
                                "Ok: head -n 10\n"
                                "\n###\n"
                                "Me: ";

std::string ltrim(const std::string& s) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

void cleanup(CURL* curl, curl_slist* headers) {
    if (curl)
        curl_easy_cleanup(curl);
    if (headers)
        curl_slist_free_all(headers);
    curl_global_cleanup();
}

// curl callback function to append response to a string
size_t curl_callback(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

int main(int argc, char** argv) {
    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = nullptr; // http headers to send with request
    struct json_object* jobj = nullptr;

    std::string response;
    json_object* json;
    enum json_tokener_error jerr = json_tokener_success;

    // get auth key from env
    std::string AUTH_KEY;
    if (const char* env_p = std::getenv("OPENAI_API_KEY")) {
        AUTH_KEY = env_p;
    } else {
        std::cerr << "\033[1;31mError\033[0m: `OPENAI_API_KEY` is not set\n"
                     "export OPENAI_API_KEY=sk-*********"
                  << std::endl;
        return 1;
    }

    curl_global_init(CURL_GLOBAL_ALL);

    // create full prompt from base prompt and command line args
    std::string prompt = INSTRUCTION;
    for (int i = 1; i < argc; i++) {
        prompt += " ";
        prompt += argv[i];
    }

    // init curl handle
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "ERROR: curl_easy_init() failed" << std::endl;
        cleanup(curl, headers);
        return 1;
    }

    // set content-type
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    const std::string BEARER = "Authorization: Bearer " + AUTH_KEY;
    headers = curl_slist_append(headers, BEARER.c_str());

    // create json object for post
    json = json_object_new_object();

    // build post data
    json_object_object_add(json, "model", json_object_new_string(MODEL.c_str()));
    json_object_object_add(json, "prompt", json_object_new_string(prompt.c_str()));
    json_object_object_add(json, "max_tokens", json_object_new_int(32));
    json_object_object_add(json, "temperature", json_object_new_double(0.0));

    // set curl options
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // set timeout to 20 seconds
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
    // Enable the automatic handling of redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);

    // ignore ssl verification if you don't have the certificate or don't want secure
    // communication
    // you don't need these two lines if you are dealing with HTTP url
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // pass the url
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
    // pass the variable to hold the received data
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // execute request
    CURLcode status = curl_easy_perform(curl);
    if (status != 0) {
        std::cout << "Error: Request failed on URL : " << URL << std::endl;
        std::cout << "Error Code: " << status << " "
                  << "Error Detail : " << curl_easy_strerror(status) << std::endl;
        cleanup(curl, headers);
        return 1;
    }

    // parse json response
    jobj = json_tokener_parse(response.c_str());
    json_object* choices = json_object_object_get(jobj, "choices");
    json_object* choices_entry = json_object_array_get_idx(choices, 0);
    json_object* text = json_object_object_get(choices_entry, "text");
    std::string out = std::string(json_object_get_string(text));

    // prettify response output string
    out = out.substr(0, out.find("\n###\n"));
    out = out.substr(out.find_first_of("Ok:") + 3, out.length());
    out.erase(std::remove(out.begin(), out.end(), '\n'), out.cend());
    out = ltrim(out);

    // print output
    std::cout << "\033[1;33mOK\033[0m: " << out << std::endl;

    // do final clean up
    cleanup(curl, headers);
    json_object_put(json); // free json object

    return 0;
}
