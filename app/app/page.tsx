import dynamic from "next/dynamic";
import { getAgentData, getTransportMode } from "@/lib/scrapeAgents";
import AutoReload from "@/components/auto-reload";
import { BiSolidNetworkChart } from "react-icons/bi";
import SettingsMenubar from "@/components/settings-menubar";
const NetworkGraph = dynamic(() => import("@/components/network-graph"), {
  ssr: false,
});

// Export the Home page component
export default async function Home() {
  // Fetch data from the Border Router's CSV based endpoint
  const data = await getAgentData();

  // If there is an error, display an error message
  if (data.status == "error") {
    return (
      <div>
        <h1>An error has occurred!</h1>
        <p>{data.error}</p>
      </div>
    );
  }

  // Fetch the transport mode
  const transport = await getTransportMode();

  // If there is an error, display an error message
  if (transport.status != "success") {
    return <div>Error getting transport status</div>;
  }

  // Calculate the average RTT, filtering out any nodes with a negative packet success rate (BR)
  const avgRttMillis =
    data.result
      .filter(({ packetSuccessRate }) => packetSuccessRate >= 0)
      .reduce((acc, node) => acc + node.avgRttMillis, 0) /
      data.result.length -
    1;

  // Calculate the average packet success rate, filtering out any nodes with a negative packet success rate (BR)
  const avgPacketSuccessRate =
    data.result
      .filter(({ packetSuccessRate }) => packetSuccessRate >= 0)
      .reduce((acc, node) => acc + node.packetSuccessRate * 100, 0) /
      data.result.length -
    1;

  return (
    <div>
      {/* Start header */}
      <div className="py-8 px-12 mb-8 border-b flex justify-between items-center select-none">
        {/* Display name and "logo" */}
        <div className="flex items-center">
          <BiSolidNetworkChart size="1.5rem" />
          <h1 className="font-semibold text-2xl">&nbsp;ThreadNet Explorer</h1>
        </div>

        {/* Display the average RTT and packet success rate */}
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
      {/* End header */}
      <div className="relative min-h-[80vh] h-full w-full">
        {/* Automatically reload the state on the page client side from upstream server side */}
        <AutoReload />

        {/* Render the network graph */}
        <NetworkGraph data={data.result} />
      </div>
    </div>
  );
}
