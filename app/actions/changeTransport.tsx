"use server";

import { setTransportMode } from "@/lib/scrapeAgents";

export async function changeTransport(
  mode: "UDP" | "TCP" | "MULTI"
): Promise<boolean> {
  return await setTransportMode(mode);
}
