import { setupGuides } from "@/lib/siteData";

export const metadata = {
  title: "Setup and Installation",
  description: "Step-by-step setup for Android, Firestick, Apple TV, Smart TV, PC/Mac, and mobile.",
};

export default function SetupInstallationPage() {
  return (
    <div className="container space-y-10 py-10">
      <section>
        <h1 className="text-3xl font-bold">Setup & Installation</h1>
        <p className="mt-2 text-zinc-300">Get started quickly on your favorite device.</p>
      </section>

      <section className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
        {setupGuides.map((guide) => (
          <article key={guide} className="rounded-xl border border-zinc-800 bg-zinc-900 p-5">
            <h2 className="text-xl font-semibold">{guide}</h2>
            <ol className="mt-3 list-decimal space-y-1 pl-5 text-sm text-zinc-300">
              <li>Install IPTV Smarters or TiviMate.</li>
              <li>Enter M3U or Xtream credentials.</li>
              <li>Sync channels and EPG.</li>
              <li>Start watching.</li>
            </ol>
          </article>
        ))}
      </section>

      <section className="rounded-xl border border-zinc-800 bg-zinc-900 p-6">
        <h2 className="text-2xl font-semibold">Quick Start Guide</h2>
        <ol className="mt-4 list-decimal space-y-2 pl-5 text-zinc-300">
          <li>Sign Up</li>
          <li>Get Your M3U Playlist & Xtream Credentials</li>
          <li>Download Compatible App</li>
          <li>Enter Credentials</li>
          <li>Start Watching</li>
        </ol>
      </section>

      <section className="grid gap-4 md:grid-cols-2">
        {["Firestick Tutorial", "Android Tutorial", "Apple TV Tutorial", "Smart TV Tutorial"].map((video) => (
          <div key={video} className="rounded-xl border border-zinc-800 bg-zinc-900 p-4">
            <h3 className="font-semibold">{video}</h3>
            <div className="mt-3 aspect-video rounded bg-zinc-800" />
          </div>
        ))}
      </section>

      <section className="rounded-xl border border-zinc-800 bg-zinc-900 p-6">
        <h2 className="text-2xl font-semibold">Troubleshooting</h2>
        <ul className="mt-4 list-disc space-y-2 pl-5 text-zinc-300">
          <li>Buffering: lower stream quality and test internet stability.</li>
          <li>Login errors: verify credentials and server URL format.</li>
          <li>Quality issues: update player and switch decoder mode.</li>
        </ul>
      </section>
    </div>
  );
}
