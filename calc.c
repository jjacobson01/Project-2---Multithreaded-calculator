/* Project 2 - Multithreaded calculator */
// Name: Jose Aguilar
// Partner: Jonathon Jacobson

#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;

char buffer[BUF_SIZE];
int num_ops;

/* Step 3: add mutual exclusion */

static pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
/* Step 6: add condition flag varaibles */
struct progress_t
{
	int add;
	int mult;
	int group;
} progress;

/* Step 7: use a semaphore */
static sem_t progress_lock;

/* Utiltity functions provided for your convenience */

/* int2string converts an integer into a string and writes it in the
   passed char array s, which should be of reasonable size (e.g., 20
   characters).  */
char *int2string(int i, char *s)
{
	sprintf(s, "%d", i);
	return s;
}

/* string2int just calls atoi() */
int string2int(const char *s)
{
	return atoi(s);
}

/* isNumeric just calls isdigit() */
int isNumeric(char c)
{
	return isdigit(c);
}

/* End utility functions */

void printErrorAndExit(char *msg)
{
	msg = msg ? msg : "An unspecified error occured!";
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

int timeToFinish()
{
	/* be careful: timeToFinish() also accesses buffer */
	return buffer[0] == '.';
}

/* Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
   and, if found, adds the two numbers and replaces the addition subexpression 
   with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
   to worry about associativity! */
void *adder(void *arg)
{
	int bufferlen;
	int value1, value2;
	int startOffset, remainderOffset;
	char nstring[50];
	int changed = 0;
	int i;

	//return NULL; /* remove this line to let the loop start*/

	while (1)
	{
		if (progress.add != 2)
		{
			/* Step 3: add mutual exclusion */
			pthread_mutex_lock(&buffer_lock);

			startOffset = remainderOffset = -1;
			value1 = value2 = -1;

			if (timeToFinish())
			{
				pthread_mutex_unlock(&buffer_lock);
				return NULL;
			}

			/* storing this prevents having to recalculate it in the loop */
			bufferlen = strlen(buffer);

			/* Step 2: implement adder */
			for (i = 0; i < bufferlen; i++)
			{
				// do we have value1 already?  If not, is this a "naked" number?
				// if we do, is the next character after it a '+'?
				// if so, is the next one after the sign a "naked" number?

				//first we make sure that we even have a number in the buffer
				//and point the buffer to our value1 for reference
				if (isNumeric(buffer[i]))
				{
					//this is where we start off the first variable
					startOffset = i;
					value1 = atoi(&buffer[i]);

					//we keep going through the array and incrementing the index
					//till we hit something that isn't a number
					while (isdigit(buffer[i]))
					{
						i++;
					}

					//this is our signal that we hit the midway point of the adder
					//operation or the next different operation
					if (buffer[i] != '+' || !isNumeric(buffer[i + 1]))
					{
						continue;
					}
					value2 = string2int(&buffer[i + 1]);

					// once we have value1, value2 and start and end offsets of the
					// expression in buffer, replace it with v1+v2

					int result = value1 + value2;
					//we keep incremeting the index till we reach the true end of the array
					//or the next operation
					do
					{
						i++;
					} while (isNumeric(buffer[i]));
					remainderOffset = i;

					int2string(result, nstring);

					//we clone the buffer to modify it to have the result
					char bufferclone[1000];
					strcpy(bufferclone, &buffer);

					//we go into the buffer and copy over the result of the
					//operation
					strcpy(buffer + startOffset, &nstring);

					//then we copy over the remainder due to the original buffer becoming modified
					strcpy(buffer + (startOffset + strlen(nstring)), &bufferclone[remainderOffset]);

					bufferlen = bufferlen - (remainderOffset - 1 - startOffset);
					i = startOffset - 1;
					changed = 1;
					num_ops++;
				}
			}

			// something missing?
			/* Step 3: free the lock */
			pthread_mutex_unlock(&buffer_lock);
			/* Step 6: check progress */
			sem_wait(&progress_lock);
			progress.add = changed ? 1 : 2;
			sem_post(&progress_lock);
			changed = 0;
			/* Step 5: let others play */
			sched_yield();
		}
		else
		{
			sched_yield();
		}
	}
}

/* Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
   "5*6" and, if found, multiplies the two numbers and replaces the
   mulitplication subexpression with the result ("1+(5*6)+8" becomes
   "1+(30)+8"). */
void *multiplier(void *arg)
{
	int bufferlen;
	int value1, value2;
	int startOffset, remainderOffset;
	int i;
	int changed = 0;
	char nstring[50];

	//return NULL; /* remove this line */

	while (1)
	{

		if (progress.mult != 2)
		{
			/* Step 3: add mutual exclusion */
			pthread_mutex_lock(&buffer_lock);
			startOffset = remainderOffset = -1;
			value1 = value2 = -1;

			if (timeToFinish())
			{
				pthread_mutex_unlock(&buffer_lock);
				return NULL;
			}

			/* storing this prevents having to recalculate it in the loop */
			bufferlen = strlen(buffer);

			/* Step 2: implement multiplier */
			for (i = 0; i < bufferlen; i++)
			{
				// do we have value1 already?  If not, is this a "naked" number?
				// if we do, is the next character after it a '+'?
				// if so, is the next one after the sign a "naked" number?

				//first we make sure that we even have a number in the buffer
				//and point the buffer to our value1 for reference
				if (isNumeric(buffer[i]))
				{
					//this is where we start off the first variable
					startOffset = i;
					value1 = atoi(&buffer[i]);

					//we keep going through the array and incrementing the index
					//till we hit something that isn't a number
					while (isNumeric(buffer[i]))
					{
						i++;
					}

					//this is our signal that we hit the midway point of the adder
					//operation or the next different operation
					if (buffer[i] != '*' || !isNumeric(buffer[i + 1]))
					{
						continue;
					}
					value2 = string2int(&buffer[i + 1]);

					// once we have value1, value2 and start and end offsets of the
					// expression in buffer, replace it with v1+v2

					int result = value1 * value2;
					//we keep incremeting the index till we reach the true end of the array
					//or the next operation
					do
					{
						i++;
					} while (isNumeric(buffer[i]));
					remainderOffset = i;

					int2string(result, nstring);
					int2string(result, nstring);
					strncpy(buffer + startOffset, &nstring[0], strlen(nstring));
					strcpy(buffer + startOffset + strlen(nstring), buffer + remainderOffset);

					bufferlen = bufferlen - (remainderOffset - 1 - startOffset);
					i = startOffset - 1;

					startOffset = 0;
					remainderOffset = 0;
					changed = 1;
					num_ops++;
				}
			}

			// something missing?
			/* Step 3: free the lock */
			pthread_mutex_unlock(&buffer_lock);
			/* Step 6: check progress */
			sem_wait(&progress_lock);
			progress.mult = changed ? 1 : 2;
			sem_post(&progress_lock);
			changed = 0;
			/* Step 5: let others play */
			sched_yield();
		}
		else
		{
			sched_yield();
		}
	}
}

/* Looks for a number immediately surrounded by parentheses [e.g.
   "(56)"] in the buffer and, if found, removes the parentheses leaving
   only the surrounded number. */
void *degrouper(void *arg)
{
	int startOffset, remainderOffset;
	int bufferlen;
	int changed;
	int i;

	//return NULL; /* remove this line */

	while (1)
	{

		if (progress.group != 2)
		{
			/* Step 3: add mutual exclusion */

			pthread_mutex_lock(&buffer_lock);

			if (timeToFinish())
			{
				return NULL;
			}

			/* storing this prevents having to recalculate it in the loop */
			bufferlen = strlen(buffer);

			/* Step 2: implement degrouper */
			for (i = 0; i < bufferlen; i++)
			{

				// check for '(' followed by a naked number followed by ')'
				// remove ')' by shifting the tail end of the expression
				// remove '(' by shifting the beginning of the expression

				//here we find the indicator that this is a degrouper function
				//that being the '(' in the buffer
				if (buffer[i] == '(')
				{

					//at that point we save the index  for starting reference
					startOffset = i;
					if (!isNumeric(buffer[i + 1]))
					{
						continue;
					}

					//we keep loopinng through the indeces till we find
					//one that is the other )
					do
					{
						i++;
					} while (isNumeric(buffer[i]));
					;

					//we find the ) and remove both
					if (buffer[i] == ')')
					{
						remainderOffset = i;
						char bufferclone[1000];
						strcpy(bufferclone, &buffer);

						strcpy(buffer + (startOffset), buffer + (startOffset + 1));

						strcpy(buffer + remainderOffset - 1, buffer + (remainderOffset));
						bufferlen = bufferlen - 2;
						changed = 1;
						num_ops++;
						i = -1;
					}
					else
					{
						continue;
					}
				}
			}

			// something missing?
			/* Step 3: free the lock */
			pthread_mutex_unlock(&buffer_lock);

			/* Step 6: check progress */
			sem_wait(&progress_lock);
			progress.group = changed ? 1 : 2; // 1 means changed, 2 didn't changed;
			sem_post(&progress_lock);
			changed = 0;
			/* Step 5: let others play */
			sched_yield();
		}
		else
		{
			sched_yield();
		}
	}
}

/* sentinel waits for a number followed by a ; (e.g. "453;") to appear
   at the beginning of the buffer, indicating that the current
   expression has been fully reduced by the other threads and can now be
   output.  It then "dequeues" that expression (and trailing ;) so work can
   proceed on the next (if available). */
void *sentinel(void *arg)
{
	char numberBuffer[20];
	int bufferlen;
	int i;

	//return NULL; /* remove this line */

	while (1)
	{

		/* Step 3: add mutual exclusion */

		pthread_mutex_lock(&buffer_lock);
		if (timeToFinish())
		{
			return NULL;
		}

		/* storing this prevents having to recalculate it in the loop */
		bufferlen = strlen(buffer);

		for (i = 0; i < bufferlen; i++)
		{
			if (buffer[i] == ';')
			{
				if (i == 0)
				{
					printErrorAndExit("Sentinel found empty expression!");
				}
				else
				{
					/* null terminate the string */
					numberBuffer[i] = '\0';
					/* print out the number we've found */
					fprintf(stdout, "%s\n", numberBuffer);
					/* shift the remainder of the string to the left */
					strcpy(buffer, &buffer[i + 1]);
					break;
				}
			}
			else if (!isNumeric(buffer[i]))
			{
				break;
			}
			else
			{
				numberBuffer[i] = buffer[i];
			}
		}

		// something missing?

		pthread_mutex_unlock(&buffer_lock);
		/* Step 6: check for progress */

		if (timeToFinish())
		{
			return NULL;
		}

		sem_wait(&progress_lock);
		if (progress.add == 1 ||
			progress.mult == 1 ||
			progress.group == 1)
		{
			progress.add = 0;
			progress.mult = 0;
			progress.group = 0;
		}
		if (progress.add == 2 &&
			progress.mult == 2 &&
			progress.group == 2)
		{
			printErrorAndExit("No progress can be made\n");
		}
		sem_post(&progress_lock);

		/* Step 5: let others play, too */
		sched_yield();
	}
}

/* reader reads in lines of input from stdin and writes them to the
   buffer */
void *reader(void *arg)
{
	while (1)
	{
		char tBuffer[100];
		int currentlen;
		int newlen;
		int free;

		fgets(tBuffer, sizeof(tBuffer), stdin);

		/* Sychronization bugs in remainder of function need to be fixed */

		newlen = strlen(tBuffer);
		currentlen = strlen(buffer);

		/* if tBuffer comes back with a newline from fgets, remove it */
		if (tBuffer[newlen - 1] == '\n')
		{
			/* shift null terminator left */
			tBuffer[newlen - 1] = tBuffer[newlen];
			newlen--;
		}

		/* -1 for null terminator, -1 for ; separator */
		free = sizeof(buffer) - currentlen - 2;

		while (free < newlen)
		{
			// spinwaiting TO DO
		}

		/* Step 3: add mutual exclusion */

		/* we can add another expression now */
		strcat(buffer, tBuffer);
		strcat(buffer, ";");

		/* Step 6: reset flag variables indicating progress */

		/* Stop when user enters '.' */
		if (tBuffer[0] == '.')
		{
			pthread_mutex_unlock(&buffer_lock);
			strcat(buffer, "\0");
			return NULL;
		}
	}
}

/* Where it all begins */
int smp3_main(int argc, char **argv)
{
	void *arg = 0; /* dummy value */

	/* Step 7: initialize your semaphore */

	sem_init(&progress_lock, 0, 1);
	pthread_mutex_lock(&buffer_lock);

	/* let's create our threads */
	if (pthread_create(&multiplierThread, NULL, multiplier, arg) || pthread_create(&adderThread, NULL, adder, arg) || pthread_create(&degrouperThread, NULL, degrouper, arg) || pthread_create(&sentinelThread, NULL, sentinel, arg) || pthread_create(&readerThread, NULL, reader, arg))
	{
		printErrorAndExit("Failed trying to create threads");
	}

	/* you need to join one of these threads... but which one? */
	/* The pthread_join() function for threads is the equivalent of wait() 
	for processes. A call to pthread_join blocks the calling thread until 
	the thread with identifier equal to the first argument terminates.*/
	// If you join do not detach.

	pthread_detach(readerThread);
	pthread_detach(multiplierThread);
	pthread_detach(adderThread);
	pthread_detach(degrouperThread);
	/* Step 1: we have to join on the ________ thread. */
	// pthread_join(____, NULL);

	pthread_join(sentinelThread, NULL);

	//we need to join the main thread that can access and execute all the others
	//from the ones displayed, they all provide a singular function that needs
	// the others to complete a "calculator"

	/* everything is finished, print out the number of operations performed */
	fprintf(stdout, "Performed a total of %d operations\n", num_ops);

	// TODO destroy semaphores and mutex
	sem_destroy(&progress_lock);
	pthread_mutex_destroy(&progress_lock);
	// return
	return EXIT_SUCCESS;
}
