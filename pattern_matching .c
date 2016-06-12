// Yoad Shiran

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pattern_matching.h"



/* Initializes the fsm parameters (the fsm itself sould be allocated).  Returns 0 on success, -1 on failure. 
*  this function should init zero state
*/
int pm_init(pm_t *root)
{
	pm_state_t *temp_p=(pm_state_t*)calloc(1,sizeof(pm_state_t));
	if (temp_p==NULL)	{return -1;}
	slist_t *output=(slist_t*)calloc(1,sizeof(slist_t));
	if (output==NULL)
	{
		free(temp_p);
		return -1;
	}
	slist_t *_transitions=(slist_t*)calloc(1,sizeof(slist_t));
	if (_transitions==NULL)
	{
		free(temp_p);
		free(output);
		return -1;
	}
	slist_init(output);
	slist_init(_transitions);
	temp_p->depth=0;
	temp_p->id = 0;
	temp_p->output=output;
	temp_p->fail = NULL;
	temp_p->_transitions=_transitions;
	root->zerostate =  temp_p;
	root->newstate=0; 
	return 0;
}
/* Adds a new string to the fsm, given that the string is of length n. 
   Returns 0 on success, -1 on failure.*/
int pm_addstring(pm_t *add_s,unsigned char * string, size_t n)
{
	if (strlen(string) != n)
	{
	printf("Error with the length of string : '%s' (the length is not true)\n line 45 in pattern_matching.c\n", string);
	return -1;
	}
	int i=0;
	int count=0;
	if (n<1)	{return 0;}
	if (add_s==NULL || string==NULL)	{return -1;}
	pm_state_t *temp_p = add_s->zerostate;
	pm_state_t *point_to_root = pm_goto_get(temp_p, string[i]);
	while (i<n && (point_to_root!=NULL))
	{
		i++;
		count++;
		temp_p=point_to_root;
		point_to_root = pm_goto_get(temp_p, string[i]);
	}
	while (i<n)
	{
		pm_state_t *new_node = (pm_state_t*)calloc(1,sizeof(pm_state_t));
		if (new_node==NULL)	{return -1;}
		slist_t *output=(slist_t*)calloc(1,sizeof(slist_t));
		if (output==NULL)
		{
			free(temp_p);
			return -1;
		}
		slist_t *transitions=(slist_t*)calloc(1,sizeof(slist_t));
		if (transitions==NULL)
		{
			free(temp_p);
			free(output);
			return -1;
		}
		slist_init(output);
		slist_init(transitions);
		add_s->newstate++;
		new_node->id += add_s->newstate;
		new_node->depth = count;
		new_node->fail = NULL;
		new_node->_transitions = transitions;
		i++;
		
		if (i == n )
		{
			if (slist_append(output,(void*)string) == -1) {return -1;}
		}
		new_node->output = output;
		pm_goto_set(temp_p,string[i-1],new_node);
		//printf("Allocating state %d \n", new_node->id);
		//printf("%d -> %c -> %d \n", temp_p->id, string[i-1], new_node->id);
		temp_p=new_node;
		count++;
	}
	return 0;
}

/* Set a transition arrow from this from_state, via a symbol, to a
   to_state. will be used in the pm_addstring and pm_makeFSM functions.
   Returns 0 on success, -1 on failure.*/   
int pm_goto_set(pm_state_t *from_state, unsigned char symbol, pm_state_t *to_state)
{
	if (from_state==NULL || to_state==NULL)	{return -1;}
	pm_labeled_edge_t *edge = (pm_labeled_edge_t*)calloc(1,sizeof(pm_labeled_edge_t));
	if (edge==NULL)	{return -1;}
	edge->state = to_state;
	edge->label = symbol;
	
	if (slist_append(from_state->_transitions, (pm_labeled_edge_t*)edge)==-1)
	{
		free(edge);
		printf("Error with append of goto_set!\n");
		return -1;
	}
	return 0;
}
/* Returns the transition state.  If no such state exists, returns NULL. 
   will be used in pm_addstring, pm_makeFSM, pm_fsm_search, pm_destroy functions. */
pm_state_t* pm_goto_get(pm_state_t *state,unsigned char symbol)
{
	int size = slist_size(state->_transitions);
	if (state==NULL  || state->_transitions==NULL)
	{
		return NULL;
	}
	
	slist_node_t* temp_p= slist_head(state->_transitions);
	if (temp_p==NULL)
	{
		 return NULL;
	}
	int i=0;
	while(i<size)
	{
		if (((pm_labeled_edge_t*)slist_data(temp_p))->label==symbol)
		{
			return ((pm_labeled_edge_t*)slist_data(temp_p))->state;
		}
		temp_p=slist_next(temp_p);
		i++;
	}
	return NULL;
}
int pm_makeFSM(pm_t *root)
{
	slist_t *queue = (slist_t*)calloc(1,sizeof(slist_t));
	if (queue==NULL)
	{
		printf("Error with calloc of makeFSM!\n");
		return -1;
	}
	slist_init(queue);
	unsigned char* symbol;
	int size = slist_size(root->zerostate->_transitions);
	slist_node_t* p = slist_head(root->zerostate->_transitions);
	pm_labeled_edge_t* edge = slist_data(p);
	symbol = slist_data(p);
	pm_state_t* depth_one;
	while (1)
	{
		depth_one = pm_goto_get(root->zerostate,*symbol);
		if (p != NULL)
		{
			depth_one->fail = root->zerostate;
			if(slist_append(queue,depth_one) == -1) {return -1;}
		}
		size--;
		if (size==0){break;}
		p = slist_next(p);
		symbol = slist_data(p);
	}
	pm_state_t *prev;
	pm_state_t *next;
	pm_state_t *this_s;
	int flag;
	size=slist_size(queue);
	slist_node_t* head_queue = NULL;
	int size_queue=0;
	while (size > 0)
	{
		size--;
		this_s = (pm_state_t*)slist_pop_first(queue);
		if (this_s->_transitions != NULL)
		{
			head_queue = slist_head(this_s->_transitions);
			size_queue = slist_size(this_s->_transitions);
		}
		if (head_queue != NULL)
		{
			symbol =  slist_data(head_queue);
		}
		
		while(size_queue>0)
		{
			flag=0;
			next = pm_goto_get(this_s,*symbol);
			if(next != NULL)
			{
				if(slist_append(queue,next) == -1) {return -1;}
				prev = this_s->fail;
				while(pm_goto_get(prev,*symbol) == NULL)
				{
					if(prev->id == 0)
					{
						next->fail = prev;
						//printf("Setting f(%d) == %d\n", next->id, next->fail->id);
						flag=1;
						size++;
						size_queue--;
						if (slist_next(head_queue) != NULL)
						{	
							head_queue = slist_next(head_queue);
							symbol =  slist_data(head_queue);
						}	
						break;
					}
					prev = prev->fail;
				}
				if(flag==1)	{continue;}
				next->fail = pm_goto_get(prev,*symbol);
				//printf("Setting f(%d) == %d\n", next->id, next->fail->id);
				size++;
				if(slist_append_list(next->output, next->fail->output)==-1)	{return -1;}
				size_queue--;
				if (slist_next(head_queue) != NULL)
				{	
					head_queue = slist_next(head_queue);
					symbol =  slist_data(head_queue);
				}	
			}
		}
	}	
	slist_destroy(queue,SLIST_FREE_DATA);
	free(queue);
	return 0;
}
slist_t* pm_fsm_search(pm_state_t *root,unsigned char *symbol,size_t length)
{
	if (strlen(symbol) != length)
	{
	printf("Error with the length of string : '%s' (the length is not true)\n line 45 in pattern_matching.c\n", symbol);
	return NULL;
	}
	if (root==NULL){return NULL;}
	if (length == 0){return NULL;}
	int j;
	int count=0;
	slist_t *match_l = (slist_t*)calloc(1,sizeof(slist_t));
	if (match_l==NULL)
	{
		printf("error[5]\n");
		return NULL;
	}
	slist_init(match_l);
	pm_state_t * temp_p = root;
	for (j = 0; j < length; j++)
	{
		while(pm_goto_get(temp_p,symbol[j])==NULL && j<length)
		{
			if (temp_p->id != 0)
			{
				temp_p = temp_p->fail;
				continue;
			}
			j++;
		}
		temp_p = pm_goto_get(temp_p,symbol[j]);
		if (temp_p == NULL)
		{
			continue; //Lior
		}
		int size= slist_size(temp_p->output);
		if (size>0)
		{
			slist_node_t *node = slist_head(temp_p->output);
			while (node != NULL)
			{
				pm_match_t* m = (pm_match_t*)calloc(1,sizeof(pm_match_t));
				if (m==NULL)
				{
					slist_destroy(match_l,SLIST_FREE_DATA);
					free(match_l);
					return NULL;
				}
				int sizeofstring = strlen(slist_data(node));
				m->start_pos = j-sizeofstring+1;

				m->end_pos = j;
				m->fstate = temp_p;
				m->pattern = slist_data(node);
				m->pattern += '\0';
				if(slist_append(match_l,m) == -1)
				{
					printf("Error with pm_fsm_search function (line 297)\n");
					exit(-1);
				} 
				node = slist_next(node);
			}
		}
		count = j+size-1;
	}
return match_l;
}

void pm_destroy(pm_t *root)
{
	unsigned char* symbol;
	int size = slist_size(root->zerostate->_transitions);
	if (size == 0)
	{
		slist_destroy(root->zerostate->_transitions, SLIST_FREE_DATA);
		slist_destroy(root->zerostate->output, SLIST_FREE_DATA);
		free(root->zerostate->_transitions);
		free(root->zerostate->output);
		free(root->zerostate);
		return;
	}
	slist_t *queue = (slist_t*)calloc(1,sizeof(slist_t));
	slist_init(queue);
	slist_node_t* p = slist_head(root->zerostate->_transitions);
	symbol = slist_data(p);
	while (1)
	{
		pm_state_t* depth_one = pm_goto_get(root->zerostate,*symbol);
		if (p != NULL)
		{
			if (slist_append(queue,depth_one) == -1)
			{
				printf("Error with append of Destroy!\n");
				return;
			}
		}
		size--;
		if (size==0) {break;}
		p = slist_next(p);
		symbol = slist_data(p);
	}
	size=slist_size(queue);
	slist_node_t* head_queue = NULL;
	int size_queue=0;
	while (size > 0)
	{
		size--;
		pm_state_t *this_s = (pm_state_t*)slist_pop_first(queue);
		if (this_s->_transitions != NULL)
		{
			head_queue = slist_head(this_s->_transitions);
			size_queue = slist_size(this_s->_transitions);
		}
		if (head_queue != NULL)
		{
			symbol =  slist_data(head_queue);
		}
		while(size_queue>0)
		{
			pm_state_t *next = pm_goto_get(this_s,*symbol);

			if(next != NULL)
			{

				if(slist_append(queue,next) == -1)
				{
					printf("Error with Destroy func(line 366)\n");
				 	exit(-1);
				}
				size++;
				size_queue--;
				if (slist_next(head_queue) != NULL)
				{	
					head_queue = slist_next(head_queue);
					symbol =  slist_data(head_queue);
				}
			}
		}
		slist_destroy(this_s->_transitions, SLIST_FREE_DATA);
		slist_destroy(this_s->output, SLIST_LEAVE_DATA);
		free(this_s->_transitions);
		free(this_s->output);
		free(this_s);
	}
slist_destroy(root->zerostate->_transitions,SLIST_FREE_DATA);
slist_destroy(root->zerostate->output,SLIST_LEAVE_DATA);
slist_destroy(queue,SLIST_FREE_DATA);
free(queue);
free(root->zerostate->_transitions);
free(root->zerostate->output);
free(root->zerostate->fail);
free(root->zerostate);
}
