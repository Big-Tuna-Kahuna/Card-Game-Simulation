#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <bits/stdc++.h>
#include <queue>
#define NUM_PLAYERS 3
using namespace std;

// Thread Global Variables
pthread_cond_t p1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t p2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t p3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t d1 = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t player_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dealer_mutex = PTHREAD_MUTEX_INITIALIZER;


//****Start Class Definition Deck****
//Contains all the functions that the threads will be using to manipulate
//the deck of cards simulated by a queue.
class Deck {
public:
    Deck();
    //variables for each of the two cards the players will have in hand
    int player1Card1 = 0;
    int player2Card1 = 0;
    int player3Card1 = 0;
    int player1Card2 = 0;
    int player2Card2 = 0;
    int player3Card2 = 0;
    int roundOver = 0;      //counter for each player thread that exits signaling round is over
    bool win = false;       //signals to dealer and players that winner is declared
    fstream logFile;        //output to log file
    void printCards();      //prints deck
    void printCardsLog();   //prints deck to logfile
    void shuffle(int);      //shuffles deck
    void returnCard(int);   //puts player card back in deck
    void setSeed(int);      //sets seed from argv for random function
    int drawCard();         //enables player and dealers to select card from deck
    int randomCard();       //returns which card player should return to deck from hand

private:
    int cardDeck[52];       //card array to shuffle cards before putting them in queue
    int seed = 0;           //seed for rand function
    void createDeck();      //Creates deck
    queue<int> cardQueue;   //Queue to hold cards
};

//Class Functions-----------------------------------------------------------------------
//Constructor that builds deck
Deck::Deck() {
    createDeck();
}

//Returns 1 or 2 randomly for player to decide which card from hand to discard
int Deck::randomCard() {
    int num = 0;
    num = 1 + (rand() % 2);
    return num;
}

//Returns a card to bottom of Deck
void Deck::returnCard(int card) {
    cardQueue.push(card);

}

//Gets a card from top of deck
int Deck::drawCard() {
    int card = 0;
    card = cardQueue.front();
    cardQueue.pop();
    return card;
}

//Creates a deck in an array
void Deck::createDeck() {
    int face[] = {1,2,3,4,5,6,7,8,9,10,11,12,13};
    int count = 0;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 13; j++) {
            cardDeck[count] = face[j];
            count++;
        }
    }
}

//Prints out deck from queue and uses a temp queue to store original queue after each pop
void Deck::printCards() {
    int card = 0;
    queue<int> temp;
    while (!cardQueue.empty()) {
        card = cardQueue.front();
        cout << card << " ";
        cardQueue.pop();
        temp.push(card);
    }
    cout << endl;
    while (!temp.empty()) {
        card = temp.front();
        temp.pop();
        cardQueue.push(card);
    }
}

//Does the same as the printCards function but outputs to logfile.txt
void Deck::printCardsLog() {
    int card = 0;
    queue<int> temp;
    while (!cardQueue.empty()) {
        card = cardQueue.front();
        logFile << card << " ";
        cardQueue.pop();
        temp.push(card);
    }
    logFile << endl;
    while (!temp.empty()) {
        card = temp.front();
        temp.pop();
        cardQueue.push(card);
    }
}

//Clears the card queue and shuffles new arrangement from card array and pushes back into queue
void Deck::shuffle( int j) {
    cardQueue = queue<int>();//clears current queue
    std::shuffle(cardDeck, cardDeck + 52, default_random_engine(seed + j));
    for (auto i : cardDeck) {cardQueue.push(i);}

}

//sets the seed for random functions
void Deck::setSeed(int i) {seed = i;}

//End Deck Class Functions---------------------------------------------------------------------

//function prototypes
void* dealer(void *roundNum);
void* player(void *pid);

//Global object declaration so threads can access deck1
Deck deck1;

//Main function---------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    //create pthread variables for players and dealer
    pthread_t d1;
    pthread_t players[NUM_PLAYERS];
    //open logfile for write
    deck1.logFile.open("logFile.txt", ios::out);
    int roundNum = 1;//round number currently on
    int userVar = 0;//variable for seed

    if (argc == 1) {userVar = 1;}//if no argument on command line passed then seed set to default of 1
    else {userVar = atoi( argv[1]);}//else assign seed from command line argument

    deck1.setSeed(userVar);
    srand(userVar);

    if (roundNum == 1) {
        pthread_create(&d1, NULL, &dealer, (void *)roundNum);//create dealer thread and pass round number
        for( long i = 0; i < 3; i++) {
            pthread_create(&players[i], NULL, &player, (void *)i);//create three player threads and pass player number
        }
        pthread_join(d1, NULL);//wait for dealer thread to continue
        roundNum++;
    }
    if (roundNum == 2) {
        pthread_create(&d1, NULL, &dealer, (void *)roundNum);
        for( long i = 0; i < 3; i++) {
            pthread_create(&players[i], NULL, &player, (void *)i);
        }
        pthread_join(d1, NULL);
        roundNum++;
    }
    if (roundNum == 3) {
        pthread_create(&d1, NULL, &dealer, (void *)roundNum);
        for( long i = 0; i < 3; i++) {
            pthread_create(&players[i], NULL, &player, (void *) i);
        }
        pthread_join(d1, NULL);
        roundNum++;
    }
    //Closing of logfile
    deck1.logFile.close();


    return 0;
}
//end Main-----------------------------------------------------------------------------------

//dealer function for thread. Changes the order of players depending on which round. Round 1 player 1 begins,
//round 2, player 2 begins, round 3, player 3 begins. Dealer shuffles deck and passes one card to each player.
//then signals to each player beginning with player 1 on round 1 to execute their code. Then dealer waits
//for player to signal back they are done with executing. Then dealer signals next player to play and waits.
//When roundOver increments to 3 the dealer stops signaling players as their threads no longer exist and dealer
//exits round.
void* dealer(void *roundNum) {
    long round = (long)roundNum;//passing round number arg

    if (round == 1) {
        cout << "Round 1!" << endl;
        deck1.logFile << "\nDEALER: Shuffling Deck..." << endl;
        deck1.logFile << "DEALER: Passing card to each player..." << endl;
        deck1.shuffle(round);
        deck1.player1Card1 = deck1.drawCard();//assigns first cards
        deck1.player2Card1 = deck1.drawCard();
        deck1.player3Card1 = deck1.drawCard();

        while (deck1.roundOver < 3) {
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p1);//signal player 1 to play
                pthread_cond_wait(&d1, &dealer_mutex);//wait for signal from player 1 that turn is over
                if (deck1.win) {deck1.roundOver++;}
            }
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p2);//signal player 2 to play
                pthread_cond_wait(&d1, &dealer_mutex);//wait for player 2
                if (deck1.win) {deck1.roundOver++;}
            }
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p3);//signal player 3 to play
                pthread_cond_wait(&d1, &dealer_mutex);//wait for player 3
                if (deck1.win) {deck1.roundOver++;}
            }
        }
        deck1.roundOver = 0;
        deck1.win = false;
    }
    else if(round == 2) {
        cout << "Round 2!" << endl;
        deck1.logFile << "\nDEALER: Shuffling Deck..." << endl;
        deck1.logFile << "Dealer passing card to each player..." << endl;
        deck1.shuffle(round);
        deck1.player1Card2 = 0;
        deck1.player2Card2 = 0;
        deck1.player3Card2 = 0;
        deck1.player2Card1 = deck1.drawCard();
        deck1.player3Card1 = deck1.drawCard();
        deck1.player1Card1 = deck1.drawCard();
        sleep(1);
        while (deck1.roundOver < 3) {
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p2);
                pthread_cond_wait(&d1, &dealer_mutex);
                if (deck1.win) {deck1.roundOver++;}
            }
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p3);
                pthread_cond_wait(&d1, &dealer_mutex);
                if (deck1.win) {deck1.roundOver++;}
            }
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p1);
                pthread_cond_wait(&d1, &dealer_mutex);
                if (deck1.win) {deck1.roundOver++;}
            }
        }
        deck1.roundOver = 0;
        deck1.win = false;
    }
    else if(round == 3) {
        cout << "Round 3!" << endl;
        deck1.logFile << "\nDEALER: Shuffling Deck..." << endl;
        deck1.logFile << "Dealer passing card to each player..." << endl;
        deck1.shuffle(round);
        deck1.player1Card2 = 0;
        deck1.player2Card2 = 0;
        deck1.player3Card2 = 0;
        deck1.player3Card1 = deck1.drawCard();
        deck1.player1Card1 = deck1.drawCard();
        deck1.player2Card1 = deck1.drawCard();
        sleep(1);
        while (deck1.roundOver < 3) {
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p3);
                pthread_cond_wait(&d1, &dealer_mutex);
                if (deck1.win) {deck1.roundOver++;}
            }
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p1);
                pthread_cond_wait(&d1, &dealer_mutex);
                if (deck1.win) {deck1.roundOver++;}
            }
            if (deck1.roundOver < 3) {
                pthread_cond_signal(&p2);
                pthread_cond_wait(&d1, &dealer_mutex);
                if (deck1.win) {deck1.roundOver++;}
            }
        }
        deck1.roundOver = 0;
        deck1.win = false;
    }
    else {
        cout << "ERROR: round variable out of bounds." << endl;
    }

}


//Function for player threads. Each section of code is only executed by the player whos PID matches that section.
//Before the player can access the Global Deck they must lock it so other threads cannot access it by mistake.
void* player(void *pid) {
    long playerNum = ((long) pid + 1);//Player PID which is unique to each thread
    int num = 0;//Variable to random card discard selection
    bool run = true;//run loop while true

    while (run) {
        if (playerNum == 1) {
            pthread_mutex_lock(&player_mutex);//lock deck access from other threads
            pthread_cond_wait(&p1, &player_mutex);//wait for signal from dealer to continue

            if (!deck1.win) {
                deck1.player1Card2 = deck1.drawCard();//draw card from top of deck
                deck1.logFile << "PLAYER 1: hand " << deck1.player1Card1 << endl;
                deck1.logFile << "Player 1: draws " << deck1.player1Card2 << endl;

                if (deck1.player1Card1 == deck1.player1Card2) {//if matching cards in hand then Win
                    deck1.win = true;
                    //output to console and logfile
                    deck1.logFile << "PLAYER 1: hand " << deck1.player1Card1 << " " << deck1.player1Card2 << endl;
                    deck1.logFile << "PLAYER 1: wins" << endl;
                    cout << "PLAYER 1:" << endl;
                    cout << "HAND " << deck1.player1Card1 << " " << deck1.player1Card2 << endl;
                    cout << "WIN yes" << endl;
                    cout << "PLAYER 2:" << endl;
                    cout << "HAND " << deck1.player2Card1 << endl;
                    cout << "WIN no" << endl;
                    cout << "PLAYER 3:" << endl;
                    cout << "HAND " << deck1.player3Card1 << endl;
                    cout << "WIN no" << endl;
                    cout << "DECK: ";
                    deck1.printCards();

                } else {//if not winning hand then discard random card
                    num = deck1.randomCard();
                    if (num == 1) {
                        deck1.returnCard(deck1.player1Card1);//Returns Card 1 to deck
                        deck1.logFile << "PLAYER 1: discards " << deck1.player1Card1 << endl;
                        deck1.player1Card1 = deck1.player1Card2;//Card 1 becomes Card 2
                        deck1.player1Card2 = 0;//Card 2 set to zero
                    } else if (num == 2) {
                        deck1.returnCard(deck1.player1Card2);//Returns Card 2 to deck
                        deck1.logFile << "PLAYER 1: discards " << deck1.player1Card2 << endl;
                        deck1.player1Card2 = 0;//Card 2 set to zero
                    }
                    deck1.logFile << "PLAYER 1: hand " << deck1.player1Card1 << endl;
                    deck1.logFile << "DECK: ";
                    deck1.printCardsLog();
                }
            }
            pthread_mutex_unlock(&player_mutex);//unlock deck
            pthread_cond_signal(&d1);//signal dealer that turn is over
            if (deck1.win) {//If player won then exit round by terminating thread
                deck1.logFile << "PLAYER 1: exits round" << endl;
                pthread_exit(NULL);
            }
        }
        if (playerNum == 2) {
            pthread_mutex_lock(&player_mutex);
            pthread_cond_wait(&p2, &player_mutex);

            if (!deck1.win) {
                deck1.player2Card2 = deck1.drawCard();
                deck1.logFile << "PLAYER 2: hand " << deck1.player2Card1 << endl;
                deck1.logFile << "Player 2: draws " << deck1.player2Card2 << endl;

                if (deck1.player2Card1 == deck1.player2Card2) {
                    deck1.win = true;
                    deck1.logFile << "PLAYER 2: hand " << deck1.player2Card1 << " " << deck1.player2Card2 << endl;
                    deck1.logFile << "PLAYER 2: wins" << endl;
                    cout << "PLAYER 2:" << endl;
                    cout << "HAND " << deck1.player2Card1 << " " << deck1.player2Card2 << endl;
                    cout << "WIN yes" << endl;
                    cout << "PLAYER 3:" << endl;
                    cout << "HAND " << deck1.player3Card1 << endl;
                    cout << "WIN no" << endl;
                    cout << "PLAYER 1:" << endl;
                    cout << "HAND " << deck1.player1Card1 << endl;
                    cout << "WIN no" << endl;
                    cout << "DECK: ";
                    deck1.printCards();
                } else {
                    num = deck1.randomCard();
                    if (num == 1) {
                        deck1.returnCard(deck1.player2Card1);//Returns Card 1 to deck
                        deck1.logFile << "PLAYER 2: discards " << deck1.player2Card1 << endl;
                        deck1.player2Card1 = deck1.player2Card2;//Card 1 becomes Card 2
                        deck1.player2Card2 = 0;//Card 2 set to zero
                    } else if (num == 2) {
                        deck1.returnCard(deck1.player2Card2);//Returns Card 2 to deck
                        deck1.logFile << "PLAYER 2: discards " << deck1.player2Card2 << endl;
                        deck1.player2Card2 = 0;//Card 2 set to zero
                    }
                    deck1.logFile << "PLAYER 2: hand " << deck1.player2Card1 << endl;
                    deck1.logFile << "DECK: ";
                    deck1.printCardsLog();
                }
            }
            pthread_mutex_unlock(&player_mutex);
            pthread_cond_signal(&d1);
            if (deck1.win) {
                deck1.logFile << "PLAYER 2: exits round" << endl;
                pthread_exit(NULL);
            }
        }
        if (playerNum == 3) {
            pthread_mutex_lock(&player_mutex);
            pthread_cond_wait(&p3, &player_mutex);

            if (!deck1.win) {
                deck1.player3Card2 = deck1.drawCard();
                deck1.logFile << "PLAYER 3: hand " << deck1.player3Card1 << endl;
                deck1.logFile << "Player 3: draws " << deck1.player3Card2 << endl;

                if (deck1.player3Card1 == deck1.player3Card2) {
                    deck1.win = true;
                    deck1.logFile << "PLAYER 3: hand " << deck1.player3Card1 << " " << deck1.player3Card2 << endl;
                    deck1.logFile << "PLAYER 3: wins" << endl;
                    cout << "PLAYER 3:" << endl;
                    cout << "HAND " << deck1.player3Card1 << " " << deck1.player3Card2 << endl;
                    cout << "WIN yes" << endl;
                    cout << "PLAYER 1:" << endl;
                    cout << "HAND " << deck1.player1Card1 << endl;
                    cout << "WIN no" << endl;
                    cout << "PLAYER 2:" << endl;
                    cout << "HAND " << deck1.player2Card1 << endl;
                    cout << "WIN no" << endl;
                    cout << "DECK: ";
                    deck1.printCards();
                } else {
                    num = deck1.randomCard();
                    if (num == 1) {
                        deck1.returnCard(deck1.player3Card1);//Returns Card 1 to deck
                        deck1.logFile << "PLAYER 3: discards " << deck1.player3Card1 << endl;
                        deck1.player3Card1 = deck1.player3Card2;//Card 2 becomes Card 1
                        deck1.player3Card2 = 0;//Card 2 set to zero
                    } else if (num == 2) {
                        deck1.returnCard(deck1.player3Card2);//Returns Card 2 to deck
                        deck1.logFile << "PLAYER 3: discards " << deck1.player3Card2 << endl;
                        deck1.player3Card2 = 0;//Card 2 set to zero
                    }
                    deck1.logFile << "PLAYER 3: hand " << deck1.player3Card1 << endl;
                    deck1.logFile << "DECK: ";
                    deck1.printCardsLog();
                }

            }

            pthread_mutex_unlock(&player_mutex);
            pthread_cond_signal(&d1);
            if (deck1.win) {
                deck1.logFile << "PLAYER 3: exits round" << endl;
                pthread_exit(NULL);
            }
        }
    }
}
