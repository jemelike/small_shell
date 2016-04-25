#include <stdio.h>
#include <stdlib.h>

struct node {
  int x;
  struct node *next;
};


void insert(struct node*,struct node*,struct node*);
void print(struct node*);
void removeNode(int, struct node*);

int main()
{
    /* This won't change, or we would lose the list in memory */
    struct node *root , *tail;       
    /* This will point to each node as it traverses the list */
    struct node *conductor, *new;  

    root = malloc( sizeof(struct node) );  
    root->next = 0;   
    root->x = 12;
	/*
	new = malloc(sizeof(struct node));
	new->next = NULL;
	new->x = 24;
	
	tail = root;
	/*
    conductor = root; 
    if ( conductor != 0 ) {
        while ( conductor->next != 0)
        {
            conductor = conductor->next;
        }
    }
    /* Creates a node at the end of the list *
    conductor->next = malloc( sizeof(struct node) );  

    conductor = conductor->next; 

    if ( conductor == 0 )
    {
        printf( "Out of memory" );
        return 0;
    }*/
    /* initialize the new memory 
    conductor->next = 0;         
    conductor->x = 42;*/
/*
	print(root);
	printf("\n\n");
	insert(new, root, tail);
	print(root);
	
	printf("\n\n");
	
	new = malloc(sizeof(struct node));
	new->next = NULL;
	new->x = 32;
	insert(new,root, tail);
	print(root);
	
	printf("\n\n");
	new = malloc(sizeof(struct node));
	new->next = NULL;
	new->x = 24;
	insert(new,root, tail);
	print(root);
	
*/
	printf("Removal Test\n\n");	
	removeNode(12, root);
	print(root);
	
    return 0;
}

void insert(struct node* new, struct node* root, struct node* tail)
{
	struct node *current;
	 
	if(root == NULL)
		root = new;
	else	 
		for( current = root; current != NULL ;current = current->next)
			if(current->next == NULL){
				current->next = new;
				break;
			}
}

void removeNode(int val, struct node* root)
{
	struct node *current, *temp, *trail;
	int isRoot = 1;
	
	for(current = root, trail = NULL; current != NULL; current = current->next)
	{
		if(current->x == val)
		{
			
			temp = current;
			if(isRoot > 1){
			trail->next =current->next;
			current = current->next;
			free(temp);
			}else
			{
				if(current->next !=NULL)
				{
					root = current->next;
				}
				else
					root = NULL;
				
				break;
			}
			
			break;
		}
		trail = current;
		isRoot++;
	}
}

 void print( struct node* root)
 {
	 struct node *current;
	 
	 for( current = root; current != NULL ;current = current->next)
		 printf("%d\n", current->x);
 }
 
