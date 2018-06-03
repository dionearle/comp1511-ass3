// fruit_bot.c
// Assignment 3, COMP1511 18s1: Fruit Bot
//
// This program by Dion Earle (z5205292) on INSERT-DATE-HERE
//
// Version 1.0.0: Assignment released.
// Version 1.0.1: minor bug in main fixed

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "fruit_bot.h"

#define SHOP_SELLS -1
#define SHOP_BUYS 1
#define SHOP_SELLS_ELEC 2
#define SHOP_SELLS_FRUIT_SAME 3
#define SHOP_SELLS_FRUIT_EMPTY 4
#define LOW_ENERGY 5

void print_player_name(void);
void print_move(struct bot *b);
void run_unit_tests(void);

// ADD PROTOTYPES FOR YOUR FUNCTIONS HERE
int location_check(struct bot *b);
int choose_location_type(struct bot *b);
int calculate_world_size(struct bot *b);
int best_location(struct bot *b, char destination_type[MAX_NAME_CHARS], int value);
//int find_fruit(struct bot *b);
int best_fruit(struct bot *b);
int check_moves(struct bot *b);


int main(int argc, char *argv[]) {

    if (argc > 1) {
        // supply any command-line argument to run unit tests
        run_unit_tests();
        return 0;
    }

    struct bot *me = fruit_bot_input(stdin);
    if (me == NULL) {
        print_player_name();
    } else {
        print_move(me);
    }

    return 0;
}

void print_player_name(void) {
    printf("MyBeautifulDarkTwistedFruitBot");

}

// print_move - should print a single line indicating
//              the move your bot wishes to make
//
// This line should contain only the word Move, Sell or Buy
// followed by a single integer

void print_move(struct bot *b) {

    // Keeping track of whether the bot has already made a play this round
    int played = 0;

    // Determining the size of the current world
    int world_size = calculate_world_size(b);
    int buy_min;

    // Depending on the size of the world, choose when to stop buying food
    if (world_size < 15) {
        buy_min = 2;
    } else {
        buy_min = 5;
    }

    // Determining what the bot can do at its current location
    int current_stop = location_check(b);

    // Buying or selling depending on our current location
    if (b->battery_level == 0) {
        printf("Move 0");
        played = 1;
    } else if (current_stop == SHOP_BUYS) {
        printf("Sell %d\n", b->fruit_kg);
        played = 1;
    } else if (current_stop == SHOP_SELLS_ELEC) {
        printf("Buy %d\n", b->battery_capacity - b->battery_level);
        played = 1;
    } else if (current_stop == SHOP_SELLS_FRUIT_SAME) {
        if (b->turns_left > buy_min) {
            printf("Buy %d\n", b->maximum_fruit_kg - b->fruit_kg);
            played = 1;
        }
    } else if (current_stop == SHOP_SELLS_FRUIT_EMPTY) {
        if (b->turns_left > buy_min) {
            // Before buying, ensure we will be able to sell the fruit somewhere
            if (best_location(b, b->location->fruit, SHOP_BUYS) != 0) {
                printf("Buy %d\n", b->maximum_fruit_kg);
                played = 1;
            }
        // If the game is nearly over, don't buy any more fruit
        } else {
            printf("Move 0");
            played = 1;
        }
    } else if (current_stop == LOW_ENERGY) {
        int find_elec = best_location(b, "Electricity", SHOP_SELLS);
        printf("Move %d", find_elec);
        played = 1;
    }

    // If we didn't buy or sell, we want to move somewhere
    if (played != 1) {
        int move_selection = choose_location_type(b);
        if (move_selection == 0) {
            move_selection = check_moves(b);
        }
        printf("Move %d", move_selection);
    }
}


// ADD A COMMENT HERE EXPLAINING YOUR OVERALL TESTING STRATEGY

// EXPLANATION IS IN run_unit_tests!!!
// Read testing strategy below for explanation of how I tested my bot. Since my created functions had structs as their arguments,
// I found it very difficult to write unit tests for them. Instead, I used different methods of testing which are described below.
// (I put them in the run_unit_tests function rather than here as my explanation took up alot of space)
// EXPLANATION IS IN run_unit_tests!!!

void run_unit_tests(void) {
    // PUT YOUR UNIT TESTS HERE
    // This is a difficult assignment to write unit tests for,
    // but make sure you describe your testing strategy above.

    /*
    The basic premise of my testing strategy was that I created numerous example
    world files (with names such as test_world1.txt) that allowed me to ensure that
    when my bot is at a particular location, it would perform the desired action.
    Before I could do this I first created the world file using the template used
    in the week 11 lab exercises.

    For example, the file world0.txt has the following written inside:

        *** Fruit Bot Parameters ***
        battery_capacity=74
        maximum_fruit_kg=21
        maximum_move=7

        *** Turn 1 of 19 *** ***

        CSE: other
        Mathews A: will buy 1 kg of Apples for $51/kg
        Quadrangle: other
        Campus Charging: will sell 100 kJ of Electricity for $4/kJ
        Kensington Apple Farm: will sell 3 kg of Apples for $18/kg
        Campus Compost Heap: will buy 1000 kg of Anything for $1/kg
        CLB 7: will buy 3 kg of Apples for $43/kg

        "Botty McBotbot" is at "CSE" with $117, battery level: 74
        "Buffalo McBuff" is at "CSE" with $117, battery level: 74
        "COMP1511 Student" is at "CSE" with $117, battery level: 74
        *** You are "Dion Earle Fruit Bot"

    From this format of file, I could basically change or add any parameter about
    both the world and my bot. This included the world size, fruit types/prices,
    my bot's position, maximum moves and battery levels. After I had created a
    suitable world and had changed my bots current info to be able to test what I wanted to test,
    I then ran my program yet passed in the text file as standard input,
    looking like ./fruit_bot < test_world1.txt.

    Doing so would then return the action the bot made for that turn, allowing me
    to compare this to what I expected to happen. If it made the correct decision,
    this meant the function that was in charge of making this decision was working as intended.

    In terms of what I actually tested using this method, I first placed my bot at
    different locations and ensured that if they could perform an action there,
    (either buy or sell) that they did (and it was the correct action).
    Doing so would test the location_check and world_size functions.

        Selling: I first placed my bot on a location that buys fruit,
    alongside giving my bot the same type of fruit. The bot should then print to attempt
    to sell all of its supply.

        Buying new fruit: I placed the bot on a location that sells fruit, and made the
    bot carry nothing. As long as the location has quantity, and the bot has enough money
    to buy some, the bot should print to buy the max amount of that fruit.

        Buying the same fruit: Placing the bot on a location that sells fruit, if I
    am already carrying fruit and it is the same type, the bot should print to buy
    as much as it can.

        Buying electricity: If the bot is on a location that sells electricity, and there
    are more than a specified amount of turns left, alongside my battery level being
    below a specified level, the bot should refill its energy.

    After this, I then tested that if I couldn't perform an action at the current
    location, that the bot would move to the most reasonable location to go to.
    This would be testing the choose_location_type, best_location & best_fruit functions.

        Finding electricity to buy: If my bot is running low on electricity, and there
    are more than a specified amount of turns left, the bot should find somewhere to
    refill its energy.

        Finding the same fruit to buy: If my bot is already carrying a fruit, yet it has
    not reached maximum capacity, it should find somewhere to buy even more of this
    fruit (as long as there are more than a specified amount of turns left).

        Finding new fruit to buy: If my bot was not carrying any fruit, then it should move
    to the location which sells the best value fruit (as long as it is within a certain
    range of its current location).

        Finding location to sell fruit: If the bot is carrying fruit and either there are not
    many turns left or its capacity is full, it should just move to somewhere to sell this
    fruit. This location should buy the fruit for the highest price out of all neraby buyers.

    Finally, I ran several tests that ensured my bot wouldn't make choices that
    could cause it to get stuck carrying fruit when either the game ends or
    it is about to run out of electricity.
    These tests would be ensuring the check_moves function works properly.

        Limited amount of turns left in game: If there are not many turns left in the game,
    the bot should not start searching for more places to buy fruit, instead its chosen action
    should be to either move to sell the fruit it is carrying or simply stay put.

        Limited battery remaining (and no more to buy): If the bot is close to running out of battery
    and there is no more supply to buy a refill, the bot should avoid buying any fruit,
    instead moving to sell any fruit being carried and then stopping for rest of game.

    For all of the tests conducted, I would first create a situation in which the bot
    SHOULD perform the action being tested, alongside creating a situation where it SHOULD
    NOT perform the desired action. To further solidify functions worked, I tested on different
    location types as well as different fruit types and prices to be sure it was not just luck.

    All of these tests were not only done in the smaller sized world described above,
    but also in medium and larger sized worlds. Testing in these different environments
    allowed me to discover many fatal bugs that were not present in the smaller worlds.
    This was actually where I spent most of my time fixing errors, with the testing
    of various world sizes proving essential.

    Additionally, once I was pleased with my bots performance in a world alone,
    I repeated many of these tests with multiple other bots in the world, both at the same
    and different locations. This tested that there were no major issues when other people
    were competing against my bot.

    */
}

// ADD YOUR FUNCTIONS HERE

// An initial check to see if our bot can buy or sell anything at its current location
int location_check(struct bot *b) {

    int world_size = calculate_world_size(b);
    int elec_min;
    int buy_min;

    // Depending on the size of the world, choose when to stop buying electricity
    if (world_size < 15) {
        elec_min = 5;
    } else if (world_size >= 15 && world_size < 50) {
        elec_min = 10;
    } else {
        elec_min = 15;
    }

    struct location *cur = b->location;
    int chosen = 0;

    // If the place buys fruit, it has quantity remaining and I have fruit on board
    if (cur->price > 0 && cur->quantity > 0 && b->fruit != NULL) {
        // If I am carrying the right type of fruit (or it is a compost heap), sell as much as I can
        if (strcmp(cur->fruit, b->fruit) == 0) {
            chosen = SHOP_BUYS;
        } else if (strcmp(cur->fruit, "Anything") == 0) {
            // If there is no where to sell the fruit I am carrying, sell at compost heap
            if (best_location(b, b->fruit, SHOP_BUYS) == 0) {
                chosen = SHOP_BUYS;
            }
        }
    // Current location sells electricity
    } else if ((cur->price < 0) && (strcmp(cur->fruit, "Electricity") == 0)) {
        if (b->battery_level <= (3 * elec_min) && cur->quantity > 0
        && b->cash > (-1)*(cur->price) && b->turns_left > elec_min) {
            chosen = SHOP_SELLS_ELEC;
        }
    // Current location sells fruit
    } else if ((cur->price < 0) && (strcmp(cur->fruit, "Electricity") != 0)
    && (b->battery_level > 3 * elec_min)) {
        // If I am carrying fruit, check it is the same type
        if (b->fruit != NULL) {
            // Bot has same fruit
            if (strcmp(cur->fruit, b->fruit) == 0 && cur->quantity > 0
            && b->cash > (-1)*(cur->price) && b->fruit_kg != b->maximum_fruit_kg) {
                chosen = SHOP_SELLS_FRUIT_SAME;
            }
        // Bot has an empty cart
        } else if (b->fruit == NULL) {
            if (cur->quantity > 0 && b->cash > (-1)*(cur->price)) {
                chosen = SHOP_SELLS_FRUIT_EMPTY;
            }
        }
    // Bot is low on battery
    } else if ((b->battery_level <= 3 * elec_min) && b->turns_left > elec_min) {
        // If there is somewhere that still has electricity to sell, go there
        if (best_location(b, "Electricity", SHOP_SELLS) != 0) {
                chosen = LOW_ENERGY;
        }
    }

    return chosen;
}

// Simply calculates how many locations there are in the world
int calculate_world_size(struct bot *b) {

    int size = 0;
    struct location *cur = b->location->east;

    // Searching east through all locations until we return to the start
    while (strcmp(cur->name, b->location->name) != 0) {
        cur = cur->east;
        size++;
    }

    return size;
}

// Determining what type of place we want our bot to move to
int choose_location_type(struct bot *b) {

    char destination_type[MAX_NAME_CHARS];
    int move;
    int shop_type;
    int world_size = calculate_world_size(b);
    int has_electricity = 0;
    int min;

    // Depending on the size of the world, choose when to stop buying electricity and food
    if (world_size < 15) {
        min = 5;
    } else if (world_size >= 15 && world_size < 50) {
        min = 10;
    } else {
        min = 15;
    }

    // First checking whether there is anywhere available to buy electricity
    if (best_location(b, "Electricity", SHOP_SELLS) != 0) {
        has_electricity = 1;
    }

    // Determining which type of location the bot should go to
    if ((b->battery_level <= 3 * min) && b->turns_left > min && has_electricity == 1) {
            // If the bot is low on energy, find a charging station
            strcpy(destination_type, "Electricity");
            shop_type = SHOP_SELLS;
    // If the bot is carrying fruit
    } else if (b->fruit_kg != 0) {
        strcpy(destination_type, b->fruit);
        // If the bot is not full, search for another shop to buy from
        if (b->fruit_kg < (1/2 * b->maximum_fruit_kg) && b->turns_left > min) {
            shop_type = SHOP_SELLS;
        // If we already close to full on fruit, just find a place that will buy it
        } else {
            shop_type = SHOP_BUYS;
        }
    // If the bot is not carrying fruit, find a shop that sells fruit
    } else {
        strcpy(destination_type, "AnyFruit");
        shop_type = SHOP_SELLS;
    }

    move = best_location(b, destination_type, shop_type);
    return move;
}

// Finding the best price for the chosen location to move to
int best_location(struct bot *b, char destination_type[MAX_NAME_CHARS], int value) {

    int distance = 0;
    int east_len = 1;
    int west_len = -1;

    // If we are looking to buy any type of fruit, use the best_fruit function to find the fruit that pays the most
    if (strcmp(destination_type, "AnyFruit") == 0) {

        int loc = best_fruit(b);
        if (loc == 0) {
            return distance;
        }
        struct location * tmp = b->location->east;
        int i = 1;
        while (i != loc) {
            i++;
            tmp = tmp->east;
        }

        strcpy(destination_type, tmp->fruit);
    }

    // When we do find a location of the required type, record its price and distance
    int prices_east_array[MAX_LOCATIONS] = {0};
    int distance_east_array[MAX_LOCATIONS] = {0};

    // Determining distance to get to destination travelling East
    struct location *cur_east = b->location->east;

    int i = 0;
    // Searching east through all locations until we return to the start
    while (strcmp(cur_east->name, b->location->name) != 0) {

        int skip_east = 0;
        // If the location buys but we want a seller etc. or no supply left, move on
        if ((cur_east->price < 0 && value > 0)
        || (cur_east->price > 0 && value < 0)
        || (cur_east->quantity == 0)) {
            skip_east = 1;
        }

        // If this is the right location type, record price and east length
        if ((strcmp(cur_east->fruit, destination_type) == 0) && (skip_east == 0)
        && east_len < (5 * b->maximum_move)) {
            prices_east_array[i] = cur_east->price;
            distance_east_array[i] = east_len;
            i++;
        }

        cur_east = cur_east->east;
        east_len++;
    }

    // When we do find a location of the required type, record its price and distance
    int prices_west_array[MAX_LOCATIONS] = {0};
    int distance_west_array[MAX_LOCATIONS] = {0};

    // Determining distance to get to destination travelling West
    struct location *cur_west = b->location->west;

    int j = 0;
    // Searching west through all locations until we return to the start
    while (strcmp(cur_west->name, b->location->name) != 0) {

        int skip_west = 0;
        // If the location buys but we want a seller etc. or no supply left, move on
        if ((cur_west->price < 0 && value > 0)
        || (cur_west->price > 0 && value < 0)
        || (cur_west->quantity == 0)) {
            skip_west = 1;
        }

        // If this is the right location type, record price and west length
        if ((strcmp(cur_west->fruit, destination_type) == 0) && (skip_west == 0)
        && west_len < (5 * b->maximum_move)) {
            prices_west_array[j] = cur_west->price;
            distance_west_array[j] = west_len;
            j++;
        }

        cur_west = cur_west->west;
        west_len--;
    }

    // Going through all of the found locations which buy/sell the correct type,
    // we now find the location which has the best price out of all of these.
    int east_best_price = prices_east_array[0];
    i = 1;
    int index_east = 0;
    while (prices_east_array[i] != 0) {
        if (prices_east_array[i] > east_best_price) {
            east_best_price = prices_east_array[i];
            index_east = i;
        }
        i++;
    }

    // Finding the best price location in the west direction
    int west_best_price = prices_west_array[0];
    j = 1;
    int index_west = 0;
    while (prices_west_array[j] != 0) {
        if (prices_west_array[j] > west_best_price) {
            west_best_price = prices_west_array[j];
            index_west = j;
        }
        j++;
    }

    // Whichever distance is shortest, make that the direction to travel in
    if ((-1)*(distance_west_array[index_west]) < distance_east_array[index_east]) {
        distance = distance_west_array[index_west];
    } else {
        distance = distance_east_array[index_east];
    }

    return distance;
}

// Search entire world for highest price fruit buyer
int best_fruit(struct bot *b) {

    // When we do find a location of the required type, record its price
    int prices_array[MAX_LOCATIONS] = {0};
    int distance_array[MAX_LOCATIONS] = {0};
    int distance = 1;

    struct location *cur = b->location->east;

    int i = 0;
    // Searching east through all locations until we return to the start
    while (strcmp(cur->name, b->location->name) != 0) {
        if ((cur->price > 0 && strcmp(cur->fruit, "Electricity") != 0
        && cur->quantity != 0)) {
            // Also check that we can still buy the fruit we are choosing to find
            if (best_location(b, cur->fruit, SHOP_SELLS) != 0) {
                prices_array[i] = cur->price;
                distance_array[i] = distance;
                i++;
            }
        }
        cur = cur->east;
        distance++;
    }

    // Checking the final location
    if ((cur->price > 0 && strcmp(cur->fruit, "Electricity") != 0
    && cur->quantity != 0)) {
        // Also check that we can still buy the fruit we are choosing to find
        if (best_location(b, cur->fruit, SHOP_SELLS) != 0) {
            prices_array[i] = cur->price;
            distance_array[i] = distance;
            i++;
        }
    }

    // Going through all of the found locations which buy the correct type,
    // we now find the location which has the highest buying price
    int best_price = prices_array[0];
    int best_distance = distance_array[0];
    int j = 1;
    while (prices_array[j] != 0) {
        if (prices_array[j] > best_price) {
            best_price = prices_array[j];
            best_distance = distance_array[j];
        }
        j++;
    }

    return best_distance;
}

// If all case-testing has been completed and our bot is going to move 0 spaces, double-check if it should do something else
int check_moves(struct bot *b) {
    int move = 0;
    char destination_type[MAX_NAME_CHARS];
    int shop_type;
    int world_size = calculate_world_size(b);
    int min;

    // Depending on the size of the world, choose when to stop buying electricity and food
    if (world_size < 15) {
        min = 5;
    } else if (world_size >= 15 && world_size < 50) {
        min = 10;
    } else {
        min = 15;
    }

    // There is no more electricity available
    if ((b->battery_level <= 3 * min) && b->turns_left > min) {
        // If we are still carrying fruit, try and move somewhere to sell it
        if (b->fruit_kg != 0) {
            strcpy(destination_type, b->fruit);
            shop_type = SHOP_BUYS;
        // If we are not carrying anything, simply stay at current location
        } else {
            return move;
        }
    // If the bot is carrying fruit
    } else if (b->fruit_kg != 0) {
        // If there are no more locations to buy from, find somewhere to sell what I'm carrying
        if (b->fruit_kg < (1/2 * b->maximum_fruit_kg) && b->turns_left > min) {
            strcpy(destination_type, b->fruit);
            shop_type = SHOP_BUYS;
        // If there are no more locations that will buy the fruit I am carrying, move to a compost heap to sell
        } else {
            strcpy(destination_type, "Anything");
            shop_type = SHOP_BUYS;
        }
    // If there is no more fruit available to buy, simply stay at current location
    } else {
        return move;
    }

    move = best_location(b, destination_type, shop_type);

    // If we still cannot sell the fruit we are carrying, attempt to move to a compost heap
    if (move == 0) {
        strcpy(destination_type, "Anything");
        shop_type = SHOP_BUYS;
        move = best_location(b, destination_type, shop_type);
    }

    return move;
}
