/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University College Cork (UCC), Ireland
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Xiuchao Wu <xw2@cs.ucc.ie>
 */

#ifndef XGPON_TCONT_OLT_H
#define XGPON_TCONT_OLT_H

#include <vector>

#include "xgpon-tcont.h"
#include "xgpon-connection-receiver.h"
#include "xgpon-xgtc-dbru.h"
#include "xgpon-xgtc-bw-allocation.h"


namespace ns3 {



/**
 * \ingroup xgpon
 * \brief This class is used at OLT-side. It contains the list of (receiver)connections that belongs to the T-CONT.
 *        Note that OLT handles upstream traffics (reassemble included) based on T-CONTs.
 *        Per-connection information is maintained here for convenience (to callculate the aggregated QoS parameters from per-conn QoS, etc.).
 *        The memory overhead is acceptable since the performance bottleneck is CPU.
 */
class XgponTcontOlt : public XgponTcont
{
public:

  /**
   * \brief Constructor
   */
  XgponTcontOlt ();
  virtual ~XgponTcontOlt ();


  ////////////////////////////////////////////          /////Main Interfaces
  /**
   * \brief  receive the status report of this T-CONT from ONU. (add to the history list)
   * \param  report the buffer occupancy status of this T-CONT.
   */
  void ReceiveStatusReport (const Ptr<XgponXgtcDbru>& report, uint64_t time);  


  //put the just created bandwidth allocation into the service history of this T-CONT.
  void AddNewBwAllocation2ServiceHistory (const Ptr<XgponXgtcBwAllocation>& allocation, uint64_t time);


  //Get the time that the latest polling grant is sent to this T-CONT.
  //the latest polling time is set when the corresponding bwalloc is put into the history list.
  uint64_t GetLatestPollingTime();





  /**
   * \brief  add one connection into the T-CONT. 
   * \param  conn the connection to be added.
   */
  void AddOneConnection (const Ptr<XgponConnectionReceiver>& conn);





  /**
   * \brief  get aggregate QoS parameters of upstream connections that belong to the same T-CONT. It is used to by DBA at OLT-side
   * \return the aggregated QoS Parameters.
   */
  const Ptr<XgponQosParameters>& GetAggregatedQosParameters () const;   







  /**
   * \brief calculate the amount of data at ONU that still needs to be served.  (needed when multi-thread dba algorithms are used.)
   * \param rtt round trip delay between OLT and the ONU that this T-CONT belongs to. unit: nanosecond
   * \param slotSize the length of one downstream frame. unit: nanosecond
   * \return the amount of data still needs to be served (unit: word). 
   * Note that this value should be calculated as follow.
   *    The last queue occupancy report - the amount of data scheduled by the report's corresponding BwAlloc and the following BwAllocs.
   */
  uint32_t CalculateRemainingDataToServe (uint64_t rtt, uint64_t slotSize);







  //////////////////////////////////////////////////////////////Reassemble related functions
  /**
   * \brief put back the received fragments for further reassemble. 
   */  
  void SetPacket4Reassemble (const Ptr<Packet>& pkt);

  /**
   * \brief Return the received segments to carry out further reassemble. 
   * \return the segments received previously. 0: have not received any segment.
   * Note that we cannot return one reference since the packet to be reassembled may be empty.
   */
  const Ptr<Packet> GetPacket4Reassemble ( );







  ////////////////////////////////////////////////////////////Functions required by NS-3
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;



private:

  //Connections of the same alloc-id. They should have the same T-CON type
  std::vector< Ptr<XgponConnectionReceiver> > m_connections;   

  Ptr<Packet> m_pkt4Reassemble;             //used to hold the packet to be reassembled (only one connection of the same T-CONT can be in reassemble mode). 

  Ptr<XgponQosParameters> m_qosParameters;  //the aggregated QoS parameters based on connections that belong to this T-CONT.

  uint64_t m_lastPollingTime;               //the time that the last polling grant is sent to this T-CONT.



  //////////////////////////////////////////////////////Clear history
  //remove based on receive_time
  virtual void ClearOldBufOccupancyReports (uint64_t time);

  //remove based on create_time
  virtual void ClearOldBandwidthAllocations (uint64_t time);


  /* more variables may be needed */
};





//////////////////////////////////////INLINE Functions
inline void 
XgponTcontOlt::SetPacket4Reassemble (const Ptr<Packet>& pkt)
{
  NS_ASSERT_MSG((pkt!=0), "The packet to be reassembled is NULL!!!");
  m_pkt4Reassemble = pkt;
}
inline const Ptr<Packet>
XgponTcontOlt::GetPacket4Reassemble () 
{
  Ptr<Packet> pkt = 0;
  
  if(m_pkt4Reassemble!=0) pkt = m_pkt4Reassemble;

  m_pkt4Reassemble = 0;

  return pkt;
}




inline const Ptr<XgponQosParameters>& 
XgponTcontOlt::GetAggregatedQosParameters () const
{
  return m_qosParameters;
}





inline uint64_t 
XgponTcontOlt::GetLatestPollingTime()
{
  return m_lastPollingTime;
}







}; // namespace ns3



#endif // XGPON_TCONT_OLT_H
