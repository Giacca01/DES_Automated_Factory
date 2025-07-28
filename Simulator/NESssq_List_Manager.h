#ifndef __LIST_MANAGER__
#define __LIST_MANAGER__


void enqueue(nodePtr new_node, dll * curr_queue);
nodePtr dequeue(dll * curr_queue);
void destroy_list(nodePtr p);

#endif