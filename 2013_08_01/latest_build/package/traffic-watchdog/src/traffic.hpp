/*
 * traffic.hpp
 *
 *  Created on: May 10, 2012
 *      Author: zguo
 */

#ifndef TRAFFIC_HPP_
#define TRAFFIC_HPP_


#ifdef __cplusplus
using namespace std;
extern "C" {
#endif


#ifdef W_DEBUG
#define W_DBG(x,args...) printf("%s: " x, __func__ , ##args)
//#define W_DBG(x) 	printf(x)
#else
#define W_DBG(args...)
#endif

#define INTERFACE "eth1"
extern  int traffic(int i);
extern int read_proc_net_dev(char* interface,unsigned long* rx,unsigned long* tx);
extern int get_uptime(void);
extern int keep_watch(int b_time, int rs_time, int rb_time,const char *iface,const char *hostname);
extern int SubProgram_Start(char *pszPathName, char *pszArguments);


#ifdef __cplusplus
}
#endif

#endif /* TRAFFIC_HPP_ */
