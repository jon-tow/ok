## ok

`ok` is a command line tool that uses the OpenAI Completion API to generate shell commands and other command line utilities so you don't have to leave your terminal to search for them.

![ok](./ok-screenshot.png)

**Note**: Make sure your OpenAI authorization key is in your environment:

```sh
export OPENAI_API_KEY=sk-***********
```

## Installation

`ok` requires 
- `json-c` (https://github.com/json-c/json-c) which can be installed on mac with `brew install json-c` and
- `libcurl` (https://curl.haxx.se/libcurl/)

Then, you can install `ok` with:

```sh
git clone https://github/jon-tow/ok
mkdir build; cd build; cmake ..
make; chmod 777 ok
cp ok /usr/local/bin  # Or any location in your PATH
```
