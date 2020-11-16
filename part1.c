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
sem_t *update_lock;
sem_t *print_lock;

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
	user_inLift = (sem_t *)malloc(sizeof(sem_t) * numFloors);
	user_outLift = (sem_t *)malloc(sizeof(sem_t) * numFloors);
	update_lock = (sem_t *)malloc(sizeof(sem_t));
	print_lock = (sem_t *)malloc(sizeof(sem_t));
	//INIT LIFT
	l1 = malloc(sizeof(struct lift));
	l1->max_capacity = maxNumPeople;
	l1->total_floors = numFloors;
	l1->current_floor = 0;
	l1->people_inside = 0;
	//FINISH INIT LIFT
	l1->waiting_out_currentfloor = malloc(sizeof(int) * numFloors);
	l1->waiting_in_currentfloor = malloc(sizeof(int) * numFloors);

	for (int i = 0; i < numFloors; i++)
	{
		sem_init(&user_inLift[i], 0, 0);
		sem_init(&user_outLift[i], 0, 0);
		sem_init(update_lock, 0, 1);
		sem_init(print_lock, 0, 1);

		l1->waiting_out_currentfloor[i] = 0;
		l1->waiting_in_currentfloor[i] = 0;
		l1->total_waiting = 0;
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
void *goingFromToP1(void *arg)
{
	struct argument *user = (struct argument *)arg;

	int u_id = user->id;
	int u_from = user->from;
	int u_to = user->to;

	sem_wait(update_lock);
	l1->total_waiting++;
	l1->waiting_out_currentfloor[u_from]++;
	sem_post(update_lock);

	sem_wait(&user_outLift[u_from]);

	sem_wait(print_lock);
	printf("got in %d\n", u_from);
	sem_post(print_lock);

	sem_wait(update_lock);
	l1->waiting_in_currentfloor[u_to]++;
	sem_post(update_lock);

	sem_wait(&user_inLift[u_to]);

	printf("%d %d %d\n", u_id, u_from, u_to);
	fflush(stdout);
}

void *liftf()
{
	int up = 1;

	while (1)
	{

		int waiting = l1->waiting_out_currentfloor[l1->current_floor];
		sem_wait(print_lock);
		printf("Curr F: %d\n", l1->current_floor);
		sem_post(print_lock);
		for (int i = 0; i < waiting; i++)
		{
			if (l1->people_inside < l1->max_capacity)
			{
				printf("Signaled: %d\n", l1->current_floor);
				sem_wait(update_lock);
				sem_post(&user_outLift[l1->current_floor]);

				l1->people_inside++;
				l1->total_waiting--;
				l1->waiting_out_currentfloor[l1->current_floor]--;
				sem_post(update_lock);
			}
			else
			{
				break;
			}
		}
		waiting = l1->waiting_in_currentfloor[l1->current_floor];

		for (int i = 0; i < waiting; i++)
		{
			sem_wait(update_lock);
			sem_post(&user_inLift[l1->current_floor]);
			l1->people_inside--;
			l1->waiting_in_currentfloor[l1->current_floor]--;
			sem_post(update_lock);
		}

		if (l1->people_inside == 0 && l1->total_waiting == 0)
		{
			break;
		}

		if (up == 1)
			l1->current_floor++;
		if (up == 0)
			l1->current_floor--;

		if (l1->current_floor == 0)
			up = 1;
		if (l1->current_floor == l1->total_floors - 1)
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
	sleep(2); // This is the only place where you are allowed to use sleep
	pthread_t l1_t1;

	pthread_create(&l1_t1, NULL, liftf, NULL);

	return;
}