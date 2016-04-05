#ifndef CONFIG_H
#define CONFIG_H


#define VERSION "1.2.0"
#define PACKAGE "memached"
#define DATA_BUFFER_SIZE 2048
#define UDP_READ_BUFFER_SIZE 65536
#define UDP_MAX_PAYLOAD_SIZE 1400
#define UDP_HEADER_SIZE 8
#define MAX_SENDBUF_SIZE (256 * 1024 * 1024)

/* Initial size of list of items being returned by "get". */
#define ITEM_LIST_INITIAL 200

/* Initial size of the sendmsg() scatter/gather array. */
#define IOV_LIST_INITIAL 400

/* Initial number of sendmsg() argument structures to allocate. */
#define MSG_LIST_INITIAL 10

/* High water marks for buffer shrinking */
#define READ_BUFFER_HIGHWAT 8192
#define ITEM_LIST_HIGHWAT 400
#define IOV_LIST_HIGHWAT 600
#define MSG_LIST_HIGHWAT 100

/* Time relative to server start. Smaller than time_t on 64-bit systems. */
typedef unsigned int rel_time_t;

#endif // end definition of CONFIG_H
