/**
  ******************************************************************************
  * @file    app_mdns.c
  * @author  MCD Application Team
  * @brief   Configuration of the Network connection.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "msg.h"
#include "app_netxduo.h"

extern NX_MDNS MdnsInstance;
NX_MDNS_SERVICE service_instance;

ULONG error_counter = 0;

VOID probing_notify(struct NX_MDNS_STRUCT *mdns_ptr, UCHAR *name, UINT state)
{
  NX_PARAMETER_NOT_USED(mdns_ptr);

  MSG_DEBUG("Service Name: %s\n", name);

  switch (state)
  {
    case NX_MDNS_LOCAL_SERVICE_REGISTERED_SUCCESS:
    {
      MSG_DEBUG("Registered success!!!\n");
      break;
    }
    case NX_MDNS_LOCAL_SERVICE_REGISTERED_FAILURE:
    {
      MSG_DEBUG("Registered failure!\n");
      break;
    }
    case NX_MDNS_LOCAL_HOST_REGISTERED_SUCCESS:
    {
      MSG_DEBUG("Registered success!!!\n");
      break;
    }
    case NX_MDNS_LOCAL_HOST_REGISTERED_FAILURE:
    {
      MSG_DEBUG("Registered failure!\n");
      break;
    }
    default:
    {
      MSG_DEBUG("Unexpected state!\n");
    }
    break;
  }
}

VOID cache_full_notify(NX_MDNS *mdns_ptr, UINT state, UINT cache_type)
{
  /* Check cache state. */
  if (state == NX_MDNS_CACHE_STATE_FULL)
  {
    MSG_DEBUG("Cache Full!!!\n");
  }
  else
  {
    MSG_DEBUG("Cache Full With Fragment!!!\n");
  }

  /* Check cache type. */
  if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
  {
    MSG_DEBUG("Clearing Local Cache:\n");
    nx_mdns_local_cache_clear(mdns_ptr);
  }
  else
  {
    MSG_DEBUG("Peer Cache:\n");
    nx_mdns_peer_cache_clear(mdns_ptr);
  }
}

VOID service_change_notify(NX_MDNS *mdns_ptr, NX_MDNS_SERVICE *service_ptr, UINT state)
{
  NX_PARAMETER_NOT_USED(mdns_ptr);

  switch (state)
  {
    case NX_MDNS_PEER_SERVICE_RECEIVED:
    {
      MSG_INFO("Received the new Service!!!\n");
      MSG_INFO("Name: %s\n", service_ptr->service_name);
      MSG_INFO("Type: %s\n", service_ptr->service_type);
      MSG_INFO("Domain: %s\n", service_ptr->service_domain);
      MSG_INFO("TXT Info: %s\n", service_ptr->service_text);
      MSG_INFO("Priority: %d\n", service_ptr->service_priority);
      MSG_INFO("Weight: %d\n", service_ptr->service_weight);
      MSG_INFO("Port: %d\n", service_ptr->service_port);
      MSG_INFO("Target Host: %s\n", service_ptr -> service_host);
      MSG_INFO("IPv4 Address: %lu.%lu.%lu.%lu\n",
               service_ptr->service_ipv4 >> 24,
               service_ptr->service_ipv4 >> 16 & 0xFF,
               service_ptr->service_ipv4 >> 8 & 0xFF,
               service_ptr->service_ipv4 & 0xFF);
      MSG_INFO("Interface: %d\n\n\n", service_ptr->interface_index);
    }
    break;

    case NX_MDNS_PEER_SERVICE_UPDATED:
    {
      MSG_INFO("Update the Service address!!!\n");
      MSG_INFO("Name: %s\n", service_ptr->service_name);
      MSG_INFO("Type: %s\n", service_ptr->service_type);
      MSG_INFO("Domain: %s\n", service_ptr->service_domain);
      MSG_INFO("TXT Info: %s\n", service_ptr->service_text);
      MSG_INFO("Priority: %d\n", service_ptr->service_priority);
      MSG_INFO("Weight: %d\n", service_ptr->service_weight);
      MSG_INFO("Port: %d\n", service_ptr->service_port);
      MSG_INFO("Target Host: %s\n", service_ptr->service_host);
      MSG_INFO("IPv4 Address: %lu.%lu.%lu.%lu\n",
               service_ptr->service_ipv4 >> 24,
               service_ptr->service_ipv4 >> 16 & 0xFF,
               service_ptr->service_ipv4 >> 8 & 0xFF,
               service_ptr->service_ipv4 & 0xFF);
      MSG_INFO("Interface: %d\n\n\n", service_ptr->interface_index);
    }
    break;

    case NX_MDNS_PEER_SERVICE_DELETED:
    {
      MSG_INFO("Delete the old Service!!!\n");
      MSG_INFO("Name: %s\n", service_ptr->service_name);
      MSG_INFO("Type: %s\n", service_ptr->service_type);
      MSG_INFO("Domain: %s\n", service_ptr->service_domain);
      MSG_INFO("TXT Info: %s\n", service_ptr->service_text);
      MSG_INFO("Priority: %d\n", service_ptr->service_priority);
      MSG_INFO("Weight: %d\n", service_ptr->service_weight);
      MSG_INFO("Port: %d\n", service_ptr->service_port);
      MSG_INFO("Target Host: %s\n", service_ptr->service_host);
      MSG_INFO("IPv4 Address: %lu.%lu.%lu.%lu\n",
               service_ptr->service_ipv4 >> 24,
               service_ptr->service_ipv4 >> 16 & 0xFF,
               service_ptr->service_ipv4 >> 8 & 0xFF,
               service_ptr->service_ipv4 & 0xFF);
      MSG_INFO("Interface: %d\n\n\n", service_ptr->interface_index);
    }
    break;

  }
}

void register_local_service(UCHAR *instance, UCHAR *type, UCHAR *subtype, UCHAR *txt, UINT ttl,
                            USHORT priority, USHORT weight, USHORT port, UCHAR is_unique)
{
  UINT status = NX_MDNS_SUCCESS;

  status = nx_mdns_service_add(&MdnsInstance, instance, type, subtype, txt, ttl,
                               priority, weight, port,
                               is_unique, 0);

  MSG_INFO("\nmDNS   announce: %s.%s.%s   [%s]\n\n",
           (instance != (UCHAR *)NULL_ADDRESS) ? (char *)instance : "",
           (type != (UCHAR *)NULL_ADDRESS) ? (char *)type : "",
           (subtype != (UCHAR *)NULL_ADDRESS) ? (char *)subtype : "",
           (status == NX_MDNS_SUCCESS) ? "Pass" : (status == NX_MDNS_PARAM_ERROR) ? "Pass" : "Fail");
}

void delete_local_service(UCHAR *instance, UCHAR *type, UCHAR *subtype)
{
  UINT status = NX_MDNS_SUCCESS;

  status = nx_mdns_service_delete(&MdnsInstance, instance, type, subtype);

  MSG_INFO("\nmDNS deannounce: %s.%s.%s   [%s]\n\n",
           (instance != (UCHAR *)NULL_ADDRESS) ? (char *)instance : "",
           (type != (UCHAR *)NULL_ADDRESS) ? (char *)type : "",
           (subtype != (UCHAR *)NULL_ADDRESS) ? (char *)subtype : "",
           (status == NX_MDNS_SUCCESS) ? "Pass" : (status == NX_PTR_ERROR) ? "Pass" : "Fail");
}

void delete_all_services(UCHAR *instance, UCHAR *type, UCHAR *subtype)
{
  UINT status = NX_MDNS_SUCCESS;
  UINT service_index = 0;
  status = nx_mdns_service_continuous_query(&MdnsInstance, instance, type, subtype);

  while (1)
  {
    status = nx_mdns_service_lookup(&MdnsInstance, instance, type, subtype,  service_index, &service_instance);

    if (status == NX_MDNS_SUCCESS)
    {
      if (service_instance.service_name)
      {
        MSG_DEBUG("instance: %s\n", service_instance.service_name);
      }

      if (service_instance.service_type)
      {
        MSG_DEBUG("type    :%s\n", service_instance.service_type);
      }

      if (subtype)
      {
        MSG_DEBUG("subtype :%s\n", subtype);
      }

      status = nx_mdns_service_delete(&MdnsInstance,
                                      service_instance.service_name, service_instance.service_type, subtype);

      if (status)
      {
        MSG_DEBUG("failed\n");
      }
      else
      {
        MSG_DEBUG("successfully\n");
      }
    }
    else if (status == NX_MDNS_NO_MORE_ENTRIES)
    {
      MSG_INFO("\nmDNS all services de-announced  [Pass]\n\n");
      break;
    }
    else
    {
      MSG_INFO("\nmDNS all services de-announced  [Fail]\n\n");
      break;
    }

    service_index++;
  }
  status = nx_mdns_service_query_stop(&MdnsInstance, instance, type, subtype);
}

void perform_oneshot_query(UCHAR *instance, UCHAR *type, UCHAR *subtype, UINT timeout)
{
  UINT status = NX_MDNS_SUCCESS;

  /* Print the query instance. */
  MSG_DEBUG("Send One Shot query:\n");
  if (instance)
  {
    MSG_DEBUG("instance: %s\n", instance);
  }

  if (type)
  {
    MSG_DEBUG("type    : %s\n", type);
  }

  if (subtype)
  {
    MSG_DEBUG("subtype :%s\n", subtype);
  }

  MSG_DEBUG("timeout %d ticks\n", timeout);

  status = nx_mdns_service_one_shot_query(&MdnsInstance, instance, type, subtype, &service_instance, timeout);

  if (status == NX_MDNS_SUCCESS)
  {
    MSG_INFO("Service Get:\n");
    MSG_INFO("Name: %s\n", service_instance.service_name);
    MSG_INFO("Type: %s\n", service_instance.service_type);
    MSG_INFO("Domain: %s\n", service_instance.service_domain);
    MSG_INFO("TXT Info: %s\n", service_instance.service_text);
    MSG_INFO("Priority: %d\n", service_instance.service_priority);
    MSG_INFO("Weight: %d\n", service_instance.service_weight);
    MSG_INFO("Port: %d\n", service_instance.service_port);
    MSG_INFO("Target Host: %s\n", service_instance.service_host);
    MSG_INFO("IPv4 Address: %lu.%lu.%lu.%lu\n",
             service_instance.service_ipv4 >> 24,
             service_instance.service_ipv4 >> 16 & 0xFF,
             service_instance.service_ipv4 >> 8 & 0xFF,
             service_instance.service_ipv4 & 0xFF);
    MSG_INFO("Interface: %d\n\n\n", service_instance.interface_index);
  }
  else
  {
    MSG_INFO("No Service!!!\n\n\n");
  }
}

void start_continous_query(UCHAR *instance, UCHAR *type, UCHAR *subtype)
{
  UINT status;

  status = nx_mdns_service_continuous_query(&MdnsInstance, instance, type, subtype);

  MSG_INFO("Start continuous query: ");

  if (instance)
  {
    MSG_INFO("instance %s ", instance);
  }

  if (type)
  {
    MSG_INFO("type %s ", type);
  }

  if (subtype)
  {
    MSG_INFO("subtype %s ", subtype);
  }

  /* CHECK FOR ERROR.  */
  if (status)
  {
    MSG_INFO("\nfailed\n");
    error_counter++;
  }
  else
  {
    MSG_INFO("\nsuccessfully\n");
  }
}
