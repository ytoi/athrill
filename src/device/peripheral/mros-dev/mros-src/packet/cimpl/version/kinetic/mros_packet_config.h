#ifndef _MROS_PACKET_CONFIG_H_
#define _MROS_PACKET_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MROS_TOPIC_RAWDATA_HEADER_SIZE					4
#define MROS_TCPROS_RAWDATA_HEADER_SIZE					4

/**************************************
 * PACKET
 **************************************/
#define MROS_PACKET_MAXSIZE_REQ_REGISTER_PUBLISHER		1024
#define MROS_PACKET_MAXSIZE_RES_REGISTER_PUBLISHER		1024
#define MROS_PACKET_MAXSIZE_REQ_REGISTER_SUBSCRIBER		1024
#define MROS_PACKET_MAXSIZE_RES_REGISTER_SUBSCRIBER		1024
#define MROS_PACKET_MAXSIZE_REQ_REQUEST_TOPIC			1024
#define MROS_PACKET_MAXSIZE_RES_REQUEST_TOPIC			1024
#define MROS_PACKET_MAXSIZE_REQ_PUBLISHER_UPDATE		1024


#define MROS_PACKET_MAXSIZE_REQ_TCPROS					512
#define MROS_PACKET_MAXSIZE_RES_TCPROS					512


#ifdef __cplusplus
}
#endif

#endif /* _MROS_PACKET_CONFIG_H_ */
