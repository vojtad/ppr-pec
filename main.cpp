#include <iostream>

#include "Game.h"

using namespace std;

int main()
{
    srand(time(0));

    // initialize game
    // first parameter of Game constructor is column count and second one is row count
    Game game(5, 5);

    // print initial game plan
    game.printGamePlan();

    // randomize game plan and calculate lower and upper bounds
    game.randomize();

    cout << endl;

    // print randomized game plan
    game.printGamePlan();

    // print calculated lower and upper bounds for bb-dfs
    cout << "q = " << game.upperBound() << endl << "d = " << game.lowerBound() << endl;

    // perform bb-dfs and find the best solution
    game.walkthru();

    // print best solution
    game.printBest();

    return 0;
}