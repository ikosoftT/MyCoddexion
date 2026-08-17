"use client";

import { useMemo, useState } from "react";
import Accordion from "@/components/Accordion";
import { faqs } from "@/lib/siteData";

export default function FaqPage() {
  const [query, setQuery] = useState("");

  const filtered = useMemo(() => {
    const normalized = query.trim().toLowerCase();
    if (!normalized) return faqs;

    return Object.fromEntries(
      Object.entries(faqs).map(([group, items]) => [
        group,
        items.filter((item) => `${item.q} ${item.a}`.toLowerCase().includes(normalized)),
      ]),
    );
  }, [query]);

  return (
    <div className="container space-y-8 py-10">
      <section>
        <h1 className="text-3xl font-bold">Frequently Asked Questions</h1>
        <input
          value={query}
          onChange={(event) => setQuery(event.target.value)}
          placeholder="Search FAQs"
          className="mt-4 w-full rounded border border-zinc-700 bg-zinc-900 px-3 py-2"
        />
      </section>

      {Object.entries(filtered).map(([group, items]) => (
        <section key={group} className="space-y-3">
          <h2 className="text-2xl font-semibold capitalize">{group}</h2>
          {items.length > 0 ? <Accordion items={items} /> : <p className="text-sm text-zinc-400">No matching questions.</p>}
        </section>
      ))}
    </div>
  );
}
