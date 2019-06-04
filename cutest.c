/*
implementation of priority queue (or maybe it's some other data structure?! who knows)
adds and pulls can alternate, but maximum number of adds is QUEUE_MAX_SIZE
after that it needs a full wipe (actually just zeroing leaf_count, root_index and write_head is enough)
(exception: if the queue is empty, the new root can go to previously used slot)
(but another new slot is still reserved :(

duplicate key values are handled in LIFO fashion

cc -std=c99 -g cutest.c -o kue

by brtbzvid 2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// RNG by Marsaglia. Here only for testing purposes.
#define rng_znew (rng_z=36969*(rng_z&65535)+(rng_z>>16))
#define rng_wnew (rng_w=18000*(rng_w&65535)+(rng_w>>16))
#define MWC ((rng_znew<<16)+rng_wnew )
uint32_t rng_z=362436069, rng_w=521288629;



typedef struct Leaf
{
	int32_t parent;
	int32_t left_child;
	int32_t right_child;
	uint32_t value;	
} Leaf;

#define QUEUE_MAX_SIZE 16384
Leaf queue[QUEUE_MAX_SIZE];
int32_t leaf_count = 0;
int32_t root_index = 0;
int32_t write_head = 0;

bool is_empty()
{
	return leaf_count == 0 ? true : false;
}

bool value_exists(uint32_t value)
{
	int32_t read_head = root_index;
	bool keep_looking = true;

	while (keep_looking)
	{
		if ( queue[read_head].value == value )
		{
			return true;
		}
		if ( queue[read_head].value > value)
		{
			if (queue[read_head].left_child != -1)
			{
				read_head = queue[read_head].left_child;
			}
			else
			{
				keep_looking = false;
			}
		}
		if ( queue[read_head]. value < value )
		{
			if (queue[read_head].right_child != -1) 
			{
				read_head = queue[read_head].right_child;
			}
			else
			{
				keep_looking = false;
			}
		}
	}

	return false;
}

int32_t peek_lowest_value( int32_t *location, uint32_t *value )
{
	if (is_empty())
	{
		return -1;
	}
	else
	{
		int32_t read_head = root_index;

		while ( queue[read_head].left_child != -1 )
		{
			read_head = queue[read_head].left_child;
		}

		*location = read_head;
		*value = queue[read_head].value;

		return 0;
	}
}

// this doesn't really delete anything, it just rearranges associations within the tree
int32_t pull_lowest_value( uint32_t *value)
{
	int32_t result = -1337;

	if (is_empty())
	{
		result = -1;
	}
	else
	{
		// first get value
		int32_t read_head = root_index;
		while( queue[read_head].left_child != -1 )
		{
			read_head = queue[read_head].left_child;
		}
		*value = queue[read_head].value;

		// then rearrange the tree

		// am I root node?
		if (queue[read_head].parent == -1)
		{
			// do I have a right child?
			if ( queue[read_head].right_child != -1 )
			{
				// make right child new root
				queue[ queue[read_head].right_child ].parent = -1;
				root_index = queue[read_head].right_child;		
			}
			result = 3;
		}
		else if ( queue[read_head].right_child != -1 )
		{
			// connect right child and parent (cut yourself out)
			queue[ queue[read_head].right_child ].parent = queue[ read_head ].parent;
			queue[ queue[read_head].parent ].left_child = queue[ read_head ].right_child;
			result = 2;
		}
		else
		{
			// disassociate yourself from your parent
			queue[ queue[read_head].parent ].left_child = -1;
			result = 1;
		}

		leaf_count--;
	}
	return result;
}

int32_t add_new_leaf(uint32_t value)
{
	if (write_head >= QUEUE_MAX_SIZE)
	{
		return -1;
	}
	if (is_empty())
	{
		// become root leaf
		queue[root_index].value = value;
		queue[root_index].parent = -1;
		queue[root_index].left_child = -1;
		queue[root_index].right_child = -1;
		write_head++;
		leaf_count++;
	}
	else
	{
		bool place_for_new_node_found = false;
		int32_t read_head = root_index;

		while ( !place_for_new_node_found )
		{
			if ( queue[read_head].value < value ) // new value is greater than currently evaluated value
			{
				// is the right child's slot free?
				if ( queue[read_head].right_child == -1 )
				{
					// add new leaf and link it to this one under evaluation
					queue[write_head].value = value;
					queue[write_head].parent = read_head;
					queue[write_head].left_child = -1;
					queue[write_head].right_child = -1;
					queue[read_head].right_child = write_head;
					write_head++;
					leaf_count++;
					place_for_new_node_found = true;
				}
				else
				{
					// go to evaluate right child
					read_head = queue[read_head].right_child;
				}
			}
			else // new value equal or lesser than the current one
			{
				// left child's slot available?
				if ( queue[read_head].left_child == -1 )
				{
					queue[write_head].value = value;
					queue[write_head].parent = read_head;
					queue[write_head].left_child = -1;
					queue[write_head].right_child = -1;
					queue[read_head].left_child = write_head;
					write_head++;
					leaf_count++;
					place_for_new_node_found = true;	
				}
				else
				{
					read_head = queue[read_head].left_child;
				}
			}
		}
	}
	return 0;
}

int main()
{
	srand((unsigned int)time(NULL));
	rng_z = rand();
	rng_w = rand();

	for (int i = 0; i < 500; i++)
	{
		add_new_leaf(MWC % 10000 );
	}
	int32_t location = -1;
	uint32_t value = 0;
	peek_lowest_value(&location, &value);

	for (uint32_t i = 400; i < 450; i++)
	{
		if ( value_exists(i) )
		{
			printf("THERE IS %u AFTER ALL!!!\n", i);
		}
	}

	for (int i = 0; i < 50; i++)
	{
		pull_lowest_value(&value);
		printf("value: %u\n", value);
	}
	return 0;
}
