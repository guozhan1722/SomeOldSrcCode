/*
 * include our .h file 
 */
#include "mh_firewall.h"  
#include "uci.h"


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

/**********************************************************************************
   Function:  int SubProgram_Start(char *pszPathName, char *pszArguments)   
   Input:
   Output:
   Return:
   Note:
**********************************************************************************/

static int SubProgram_Start(char *pszPathName, char *pszArguments)
{
    int  sub_status; 
    int MAX_ARGS=32;
    char * pArgs[MAX_ARGS];
    unsigned long ulNumArgs = 1;
    char tmpArgs[256];
    char *pArg = NULL;
    int pid = 0;    

    pArgs[0]=pszPathName;
    if (pszArguments != NULL)
        {
        strcpy(tmpArgs, pszArguments);    
        /* 'execl' return -1 on error. */
        pArg = strtok(tmpArgs, " ");
        while ((pArg != NULL) && (ulNumArgs < MAX_ARGS))
            {
            pArgs[ulNumArgs] = pArg;
            ulNumArgs++;          
            pArg = strtok(NULL, " ");
            }

        /*
        pArgs[0]=pszPathName;  
        pArgs[1]=pszArguments;  
        pArgs[2] = NULL; */   
        }
    pArgs[ulNumArgs] = NULL; 
    if (!(pid = vfork()))
        {
        seteuid((uid_t)0);
        /* We are the child! */            
        if (execvp(pszPathName, pArgs) == -1)
            {
            //syslog(LOGOPTS,"SubProgram_Start false\n"); 
            return false;
            }
        //syslog(LOGOPTS,"Child PID after = %d\n",pid);          
        }
    else
        {
        waitpid(pid,&sub_status,0);
        return true;        
        }    
    return true; 
}

static int display_header(void)
{
    int retval = 0;
    
    printf("\n");
    printf("mh_firewall [OPTIONS] [-?]=help\n");
    printf("(C)2011 Microhard Systems Inc.\n\n");
    
    retval = 1;
    
    return(retval);
}

static int display_args(void)
{
    int retval = 0;
    
    printf("Options[ rule ]\n");
    printf("       [ defaults ]\n");
    printf("       [ zone ]\n");
    printf("       [ forwarding ]\n");
    printf("       [ redirect ]\n");
    
    retval = 1;
    
    return(retval);
}

void debug_struct(struct fw_rules *fwrules_head)
{
    /*here is for debug*/
    struct fw_rules *p_rule;

    p_rule=fwrules_head;

    while (p_rule->next != NULL) {

        printf(" Index: %d",p_rule->index);

        printf(" src: %s",p_rule->src);

        printf(" src_ip: %s ",p_rule->src_ip);

        printf(" src_mac: %s ",p_rule->src_mac);

        printf(" src_port: %s ",p_rule->src_port);

        printf(" dest: %s ",p_rule->dest);

        printf(" dest_ip: %s ",p_rule->src_port);

        printf(" dest_port: %s",p_rule->dest_port);

        printf(" proto: %s",p_rule->proto);

        printf(" icmp_type: %s",p_rule->icmp_type);

        printf(" target: %s",p_rule->target);

        printf(" pri: %s",p_rule->pri);

        printf(" ZONE: %s",p_rule->ZONE);

        printf(" TARGET: %s",p_rule->TARGET);

        printf(" ruleset: %s\n",p_rule->ruleset);

        p_rule=p_rule->next;
    }

}

static int add_rule(struct fw_rules *fwrules_head)
{
    struct fw_rules *p_rule;
    char cmdstr[32] = "iptables";
    char argstr[256] = " -I ";

    p_rule = fwrules_head;

    strcat(argstr, p_rule->ZONE);
    strcat(argstr, " ");

    if(strcmp(p_rule->proto,"")!=0){
        strcat(argstr, "-p ");
        strcat(argstr, p_rule->proto);
        strcat(argstr, " ");
    }

    if(strcmp(p_rule->icmp_type,"")!=0){
        strcat(argstr, "--icmp-type ");
        strcat(argstr, p_rule->icmp_type);
        strcat(argstr, " ");
    }

    if(strcmp(p_rule->src_ip,"")!=0){
        strcat(argstr, "-s ");
        strcat(argstr, p_rule->src_ip);
        strcat(argstr, " ");
    }

    if(strcmp(p_rule->src_port,"")!=0){
        strcat(argstr, "--sport ");
        strcat(argstr, p_rule->src_port);
        strcat(argstr, " ");
    }

    if(strcmp(p_rule->src_mac,"")!=0){
        strcat(argstr, "-m mac --mac-source ");
        strcat(argstr, p_rule->src_mac);
        strcat(argstr, " ");
    }

    if(strcmp(p_rule->dest_ip,"")!=0){
        strcat(argstr, "-d ");
        strcat(argstr, p_rule->dest_ip);
        strcat(argstr, " ");
    }

    if(strcmp(p_rule->dest_port,"")!=0){
        strcat(argstr, "--dport ");
        strcat(argstr, p_rule->dest_port);
        strcat(argstr, " ");
    }

    strcat(argstr, "-j ");
    strcat(argstr, p_rule->TARGET);

#ifdef  FW_DEBUG
    printf("%s\n", argstr);
#endif
    //system(argstr);
    SubProgram_Start(cmdstr,argstr);

    return 0;
}

static int complete_rule(struct fw_rules *fwrules)
{
    struct fw_rules *p_rule;

    p_rule=fwrules;

    strcpy(p_rule->ZONE,"input");

    if(strcmp(p_rule->target,"")==0) {
        strcpy(p_rule->target,"DROP");
    }

    strcpy(p_rule->TARGET,p_rule->target);

    if((strcmp(p_rule->src,"")!=0)&&(strcmp(p_rule->dest,"")==0)) {
        strcpy(p_rule->ZONE,"zone_");
        strcat(p_rule->ZONE,p_rule->src);
    }

    if((strcmp(p_rule->src,"")!=0)&&(strcmp(p_rule->dest,"")!=0)) {
        strcpy(p_rule->ZONE,"zone_");
        strcat(p_rule->ZONE,p_rule->src);
        strcat(p_rule->ZONE,"_forward");
    }

    if(strcmp(p_rule->dest,"")!=0) {
        strcpy(p_rule->TARGET,"zone_");
        strcat(p_rule->TARGET,p_rule->dest);
        strcat(p_rule->TARGET,"_");
        strcat(p_rule->TARGET,p_rule->target);
    }

    return 0;
}

static int set_rule_pri(struct fw_rules *fwrules)
{
    struct fw_rules *p_rule;

    p_rule=fwrules;

    if (strcmp(p_rule->src_mac,"")!=0) {
        strcpy(p_rule->pri,"100");
    }else if ((strcmp(p_rule->src_ip,"")!=0) || (strcmp(p_rule->dest_ip,"")!=0)) {
        strcpy(p_rule->pri,"200");
    }else if(strcmp(p_rule->proto,"")!=0) {
        if((strcmp(p_rule->src_port,"")!=0)||(strcmp(p_rule->dest_port,"")!=0)) {
            strcpy(p_rule->pri,"400");
        }else {
            strcpy(p_rule->pri,"500");
        }
    }else
        strcpy(p_rule->pri,"600");

    return 0;
}

static int excu_rule(struct fw_rules *fwrules_head)
{
    struct fw_rules *p_rule;

    p_rule = fwrules_head;
    while (p_rule->next != NULL) {
        if((strcmp(p_rule->proto,"tcpudp")==0)||(strcmp(p_rule->proto,"")==0)) {
            strcpy(p_rule->proto,"tcp");
            add_rule(p_rule);
            strcpy(p_rule->proto,"udp");
            add_rule(p_rule);
        }else
            add_rule(p_rule);
        p_rule = p_rule->next;
    }

    return 0;
}
static int mk_rule_order(struct fw_rules *fwrules_head)
{
    struct fw_rules *p_rule, *tmp_rule, bri_rule;
    int pri_p, pri_s,tmp_idx;
    int cus_pri;

    p_rule = fwrules_head ;
    cus_pri = 0;
    while (p_rule->next != NULL) {
        if (strcmp(p_rule->pri,"")==0 ) {
            set_rule_pri(p_rule);
        }
        pri_p=atoi(p_rule->pri);
        tmp_rule=p_rule->next;

        while(tmp_rule->next!=NULL) {
        
            if (strcmp(tmp_rule->pri,"")==0 ) {
                set_rule_pri(tmp_rule);
            }

            pri_s=atoi(tmp_rule->pri);

            if (pri_s > pri_p) {
                strcpy(bri_rule.src, p_rule->src);
                strcpy(bri_rule.src_ip, p_rule->src_ip);
                strcpy(bri_rule.src_mac, p_rule->src_mac);
                strcpy(bri_rule.src_port, p_rule->src_port);
                strcpy(bri_rule.dest, p_rule->dest);
                strcpy(bri_rule.dest_ip, p_rule->dest_ip);
                strcpy(bri_rule.dest_port, p_rule->dest_port);
                strcpy(bri_rule.proto, p_rule->proto);
                strcpy(bri_rule.icmp_type, p_rule->icmp_type);
                strcpy(bri_rule.target, p_rule->target);
                strcpy(bri_rule.ruleset, p_rule->ruleset);
                strcpy(bri_rule.pri, p_rule->pri);
                strcpy(bri_rule.ZONE, p_rule->ZONE);
                strcpy(bri_rule.TARGET, p_rule->TARGET);

                strcpy(p_rule->src, tmp_rule->src);
                strcpy(p_rule->src_ip, tmp_rule->src_ip);
                strcpy(p_rule->src_mac, tmp_rule->src_mac);
                strcpy(p_rule->src_port, tmp_rule->src_port);
                strcpy(p_rule->dest, tmp_rule->dest);
                strcpy(p_rule->dest_ip, tmp_rule->dest_ip);
                strcpy(p_rule->dest_port, tmp_rule->dest_port);
                strcpy(p_rule->proto, tmp_rule->proto);
                strcpy(p_rule->icmp_type, tmp_rule->icmp_type);
                strcpy(p_rule->target, tmp_rule->target);
                strcpy(p_rule->ruleset, tmp_rule->ruleset);
                strcpy(p_rule->pri, tmp_rule->pri);
                strcpy(p_rule->ZONE, tmp_rule->ZONE);
                strcpy(p_rule->TARGET, tmp_rule->TARGET);

                strcpy(tmp_rule->src, bri_rule.src);
                strcpy(tmp_rule->src_ip, bri_rule.src_ip);
                strcpy(tmp_rule->src_mac, bri_rule.src_mac);
                strcpy(tmp_rule->src_port, bri_rule.src_port);
                strcpy(tmp_rule->dest, bri_rule.dest);
                strcpy(tmp_rule->dest_ip, bri_rule.dest_ip);
                strcpy(tmp_rule->dest_port, bri_rule.dest_port);
                strcpy(tmp_rule->proto, bri_rule.proto);
                strcpy(tmp_rule->icmp_type, bri_rule.icmp_type);
                strcpy(tmp_rule->target, bri_rule.target);
                strcpy(tmp_rule->ruleset, bri_rule.ruleset);
                strcpy(tmp_rule->pri, bri_rule.pri);
                strcpy(tmp_rule->ZONE, bri_rule.ZONE);
                strcpy(tmp_rule->TARGET, bri_rule.TARGET);
                pri_p=atoi(p_rule->pri);
            }
            tmp_rule=tmp_rule->next;
        }
        p_rule = p_rule->next;
    }

#ifdef FW_DEBUG
    debug_struct(fwrules_head);
#endif

    return 0;
}

static int get_rule(struct uci_context * ctx, char *fw_section, struct fw_rules *fwrules_head)
{
    int i;
    struct fw_rules *p_rule,*tmp_rule, *p_next;
    char *value, item[32],result[32],section[32],rBuff[32];
    char *src;
    int last=0;

    p_rule = fwrules_head;

    p_rule->index = 0;

    for(i=0;i<MAX_RULES;i++) {

        last=0;

        sprintf(section, "@%s[%d]",fw_section , i);

        bzero(p_rule->src,sizeof(p_rule->src));
        bzero(p_rule->src_ip,sizeof(p_rule->src_ip));
        bzero(p_rule->src_mac,sizeof(p_rule->src_mac));
        bzero(p_rule->src_port,sizeof(p_rule->src_port));
        bzero(p_rule->dest,sizeof(p_rule->dest));
        bzero(p_rule->dest_ip,sizeof(p_rule->dest_ip));
        bzero(p_rule->dest_port,sizeof(p_rule->dest_port));
        bzero(p_rule->proto,sizeof(p_rule->proto));
        bzero(p_rule->icmp_type,sizeof(p_rule->icmp_type));
        bzero(p_rule->target,sizeof(p_rule->target));
        bzero(p_rule->ruleset,sizeof(p_rule->ruleset));
        bzero(p_rule->pri,sizeof(p_rule->pri));

        get_option_by_section_name(ctx, CONFILE, section, "src", rBuff);
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->src, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "src_ip", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->src_ip, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "src_mac", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->src_mac, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "src_port", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->src_port, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "dest", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->dest, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "dest_ip", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->dest_ip, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "dest_port", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->dest_port, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "proto", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->proto, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "icmp_type", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->icmp_type, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "target", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->target, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "ruleset", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->ruleset, rBuff);
            last++;
        }

        get_option_by_section_name(ctx, CONFILE, section, "pri", rBuff); 
        if (strcmp(rBuff,"")!=0) {
            strcpy(p_rule->pri, rBuff);
            last++;
        }

        p_rule->index = i+1;

        if(last==0) {
            p_rule->next = NULL;
            break ;
        }else {
            complete_rule(p_rule);
            tmp_rule=(struct fw_rules *)malloc(sizeof(fwrules));
            p_rule->next =tmp_rule;

            p_rule=tmp_rule;
        }
    }

#ifdef FW_DEBUG
    debug_struct(fwrules_head);
#endif

    return 0;
}

void free_fwrule(struct fw_rules *fwrules_head)
{
    struct fw_rules *p_rule, *tmp_rule;
    
    p_rule = fwrules_head;

    while (p_rule != NULL) {
        tmp_rule = p_rule;
        p_rule = p_rule->next;
        free(tmp_rule);
    }
}

int main(int argc, char *argv[])
{
    int retval = -1,i;
    unsigned char section[64],item[64],rBuff[64];
    struct fw_rules *rule_head;

    if (argc < 2 )
    {
        /* Display Application Information */
        display_header();
        display_args();
        return(retval);

    }

    if ((strcmp(argv[1],"rule") == 0)       || 
        (strcmp(argv[1],"defaults") == 0)   ||
        (strcmp(argv[1],"zone") == 0)       ||
        (strcmp(argv[1],"forwarding") == 0) ||
        (strcmp(argv[1],"redirect") == 0) ) {
        strcpy(section, argv[1]);
    } else {
        /* Display Application Information */
        display_header();
        display_args();
        return(retval);
    }

    if (ctx)
    {
        uci_free_context(ctx);
        ctx=NULL;
    }
    ctx = uci_alloc_context();
    if (!ctx)
    {
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    (void) setsid();

    rule_head = (struct fw_rules *)malloc(sizeof(struct fw_rules));
    get_rule(ctx, section, rule_head);
    mk_rule_order(rule_head);
    excu_rule(rule_head);
    free_fwrule(rule_head);

    return 0;

}

/**********************************************************************************
                  END THANK YOU  
**********************************************************************************/
