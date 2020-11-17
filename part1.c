#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "part1.h"
#include "main.h"

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */

/**
* Declare semaphores here so that they are available to all functions.
*/
sem_t *user_inLift;
sem_t *user_outLift;
sem_t *mutex;
sem_t *wait_lift;
sem_t *printer;

struct lift *l1;

const int MAX_NUM_FLOORS = 20;

/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here. Remember this is C and uses Malloc for memory allocation.
 *
 * numFloors: Total number of floors elevator can go to. numFloors will be smaller or equal to MAX_NUM_FLOORS
 * maxNumPeople: The maximum capacity of the elevator
 *
 */
void initializeP1(int numFloors, int maxNumPeople)
{
	//INIT SEMAPHORES
	user_inLift = malloc(sizeof(sem_t) * numFloors);
	user_outLift = malloc(sizeof(sem_t) * numFloors);
	mutex = (sem_t *)malloc(sizeof(sem_t));
	wait_lift = malloc(sizeof(sem_t));

	printer = malloc(sizeof(sem_t));
	//INIT LIFT
	l1 = malloc(sizeof(struct lift));
	l1->max_capacity = maxNumPeople;
	l1->total_floors = numFloors;
	l1->current_floor = 0;
	l1->people_inside = 0;
	l1->total_waiting = 0;

	//FINISH INIT LIFT
	l1->waiting_out_currentfloor = malloc(sizeof(int) * numFloors);
	l1->waiting_in_currentfloor = malloc(sizeof(int) * numFloors);
	sem_init(mutex, 0, 1);
	sem_init(wait_lift, 0, 0);
	sem_init(printer, 0, 1);

	for (int i = 0; i < numFloors; i++)
	{
		sem_init(&user_inLift[i], 0, 0);
		sem_init(&user_outLift[i], 0, 0);

		l1->waiting_out_currentfloor[i] = 0;
		l1->waiting_in_currentfloor[i] = 0;
	}

	//FINISH INITIALIZING SEMAPHORES

	return;
}

/**
 * Every passenger will call this function when 
 * he/she wants to take the elevator. (Already
 * called in main.c)
 * 
 * This function should print info "id from to" without quotes,
 * where:
 * 	id = id of the user (would be 0 for the first user)
 * 	from = source floor (from where the passenger is taking the elevator)
 * 	to = destination floor (floor where the passenger is going)
 * 
 * info of a user x_1 getting off the elevator before a user x_2
 * should be printed before.
 * 
 * Suppose a user 1 from floor 1 wants to go to floor 4 and
 * a user 2 from floor 2 wants to go to floor 3 then the final print statements
 * will be 
 * 2 2 3
 * 1 1 4
 *
 */

void print(char *str, int d)
{
	sem_wait(printer);
	printf(str, d);
	sem_post(printer);
}
void *goingFromToP1(void *arg)
{
	struct argument *user = (struct argument *)arg;

	// int max_capacity;
	// int people_inside;
	// int total_floors;
	// int current_floor;
	// int *waiting_out_currentfloor;
	// int *waiting_in_currentfloor;
	// int total_waiting;

	int id = user->id;
	int from = user->from;
	int to = user->to;

	sem_wait(mutex);
	l1->waiting_out_currentfloor[from]++;
	l1->total_waiting++;
	sem_post(mutex);

	//print("Entered %d\n", id);

	sem_wait(&user_outLift[from]);

	//print("Got in %d\n", from);

	sem_wait(mutex);
	l1->waiting_out_currentfloor[from]--;
	l1->total_waiting--;
	l1->waiting_in_currentfloor[to]++;
	l1->people_inside++;
	sem_post(mutex);

	sem_post(wait_lift);

	sem_wait(&user_inLift[to]);

	sem_wait(mutex);
	l1->waiting_in_currentfloor[to]--;
	l1->people_inside--;
	sem_post(mutex);
	sem_wait(printer);
	printf("%d %d %d\n", id, from, to);
	fflush(stdout);
	sem_post(printer);
	sem_post(wait_lift);
}

void *liftf()
{
	int up = 1;
	int floor = 0;
	int ppl = 0;
	int max = l1->max_capacity;

	while (1)
	{
		//print("Curr Floor %d\n", floor);
		sem_wait(mutex);
		int waiting = l1->waiting_out_currentfloor[floor];
		sem_post(mutex);
		if (ppl < max && waiting > 0)
		{
			//print("Signalled %d\n", floor);
			sem_post(&user_outLift[floor]);
			sem_wait(wait_lift);
			ppl++;
		}

		sem_wait(mutex);
		waiting = l1->waiting_in_currentfloor[floor];
		sem_post(mutex);

		if (waiting > 0)
		{
			//print("Signalled out %d\n", floor);

			sem_post(&user_inLift[floor]);
			sem_wait(wait_lift);
			ppl--;
		}

		if (ppl < 1 && l1->total_waiting < 1)
		{
			break;
		}

		if (up == 1)
			floor++;
		if (up == 0)
			floor--;

		if (floor == 0)
			up = 1;
		if (floor == l1->total_floors - 1)
			up = 0;
	}
}

/*If you see the main file, you will get to 
know that this function is called after setting every
passenger.

So use this function for starting your elevator. In 
this way, you will be sure that all passengers are already
waiting for the elevator.
*/
void startP1()
{
	sleep(1); // This is the only place where you are allowed to use sleep
	pthread_t l1_t1;

	pthread_create(&l1_t1, NULL, liftf, NULL);
	return;
}
