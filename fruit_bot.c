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
int find_fruit(struct bot *b);
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

    // Determining what the bot can do at its current location
    int current_stop = location_check(b);

    // Buying or selling depending on our current location
    if (current_stop == SHOP_BUYS) {
        printf("Sell %d\n", b->fruit_kg);
        played = 1;
    } else if (current_stop == SHOP_SELLS_ELEC) {
        printf("Buy %d\n", b->battery_capacity - b->battery_level);
        played = 1;
    } else if (current_stop == SHOP_SELLS_FRUIT_SAME) {
        if (b->turns_left > 2) {
            printf("Buy %d\n", b->maximum_fruit_kg - b->fruit_kg);
            played = 1;
        }
    } else if (current_stop == SHOP_SELLS_FRUIT_EMPTY) {
        if (b->turns_left > 2) {
            // Before buying, ensure we will be able to sell the fruit somewhere
            if (best_location(b, b->location->fruit, SHOP_BUYS) != 0) {
                printf("Buy %d\n", b->maximum_fruit_kg);
                played = 1;
            }
        } else { // If the game is nearly over, don't buy any more fruit
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

void run_unit_tests(void) {
    // PUT YOUR UNIT TESTS HERE
    // This is a difficult assignment to write unit tests for,
    // but make sure you describe your testing strategy above.
}

// ADD YOUR FUNCTIONS HERE

// An initial check to see if our bot can buy or sell anything at its current location
int location_check(struct bot *b) {

    int world_size = calculate_world_size(b);

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
        if (b->battery_level < b->battery_capacity && cur->quantity > 0
        && b->cash > (-1)*(cur->price)) {
            chosen = SHOP_SELLS_ELEC;
        }
    // Current location sells fruit
    } else if ((cur->price < 0) && (strcmp(cur->fruit, "Electricity") != 0)) {
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
    } else if ((b->battery_level < world_size) && b->turns_left > 6) {
        printf("We are low on energy!!!");
        chosen = LOW_ENERGY;
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

    // Determining which type of location the bot should go to
    if ((b->battery_level < world_size) && b->turns_left > 6) { // If the bot is low on energy, find a charging station
        strcpy(destination_type, "Electricity");
        shop_type = SHOP_SELLS;
    } else if (b->fruit_kg != 0) { // If the bot is carrying fruit
        strcpy(destination_type, b->fruit);
        if (b->fruit_kg < (1/2 * b->maximum_fruit_kg) && b->turns_left > 2) { // If the bot is not full, search for another shop to buy from
            shop_type = SHOP_SELLS;
        } else { // If we already close to full on fruit, just find a place that will buy it
            shop_type = SHOP_BUYS;
        }
    } else { // If the bot is not carrying fruit, find a shop that sells fruit
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

    // If we are looking to buy any type of fruit, use the find_fruit function to find the closest shop that sells fruit
    if (strcmp(destination_type, "AnyFruit") == 0) {
        distance = find_fruit(b);
        return distance;
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
        if ((strcmp(cur_east->fruit, destination_type) == 0) && (skip_east == 0)) {
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
        if ((strcmp(cur_west->fruit, destination_type) == 0) && (skip_west == 0)) {
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

// Finding the cheapest shop which will sell any fruit
int find_fruit(struct bot *b) {

    int distance = 0;
    int east_len = 1;
    int west_len = -1;

    // When we do find a location of the required type, record its price and distance
    int prices_east_array[MAX_LOCATIONS] = {0};
    int distance_east_array[MAX_LOCATIONS] = {0};

    // Determining distance to get to destination travelling East
    struct location *cur_east = b->location->east;

    int i = 0;
    // Searching east through all locations until we return to the start
    while (strcmp(cur_east->name, b->location->name) != 0) {

        // If the location sells fruit, doesn't sells electricity and has quantity, record price and east length
        if ((cur_east->price < 0
        && strcmp(cur_east->fruit, "Electricity") != 0
        && cur_east->quantity != 0)) {
            // Also check that if we were to buy from this location, we will be able to sell it somewhere
            if (best_location(b, cur_east->fruit, SHOP_BUYS) != 0) {
                prices_east_array[i] = cur_east->price;
                distance_east_array[i] = east_len;
                i++;
            }
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

        // If the location sells fruit, doesn't sells electricity and has quantity, record price and west length
        if (cur_west->price < 0
        && strcmp(cur_west->fruit, "Electricity") != 0
        && cur_west->quantity != 0) {
            // Also check that if we were to buy from this location, we will be able to sell it somewhere
            if (best_location(b, cur_west->fruit, SHOP_BUYS) != 0) {
                prices_west_array[j] = cur_west->price;
                distance_west_array[j] = west_len;
                j++;
            }
        }

        cur_west = cur_west->west;
        west_len--;
    }

    // With all of the fruit selling locations, we now want to find the cheapest one
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

// If all case-testing has been completed and our bot is going to move 0 spaces, double-check if it should do something else
int check_moves(struct bot *b) {

    int move = 0;
    char destination_type[MAX_NAME_CHARS];
    int shop_type;
    int world_size = calculate_world_size(b);

    if ((b->battery_level < world_size) && b->turns_left > 6) { // There is no more electricity available
        if (b->fruit_kg != 0) { // If we are still carrying fruit, try and move somewhere to sell it
            strcpy(destination_type, b->fruit);
            shop_type = SHOP_BUYS;
        } else { // If we are not carrying anything, simply stay at current location
            return move;
        }
    } else if (b->fruit_kg != 0) { // If the bot is carrying fruit
        if (b->fruit_kg < (1/2 * b->maximum_fruit_kg) && b->turns_left > 2) { // If there are no more locations to buy from, find somewhere to sell what I'm carrying
            strcpy(destination_type, b->fruit);
            shop_type = SHOP_BUYS;
        } else { // If there are no more locations that will buy the fruit I am carrying, move to a compost heap to sell
            strcpy(destination_type, "Anything");
            shop_type = SHOP_BUYS;
        }
    } else { // If there is no more fruit available to buy, simply stay at current location
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
