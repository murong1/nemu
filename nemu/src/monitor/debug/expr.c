#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256,EQ,UEQ,Logical_AND,Logical_OR,Logical_NOT,REGISTER,VARIES,NUMBER,_16NUMBER,HEX

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
	
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"[0-9]",NUMBER,1},                             //number
	{"0[xX][0-9a-fA-F]",_16NUMBER,1},                        //16 number
	{"[%$][eE]?(ax|bx|cx|dx|bp|si|di|sp)",REGISTER,1},       //register
	{"[a-zA-Z_0-9]",VARIES,1},                               //varies
	{" +",	NOTYPE,1},				// spaces
	{"	",NOTYPE,1},                         //taps
	{"\\|\\|",Logical_OR,2},                   //or
	{"&&",Logical_AND,3},                         //and
	{"==",EQ,4},                                    //equal
	{"!=",UEQ,4},                                 // not equal
	{"\\+", '+',5},					// plus
	{"-",'-',5},                                          //sub
	{"\\*",'*',6},                        //mulity
	{"/",'/',6},                         //div
	{"!",Logical_NOT,7},                      //not
	{"\\(",'(',8},
	{"\\)",')',8},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				if(substr_len<33)
				{
					switch(rules[i].token_type) {
						case 256 : break;
						case REGISTER : 
							tokens[nr_token].type = rules[i].token_type;
							strncpy(tokens[nr_token].str,&e[position-substr_len+1],substr_len);
							tokens[nr_token].str[substr_len]='\0';
							tokens[nr_token].priority = rules[i].priority;
							break;
						default: 
							tokens[nr_token].type = rules[i].token_type;
							tokens[nr_token].priority = rules[i].priority;
							strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
							tokens[nr_token].str[substr_len]='\0';
					}
					nr_token++;
					break;
				}
				else {
					printf("Buffer overflow");
					return 0;
				}
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}
bool check_parebtheses(int p, int q)
{
	if(tokens[p].type != '(' || tokens[q].type != ')')   return false;
	int i=0,mark=0;
	for(i=p;i<=q;i++)
	{	
		if(tokens[i].type == '(')  mark++;
		else 
			if(tokens[i].type == ')' ) mark--;
		if(mark == 0 && i<q ) return false ;
	}
	if( mark != 0) return false;

	return true;
}

int find_dominant_op(int p ,int q)
{
	int i = 0,j = 0;
	int minpriority = 10;
 	int oper = p;
	for(i=p;i<=q;i++)
	{
		if(tokens[i].priority == 1) continue;
		
		int num = 0;
		bool flag = 1;
		for(j=i-1;j>=i;j--)
		{
			if(tokens[j].type == '('&&!num)
			{
				flag = 0;
				break;
			}
			if(tokens[j].type == '(')  num++;
			if(tokens[j].type == ')')  num--;
		}
		if (!flag)  continue;
		if(tokens[i].priority <= minpriority)
		{
			minpriority = tokens[i].priority;
			oper = i;	
		}

	}	
	return oper;
}

int eval(int p,int q)
{
	int value;
	if(p>q) assert(0);
	if(p==q)
	{
		if(tokens[p].type == NUMBER)
		{
			sscanf(tokens[p].str,"%d",&value);
			return value;
		}
		if(tokens[p].type == _16NUMBER)
		{
			sscanf(tokens[p].str,"%x",&value);
			return value;
		}
	}





	return 0;


}


uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

