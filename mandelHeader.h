//
// Michal Bochnak
// Netid: mbochn2
// Homework #4
// Nov 16, 2017
//
// mandelHeader.h
//


//
// Needed libraries
//
#include <stdio.h>          // perror
#include <iostream>         // std::cout, std::fixed
#include <iomanip>
#include <unistd.h>         // pipe
#include <sys/wait.h>
#include <cstdlib>          // EXIT_FAILURE etc
#include <sys/resource.h>   // struct rusage
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/msg.h>        // message queue
#include <sys/sem.h>        // semafor
#include <sys/ipc.h>        // shared memory
#include <sys/shm.h>        // shared memory
#include <string>
#include <string.h>
#include <stdlib.h>

using namespace std;

//
// struct used for sending messages through message queues
//
struct msg_struct {
    long   mtype;        /* Message type. */
    char   mtext[128];   /* Message text. */
}msg_struct;


//
// Displays author information
//
void showAuthorInfo() {
    cout << endl;
    cout << "   |----------------|" << endl;
    cout << "   | Michal Bochnak |" << endl;
    cout << "   | Netid: mbochn2 |" << endl;
    cout << "   | Homework #4    |" << endl;
    cout << "   | Nov 16, 2017   |" << endl;
    cout << "   |----------------|" << endl << endl;
}

//
// Converts given integer to character array
//
char* convIntToCharArr(int val) {
    char* temp = (char*)malloc(sizeof(char) * 25);
    sprintf(temp, "%d", val);
    return temp;
}

//
// Converts string to character array
//
char* convStringToCharArray(string s) {
    char* temp = (char*)malloc(sizeof(char) * s.length() + 1);
    int i = 0;
    for (i = 0; i < s.length(); ++i)
        temp[i] = s[i];

    temp[i] = '\0';
    return temp;
}

//
// Get filename from user and return it
//
string getFilename() {
    string fn;
    cout << "Enter filename that program output will be written to: ";
    cin >> fn;
    return fn;
}


//
// Asks user if he want to continue
// Return true if so, false otherwise
//
bool done() {
    string input;
    cout << "Would you like to process another file? (y/n) ";
    cin >> input;
    while (input != "y" && input != "Y"
           && input != "n" && input != "N") {
        cout << "Incorrect. Try again. Would you "
                "like to process another file? (y/n) ";
        cin >> input;
    }
    if (input == "y" || input == "Y")
        return false;
    else
        return true;
}

//
// Get the problem specs from user,
// and stores it in parameters by reference
//
void getProblemInfo(double &xMin, double &xMax, double &yMin,
            double &yMax,int &nRows, int &nCols, int &maxIters) {
    cout << "Enter lower bound for X-axis:   ";
    cin >> xMin;
    cout << "Enter upper bound for X-axis:   ";
    cin >> xMax;
    cout << "Enter lower bound for Y-axis:   ";
    cin >> yMin;
    cout << "Enter upper bound for Y-axis:   ";
    cin >> yMax;
    cout << "Enter number of rows:           ";
    cin >> nRows;
    cout << "Enter number of columns:        ";
    cin >> nCols;
    cout << "Enter number of max iterations: ";
    cin >> maxIters;
}

//
// Display error type
//
void showErrorMessage(int id) {
    if (id == -1) {
        cout << "Shared memory error occured." << endl;
    }
    if (id == -2) {
        cout << "Message queue error occured." << endl;
    }
}

//
// Display the info about program termination reason
//
void showErrorInfo(int st_ch_1, int st_ch_2) {
    // child 1
    if (st_ch_1 < 0) {
        cout << "mandelCalc terminated. ";
        showErrorMessage(st_ch_1);
    }
    // child 2
    if (st_ch_2 < 0) {
        cout << "mandelDisplay terminated. ";
        showErrorMessage(st_ch_2);
    }
}
















