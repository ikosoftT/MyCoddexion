import Link from "next/link";
import { navItems } from "@/lib/siteData";

export default function Header() {
  return (
    <header className="border-b border-zinc-800 bg-zinc-950/95 backdrop-blur">
      <div className="mx-auto flex max-w-7xl flex-wrap items-center justify-between gap-4 px-4 py-4">
        <Link href="/" className="text-xl font-bold text-white">
          KroozTV <span className="text-red-500">US</span>
        </Link>
        <nav className="flex flex-wrap items-center gap-3 text-sm text-zinc-200">
          {navItems.map((item) => (
            <Link key={item.href} href={item.href} className="rounded px-2 py-1 hover:bg-zinc-800">
              {item.label}
            </Link>
          ))}
          <Link href="/pricing" className="rounded bg-red-600 px-4 py-2 text-white hover:bg-red-500">
            Start Trial
          </Link>
        </nav>
      </div>
    </header>
  );
}
