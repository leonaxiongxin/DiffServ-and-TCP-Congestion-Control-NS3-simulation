#include <fstream>
#include <string> 
#include <string>  
#include <cassert> 
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/animation-interface.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/double.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-l4-protocol.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Project2");

//declaration of the MyApp Application that we put together to allow the Socket to be created at configuration time
class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp();
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize,uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

//MyApp Application needs a constructor and a destructor
MyApp::MyApp ()
   : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

//the whole reason why we wrote this Application in the first place
void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize,uint32_t nPackets,DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

//We are just initializing member variables
void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

//stop creating simulation events
void
MyApp::StopApplication (void)
{
  m_running = false;
  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }
  if (m_socket)
    {
      m_socket->Close ();
    }
}


//Recall that StartApplication called SendPacket to start the chain of events that describes the Application behavior.
void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }

}

//call ScheduleTx to schedule another transmit event (a SendPacket) until the Application decides it has sent enough.
void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void 
PrintTxQueue (Ptr <Queue> q0,Ptr <Queue> q1, Ptr <Queue> q2, Ptr <Queue> q3, Ptr <Queue> q4)
{
Simulator::Schedule (Seconds (0.001), &PrintTxQueue, q0, q1, q2, q3, q4);
std::cout << Simulator::Now ().GetSeconds () << "\t" << q0->GetNPackets()<< "\t" 
<< q1->GetNPackets()<< "\t" << q2->GetNPackets()<< "\t" << q3->GetNPackets()<< "\t" 
<< q4->GetNPackets()<<"\n";
}

//This function just logs the current simulation time and the new value of the congestion window every time it is changed.
/*static void
CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
  std::cout << Simulator::Now ().GetSeconds () << "\t" << newCwnd <<"\n";
}*/
//                                        +--n10
//                                        | 
//         n0--+            +--n6         +---n11
//             |            |             |
//             n2 ---n3 --- n5 --- n7 --- n9---n12
//             |     |             |      |
//         n1--+     +--n4         +--n8  +---n13
//                                        |
//                                        +--n14

int main (int argc, char *argv[])
{
  
  uint32_t nFlows=5;
  uint32_t packetSize=1400;
  uint32_t nPackets=2000;
  uint32_t j=1;

for(j=1;j<=5;j++)
{
  uint bufSize=j*packetSize;
  CommandLine cmd;
  cmd.AddValue("nFlows","Number of TCP Flows",nFlows);
  cmd.Parse(argc,argv);
  
  Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpReno"));
  Config::SetDefault("ns3::DropTailQueue::Mode", StringValue("QUEUE_MODE_PACKETS"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets",UintegerValue (j));
  /*Config::SetDefault ("ns3::RedQueue::Mode", StringValue ("QUEUE_MODE_PACKETS"));
  Config::SetDefault ("ns3::RedQueue::MinTh", DoubleValue (1));
  Config::SetDefault ("ns3::RedQueue::MaxTh", DoubleValue (3));
  Config::SetDefault ("ns3::RedQueue::QueueLimit", UintegerValue (4));*/
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1000));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue (1));
  Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue (20*1000));

  //在輸出和接收資料包後輸出log消息
  //LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);
   
  NS_LOG_INFO ("Create Nodes.");
  NodeContainer nodes;
  nodes.Create (15);
  NodeContainer n0n2 = NodeContainer (nodes.Get (0), nodes.Get (2)); //h1r1
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2)); //h2r1
  NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3)); //r1r2
  NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4)); //r2h3
  NodeContainer n3n5 = NodeContainer (nodes.Get (3), nodes.Get (5)); //r2r3
  NodeContainer n5n6 = NodeContainer (nodes.Get (5), nodes.Get (6)); //r3h4
  NodeContainer n5n7 = NodeContainer (nodes.Get (5), nodes.Get (7)); //r3r4
  NodeContainer n7n8 = NodeContainer (nodes.Get (7), nodes.Get (8)); //r4h5
  NodeContainer n7n9 = NodeContainer (nodes.Get (7), nodes.Get (9)); //r4r5
  NodeContainer n9n10 = NodeContainer (nodes.Get (9), nodes.Get (10)); //r5h6
  NodeContainer n9n11 = NodeContainer (nodes.Get (9), nodes.Get (11)); //r5h7
  NodeContainer n9n12 = NodeContainer (nodes.Get (9), nodes.Get (12)); //r5h8
  NodeContainer n9n13 = NodeContainer (nodes.Get (9), nodes.Get (13)); //r5h9
  NodeContainer n9n14 = NodeContainer (nodes.Get (9), nodes.Get (14)); //r5h10

  InternetStackHelper internet;
  internet.Install (nodes);

  NS_LOG_INFO ("Create Channels.");
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate",  StringValue("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("100us"));
  /*pointToPoint.SetQueue("ns3::RedQueue");*/
  NetDeviceContainer d0d2 = pointToPoint.Install (n0n2);
  NetDeviceContainer d1d2 = pointToPoint.Install (n1n2);
  NetDeviceContainer d3d4 = pointToPoint.Install (n3n4);
  NetDeviceContainer d5d6 = pointToPoint.Install (n5n6);
  NetDeviceContainer d7d8 = pointToPoint.Install (n7n8);
  NetDeviceContainer d9d10 = pointToPoint.Install (n9n10);
  NetDeviceContainer d9d11 = pointToPoint.Install (n9n11);
  NetDeviceContainer d9d12 = pointToPoint.Install (n9n12);
  NetDeviceContainer d9d13 = pointToPoint.Install (n9n13);
  NetDeviceContainer d9d14 = pointToPoint.Install (n9n14);
  NetDeviceContainer d2d3 = pointToPoint.Install (n2n3);
  NetDeviceContainer d3d5 = pointToPoint.Install (n3n5);
  NetDeviceContainer d5d7 = pointToPoint.Install (n5n7);
  NetDeviceContainer d7d9 = pointToPoint.Install (n7n9);
  
  NS_LOG_INFO ("Assign IP Address5s.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = ipv4.Assign (d0d2);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i3 = ipv4.Assign (d2d3);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = ipv4.Assign (d3d4);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i5 = ipv4.Assign (d3d5);

  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign (d5d6);

  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i7 = ipv4.Assign (d5d7);

  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer i7i8 = ipv4.Assign (d7d8);

  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  Ipv4InterfaceContainer i7i9 = ipv4.Assign (d7d9);

  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i10 = ipv4.Assign (d9d10);

  ipv4.SetBase ("10.1.11.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i11 = ipv4.Assign (d9d11);

  ipv4.SetBase ("10.1.12.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i12 = ipv4.Assign (d9d12);

  ipv4.SetBase ("10.1.13.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i13 = ipv4.Assign (d9d13);

  ipv4.SetBase ("10.1.14.0", "255.255.255.0");
  Ipv4InterfaceContainer i9i14 = ipv4.Assign (d9d14);

  NS_LOG_INFO ("Enable static global routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /*set up tcp server connection*/
  uint16_t port = 8000;
  ApplicationContainer sinkApp;  
  Address sinkLocalAddress(InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

  sinkApp.Add(sinkHelper.Install(n9n10.Get(1)));  
  sinkApp.Add(sinkHelper.Install(n9n11.Get(1)));
  sinkApp.Add(sinkHelper.Install(n9n12.Get(1)));  
  sinkApp.Add(sinkHelper.Install(n9n13.Get(1)));
  sinkApp.Add(sinkHelper.Install(n9n14.Get(1)));  

  sinkApp.Start (Seconds (0.5));
  sinkApp.Stop (Seconds (3.5));

  /*create the socket and connect the trace source*/
  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  //ns3TcpSocket1->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (nodes.Get (1), TcpSocketFactory::GetTypeId ());
  //ns3TcpSocket2->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (nodes.Get (4), TcpSocketFactory::GetTypeId ());
  //ns3TcpSocket3->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
  Ptr<Socket> ns3TcpSocket4 = Socket::CreateSocket (nodes.Get (6), TcpSocketFactory::GetTypeId ());
  //ns3TcpSocket4->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
  Ptr<Socket> ns3TcpSocket5 = Socket::CreateSocket (nodes.Get (8), TcpSocketFactory::GetTypeId ());
  //ns3TcpSocket5->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
  
  /**set up Application**/
  Ptr<MyApp> clientApp1 = CreateObject<MyApp>();
  Ptr<MyApp> clientApp2 = CreateObject<MyApp>();
  Ptr<MyApp> clientApp3 = CreateObject<MyApp>();
  Ptr<MyApp> clientApp4 = CreateObject<MyApp>();
  Ptr<MyApp> clientApp5 = CreateObject<MyApp>();

  /**make connection**/
  Ipv4Address clientAddress1 = i0i2.GetAddress (0);
  AddressValue localAddress1 (InetSocketAddress (clientAddress1, port));
  Ipv4Address serverAddress1 = i9i10.GetAddress (1);
  Address remoteAddress1 (InetSocketAddress (serverAddress1, port));
  
  Ipv4Address clientAddress2 = i1i2.GetAddress (0);
  AddressValue localAddress2 (InetSocketAddress (clientAddress2, port));
  Ipv4Address serverAddress2 = i9i11.GetAddress (1);
  Address remoteAddress2 (InetSocketAddress (serverAddress2, port));

  Ipv4Address clientAddress3 = i3i4.GetAddress (1);
  AddressValue localAddress3 (InetSocketAddress (clientAddress3, port));
  Ipv4Address serverAddress3 = i9i12.GetAddress (1);
  Address remoteAddress3 (InetSocketAddress (serverAddress3, port));

  Ipv4Address clientAddress4 = i5i6.GetAddress (1);
  AddressValue localAddress4 (InetSocketAddress (clientAddress4, port));
  Ipv4Address serverAddress4 = i9i13.GetAddress (1);
  Address remoteAddress4 (InetSocketAddress (serverAddress4, port));

  Ipv4Address clientAddress5 = i7i8.GetAddress (1);
  AddressValue localAddress5 (InetSocketAddress (clientAddress5, port));
  Ipv4Address serverAddress5 = i9i14.GetAddress (1);
  Address remoteAddress5 (InetSocketAddress (serverAddress5, port));

  /*bind socket with application and connect to server*/
  //set sending date rate and packet size
  clientApp1->Setup(ns3TcpSocket1, remoteAddress1, packetSize,nPackets, DataRate("2Mbps"));
  clientApp2->Setup(ns3TcpSocket2, remoteAddress2, packetSize,nPackets, DataRate("2Mbps"));
  clientApp3->Setup(ns3TcpSocket3, remoteAddress3, packetSize,nPackets, DataRate("2Mbps"));
  clientApp4->Setup(ns3TcpSocket4, remoteAddress4, packetSize,nPackets, DataRate("2Mbps"));
  clientApp5->Setup(ns3TcpSocket5, remoteAddress5, packetSize,nPackets, DataRate("2Mbps"));

  /*install application to Client*/
  nodes.Get (0)->AddApplication(clientApp1);
  nodes.Get (1)->AddApplication(clientApp2);
  nodes.Get (4)->AddApplication(clientApp3);
  nodes.Get (6)->AddApplication(clientApp4);
  nodes.Get (8)->AddApplication(clientApp5);

  /*set application start time and stop time*/
  clientApp1->SetStartTime(Seconds(1.0));
  clientApp1->SetStopTime(Seconds(3.0));
  clientApp2->SetStartTime(Seconds(1.0));
  clientApp2->SetStopTime(Seconds(3.0));
  clientApp3->SetStartTime(Seconds(1.0));
  clientApp3->SetStopTime(Seconds(3.0));
  clientApp4->SetStartTime(Seconds(1.0));
  clientApp4->SetStopTime(Seconds(3.0));
  clientApp5->SetStartTime(Seconds(1.0));
  clientApp5->SetStopTime(Seconds(3.0));

  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("pro2.tr"));

  // Calculate Throughput using Flowmonitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  /*
  //queue length
  Ptr<PointToPointNetDevice> net0 = DynamicCast <PointToPointNetDevice> (d0d2.Get (1));
  Ptr <Queue> q0 = net0->GetQueue();
  Ptr<PointToPointNetDevice> net1 = DynamicCast <PointToPointNetDevice> (d2d3.Get (0));
  Ptr <Queue> q1 = net1->GetQueue();
  Ptr<PointToPointNetDevice> net2 = DynamicCast <PointToPointNetDevice> (d3d5.Get (0));
  Ptr <Queue> q2 = net2->GetQueue();
  Ptr<PointToPointNetDevice> net3 = DynamicCast <PointToPointNetDevice> (d5d7.Get (0));
  Ptr <Queue> q3 = net3->GetQueue();
  Ptr<PointToPointNetDevice> net4 = DynamicCast <PointToPointNetDevice> (d7d9.Get (0));
  Ptr <Queue> q4 = net4->GetQueue();
  Simulator::Schedule (Seconds (0.0001), &PrintTxQueue, q0, q1, q2, q3, q4);
  */
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds(3));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  
  std::cout<<   j   <<"\t";
  std::cout <<"  Buffer size  " <<bufSize <<"\n" ; 
 
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
  
      std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ") - "<<"\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  PacketLoss: " <<i->second.lostPackets<<"\n"; 
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
      }

    Simulator::Destroy ();
   }
 }
