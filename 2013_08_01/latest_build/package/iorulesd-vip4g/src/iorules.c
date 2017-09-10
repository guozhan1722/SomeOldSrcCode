#include <stdio.h>
#include <linux/version.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <pthread.h>
#include <time.h>
#include <syslog.h>
#include <signal.h>



#include "uci.h"

#define	LOGOPTS		(LOG_PERROR|LOG_PID|LOG_LPR|LOG_ERR)

#define PRINT_DEBUG_RULES_INFO    1

//#define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements

#if DEBUG_PRINT_ENABLED
#define DEBUG printf
#else
#define DEBUG(format, args...) ((void)0)
#endif


/*
 *	Signal handling.
 */
struct sigaction	sact;

/* io config file  */
static char *ioports_confile="/etc/config/ioports";
static char *iorules_confile="/etc/config/iorules";

//struct uci_context *ctx = NULL;

#define MAX_RULES 100
int g_custom_rules_count = 0;
int g_custom_rules_timer_only_count = 0;

pthread_t tid[MAX_RULES];
void      *tret[MAX_RULES];


char InputStatus[4];

char iorule_define[8];

struct io_rules *rule_head;

struct io_rules {
    char rule_mode[32];
    char timercount[32];
    char input1[8];
    char input2[8];
    char input3[8];
    char input4[8];
    char output1[8];
    char output2[8];
    char output3[8];
    char output4[8];

    int index;
    struct io_rules *next;
};

struct io_rules *rule_head;


/**********************************************************************************
   Function:    int main(void) 
   Input:       void
   Output:      None
   Return:      int   
   Note:	read database and config com port and according to selected IP protocol 
        to process.        
**********************************************************************************/

static bool get_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_element *e= NULL;
    struct uci_ptr ptr;
    struct uci_option *p_option = NULL;

    //printf("%s option_name: %s\n",__func__,option_name);
    strcpy(value, "");

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name)+ 3 ); /* "." and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {

        e = ptr.last;

        if ( (ptr.flags & UCI_LOOKUP_COMPLETE) &&
             (e->type == UCI_TYPE_OPTION) )
            {
            p_option = ptr.o;
            strcpy(value, p_option->v.string);
            //printf("%s find %s\n",__func__, value);
            free(tuple);
            return true;
            }
        }

    free(tuple);

    return false;
}


/**********************************************************************************
   Function:  bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

static bool set_option_by_section_name(struct uci_context *ctx, char *package_name, char *section_name, char *option_name, char *value)
{
    char *tuple;
    struct uci_ptr ptr;

    tuple = malloc( strlen(package_name) + strlen(section_name) + strlen(option_name) + strlen(value) + 4 );    /* "." "=" and trailing \0 */

    if (!tuple) return false;

    strcpy(tuple, package_name);
    strcat(tuple, ".");
    strcat(tuple, section_name);
    strcat(tuple, ".");
    strcat(tuple, option_name);
    strcat(tuple, "=");
    strcat(tuple, value);

    if (uci_lookup_ptr(ctx, &ptr, tuple, true) == UCI_OK)
        {
        if ( UCI_OK == uci_set(ctx, &ptr) )
            {
            uci_save(ctx, ptr.p);
         //   uci_commit(ctx, &ptr.p, false);
            }

        free(tuple);
        return true;
        }

    free(tuple);

    return false;
}


/*
 * Debug function 
 * 
 *  
 */
void debug_struct(struct io_rules *iorules_head)
{
    /*here is for debug*/
    struct io_rules *p_rule;

    DEBUG("iorulesd debug [%s]\n", __FUNCTION__);

    p_rule=iorules_head;

    while (p_rule->next != NULL) {

        DEBUG(" rule_mode: %s",p_rule->rule_mode);
        DEBUG(" timercount: %s",p_rule->timercount);
        DEBUG(" input1: %s ",p_rule->input1);
        DEBUG(" input2: %s ",p_rule->input2);
        DEBUG(" input3: %s ",p_rule->input3);
        DEBUG(" input4: %s ",p_rule->input4);
        DEBUG(" output1: %s ",p_rule->output1);
        DEBUG(" output2: %s ",p_rule->output2);
        DEBUG(" output3: %s ",p_rule->output3);
        DEBUG(" output4: %s \n",p_rule->output4);
        p_rule=p_rule->next;
    }

}


/*
 *  Read all user custom ioport rules
 * 
 *  
 */
static int read_ioports_config(struct io_rules *iorules_head)
{
    int i = 0;    
    struct uci_context *ctx = NULL;
    struct io_rules *p_rule,*tmp_rule, *p_next;
    char section[32], rBuff[32];
    char *src;
    int last=0;

    p_rule = iorules_head;

    p_rule->index = 0;

    for(i=0;i<MAX_RULES;i++) {

        last=0;

        sprintf(section, "@rule[%d]", i);

        bzero(p_rule->rule_mode, sizeof(p_rule->rule_mode));
        bzero(p_rule->timercount, sizeof(p_rule->timercount));
        bzero(p_rule->input1, sizeof(p_rule->input1));
        bzero(p_rule->input1, sizeof(p_rule->input2));
        bzero(p_rule->input1, sizeof(p_rule->input3));
        bzero(p_rule->input1, sizeof(p_rule->input4));
        bzero(p_rule->output1, sizeof(p_rule->output1));
        bzero(p_rule->output1, sizeof(p_rule->output2));
        bzero(p_rule->output1, sizeof(p_rule->output3));
        bzero(p_rule->output1, sizeof(p_rule->output4));
        
        ///////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }    
        if( get_option_by_section_name(ctx, iorules_confile, section, "rule_mode", rBuff) == false ){
            //printf("can not get rule_mode from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{      
            if (strcmp(rBuff, "")!=0) { 
                DEBUG("iorulesd debug [%s] rule_mode=%s\n", __FUNCTION__, rBuff);             
                strcpy(p_rule->rule_mode, rBuff);
                last++;
            } 
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "timercount", rBuff) == false ){
            //printf("can not get timercount from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {               
                DEBUG("iorulesd debug [%s] timercount=%s\n", __FUNCTION__, rBuff);               
                strcpy(p_rule->timercount, rBuff);
                last++;
            }        
                
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "input1", rBuff) == false ){
            //printf("can not get input1 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {              
                DEBUG("iorulesd debug [%s] input1=%s\n", __FUNCTION__, rBuff);            
                strcpy(p_rule->input1, rBuff);
                last++;
            }   
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "input2", rBuff) == false ){
            //printf("can not get input2 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {               
                DEBUG("iorulesd debug [%s] input2=%s\n", __FUNCTION__, rBuff);           
                strcpy(p_rule->input2, rBuff);
                last++;
            }  
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "input3", rBuff) == false ){
            //printf("can not get input3 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {                
                DEBUG("iorulesd debug [%s] input3=%s\n", __FUNCTION__, rBuff);           
                strcpy(p_rule->input3, rBuff);
                last++;
            }   
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "input4", rBuff) == false ){
            //printf("can not get input4 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {
                DEBUG("iorulesd debug [%s] input4=%s\n", __FUNCTION__, rBuff);
                strcpy(p_rule->input4, rBuff);
                last++;
            }  
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "output1", rBuff) == false ){
            //printf("can not get output1 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {
                DEBUG("iorulesd debug [%s] output1=%s\n", __FUNCTION__, rBuff);
                strcpy(p_rule->output1, rBuff);
                last++;
            }   
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "output2", rBuff) == false ){
            //printf("can not get output2 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {
                DEBUG("iorulesd debug [%s] output2=%s\n", __FUNCTION__, rBuff);
                strcpy(p_rule->output2, rBuff);
                last++;
            }   
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "output3", rBuff) == false ){
            //printf("can not get output3 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {
                DEBUG("iorulesd debug [%s] output3=%s\n", __FUNCTION__, rBuff);
                strcpy(p_rule->output3, rBuff);
                last++;
            }   
        }
        if (ctx)
        {
            uci_free_context(ctx);
            ctx=NULL;
         }
        ////////////////////////////////////////////////////////////
        ctx = uci_alloc_context();  
        if (!ctx) 
        {
            fprintf(stderr, "uci_alloc_context Out of memory.\n");
            return -1;
        }
        if( get_option_by_section_name(ctx, iorules_confile, section, "output4", rBuff) == false ){
            //printf("can not get output4 from %s\n", iorules_confile);
            if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
            }
            //return -1;
        }else{        
            if (strcmp(rBuff, "")!=0) {
                DEBUG("iorulesd debug [%s] output4=%s\n", __FUNCTION__, rBuff);
                strcpy(p_rule->output4, rBuff);
                last++;
            } 
        } 
        if (ctx)
          {
            uci_free_context(ctx);
            ctx=NULL;
          }
        ///////////////////////////////////////////////////////////
        
        p_rule->index = i+1;

        if(last==0) {
            p_rule->next = NULL;
            break ;
        }else {
            //complete_rule(p_rule);
            tmp_rule = (struct io_rules *) malloc(sizeof(struct io_rules));
            p_rule->next =tmp_rule;

            p_rule=tmp_rule;
        }

    } //end for loop

    g_custom_rules_count = i;

    DEBUG("iorulesd debug [%s] >>>>>>>>>>>>Total Number of Custom Rules: %d\n", __FUNCTION__, g_custom_rules_count);

#ifdef PRINT_DEBUG_RULES_INFO
    debug_struct(iorules_head);
#endif

}



/*
 *  Read input status
 * 
 *  
 */
void get_input_status()
{
    FILE* f;
    char tmp_buf[10];
    

    if ((f = fopen("/sys/class/button/INPUT1/status", "r"))) 
    {
        if(fgets(tmp_buf, 10, f)) 
        {
            InputStatus[0]=tmp_buf[0];
        }
    }
    if(f) fclose(f);
    //printf("INPUT STATUS: %c\n", InputStatus[0]);
    if ((f = fopen("/sys/class/button/INPUT2/status", "r"))) 
    {
        if(fgets(tmp_buf, 10, f)) 
        {
            InputStatus[1]=tmp_buf[0];
        }
    }
    if(f) fclose(f);
    if ((f = fopen("/sys/class/button/INPUT3/status", "r"))) 
    {
        if(fgets(tmp_buf, 10, f)) 
        {
            InputStatus[2]=tmp_buf[0];
        }
    }
    if(f) fclose(f);
    if ((f = fopen("/sys/class/button/INPUT4/status", "r"))) 
    {
        if(fgets(tmp_buf, 10, f)) 
        {
            InputStatus[3]=tmp_buf[0];
        }
    }
    if(f) fclose(f);

    DEBUG("INPUT STATUS: %c %c %c %c\n",InputStatus[0],InputStatus[1],InputStatus[2],InputStatus[3]);

} //end function



/*
 *  Do action to output port according to port and value
 * 
 *  
 */
void do_output_action(int port, int value)
{
    FILE* f;
    char tmpbuf[64];
    char ioport[16];
    struct uci_context *ctx = NULL;

    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx) {
        DEBUG("Out of memory\n");
        //return(false);
    }

    sprintf(tmpbuf, "/sys/class/leds/OUTPUT%d/brightness", port); 
    sprintf(ioport, "output%d", port);    

    if ((f = fopen(tmpbuf, "w"))) 
    {        
        if (value == 1) {
            //printf("open %s, value %d\n", tmpbuf, value);
            fputs ("1", f );
            set_option_by_section_name(ctx, ioports_confile, "output", ioport, "1");
        }else if (value == 0){
            //printf("open %s, value %d\n", tmpbuf, value);
            fputs ("0", f );
            set_option_by_section_name(ctx, ioports_confile, "output", ioport, "0");
        }
    }
    
    if (ctx) {
        uci_free_context(ctx);
        ctx=NULL;
    }
    if(f) fclose(f);
} //end function



/* 
 * I/O Default Rule: pass-through
 *    
 *   input1 --> output1 
 *   input2 --> output2 
 *   input3 --> output3 
 *   input4 --> output4 
 *  
 */
void single_port_pass_through()
{
        // one input to one output rule

        if (InputStatus[0] == '1' ) {
            do_output_action(1, 1);
        } else if (InputStatus[1] == '0' ) {
            do_output_action(1, 0);
        }

        if (InputStatus[1] == '1' ) {
            do_output_action(2, 1);
        } else if (InputStatus[1] == '0' ) {
            do_output_action(2, 0);
        }

        if (InputStatus[2] == '1' ) {
            do_output_action(3, 1);
        } else if (InputStatus[2] == '0' ) {
            do_output_action(3, 0);
        }

        if (InputStatus[3] == '1' ) {
            do_output_action(4, 1);
        } else if (InputStatus[3] == '0' ) {
            do_output_action(4, 0);
        }
}


/*
 *  
 * 
 *  
 */
void *do_timer_thread(void *vptr)
{
    int i = 0;
    struct io_rules *p_rule = (struct io_rules *)vptr;

    
    DEBUG("iorulesd debug [%s] threadid=%x, timercount: %d\n", __FUNCTION__, (unsigned int)pthread_self(), atoi(p_rule->timercount));
    
    for (;;) {

        sleep(atoi(p_rule->timercount));

        if ( strcmp(p_rule->output1, "n/a") != 0 ){
            do_output_action(1, atoi(p_rule->output1));
        }
        if ( strcmp(p_rule->output2, "n/a") != 0 ){
            do_output_action(2, atoi(p_rule->output2));
        }
        if ( strcmp(p_rule->output3, "n/a") != 0 ){
            do_output_action(3, atoi(p_rule->output3));
        }
        if ( strcmp(p_rule->output4, "n/a") != 0 ){
            do_output_action(4, atoi(p_rule->output4));
        }

        sleep(1);

        if ( strcmp(p_rule->output1, "n/a") != 0 ){
            do_output_action(1, !atoi(p_rule->output1));
        }
        if ( strcmp(p_rule->output2, "n/a") != 0 ){
            do_output_action(2, !atoi(p_rule->output2));
        }
        if ( strcmp(p_rule->output3, "n/a") != 0 ){
            do_output_action(3, !atoi(p_rule->output3));
        }
        if ( strcmp(p_rule->output4, "n/a") != 0 ){
            do_output_action(4, !atoi(p_rule->output4));
        }    
    } //for loop

}



/*
 *  do action (timer trigger only) to output
 * 
 *  
 */
void do_custom_rules_timer_only(struct io_rules *iorules_head)
{
    struct io_rules *p_rule;
    int i = 0;

    DEBUG("iorulesd debug [%s]\n", __FUNCTION__);

    p_rule=iorules_head;

    while (p_rule->next != NULL) {

        DEBUG(" rule_mode: %s",p_rule->rule_mode);
        DEBUG(" timercount: %d",atoi(p_rule->timercount));        
        DEBUG(" input1: %d ",atoi(p_rule->input1));        
        DEBUG(" input2: %d ",atoi(p_rule->input2));        
        DEBUG(" input3: %d ",atoi(p_rule->input3));        
        DEBUG(" input4: %d ",atoi(p_rule->input4));        
        DEBUG(" output1: %d ",atoi(p_rule->output1));        
        DEBUG(" output2: %d ",atoi(p_rule->output2));        
        DEBUG(" output3: %d ",atoi(p_rule->output3));        
        DEBUG(" output4: %d \n",atoi(p_rule->output4));
            
        if ( strcmp(p_rule->rule_mode, "TIMER_ONLY") == 0 ){
                DEBUG("iorulesd debug [%s] do TIMER_ONLY\n", __FUNCTION__);
                pthread_create(&tid[i], NULL, &do_timer_thread, p_rule);
                i++;
        } 

        p_rule=p_rule->next;
    }
    g_custom_rules_timer_only_count = i;

    DEBUG("iorulesd debug [%s] >>>>>>>>>>>>Total Number of Timer Only Custom Rules: %d\n", __FUNCTION__, g_custom_rules_timer_only_count);
}



/*
 *  For Debug
 * 
 *  
 */
void print_info(struct io_rules *p_rule)
{
    DEBUG(" rule_mode: %s",p_rule->rule_mode);
    DEBUG(" timercount: %d",atoi(p_rule->timercount));        
    //DEBUG(" input1: %d ",atoi(p_rule->input1));        
    //DEBUG(" input2: %d ",atoi(p_rule->input2));        
    //DEBUG(" input3: %d ",atoi(p_rule->input3));        
    //DEBUG(" input4: %d ",atoi(p_rule->input4));        
    //DEBUG(" output1: %d ",atoi(p_rule->output1));        
    //DEBUG(" output2: %d ",atoi(p_rule->output2));        
    //DEBUG(" output3: %d ",atoi(p_rule->output3));        
    //DEBUG(" output4: %d \n",atoi(p_rule->output4));
    DEBUG(" input: %d %d %d %d",atoi(p_rule->input1),atoi(p_rule->input2),atoi(p_rule->input3),atoi(p_rule->input4));
    DEBUG(" output: %d %d %d %d\n",atoi(p_rule->output1),atoi(p_rule->output2),atoi(p_rule->output3),atoi(p_rule->output4));

}



/*
 * do custom io rule: 
 * INPUT_ONLY or INPUT_TIMER
 *  
 */
void do_custom_rules_input(struct io_rules *iorules_head)
{
    struct io_rules *p_rule;
    int x = 1;

    DEBUG("iorulesd debug [%s]\n", __FUNCTION__);

    p_rule=iorules_head;

    while (p_rule->next != NULL) {

        //print_info(p_rule);
       
        if (( strcmp(p_rule->rule_mode, "INPUT_ONLY") == 0 ) || ( strcmp(p_rule->rule_mode, "INPUT_TIMER") == 0 )){
            DEBUG("iorulesd debug [%s] do INPUT_TIMER or INPUT_ONLY\n", __FUNCTION__);
            print_info(p_rule);
            // do something here
            
            if ( (InputStatus[0] == atoi(p_rule->input1)) && (InputStatus[1] == atoi(p_rule->input2)) && (InputStatus[2] == atoi(p_rule->input3)) && (InputStatus[3] == atoi(p_rule->input4)) ){
                if (strcmp(p_rule->rule_mode, "INPUT_TIMER") == 0 ){
                    // INPUT_TIMER: consider input with timer
                    while(x <= atoi(p_rule->timercount)) {
                        sleep(1);
                        x++;
                        get_input_status();
                        
                        if ( (InputStatus[0] == atoi(p_rule->input1)) && (InputStatus[1] == atoi(p_rule->input2)) && (InputStatus[2] == atoi(p_rule->input3)) && (InputStatus[3] == atoi(p_rule->input4)) ){
                            fprintf(stderr, "iorulesd debug [%s] do INPUT_TIMER, however Event happen while the timer elapsing.(%c %c %c %c)\n", __FUNCTION__,InputStatus[0],InputStatus[1],InputStatus[2],InputStatus[3]);
                            return;
                        }
                    }
                    // output_action
                    if ( strcmp(p_rule->output1, "n/a") != 0 ){
                        do_output_action(1, atoi(p_rule->output1));
                    }
                    if ( strcmp(p_rule->output2, "n/a") != 0 ){
                        do_output_action(2, atoi(p_rule->output2));
                    }
                    if ( strcmp(p_rule->output3, "n/a") != 0 ){
                        do_output_action(3, atoi(p_rule->output3));
                    }
                    if ( strcmp(p_rule->output4, "n/a") != 0 ){
                        do_output_action(4, atoi(p_rule->output4));
                    }

                } else{
                    // INPUT_ONLY: consider input only
                    // output_action
                    if ( strcmp(p_rule->output1, "n/a") != 0 ){
                        do_output_action(1, atoi(p_rule->output1));
                    }
                    if ( strcmp(p_rule->output2, "n/a") != 0 ){
                        do_output_action(2, atoi(p_rule->output2));
                    }
                    if ( strcmp(p_rule->output3, "n/a") != 0 ){
                        do_output_action(3, atoi(p_rule->output3));
                    }
                    if ( strcmp(p_rule->output4, "n/a") != 0 ){
                        do_output_action(4, atoi(p_rule->output4));
                    }

                }

            } // if input match with input rule

        } // if INPUT_ONLY or INPUT_TIMER


        p_rule=p_rule->next;
    }
}



/*
 *  Read define io rules, 
 *            
 *   option:  Disable
 *   option:  Default Rules (pass through)
 *   option:  User Custom Rules 
 *  
 */
int read_iorule_define()
{
    struct uci_context *ctx = NULL;
    char rBuff[8];

    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
     }
    ////////////////////////////////////////////////////////////
    ctx = uci_alloc_context();  
    if (!ctx) 
    {
        fprintf(stderr, "uci_alloc_context Out of memory.\n");
        return -1;
    }
    if( get_option_by_section_name(ctx, iorules_confile, "rule_define", "rule_define", rBuff) == false ){
        fprintf(stderr, "iorulesd debug [%s] can not get rule_define from %s\n", __FUNCTION__, iorules_confile);
        if (ctx) {
                uci_free_context(ctx);
                ctx=NULL;
        }
        return -1;
    }else{       
        if (strcmp(rBuff, "")!=0) {
            DEBUG("iorulesd debug [%s] rule_define=%s\n", __FUNCTION__, rBuff);
            strcpy(iorule_define, rBuff);
        }                
    }
    /////////////////////////////////////////////////////////// 
    if (ctx)
      {
        uci_free_context(ctx);
        ctx=NULL;
      }
    ///////////////////////////////////////////////////////////
    return 0;
}



/*
 *  Free dynamic allocated memory 
 *            
 *  
 */
void free_iorule(struct io_rules *iorules_head)
{
    struct io_rules *p_rule, *tmp_rule;
    
    p_rule = iorules_head;


    while (p_rule != NULL) {
        tmp_rule = p_rule;
        p_rule = p_rule->next;
        free(tmp_rule);
    }
    iorules_head = NULL;
   
}


/*
 *  Signal handler: do cleanup job
 *  
 *  
 *  
 */
void sighandler(int signal)
{
    int i = 0;
    static int reentrant_flag = 0;


    printf("\n\nGot signal %d!\n", signal);
    printf("iorulesd [%s] Cleaning up...\n", __FUNCTION__);
    // Do Cleanup job here
    //if (ctx) {
    //    uci_free_context(ctx);
    //    ctx=NULL;
    //}
    //
    for(i=0; i < g_custom_rules_timer_only_count; i++) {
        pthread_join(tid[i], &tret[i]);
        printf("iorulesd [%s] pthread_join done\n", __FUNCTION__);
    }

    printf("iorulesd [%s] reentrant_flag=%d\n", __FUNCTION__, reentrant_flag);     
    if (reentrant_flag != 1) {
        if(rule_head) {
            free_iorule(rule_head);
            
        } 
        reentrant_flag = 1;
        printf("iorulesd [%s] free_iorule done flag=%d\n", __FUNCTION__, reentrant_flag);      
    } 

    printf("iorulesd [%s] Cleaning up done\n", __FUNCTION__);
	
	exit(1);
}



/*
 *  Main function 
 *            
 *
 *  
 */

int main(void)
{
    
    int i = 0;

    DEBUG("Hello iorulesd for VIP4G.\n");
    printf("Hello iorulesd for VIP4G.\n");

    rule_head = (struct io_rules *)malloc(sizeof(struct io_rules));

    read_iorule_define();

    if ( strcmp(iorule_define, "A") == 0 ) {
        DEBUG("iorulesd debug [%s] rule_define=%s exit, because user disable this function.\n", __FUNCTION__, iorule_define);
        syslog(LOGOPTS, "iorulesd debug [%s] rule_define=%s exit, because user disable this function.\n", __FUNCTION__, iorule_define);
        exit(-1);         
    }
    if ( strcmp(iorule_define, "B") == 0 ) {
        DEBUG("iorulesd debug [%s] rule_define=%s default pass-through rule.\n", __FUNCTION__, iorule_define);
        syslog(LOGOPTS, "iorulesd debug [%s] rule_define=%s default pass-through rule.\n", __FUNCTION__, iorule_define);     
    }    

    if ( strcmp(iorule_define, "C") == 0 ) {
        DEBUG("iorulesd debug [%s] rule_define=%s User Custom I/O Rules\n", __FUNCTION__, iorule_define);
        syslog(LOGOPTS, "iorulesd debug [%s] rule_define=%s User Custom I/O Rules\n", __FUNCTION__, iorule_define);
        read_ioports_config(rule_head);
        do_custom_rules_timer_only(rule_head);        
    }

   	/*
	 *	Set the signal handler to do cleanup job .
	 */
	sact.sa_handler = sighandler;
	sigaction(SIGHUP, &sact, NULL);
	sigaction(SIGINT, &sact, NULL);
	sigaction(SIGQUIT, &sact, NULL);	
	sigaction(SIGTERM, &sact, NULL);
    sigaction(SIGKILL, &sact, NULL);
    

    ////////////////////////////////////////////////////////////////
    for(;;) {
        get_input_status();

        if ( strcmp(iorule_define, "B") == 0 ) {
            // Default Rule: one input to one output rule
            single_port_pass_through(); 
        }
       
        if ( strcmp(iorule_define, "C") == 0 ) {
            // User Custom Rule: INPUT_ONLY or INPUT_TIMER
            do_custom_rules_input(rule_head);        
        }

        sleep(3);
    }
    ////////////////////////////////////////////////////////////////
   
    if(rule_head) {
        free_iorule(rule_head);
    }

    for(i=0; i < g_custom_rules_timer_only_count; i++) {
        pthread_join(tid[i], &tret[i]);
    }
    

    return 0;
}


