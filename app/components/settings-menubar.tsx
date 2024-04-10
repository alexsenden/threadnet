"use client";

import React from "react";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { changeTransport } from "@/actions/changeTransport";

interface ISettingsMenubarProps {
  transport: "UDP" | "TCP" | "MULTI";
}
function SettingsMenubar(props: ISettingsMenubarProps) {
  // Function to update the server transport mode
  async function updateServer(value: "UDP" | "TCP" | "MULTI") {
    const res = await changeTransport(value);
    console.log(res);
  }

  return (
    <Select defaultValue={props.transport} onValueChange={updateServer}>
      <SelectTrigger className="w-[180px]">
        <SelectValue />
      </SelectTrigger>
      <SelectContent>
        <SelectItem value="UDP">UDP</SelectItem>
        <SelectItem value="MULTI">Multicast</SelectItem>
        <SelectItem value="TCP" disabled>
          TCP
        </SelectItem>
      </SelectContent>
    </Select>
  );
}

export default SettingsMenubar;
