"use client";

import { useDimensions } from "@/lib/useDimensions";
import React, { useCallback, useRef } from "react";
import ForceGraph3D from "react-force-graph-3d";
import { renderToString } from "react-dom/server";
import { NodeEntry, NodeRole } from "@/lib/scrapeAgents";
import { NodeDetailsCard } from "./node-details-card";

interface INetworkGraphProps {
  data: NodeEntry[];
}

function NetworkGraph({ data }: INetworkGraphProps) {
  // Create a reference to the parent div
  const parentDiv = useRef<HTMLDivElement>(null);

  // Create a reference to the force graph
  const fgRef = useRef();

  // Get the width and height of the parent div
  const { width, height } = useDimensions(parentDiv);

  // Create a state variable to store the selected node
  const [selectedNode, setSelectedNode] = React.useState<null | number>(null);

  // Create a deep copy of the nodes
  const nodes = data.map((d) => ({ ...d }));

  // Create a links array from the data
  const links = data.flatMap((node) =>
    node.parent
      ? [
          {
            source: node.id,
            target: node.parent,
          },
        ]
      : []
  );

  // Use a callback to handle the click event and prevent unnecessary re-renders
  const handleClick = useCallback(
    (node: any) => {
      // Aim at node from outside it
      const distance = 40;
      const distRatio = 1 + distance / Math.hypot(node.x, node.y, node.z);

      if (fgRef.current) {
        (fgRef.current as any).cameraPosition(
          {
            x: node.x * distRatio,
            y: node.y * distRatio,
            z: node.z * distRatio,
          },
          node,
          3000
        );

        // Set the selected node
        setSelectedNode(node.id);
      }
    },
    [fgRef]
  );

  // Handle the background click event
  const handleBackgroundClick = () => {
    // Clear the selected node
    setSelectedNode(null);
  };

  // Find the selected node object
  const selectedNodeObj = nodes.find((node) => node.id === selectedNode);

  // If the selected node is not found, clear the selected node
  if (selectedNode && !selectedNodeObj) {
    setSelectedNode(null);
  }

  return (
    <>
      {selectedNode && (
        <NodeDetailsCard
          onExit={() => {
            setSelectedNode(null);
          }}
          node={selectedNodeObj}
        />
      )}
      <div className="absolute inset-0 z-10" ref={parentDiv}>
        <ForceGraph3D
          ref={fgRef}
          graphData={{ nodes, links }}
          width={width}
          height={height}
          backgroundColor="rgba(0, 0, 0, 0)"
          linkColor={() => "rgba(0, 0, 0, 0.8)"}
          nodeLabel={(node) => {
            return renderToString(
              <div className="bg-white rounded-md border shadow-md px-6 py-4 flex flex-col min-w-max">
                <div>
                  <span className="text-foreground font-semibold">
                    Role:&nbsp;
                  </span>
                  <span className="text-foreground">{node.state}</span>
                </div>
                <div>
                  <span className="text-foreground font-semibold">
                    RLOC:&nbsp;
                  </span>
                  <span className="text-foreground">0x{node.name}</span>
                </div>
                <div>
                  <span className="text-foreground font-semibold">
                    Mesh Local EID:&nbsp;
                  </span>
                  <span className="text-foreground">{node.id}</span>
                </div>
              </div>
            );
          }}
          onNodeClick={handleClick}
          onBackgroundClick={handleBackgroundClick}
          nodeColor={(node) => {
            switch (node.role) {
              case NodeRole.leader:
                return "#f6ad55";
              case NodeRole.router:
                return "#f687b3";
              case NodeRole.child:
                return "#63b3ed";
              default:
                return "#ed64a6";
            }
          }}
          showNavInfo={false}
        />
      </div>
    </>
  );
}

export default NetworkGraph;
