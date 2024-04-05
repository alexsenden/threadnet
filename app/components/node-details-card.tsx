import { NodeEntry } from "@/lib/scrapeAgents";
import React, { type MouseEventHandler } from "react";

interface INodeDetailsCardProps {
  onExit: MouseEventHandler<HTMLButtonElement>;
  node: NodeEntry | undefined;
}

function NodeDetailsCard(props: INodeDetailsCardProps) {
  return (
    <div className="absolute z-20 top-0 right-0 p-4 bg-white shadow-md border rounded-md">
      <div className="flex justify-between">
        <div className="text-foreground font-semibold">Selected Node</div>
        <button onClick={props.onExit}>[x]</button>
      </div>
      <div>
        <span className="text-foreground font-semibold">Role:&nbsp;</span>
        <span className="text-foreground">{props.node?.role}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">RLOC:&nbsp;</span>
        <span className="text-foreground">0x{props.node?.rloc16}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Mesh Local EID:&nbsp;
        </span>
        <span className="text-foreground">{props.node?.meshLocalEid}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Last Updated:&nbsp;
        </span>
        <span className="text-foreground">{props.node?.lastUpdated}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Partition ID:&nbsp;
        </span>
        <span className="text-foreground">{props.node?.partitionId}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">Leader ID:&nbsp;</span>
        <span className="text-foreground">{props.node?.leaderId}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Parent RLOC16:&nbsp;
        </span>
        <span className="text-foreground">{props.node?.parentRloc16}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Parent Average RSSI:&nbsp;
        </span>
        <span className="text-foreground">{props.node?.parentAvgRssi}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Parent Last RSSI:&nbsp;
        </span>
        <span className="text-foreground">{props.node?.parentLastRssi}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">Parent Age:&nbsp;</span>
        <span className="text-foreground">{props.node?.parentAge}</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Average Round Trip Time:&nbsp;
        </span>
        <span className="text-foreground">{(props.node?.avgRttMillis || 0) / 1000}s</span>
      </div>
      <div>
        <span className="text-foreground font-semibold">
          Packet Transmission Success Rate:&nbsp;
        </span>
        <span className="text-foreground">{(props.node?.packetSuccessRate || 0) * 100}%</span>
      </div>
    </div>
  );
}

export { NodeDetailsCard };
