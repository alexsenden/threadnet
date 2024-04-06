import dynamic from "next/dynamic";
import { getAgentData, getSampleAgentData } from "@/lib/scrapeAgents";
import AutoReload from "@/components/auto-reload";
const NetworkGraph = dynamic(() => import("@/components/network-graph"), {
  ssr: false,
});

export default async function Home() {
  // const data = await getAgentData();
  const data = await getSampleAgentData();

  if (data.status == "error") {
    return (
      <div>
        <h1>An error has occurred!</h1>
        <p>{data.error}</p>
      </div>
    );
  }

  return (
    <div className="relative min-h-[80vh] h-full w-full">
      <AutoReload />
      <NetworkGraph data={data.result} />
    </div>
  );
}
