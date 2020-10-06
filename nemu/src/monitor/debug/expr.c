#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
enum { EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI};
enum {AX,CX,DX,BX,SP,BP,SI,DI};
enum {AL,CL,DL,BL,AH,CH,DH,BH};
enum {
	NOTYPE = 256,EQ,UEQ,Logical_AND,Logical_OR,Logical_NOT,REGISTER,VARIES,NUMBER,_16NUMBER,HEX,FU,POINT

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
	{"\\b[0-9]+\\b",NUMBER,1},                             //number
	{"\\b0[xX][0-9a-fA-F]+\\b",_16NUMBER,1},                        //16 number
	{"[%$][eE]?(ax|bx|cx|dx|bp|si|di|sp)",REGISTER,1},       //register
	{"\\b[a-zA-Z_0-9]",VARIES,1},                               //varies
	{" ",	NOTYPE,1},				// spaces
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
bool check_parentheses(int p, int q)
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
		for(j=i;j>=i;j--)
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
	int value=0;
	if(p>q) Assert(1,"?");
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
		if(tokens[p].type == REGISTER)
		{
			if(strlen(tokens[p].str) == 3)
			{
				int i=0;
				for(i = EAX;i <= EDI;i++)
				{
					if(strcmp(tokens[p].str,regsl[i]) == 0)
						break;
				}
				if(i > EDI)
				{
					if(strcmp(tokens[p].str,"eip")==0)
						value = cpu.eip;
					else Assert(1,"?");
				}
				else value = reg_l(i);
			}
			else if(strlen(tokens[p].str)==2)
			{
				if(tokens[p].str[1] == 'x' || tokens[p].str[1] == 'p' || tokens[p].str[1] == 'i' )
				{
					int i=0;
					for(i=AX;i <= DI;i++)
						if (strcmp(tokens[p].str,regsw[i]) == 0)
							break;
					value = reg_w(i);
				}
				if(tokens[p].str[1] == 'l' || tokens[p].str[1] == 'h')
				{
					int i=0;
					for(i = AL;i <= BH; i++)
						if(strcmp(tokens[p].str,regsb[i]) == 0)
							break;
					value = reg_b(i);
				}
				else Assert(1,"?");
			}
			return value;
		}
		if(tokens[p].type == VARIES)
		{
			printf("%s",tokens[p].str);
		}
	}
	else if(check_parentheses(p,q) != 1)
	{
		//printf("shizhe?\n");
		int op = find_dominant_op(p,q);
		printf("%d %d",p,q);
		//printf("op is %d\n",tokens[op].type);
		if(p == op||tokens[op].type == FU || tokens[op].type == POINT ||tokens[op].type == '!')
		{
			int val = eval(p+1,q);
			switch(tokens[op].type)
			{
				case FU: return -val;
				case POINT: return swaddr_read(val,4);
				case '!':return !val;
				default : Assert(1,"?");
			}
			
		}
		return 0;
		int val1 = eval(p,op-1);
		int val2 = eval(op+1,q);
		switch(tokens[op].type)
		{
			case '+': return val1+val2;
			case '*': return val1*val2;
			case '-': return val1-val2;
			case '/': return val1/val2;
			case EQ: return val1 == val2;
			case UEQ: return val1!=val2;
			case Logical_AND: return val1&&val2;
			case Logical_OR : return val1||val2;
		}
	}
	else 
	{
		printf("tuoke\n");
		return eval(p+1,q-1);
		
	}
	printf("zhecuole?\n");
	Assert(1,"?");
	return 0;


}


uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	int i;

	for(i=0;i<nr_token;i++)
	{	
		bool a = (i == 0 || ((tokens[i-1].type != NUMBER )&&(tokens[i-1].type != _16NUMBER) &&(tokens[i-1].type != REGISTER) && (tokens[i-1].type != VARIES)&&(tokens[i-1].type != ')')));
		if(tokens[i].type == '*'&& a)
		{
			tokens[i].type = POINT;
			tokens[i].priority = 6;
		}
		if(tokens[i].type == '-'&&a )
		{
			tokens[i].type = FU;
			tokens[i].priority = 6;

		}
	}

	/* TODO: Insert codes to evaluate the expression. */
	//panic("please implement me");
	*success = true;
	return eval(0,nr_token-1);
}

