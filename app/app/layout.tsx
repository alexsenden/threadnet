import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "ThreadNet Explorer",
  description: "Explore the ThreadNet network",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body>
        <div>{children}</div>
      </body>
    </html>
  );
}
