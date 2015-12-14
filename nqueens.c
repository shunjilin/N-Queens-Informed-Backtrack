/* Exercise 3.2 
2) Local Search using the Min-Conflicts Heuristic 
*/

/* Lin Gengxian Shunji 
   08-144505
   10th September 2015
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include "dbg.h"

#define N 1000000 /* number of queens */
#define D 1000000 /* limit depth of search if D == N, no limit*/


typedef struct value_node {
  int col_index; /* column index */
  int conflict_value; /* number of queens in conflict with this assignment */
} value_node;

typedef struct domain_node {
  int nelements; /* number of elements in constraint group */
  bool var_done; /* whether constraint group has element that has been set - put in vars_done */
} domain_node;
  
typedef struct board {
  int row[N]; /* representation of the board, indicate which column of the row-index is filled */
  
  /* Alldiff groups */
  domain_node col_domain[N]; /* column groups */
  domain_node diag1_domain[2*N - 1]; /* diagonal groups from top left to bottom right */
  domain_node diag2_domain[2*N - 1]; /* diagonal groups from bottom left to top right */
  
  int nconflicts; /* total number of conflicts */
} board;

typedef struct columns_remaining { /* used to keep track of assignable columns */
  int array_of_columns[N];
  int max_index;
} columns_remaining;

typedef struct rows_remaining { /* used to keep track of vars left */
  int array_of_rows[N];
  int max_index;
} rows_remaining;

void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

int cmpfunc (const void *a, const void *b)
{ /* sort array of pointers to board, with lowest conflict first*/
  const value_node *elem1 = a;
  const value_node *elem2 = b;
   
  
  return (elem1->conflict_value - elem2->conflict_value);
}


void print_board(board *boardp)
{ /* print board */
  srand(time(NULL));
  int i, j;
  int current;
  printf("Board:\n");
  for (i = 0; i < N; i++) {
    current = boardp->row[i];
    printf("|");
    if (current == -1) { /* if not initialized, print entire row empty */
      for (j = 0; j< N; j++) {
	printf(" |");
      }
    } else {
	for (j = 0; j < current; j++) {
	  printf(" |");
	}
	printf("Q|");
	for (j = current + 1; j < N; j++) {
	  printf(" |");
	}
    }
    printf("\n");
  }
}



board *board_init(void)
{ /* initialize board */
  board *boardp = malloc(sizeof(board));

  boardp->nconflicts = 0;
 
  int i;

  for (i = 0; i < (N*2 - 1); i++) {
    if (i < N) {
      boardp->row[i] = -1; /* indicates not yet assigned */
      boardp->col_domain[i].nelements = 0;
      boardp->col_domain[i].var_done = false;
    }
    boardp->diag1_domain[i].nelements = 0;
    boardp->diag1_domain[i].var_done = false;
    boardp->diag2_domain[i].nelements = 0;
    boardp->diag2_domain[i].var_done = false;
  }			  
  return boardp;
}

columns_remaining *colQ_init(void)
{ /* initialize queue of columns yet assigned */
  columns_remaining *colQp = malloc(sizeof(columns_remaining) * N);
  colQp->max_index = (N - 1);
  int i;
  for (i = 0; i < N; i++) {
    colQp->array_of_columns[i] = i;
  }
  
  return colQp;
}

rows_remaining *rowQ_init(void)
{ /* initialize queue of rows in vars_left */
  rows_remaining *rowQp = malloc(sizeof(rows_remaining) * N);
  rowQp->max_index = (N - 1);
  int i;
  for (i = 0; i < N; i++) {
    rowQp->array_of_rows[i] = i;
  }
  //shuffle(rowQp->array_of_rows, N);
  
  return rowQp;
}

value_node *value_node_array_init(void)
{ /* initialize array for value selection (min_conflict) */
  value_node *values_array = malloc(sizeof(value_node) * N);
  int i;
  for (i = 0; i < N; i++) {
    values_array[i].col_index = -1;
    values_array[i].conflict_value = -1;
  }
  return values_array;
}


/**************************************************
 *      GREEDY INITIALIZATION OF BOARD            *
 **************************************************/
int cmpfunc (const void *a, const void *b);

bool greedy_init(board *current, columns_remaining *colQp, int current_row)
{ /* greedy assignment, return true if valid assignment found, false if otherwise */
 
  bool greedy = false; /* if set to false, indicates need to switch from greedy  selection of columns to random */
  
  int i = 0; /* counts how many columns have been looked at */
  int index = rand() % (colQp->max_index + 1); /* random index */
  int col_index;
  

  while (i <= colQp->max_index) { /* while not yet visited all columns */
    if (index > colQp->max_index) index = 0; /* wrap arround */
    
    col_index = colQp->array_of_columns[index]; /* get column */
   
if (current->diag1_domain[current_row + col_index].nelements  + current->diag2_domain[current_row + (N - 1) - col_index].nelements == 0)  {/* if no conflicts */
      current->row[current_row] = col_index; /* assign queen */
      /* increase conflicts */
      current->col_domain[col_index].nelements++;  
      current->diag1_domain[current_row + col_index].nelements++;
      current->diag2_domain[current_row + (N - 1) - col_index].nelements++;
      greedy = true;

      /* decrease elements in column left */
      colQp->array_of_columns[index] = colQp->array_of_columns[colQp->max_index];
      colQp->max_index--;
      
      break;
    } else {
      index++;
    }
    i++;
  }

  return greedy; /* if false, no more zero conflict assignments available */
}

  
void random_init(board *current, columns_remaining *colQp, int current_row)
{ /* random assignment of queens */

  int col_index = colQp->array_of_columns[colQp->max_index]; /* columns remaining already jumbled up */

  /* increase conflicts */
  int current_conflicts = current->diag1_domain[current_row + col_index].nelements + current->diag2_domain[current_row + (N - 1) - col_index].nelements;
  current->row[current_row] = col_index;
  current->col_domain[col_index].nelements++;
  current->diag1_domain[current_row + col_index].nelements++;
  current->diag2_domain[current_row + (N - 1) - col_index].nelements++;
  current->nconflicts += 2 * current_conflicts;
  
  /* decrease elements in column left */;
  colQp->max_index--;
}
  
   

  
  

/**************************************************
 *       OPERATIONS FOR BACKTRACKING              *
 **************************************************/
void print_board(board *boardp);

bool backtrack(board *current, rows_remaining *vars_left, int assign_row, int assign_col, int conflict_value, board **solution)
{/* informed backtrack with min conflict heuristic */
  
  static bool solved = false; /* keeps track of whether board is solved */

  /* ASSIGNMENT */
  int old_assign_col_domain;
  int old_assign_conflicts;
  int old_assign_diag1_domain;
  int old_assign_diag2_domain;
  int old_assign_col;
  if ((assign_row >= 0) && (assign_col >= 0) && (conflict_value >= 0)) { /* if valid assignment given */
    
    /* keep track of old values to undo assignment */
     old_assign_col_domain = current->col_domain[assign_col].nelements;
     old_assign_conflicts = current->nconflicts;
     old_assign_diag1_domain =  current->diag1_domain[assign_row + assign_col].nelements;
     old_assign_diag2_domain = current->diag2_domain[assign_row + (N - 1) - assign_col].nelements;
     old_assign_col = current->row[assign_row]; /* log old column assignment */

     /* assign queen at (assign_row, assign_col) */
    current->row[assign_row] = assign_col;

    /* increase nelements in constraint domains */
    current->col_domain[assign_col].nelements++;
    current->diag1_domain[assign_row + assign_col].nelements++;
    current->diag2_domain[assign_row + (N - 1) - assign_col].nelements++;

    /* set constraint domain to var_done */
    current->col_domain[assign_col].var_done = true;
    current->diag1_domain[assign_row + assign_col].var_done = true;
    current->diag2_domain[assign_row + (N - 1) - assign_col].var_done = true;
 

    /* increase total conflict count */
    current->nconflicts += (2 * conflict_value);
    //log_info("nconflicts is %d", current->nconflicts);
  }
  
  /* CHECK IF SOLVED */
  if (current->nconflicts == 0) {
    if (N < 100) print_board(current);
    solved = true;
    log_info("solved");
    log_info("conflicts: %d", current->nconflicts);
    log_info("depth is %d", vars_left->max_index);
    **solution = *current;
    return true;
  }
  
  /* LIMIT DEPTH OF depth first search */
  //if (vars_left->max_index < (N - D)) log_info("backtracked");

  
  if (!(vars_left->max_index < (N - D))) { /* if depth limit not exceeded */

    /* SELECTING ROW TO REASSIGN FROM VARS_LEFT  */
    
 
    int current_conflicts;
    int i;
    int max_i;
    int row_index;
    int col_index;
    int max_conflicts = -1;
    int max_row;
    int max_col;

    /* select most conflicted row */
    for (i = 0; i <= vars_left->max_index; i++) {
      row_index = vars_left->array_of_rows[i];
      col_index = current->row[row_index];
    
      current_conflicts = current->col_domain[col_index].nelements + current->diag1_domain[row_index + col_index].nelements  + current->diag2_domain[row_index + (N - 1) - col_index].nelements - 3;
      if (current_conflicts > max_conflicts) {
	max_conflicts = current_conflicts;
	max_row = row_index;
	max_col = col_index;
	max_i = i;
      }
    }

    if (!(max_conflicts == -1)) {

      /* remove from vars left */
      int temp = vars_left->array_of_rows[max_i];
      vars_left->array_of_rows[max_i] = vars_left->array_of_rows[vars_left->max_index];
      vars_left->array_of_rows[vars_left->max_index] = temp; /* keep removed element in positions after max_index for undoing */
      vars_left->max_index--;
  
      /* DEASSIGNING CURRENT COLUMN ASSIGNMENT */
  
      /* keep track of old values to undo deassign */
      int old_deassign_conflicts = current->nconflicts;
      int old_deassign_col_domain = current->col_domain[max_col].nelements;
      int old_deassign_diag1_domain = current->diag1_domain[max_row + max_col].nelements;
      int old_deassign_diag2_domain = current->diag2_domain[max_row + (N - 1) - max_col].nelements;

      /* deassign */
      current->row[max_row] = -1;
  
      /*reduce conflicts */
      current->col_domain[max_col].nelements--;
      current->diag1_domain[max_row + max_col].nelements--;
      current->diag2_domain[max_row + (N - 1) - max_col].nelements--;

      /*reduce total conflict count */
      current->nconflicts -=  (2 * current->col_domain[max_col].nelements);
      current->nconflicts -= (2 *current->diag1_domain[max_row + max_col].nelements);
      current->nconflicts -= (2 * current->diag2_domain[max_row + (N - 1) - max_col].nelements);
  

      /* SELECTING COLUMN TO REASSIGN TO */
 
      value_node *conflict_array = value_node_array_init();
      int conflict_array_index = 0;
      for (i = 0; i < N; i++) {
	if (current->col_domain[i].var_done == false && current->diag1_domain[max_row + i].var_done == false && current->diag2_domain[max_row + (N - 1) - i].var_done == false) {  /* if does not conflict with vars_done */
	  current_conflicts =  current->col_domain[i].nelements + current->diag1_domain[max_row + i].nelements  + current->diag2_domain[max_row + (N - 1) - i].nelements;
	  conflict_array[conflict_array_index].col_index = i;
	  conflict_array[conflict_array_index].conflict_value = current_conflicts;
	  conflict_array_index++;
	}
      }
  
      qsort(conflict_array, conflict_array_index, sizeof(board *), cmpfunc); /* sort in ascending conflicts */

  
      /*for (i = 0; i < conflict_array_index; i++) {
	log_info("col index is %d", conflict_array[i].col_index);
	log_info("conflict value is %d",  conflict_array[i].conflict_value);
	}*/

      for (i = 0; i < conflict_array_index && solved == false; i++) {
          backtrack(current, vars_left, max_row, conflict_array[i].col_index, conflict_array[i].conflict_value, solution);
      }
      free(conflict_array);

    /* RESTORE PRE-DEASSIGN */
    current->row[max_row] = max_col;
    current->nconflicts = old_deassign_conflicts;
    current->col_domain[max_col].nelements =  old_deassign_col_domain;
    current->diag1_domain[max_row + max_col].nelements =  old_deassign_diag1_domain;
    current->diag2_domain[max_row + (N - 1) - max_col].nelements = old_deassign_diag2_domain;

    /* restore element in vars left */
    vars_left->max_index++;
    }
  }
    
  if ((assign_row >= 0) && (assign_col >= 0) && (conflict_value >=0)) { /* if assigned */
    /* RESTORE PRE-ASSIGN */
    current->nconflicts = old_assign_conflicts;
    current->col_domain[assign_col].var_done = false;
    current->diag1_domain[assign_row + assign_col].var_done = false;
    current->diag2_domain[assign_row + (N - 1) - assign_col].var_done = false;
    
    current->col_domain[assign_col].nelements = old_assign_col_domain; 
    current->diag1_domain[assign_row + assign_col].nelements = old_assign_diag1_domain;
    current->diag2_domain[assign_row + (N - 1) - assign_col].nelements = old_assign_diag2_domain;
    current->row[assign_row] = old_assign_col;
  }

  return solved ;
}




  
int main(int argc, char *argv[])
{
  srand(time(NULL));
  board *test = board_init();
  board *solution = malloc(sizeof(board));
  columns_remaining *colQp = colQ_init();
  rows_remaining *rowQp = rowQ_init();
  clock_t start, end;
  long double cpu_time_used;
  int i = 0;

  start = clock();

  /* initialize starting state */
  while ((i < N) && (greedy_init(test, colQp, i))) {
    i++;
  }
 
  while (i < N) {
    random_init(test, colQp, i);
    i++;
  }

  if (N < 100)  {
    log_info("initial board is: ");
    print_board(test);
  }

  log_info("solving for %d-queens", N);
  log_info("initial conflicts is %d", test->nconflicts);


  backtrack(test, rowQp, -1, -1, -1,  &solution);

  end = clock();
  cpu_time_used = ((long double)(end - start))/ CLOCKS_PER_SEC;
 
  log_info("time taken is %Lfs", cpu_time_used);

   free(test);
   free(solution);
   free(rowQp);
   free(colQp);
   
}
