

#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <vector>

using namespace std;

void handle_SIGCHLD(int signum) {
    //Child process paused or exited
    pid_t pid;
    pid = wait(NULL);
    cout << "pid: " << pid << " exited" << endl;
}

int launch(char** args) {
    pid_t pid, wpid;
    int status;
    string s(args[0]);
    bool bk_process = (s.back() == '&');

    if (bk_process) {
        s.erase(s.end() - 1);
        cout << s << endl;
        const char* prog = s.c_str();
        args[0] = strdup(prog);
    }
    
    pid = fork();
    if (pid == 0) {
        //Kind Prozess
        if (execvp(args[0], args) == -1) {
            cerr << "Error on exec." << endl;
            //Beendet den Process falls etwas schief geht
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        //Fehler beim fork-Systemaufruf
        cerr << "Error on fork." << endl;
    } else {
        if (bk_process) {
            cout << "Started " << args[0] << " in background with PID: " << pid << endl;
        } else {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    return 1;
}

int execute(char** args) {
    string temp(args[0]);
    if (args[0] == NULL) {
        cout << "Invalid command." << endl;
        return 1;
    }
    else if (temp == "logout" || temp == "Logout") return 99;
    return launch(args);
}

char** split_line(char* line) {
    vector<char*> vec;
    char* token;

    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        vec.push_back(token);
        token = strtok(NULL, " \t\r\n\a");
    }
    char** tokens = new char*[vec.size()];
    for (int i = 0; i < vec.size(); i++)
        tokens[i] = vec[i];
    return tokens;
}

char* read_line() {
    char* temp = new char[256];
    cin.getline(temp, 256);
    return temp;
}

void loop() {
    string sinput;
    int status = 1;
    char* input;
    char** params;
    do {
        cout << ">" << flush;
        input = read_line();
        params = split_line(input);
        status = execute(params);
        //for (int i = 0; i < 5; i++)
        //   cout << "Input: " << params[i] << endl;
        delete input;
        delete[] params;
        if (status == 0)
            cout << "Error launching!" << endl;
        else if (status == 99) {
            cout << "Wollen Sie die Shell wirklich beenden (J/N)?" << endl << ">" << flush;
            cin >> sinput;
            cout << endl;
            if (sinput == "J" || sinput == "j") return;
            else if (sinput == "N" || sinput == "n") status = 1;
            else {
                cout << "Falsche Eingabe!" << endl;
                status = 1;
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