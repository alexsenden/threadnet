"use client";

import { useEffect } from "react";
import { useRouter } from "next/navigation";

function AutoReload() {
  const router = useRouter();

  useEffect(() => {
    if (router) {
      const intervalId = setInterval(() => {
        router.refresh();
      }, 10000);

      return () => clearInterval(intervalId);
    }
  }, [router]);

  return null;
}

export default AutoReload;
