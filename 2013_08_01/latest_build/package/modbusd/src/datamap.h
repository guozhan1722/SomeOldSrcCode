#ifndef MODBUS_SLAVE_DATAMAP_H
#define MODBUS_SLAVE_DATAMAP_H

#define MODBUS_DMAP_TYPE_MAX 3
#define MODBUS_DMAP_TYPE_C 0
#define MODBUS_DMAP_TYPE_I 1
#define MODBUS_DMAP_TYPE_R 2

#define MODBUS_DMAP_C_MAX 48   //bits address
#define MODBUS_DMAP_C_MAXBYTES 6   
enum{
    MODBUS_DMAP_C_O1=0,  
    MODBUS_DMAP_C_O2,  
    MODBUS_DMAP_C_O3,  
    MODBUS_DMAP_C_O4,  
    MODBUS_DMAP_C_O5,  
    MODBUS_DMAP_C_O6,  
    MODBUS_DMAP_C_O7,  
    MODBUS_DMAP_C_O8,  

    MODBUS_DMAP_C_COM1,  
    MODBUS_DMAP_C_COM2,  
    MODBUS_DMAP_C_COM3,  
    MODBUS_DMAP_C_COM4,  
    MODBUS_DMAP_C_ETH0,  
    MODBUS_DMAP_C_ETH1,  
    MODBUS_DMAP_C_ETH2,  
    MODBUS_DMAP_C_ETH3,  

    MODBUS_DMAP_C_CARR0,  
    MODBUS_DMAP_C_CARR1,  
    MODBUS_DMAP_C_WIFI0,  
    MODBUS_DMAP_C_WIFI1,  
    MODBUS_DMAP_C_USB0,  
    MODBUS_DMAP_C_USB1,
    MODBUS_DMAP_C_GPSD,
    MODBUS_DMAP_C_LOCNET,
      
    MODBUS_DMAP_C_EUR1,  
    MODBUS_DMAP_C_EUR2,  
    MODBUS_DMAP_C_EUR3,  
    MODBUS_DMAP_C_NMS,  
    MODBUS_DMAP_C_WEB,  
    MODBUS_DMAP_C_FWL,  
    MODBUS_DMAP_C_31,  
    MODBUS_DMAP_C_32,  

    MODBUS_DMAP_C_33,  
    MODBUS_DMAP_C_34,  
    MODBUS_DMAP_C_35,  
    MODBUS_DMAP_C_36,  
    MODBUS_DMAP_C_37,  
    MODBUS_DMAP_C_38,  
    MODBUS_DMAP_C_39,  
    MODBUS_DMAP_C_40,  

    MODBUS_DMAP_C_REBT,  

};

#define MODBUS_DMAP_I_MAX 8
#define MODBUS_DMAP_I_MAXBYTES 1
enum{
    MODBUS_DMAP_I_O1=0,  
    MODBUS_DMAP_I_O2,  
    MODBUS_DMAP_I_O3,  
    MODBUS_DMAP_I_O4,  
    MODBUS_DMAP_I_O5,  
    MODBUS_DMAP_I_O6,  
    MODBUS_DMAP_I_O7,  
    MODBUS_DMAP_I_O8,  
};

#define MODBUS_DMAP_R_MAX 24
#define MODBUS_DMAP_R_MAXBYTES 48

enum{
    MODBUS_DMAP_R_MODEL=0,  
    MODBUS_DMAP_R_BUILD,  
    MODBUS_DMAP_R_MID0,  //same to big endian.
    MODBUS_DMAP_R_MID1,  
    MODBUS_DMAP_R_MID2,  
    MODBUS_DMAP_R_MID3,  
    MODBUS_DMAP_R_RSSI,  
    MODBUS_DMAP_R_VDC,  

    MODBUS_DMAP_R_TEMP,  
    MODBUS_DMAP_R_CARR_RM,  //receive MB
    MODBUS_DMAP_R_CARR_TM,  //translate MB
    MODBUS_DMAP_R_GPS_ALT, 
    MODBUS_DMAP_R_GPS_LAT0,  //big endian high  
    MODBUS_DMAP_R_GPS_LAT1,  
    MODBUS_DMAP_R_GPS_LON0,  
    MODBUS_DMAP_R_GPS_LON1,  

    MODBUS_DMAP_R_COM1_RATE,//300-460800  
    MODBUS_DMAP_R_COM1_FMT,//0,1,2,...  
    MODBUS_DMAP_R_COM2_RATE,//300-460800  
    MODBUS_DMAP_R_COM2_FMT,//0,1,2,...  

};

#define MODBUS_DODEM_MODEL_MAX 16
enum{
    MODBUS_MODEL_00=0,//unknown
    MODBUS_MODEL_01,
    MODBUS_MODEL_02,
    MODBUS_MODEL_03,
    MODBUS_MODEL_04,
    MODBUS_MODEL_05,
    MODBUS_MODEL_IPn3G,
    MODBUS_MODEL_VIP4G,
    MODBUS_MODEL_IPn4G,
    MODBUS_MODEL_09,
    MODBUS_MODEL_10,
    MODBUS_MODEL_11,
    MODBUS_MODEL_12,
    MODBUS_MODEL_13,
    MODBUS_MODEL_14,
    MODBUS_MODEL_15,
};

#define MODBUS_COM_DATAFMT_MAX 11
enum{
    MODBUS_COMDFMT_00=0,//unknown
    MODBUS_COMDFMT_8N1,
    MODBUS_COMDFMT_8N2,
    MODBUS_COMDFMT_8E1,
    MODBUS_COMDFMT_8O1,
    MODBUS_COMDFMT_7N1,
    MODBUS_COMDFMT_7N2,
    MODBUS_COMDFMT_7E1,
    MODBUS_COMDFMT_7O1,
    MODBUS_COMDFMT_7E2,
    MODBUS_COMDFMT_7O2,
};


//if first character is * means this module may not support
char Modbus_DMAP_C_name[MODBUS_DMAP_C_MAX][30] ={
    "OUTPUT 1","OUTPUT 2","OUTPUT 3","OUTPUT 4","*OUTPUT 5","*OUTPUT 6","*OUTPUT 7","*OUTPUT 8",
    "*COM1 Status","COM2 Status","*COM3 Status","*COM4 Status","LAN/eth0 Status","WAN/eth1 Status","*eth2 Status","*eth3 Status",
    "Carrier Status","*Second Carrier Status","Wifi Status","*Second Wifi Status","*USB Status","*Second USB Status","GPS Status","Location Over Network",
    "Event UDP Report 1","Event UDP Report 2","Event UDP Report 3","NMS Report","Web Client Service","Firewall Status","*No. 31","*No. 32",
    "*No. 33","*No. 34","*No. 35","*No. 36","*No. 37","*No. 38","*No. 39","*No. 40",
    "SYSTEM Reboot",
};
char Modbus_DMAP_I_name[MODBUS_DMAP_I_MAX][20] ={
    "INPUT 1","INPUT 2","INPUT 3","INPUT 4","*INPUT 5","*INPUT 6","*INPUT 7","*INPUT 8",
};
char Modbus_DMAP_R_name[MODBUS_DMAP_R_MAX][35] ={
    "Modem Model Type...","Build Version","Modem ID Highest 2 Bytes","Modem ID Higher 2 Bytes","Modem ID Lower 2 Bytes","Modem ID Lowest 2 Bytes","RSSI(db)","*VDC(x100)(V)",
    "Core Temperature(C)","Carrier Received Bytes(MB)","Carrier Transmitted Bytes(MB)","GPS Altitude(m)","GPS Latitude High 2 Bytes","Latitude Low 2 Bytes(x1000000)","GPS Longitude High 2 Bytes","Longitude Low 2 Bytes(x1000000)",
    "*COM1 Baud Rate(/100)(bps)","*COM1 Data Format...","COM2 Baud Rate(/100)(bps)","COM2 Data Format...",
};
char Modbus_Dodem_Model_type[MODBUS_DODEM_MODEL_MAX][20] ={
    "Unknow","To be defined","To be defined","To be defined","To be defined","To be defined","IPn3G","VIP4G",
    "IPn4G","To be defined","To be defined","To be defined","To be defined",
};
char Modbus_Com_Data_Format[MODBUS_COM_DATAFMT_MAX][10] ={
    "Unknow","8N1","8N2","8E1","8O1","7N1","7N2","7E1","7O1","7E2","7O2",
};

/* 
#define MENU_SUB_MODBUS_SLAVE  0
enum{
    MODBUS_SLAVE_ENABLE=0,  //service enable
    MODBUS_TCP_ENABLE,//TCP model enable:A/B
    MODBUS_TCP_PORT,//TCP model port
    MODBUS_TCP_TIMEOUT,//TCP connect timeout
    MODBUS_TCP_ID,//TCP model id
    MODBUS_TCP_OFFSET_C,//TCP mode offset address for 01 output
    MODBUS_TCP_OFFSET_I,//TCP mode offset address for 02 intput
    MODBUS_TCP_OFFSET_R,//TCP mode  offset address for 03 register
    MODBUS_TCP_IPFILTER_EN,//TCP master ip filter enable
    MODBUS_COM_ENABLE,//A:com mode disable,B:COM1 ASCII Enable, C:COM1 RTU, D:COM2 ASCII, E:COM2 RTU
    MODBUS_COM_BAUD_RATE,//com mode: rate
    MODBUS_COM_DATA_FORMAT,//com mode: dataformat
    MODBUS_COM_DATA_CHANNEL_MODE,//com mode: channelmode rs232/485
    MODBUS_COM_TIMEOUT,//com mode: time out
    MODBUS_COM_ID,//TCP model id
    MODBUS_COM_OFFSET_C,//COM mode offset address for 01 input
    MODBUS_COM_OFFSET_I,//COM mode offset address for 02 output
    MODBUS_COM_OFFSET_R,//COM mode  offset address for 03 register
    MODBUS_TCP_IPFILTER1,//TCP master ip filter 
    MODBUS_TCP_IPFILTER2,//TCP master ip filter 
    MODBUS_TCP_IPFILTER3,//TCP master ip filter 
    MODBUS_TCP_IPFILTER4,//TCP master ip filter 
};
 
#define MENUS_NUMBER 1
#define ITEM_MAX_NUMBER 23
char Display_Buff3[MENUS_NUMBER][ITEM_MAX_NUMBER][30]={
    //ADVANCE_SUB_MODBUS_SLAVE Modbus slave:  add by alex 2012-11-12
    {"Modbus Slave Status:","Modbus TCP Enable:"," Port:"," Active Timeout:"," Slave ID:"," Coils Address Offset:"," Input Address Offset:"," Register Address Offset:"," Master IP Filter Set:",
     "Modbus COM Enable:"," Baud Rate:"," Data Format:"," Data Mode:"," Character Timeout:"," Slave ID:"," Coils Address Offset:"," Input Address Offset:"," Register Address Offset:",
     " Accept Master IP1:"," Accept Master IP2:"," Accept Master IP3:"," Accept Master IP4:",
    },
};// end of Display_Buff3 

char DataBase1_Buff[MENUS_NUMBER][ITEM_MAX_NUMBER][10]=
{//  Default value   
    //ADVANCE_SUB_MODBUS_SLAVE Modbus slave:  add by alex 2012-11-12
    {"A","A","502","300","1","0","0","0","A","A","H","A","A","0","1","0","0","0","0.0.0.0","0.0.0.0","0.0.0.0","0.0.0.0"},

};

char Menu_Item_Options[MENUS_NUMBER][ITEM_MAX_NUMBER][OPTION_MAX_NUMBER][30]=
{	
    //ADVANCE_SUB_MODBUS_SLAVE Modbus slave:  add by alex 2012-11-12
    {
        {"Disable","Enable"},
        {"Disable","Enable Modbus TCP Mode"},
        {},{},{},{},{},{},{"Disable","Enable Modbus Master IP Filter"},
        {"Disable","Enable COM1 ASCII Mode","Enable COM1 RTU Mode","Enable COM2 ASCII Mode","Enable COM2 RTU Mode"},
        {"300","600","1200","2400","3600","4800","7200","9600","14400","19200","28800","38400","57600","115200","230400","460800","921600"},
        {"8N1","8N2","8E1","8O1","7N1","7N2","7E1","7O1","7E2","7O2",},  
        {"RS232","RS485","RS422"},
        {},{},{},{},{},
    }
};

char Item_Keywords[MENUS_NUMBER][ITEM_MAX_NUMBER][40]=
{
    //ADVANCE_SUB_MODBUS_SLAVE Modbus slave:  add by alex 2012-11-12
    {"Modbus_S_Enable=","Modbus_S_TCP_en=","Modbus_S_TCP_port=","Modbus_S_TCP_timeout=","Modbus_S_TCP_id=","Modbus_S_TCP_C=","Modbus_S_TCP_I=","Modbus_S_TCP_R=","Modbus_S_TCP_IPF_EN=",
        "Modbus_S_COM_en=","Modbus_S_COM_rate=","Modbus_S_COM_format=","Modbus_S_COM_chmode=","Modbus_S_COM_timeout=","Modbus_S_COM_id=","Modbus_S_COM_C=","Modbus_S_COM_I=","Modbus_S_COM_R=",
        "Modbus_S_TCP_IPF1=","Modbus_S_TCP_IPF2=","Modbus_S_TCP_IPF3=","Modbus_S_TCP_IPF4=",
    }
} ;
*/

#endif
