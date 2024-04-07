import dynamic from "next/dynamic";
import { getAgentData, getTransportMode } from "@/lib/scrapeAgents";
import AutoReload from "@/components/auto-reload";
import { BiSolidNetworkChart } from "react-icons/bi";
import SettingsMenubar from "@/components/settings-menubar";
const NetworkGraph = dynamic(() => import("@/components/network-graph"), {
  ssr: false,
});

export default async function Home() {
  const data = await getAgentData();
  //const data = await getSampleAgentData();

  if (data.status == "error") {
    return (
      <div>
        <h1>An error has occurred!</h1>
        <p>{data.error}</p>
      </div>
    );
  }

  const avgRttMillis =
    data.result.reduce((acc, node) => acc + node.avgRttMillis, 0) /
    data.result.length;
  const avgPacketSuccessRate =
    data.result.reduce((acc, node) => acc + node.packetSuccessRate * 100, 0) /
    data.result.length;

  const transport = await getTransportMode();
  if (transport.status != "success") {
    <div>Error getting transport status</div>;
  }

  return (
    <div>
      <div className="py-8 px-12 mb-8 border-b flex justify-between items-center select-none">
        <div className="flex items-center">
          <BiSolidNetworkChart size="1.5rem" />
          <h1 className="font-semibold text-2xl">&nbsp;ThreadNet Explorer</h1>
        </div>
        <div className="flex gap-4 items-center">
          <div className="flex gap-2 items-center">
            <p className="font-semibold">Avg RTT:</p>
            <p>{avgRttMillis.toFixed(2)}ms</p>
          </div>
          <div className="flex gap-2 items-center">
            <p className="font-semibold">Avg Packet Success Rate:</p>
            <p>{avgPacketSuccessRate.toFixed(2)}%</p>
          </div>
          <SettingsMenubar
            transport={
              transport.status === "success" ? transport.result : "UDP"
            }
          />
        </div>
      </div>
      <div className="relative min-h-[80vh] h-full w-full">
        <AutoReload />
        <NetworkGraph data={data.result} />
      </div>
    </div>
  );
}
