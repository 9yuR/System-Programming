#include <iostream>
#include <string>
using namespace std;

int main(void) {
    string ANSWER = "213";
    string GUESS = "154";
    
    int STRLEN = 3;
    
    int BALL = 0;
    int STK = 0;
    int OUT = 3;
    
    for (int i = 0; i < STRLEN; i++) {
        for (int j = 0; j < STRLEN; j++) {
            if (i == j && ANSWER[i] == GUESS[j]) {
                STK += 1;
                OUT -= 1;
            }
            else if (i != j && ANSWER[i] == GUESS[j]) {
                BALL += 1;
                OUT -= 1;
            }
        }
    }
    return 0;
}
