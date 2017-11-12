

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

void handle_SIGCHLD(int signum) {
    //Child process paused or exited
    pid_t pid;
    pid = wait(NULL);
    cout << "pid: " << pid << " exited" << std::endl;
}

int launch(vector<string> args) {
    //Converting vector to char* to call execvp with
    char ** cstr = new char* [args.size()];
    for (int i = 0; i < args.size(); i++) {
        cstr[i] = (char*) args[i].c_str();
    }
    int pid, wpid, status;

    pid = fork();

    if (pid == 0) {
        //Child Process
        if (execvp(cstr[0], cstr) == -1)
            //Error on exec
            return 0;
    } else if (pid < 0) {
        //error forking
        return 0;
    } else {
        //Parent Process
        if (args[1] == "&" || args.at(0).back() == '&') {
            //Run in Backgound -> Dont wait for process to finish
            cout << "started pid: " << pid << endl;
        } else {
            //Wait for Child Process to finish
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }

    delete[] cstr;
    //Child Process done and the Shell can continue
    return 1;
}

int execute(vector<string> args) {
    if (args[0] == "logout")
        return 99;
    else
        return launch(args);
}

vector<string> split_line(const string& line) {
    vector<string> args;
    stringstream ss;
    ss.str(line);
    string temp;
    while (!ss.eof()) {
        ss >> temp;
        args.push_back(temp);
    }
    return args;
}

string read_line() {
    string line;
    getline(cin, line);
    return line;
}

void loop() {
    int status = 1;
    string input;
    vector<string> params;
    do {
        cout << ">" << flush;
        input = read_line();
        //cout << "Input: " << input << endl;
        if (!input.empty()) {
            params = split_line(input);
            status = execute(params);
            if (status == 0)
                cout << "Error launching!" << endl;
            else if (status == 99) {
                cout << "Wollen Sie die Shell wirklich beenden (J/N)?" << endl << ">";
                cin >> input;
                cout << endl;
                if (input == "J" || input == "j") return;
                else if (input == "N" || input == "n") status = 1;
                else {
                    cout << "Falsche Eingabe!" << endl;
                    status = 1;
                }
            }
        }
    } while (status);
}

/*
 * 
 */
int main(int argc, char** argv) {
    signal(SIGCHLD, handle_SIGCHLD);
    loop();
    return 0;
}

