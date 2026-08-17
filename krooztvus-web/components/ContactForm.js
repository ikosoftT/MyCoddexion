"use client";

import { useState } from "react";

export default function ContactForm() {
  const [loading, setLoading] = useState(false);
  const [sent, setSent] = useState(false);

  const onSubmit = (event) => {
    event.preventDefault();
    setLoading(true);
    setSent(false);
    setTimeout(() => {
      setLoading(false);
      setSent(true);
    }, 900);
  };

  return (
    <form onSubmit={onSubmit} className="space-y-4 rounded-xl border border-zinc-800 bg-zinc-900 p-5">
      {[
        { name: "name", label: "Name", type: "text" },
        { name: "email", label: "Email", type: "email" },
        { name: "subject", label: "Subject", type: "text" },
      ].map((field) => (
        <label key={field.name} className="block text-sm text-zinc-200">
          {field.label}
          <input
            required
            type={field.type}
            name={field.name}
            className="mt-1 w-full rounded border border-zinc-700 bg-zinc-950 px-3 py-2 text-white"
          />
        </label>
      ))}
      <label className="block text-sm text-zinc-200">
        Message
        <textarea required name="message" rows={5} className="mt-1 w-full rounded border border-zinc-700 bg-zinc-950 px-3 py-2 text-white" />
      </label>
      <button disabled={loading} className="min-h-11 w-full rounded bg-red-600 px-4 py-2 font-semibold text-white disabled:opacity-70">
        {loading ? "Sending..." : "Submit"}
      </button>
      {sent && <p className="text-sm text-emerald-400">Message sent successfully.</p>}
    </form>
  );
}
