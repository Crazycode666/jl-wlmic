/*********************************************************************************************
    *   Filename        : broadcast_api.h

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-07-07 14:37

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _BROADCAST_API_H
#define _BROADCAST_API_H

/*  Include header */
#include "system/includes.h"
#include "typedef.h"
#include "wireless_params.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Data Types
**************************************************************************************************/
/*! \brief 广播角色枚举 */
enum {
    BROADCAST_ROLE_UNKNOW,
    BROADCAST_ROLE_TRANSMITTER,
    BROADCAST_ROLE_RECEIVER,
};

//time 类型
enum {
    CURRENT_TIME,
    PACKET_RX_TIME,
};

/*! \brief 广播同步模块结构体 */
struct broadcast_sync_hdl {
    struct list_head entry; /*!< 同步模块链表项，用于多同步模块管理 */
    u8 big_hdl;         /*!< big句柄，用于big控制接口 */
    u16 seqn;
    u32 tx_sync_time;
    OS_SEM sem;
    void *bcsync;
    void *broadcast_hdl;
};

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
extern const big_callback_t big_rx_cb;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/* ***************************************************************************/
/**
 * @brief open broadcast as receiver
 *
 * @return err:-1, success:available_big_hdl
 */
/* *****************************************************************************/
int broadcast_receiver(big_parameter_t *params);

/* ***************************************************************************/
/**
 * @brief close broadcast function
 *
 * @param big_hdl:need closed of big_hdl
 */
/* *****************************************************************************/
void broadcast_close(u8 big_hdl);

/* --------------------------------------------------------------------------*/
/**
 * @brief get current broadcast role
 *
 * @return broadcast role
 */
/* ----------------------------------------------------------------------------*/
u8 get_broadcast_role(void);

/* --------------------------------------------------------------------------*/
/**
 * @brief init broadcast params
 */
/* ----------------------------------------------------------------------------*/
void broadcast_init(void);

/* --------------------------------------------------------------------------*/
/**
 * @brief uninit broadcast params
 */
/* ----------------------------------------------------------------------------*/
void broadcast_uninit(void);

/* --------------------------------------------------------------------------*/
/**
 * @brief 初始化同步的状态数据的内容
 *
 * @param data:用来同步的数据
 */
/* ----------------------------------------------------------------------------*/
void broadcast_sync_data_init(void *data);

int broadcast_enter_pair(u8 role, void *pair_event_cb, u8 mode);

int broadcast_exit_pair(u8 role);

int broadcast_receiver_connect_deal(void *priv, int crc16);

int broadcast_receiver_disconnect_deal(void *priv);

int broadcast_get_rssi(void);

#ifdef __cplusplus
};
#endif

#endif //_BROADCASE_API_H

