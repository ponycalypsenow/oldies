#include "error.h"

/*

  list

*/

template <class T>
class LIST{
public:
	T *head,*tail,*actual;

	LIST(void){
		head=tail=actual=0;
	}

	~LIST(void){
		for(Rewind();tail;Delete(tail));
	}

	void Add(T *t){
		if(!t) error("out of memory");
		if(!head){
			head=t;
			t->next=t->previous=0;
		}else{
			tail->next=t;
			t->previous=tail;
			t->next=0;
		}
		tail=t;
	};
	
	void Delete(T *t){
		if(t){
			if(t==head) head=t->next;
			else t->previous->next=t->next;
			if(t==tail) tail=t->previous;
			else t->next->previous=t->previous;
			delete t;
		}
	}
	
	void Rewind(void){
		actual=head;
	}
	
	void Next(void){
		if(actual)
			actual=actual->next;
	}

	void Previous(void){
		if(actual)
			actual=actual->previous;
	}
};
