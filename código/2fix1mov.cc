#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/core-module.h"
#include "ns3/gnuplot.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

int main(int argc, char *argv[]) {
  CommandLine cmd(__FILE__);
  cmd.Parse(argc, argv);

  NodeContainer carro;
  carro.Create(1);

  NodeContainer pontosDeConexao;
  pontosDeConexao.Create(2);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss(
      "ns3::RangePropagationLossModel", "MaxRange",
      DoubleValue(50.0)); // Definindo o alcance máximo para 50 metros

  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
              BooleanValue(false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install(phy, mac, carro);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install(phy, mac, pontosDeConexao);

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  mobility.Install(carro);

  Ptr<ConstantVelocityMobilityModel> mobC =
      carro.Get(0)->GetObject<ConstantVelocityMobilityModel>();
  mobC->SetPosition(Vector(0, 30, 0));
  mobC->SetVelocity(Vector(10, 0, 0));

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(pontosDeConexao);

  Ptr<ConstantPositionMobilityModel> mobA =
      pontosDeConexao.Get(0)->GetObject<ConstantPositionMobilityModel>();
  mobA->SetPosition(Vector(0, 0, 0));

  mobA = pontosDeConexao.Get(1)->GetObject<ConstantPositionMobilityModel>();
  mobA->SetPosition(Vector(80, 0, 0));

  Simulator::Stop(Seconds(10.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
