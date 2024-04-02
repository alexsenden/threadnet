import { data } from "@/data";
import dynamic from "next/dynamic";
import { getAgentData } from "@/lib/scrapeAgents";
const NetworkGraph = dynamic(() => import("@/components/network-graph"), {
  ssr: false,
});

export default async function Home() {
  const data = await getAgentData();

  if (data.status == "error") {
    return (
      <div>
        <h1>An error has occured!</h1>
        <p>{data.error}</p>
      </div>
    );
  }

  return (
    <div className="relative min-h-[80vh] h-full w-full">
      {/* {data.result.map((e, i) => (
        <div key={i}>
          <h1>{e.meshLocalEid}</h1>
        </div>
      ))} */}
      <NetworkGraph data={data.result} />
    </div>
  );
}
