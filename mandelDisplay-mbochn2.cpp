//
// Michal Bochnak
// Netid: mbochn2
// Homework #4
// Nov 16, 2017
//
// mandelDisplay-mbochn2.cpp
//

//
// include header with functions
//
#include "mandelHeader.h"

//
// Function prototypes
//
void sigusr1_handler(int);

//
// global to count image processed
//
int imgsProcessed = 0;


int main (int argc, char** argv) {

    double  xMin, xMax, yMin, yMax;
    int     nRows, nCols, maxIters;
    int*    data;           // shared memory pointer
    struct msg_struct msg;  // for using message queues

    // message type is 1
    msg.mtype = 1;

    // set signal
    signal(SIGUSR1, sigusr1_handler);

    // get shared memory segment
    if ((data = (int*)shmat(atoi(argv[1]), NULL, 0)) < 0) {
        perror("shmat error");
        exit(-1);
    }

    while (true) {

        //
        // wait for the input
        //
        fscanf(stdin, "%lf %lf %lf %lf %d %d %d",
               &xMin, &xMax, &yMin, &yMax, &nRows, &nCols, &maxIters);
        fflush(stdin);

        // wait for filename from parent
        while ((msgrcv(atoi(argv[3]), &msg,
                       (sizeof(msg_struct) - sizeof(msg_struct.mtype)),
                       1, IPC_NOWAIT)) < 0);

        // open file to write data to
        FILE* fp;
        if((fp = fopen(msg.mtext, "w")) < 0 ) {
            cout << "Opening the file failed. Data will not be saved to file.\n";
        }

        //
        // Display image on screen
        // Write data to file,
        // cout statements used for formatting
        //
        char colors[16] = ".-~:+*%O8&?$@#X";
        cout << endl << endl;
        int n;
        cout << endl << endl;
        // traverse through rows
        for (int r = nRows - 1; r >= 0; --r) {
            if (r == nRows - 1) {
                cout.width(9); cout << left << yMax;
            }
            else if (r == 0) {
                cout.width(9); cout << left << yMin;
            }
            else {
                cout << "         ";
            }
            // traverse through columns
            for (int c = 0; c < nCols; ++c) {
                n = *(data + r * nCols + c);
                if (n < 0) {
                    cout << " ";
                }
                else {
                    cout << colors[n % 15];
                }
                // save
                if (fp != NULL ) {
                    fprintf(fp, "%d ", n);
                }
            }
            cout << endl;
            if(fp != NULL) {
                fprintf(fp, "\n");
            }
        }

        // output format
        cout << endl;
        cout.width(11); cout << right << xMin;
        cout.width(nCols - 2); cout << right << xMax;
        cout << endl << endl << endl;

        // increment images processed
        imgsProcessed++;

        // close the file
        fclose(fp);

        // inform parent that job is done
        strcpy(msg.mtext, "mandelDisplay is done.");
        if ((msgsnd(atoi(argv[2]), &msg,
                    (sizeof(msg_struct) - sizeof(msg_struct.mtype)),
                    IPC_NOWAIT)) < 0) {
            perror("msgsnd error");
            exit(-2);
        }
    }


    // never reached
    return 2;
}


//
// SIGUSR1 handler
//
void sigusr1_handler(int signal) {
    exit(imgsProcessed);
}



















