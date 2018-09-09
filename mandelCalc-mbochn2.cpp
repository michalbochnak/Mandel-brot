//
// Michal Bochnak
// Netid: mbochn2
// Homework #4
// Nov 16, 2017
//
// mandelCalc-mbochn2.cpp
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
    double  deltaX, deltaY;
    double  Cy, Cx, Zx, Zy, Zx_next, Zy_next;
    int     nRows, nCols, maxIters;
    int*    data;           // shared memory pointer
    struct  msg_struct msg; // for sending through message queues

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

        //
        // Mandlebrot calculations
        //
        deltaX = (xMax - xMin) / (nCols - 1);
        deltaY = (yMax - yMin) / (nRows - 1);

        int index;
        // traverse rows
        for (int r = 0; r < nRows; ++r) {
            Cy = yMin + (r * deltaY);
            // traverse columns
            for (int c = 0; c < nCols; ++c) {
                Cx = xMin + (c * deltaX);
                Zx = Zy = 0.0;
                int n = 0;
                for (n = 0; n < maxIters; ++n) {
                    if (Zx * Zx + Zy * Zy >= 4.0) {
                        break;
                    }
                    Zx_next = Zx * Zx - Zy * Zy + Cx;
                    Zy_next = 2.0 * Zx * Zy + Cy;
                    Zx = Zx_next;
                    Zy = Zy_next;
                }
                // calculate index, save to shared memory
                index = r * nCols + c;
                if (n >= maxIters) {
                    data[index] = -1;
                }
                else {
                    data[index] = n;
                }
            }
        }

        // increment images processed
        imgsProcessed++;

        //
        // Forward data to mandelDisplay
        //
        fprintf(stdout, "%lf %lf %lf %lf %d %d %d\n",
                xMin, xMax, yMin, yMax, nRows, nCols, maxIters);
        fflush(stdout);

        //
        // inform parent that job is done
        //
        strcpy(msg.mtext, "mandelCalc is done.");
        if ((msgsnd(atoi(argv[2]), &msg,
                    (sizeof(msg_struct) - sizeof(msg_struct.mtype)),
                    IPC_NOWAIT)) < 0) {
            perror("msgsnd error");
            exit(-2);
        }
    }


    // never reached
    return 1;
}


//
// SIGUSR1 handler
//
void sigusr1_handler(int signal) {
    exit(imgsProcessed);
}













