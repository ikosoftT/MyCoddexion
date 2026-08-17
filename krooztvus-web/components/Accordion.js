"use client";

import { useState } from "react";

export default function Accordion({ items }) {
  const [open, setOpen] = useState(null);

  return (
    <div className="space-y-3">
      {items.map((item, index) => {
        const isOpen = open === index;
        return (
          <div key={item.q} className="rounded-lg border border-zinc-800 bg-zinc-900">
            <button
              className="flex min-h-11 w-full items-center justify-between px-4 py-3 text-left font-medium text-white"
              onClick={() => setOpen(isOpen ? null : index)}
            >
              {item.q}
              <span>{isOpen ? "−" : "+"}</span>
            </button>
            {isOpen && <p className="px-4 pb-4 text-sm text-zinc-300">{item.a}</p>}
          </div>
        );
      })}
    </div>
  );
}
