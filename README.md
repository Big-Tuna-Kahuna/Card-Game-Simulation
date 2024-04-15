# Card-Game-Simulation
This project simulates a simple card game using pthreads where multiple players and a dealer interact through a shared deck. It is implemented in C++ and utilizes synchronization mechanisms like mutexes and condition variables to manage access to shared resources between threads. 

Overview
In this simulation:

Dealer: Shuffles the deck and distributes cards to the players in each round. The dealer controls the flow of the game by signaling players when it is their turn.
Players: Each player tries to match the cards they hold. They draw a card, check for a match, and if they don't find one, discard a card.
Deck: A shared resource managed by a Deck class, which includes operations to shuffle, draw, and return cards.

Features
Multithreading: Utilizes pthreads to simulate the concurrent actions of players and a dealer.
Synchronization: Uses mutexes and condition variables to manage resource access and synchronize thread operations.
Logging: Records each action in a logfile for further inspection and debugging.

Requirements
C++ compiler with C++11 support (e.g., g++, clang++)
pthread library

Compilation
Compile the program with the following command:
g++ -std=c++11 -pthread -o card_game card_game.cpp
Execution
Run the program with an optional seed argument for card shuffling:
./card_game [seed]
If no seed is provided, the program defaults to a seed of 1.

Design
The program initializes threads for the dealer and each player. Each thread represents a participant in the game.
The Deck class manages the deck of cards. Players and the dealer access the deck to draw and return cards, ensuring thread safety using mutex locks.
Player actions and outcomes are logged to logFile.txt, which captures the sequence of game events and the state of the deck after each action.
