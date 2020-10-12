#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char* string)
{
	if (free_ == NULL)
		assert(0);
	WP *new_wp;
	new_wp = free_;
	free_ = free_->next;
	strcpy(new_wp->str,string);
	bool success;
	new_wp->new_val = expr(string,&success);
	if(!success) assert(0);
	printf("value : 0x%x\n",new_wp->new_val);

	WP* p;
	p = head;
	if(p == NULL)
	{
		head = new_wp;
		p = head;
	}
	else 
	{
		while(p->next != NULL)
			p = p->next;
		p->next = new_wp;
	}
	new_wp->next = NULL;
	return new_wp;
	
}

void free_wp(WP *wp)
{
	WP *f,*p;
	p = free_;
	if(p==NULL)
	{
		free_=wp;
		p=free_;
	}	
	else{
		while(p->next!=NULL)
			p= p->next;
		p->next=wp;
		
	}
	f=head;
	if(head == NULL)
	{
		assert(0);
	}
	if(head->NO==wp->NO)
	{
		head=head->next;
	}
	else
	{
		while(f->next != NULL&&f->next->NO!=wp->NO)
			f =f->next;
		if( f->next->NO == wp->NO)
			f->next = f->next->next;
		else assert(0);
	}
	printf("Free %d\n",wp->NO);
	wp->next=NULL;
	wp->old_val=0;
	wp->b=0;
	wp->new_val=0;
	wp->str[0]='\0';


}

bool check_wp()
{
	WP* f;
	f=head;
	bool key;
	bool suc;
	while(f!=NULL)
 	{
		int tem = expr(f->str,&suc);
		if(!suc) assert(0);
		if(tem!=f->new_val)
		{
			key= 0;
			if(f->b)
			{
				printf("Hit breakpoint %d at 0x%08x\n",f->b,cpu.eip);
				f = f->next;
				continue;
			}
			f->old_val=f->new_val;
			f->new_val=tem;
			printf("Watchpoint %d:%s\n",f->NO,f->str);
			printf("Old value:%d",f->old_val);
			printf("New value:%d",f->new_val);
		}
		f=f->next;
		
	
	}
	return key;

	
}

void delete_wp(int num)
{
	WP* f;
	f=&wp_pool[num];
	free_wp(f);
}

void info_wp()
{
	WP* f;
	f = head;
	while(f!=NULL)
	{
		printf("Watchpoint %d:  %s = 0x%x\n",f->NO,f->str,f->new_val);
		f=f->next;
	}
}

