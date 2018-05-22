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

#define SHOP_BUYS 1
#define SHOP_SELLS_ELEC 2
#define SHOP_SELLS_FRUIT_SAME 3
#define SHOP_SELLS_FRUIT_EMPTY 4

void print_player_name(void);
void print_move(struct bot *b);
void run_unit_tests(void);

// ADD PROTOTYPES FOR YOUR FUNCTIONS HERE
int location_check(struct bot *b);
int choose_location_type(struct bot *b);
int nearest_location(struct bot *b, char destination_type[MAX_NAME_CHARS], int value);
int find_fruit(struct bot *b);


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

    // Insert functions to determine where the bot should travel
    // and determine what type of place this is. Assign to variable
    // destination_type.

    // Keeping track of whether the bot has already made a play this round
    int played = 0;

    int current_stop = location_check(b);

    // Buying or selling depending on our current location
    if (current_stop == SHOP_BUYS) {
        printf("Sell %d\n", b->fruit_kg);
        played = 1;
    } else if (current_stop == SHOP_SELLS_ELEC) {
        printf("Buy %d\n", b->battery_capacity - b->battery_level);
        played = 1;
    } else if (current_stop == SHOP_SELLS_FRUIT_SAME) {
        printf("Buy %d\n", b->maximum_fruit_kg - b->fruit_kg);
        played = 1;
    } else if (current_stop == SHOP_SELLS_FRUIT_EMPTY) {
        printf("Buy %d\n", b->maximum_fruit_kg);
        played = 1;
    }

    // If we didn't buy or sell, we want to move somewhere
    if (played != 1) {
        int move_selection = choose_location_type(b);
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

    struct location *cur = b->location;
    int chosen = 0;

    // If the place sells fruit, it has fruit remaining and I have fruit on board
    if (cur->price > 0 && cur->quantity > 0 && b->fruit != NULL) {
        // If I am carrying the right type of fruit, sell as much as I can
        if (strcmp(cur->fruit, b->fruit) == 0) {
            chosen = SHOP_BUYS;
        }
    } else if (cur->price < 0) { // Shop only sells fruit or electricity
        if (strcmp(cur->fruit, "Electricity") == 0) { // Shop sells electricity
            if (b->battery_level < b->battery_capacity && cur->quantity > 0
            && b->cash > (-1)*(cur->price)) {
                chosen = SHOP_SELLS_ELEC;
            }
        } else { // Shop sells fruit
            if (b->fruit != NULL) {
                if (strcmp(cur->fruit, b->fruit) == 0 && cur->quantity > 0
                && b->cash > (-1)*(cur->price)) { // Bot has same fruit
                    chosen = SHOP_SELLS_FRUIT_SAME;
                }
            } else if (b->fruit == NULL) { // Bot has an empty cart
                if (cur->quantity > 0 && b->cash > (-1)*(cur->price)) {
                chosen = SHOP_SELLS_FRUIT_EMPTY;
                }
            }
        }
    }

    return chosen;
}

// Determining what type of place we want our bot to move to
int choose_location_type(struct bot *b) {

    char destination_type[MAX_NAME_CHARS];
    int move;

    // Determining which type of location the bot should go to
    if (b->battery_level < b->maximum_move) { // If the bot is low on energy, find a charging station
        strcpy(destination_type, "Electricity");
        move = nearest_location(b, destination_type, -1);
    } else if (b->fruit_kg != 0) { // If the bot is carrying fruit, find a shop that buys fruit
        strcpy(destination_type, b->fruit);
        move = nearest_location(b, destination_type, 1);
    } else { // If the bot is not carrying fruit, find a shop that sells fruit
        move = find_fruit(b);
    }

    return move;
}

// Determining whether East or West is quicker to get to a location
int nearest_location(struct bot *b, char destination_type[MAX_NAME_CHARS], int value) {

    int distance = 0;
    int east_len = 0;
    int west_len = 0;

    // Determining distance to get to destination travelling East
    struct location *cur_east = b->location;

    while (strcmp(cur_east->fruit, destination_type) != 0
    || (cur_east->price < 0 && value > 0)
    || (cur_east->price > 0 && value < 0)
    || cur_east->quantity == 0) {
        cur_east = cur_east->east;
        east_len++;
        
        //Breaking loop if we have looped through all spots and it doesn't exist
        if (cur_east->name == b->location->name) { 
            east_len = 1;
            break;
        }

    }

    // Determining distance to get to destination travelling West
    struct location *cur_west = b->location;

    while (strcmp(cur_west->fruit, destination_type) != 0
    || (cur_west->price < 0 && value > 0)
    || (cur_west->price > 0 && value < 0)
    || cur_west->quantity == 0) {
        cur_west = cur_west->west;
        west_len--;
        
        //Breaking loop if we have looped through all spots and it doesn't exist
        if (cur_west->name == b->location->name) { 
            west_len = 1;
            break;
        }
    }

    // Whichever distance is shortest, make that the direction to travel in
    if ((-1)*(west_len) < east_len) {
        distance = west_len;
    } else {
        distance = east_len;
    }

    return distance;
}

// Finds the closest shop which my bot can buy fruit from
int find_fruit(struct bot *b) {

    int distance = 0;
    int east_len = 0;
    int west_len = 0;

    // Determining distance to get to destination travelling East
    struct location *cur_east = b->location;

    while (cur_east->price >= 0
    || strcmp(cur_east->fruit, "Electricity") == 0
    || cur_east->quantity == 0) { // Whilst the locations do not sell anything
        cur_east = cur_east->east;
        east_len++;
        
        //Breaking loop in case there are no more spots selling fruit
        if (cur_east->name == b->location->name) { 
            east_len = 1;
            break;
        }
    }

    // Determining distance to get to destination travelling West
    struct location *cur_west = b->location;

    while (cur_west->price >= 0
    || strcmp(cur_west->fruit, "Electricity") == 0
    || cur_west->quantity == 0) {
        cur_west = cur_west->west;
        west_len--;
        
        //Breaking loop in case there are no more spots selling fruit
        if (cur_west->name == b->location->name) { 
            west_len = 0;
            break;
        }
    }

    // Whichever distance is shortest, make that the direction to travel in
    if ((-1)*(west_len) < east_len) {
        distance = west_len;
    } else {
        distance = east_len;
    }

    return distance;
}
