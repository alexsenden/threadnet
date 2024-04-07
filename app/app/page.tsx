import dynamic from "next/dynamic";
import { getAgentData, getSampleAgentData } from "@/lib/scrapeAgents";
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

  return (
    <div>
      <div className="py-8 px-12 mb-8 border-b flex justify-between items-center select-none">
        <div className="flex items-center">
          <BiSolidNetworkChart size="1.5rem" />
          <h1 className="font-semibold text-2xl">&nbsp;ThreadNet Explorer</h1>
        </div>
        <div className="flex gap-4 items-center">
          <SettingsMenubar />
        </div>
      </div>
      <div className="relative min-h-[80vh] h-full w-full">
        <AutoReload />
        <NetworkGraph data={data.result} />
      </div>
    </div>
  );
}
