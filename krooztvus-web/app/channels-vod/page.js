import Image from "next/image";

export const metadata = {
  title: "Channels and VOD",
  description: "Browse channels, genres, and VOD library on KroozTV US.",
};

const channels = ["Sports One", "Movie Plus", "Kids Planet", "World News", "Series Hub", "Action Max", "Comedy Spot", "Live Arena"];
const categories = ["Action", "Comedy", "Sports", "Kids", "News", "Entertainment"];

export default function ChannelsVodPage() {
  return (
    <div className="container space-y-10 py-10">
      <section>
        <h1 className="text-3xl font-bold">Browse Our Massive Library</h1>
        <div className="mt-4 grid gap-3 sm:grid-cols-3">
          <input placeholder="Filter by genre" className="rounded border border-zinc-700 bg-zinc-900 px-3 py-2" />
          <input placeholder="Filter by language" className="rounded border border-zinc-700 bg-zinc-900 px-3 py-2" />
          <input placeholder="Filter by country" className="rounded border border-zinc-700 bg-zinc-900 px-3 py-2" />
        </div>
      </section>

      <section className="grid gap-4 sm:grid-cols-2 lg:grid-cols-4">
        {channels.map((channel) => (
          <article key={channel} className="group rounded-xl border border-zinc-800 bg-zinc-900 p-4">
            <div className="h-12 w-12 rounded bg-red-600/20" />
            <h2 className="mt-3 font-semibold">{channel}</h2>
            <div className="mt-2 flex flex-wrap gap-2 text-xs">
              {categories.slice(0, 2).map((cat) => <span key={`${channel}-${cat}`} className="rounded bg-zinc-800 px-2 py-1">{cat}</span>)}
            </div>
            <div className="mt-3 hidden text-sm text-zinc-300 group-hover:block">
              Live channel with premium quality stream.
              <button className="mt-2 block rounded bg-red-600 px-3 py-2 text-white">Watch Now</button>
            </div>
          </article>
        ))}
      </section>

      <section>
        <h2 className="mb-4 text-2xl font-semibold">VOD Categories</h2>
        <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-4">
          {categories.map((category) => (
            <article key={category} className="overflow-hidden rounded-xl border border-zinc-800">
              <Image src="https://images.pexels.com/photos/1201996/pexels-photo-1201996.jpeg" alt={category} width={400} height={300} className="h-40 w-full object-cover" />
              <div className="p-3">
                <h3 className="font-semibold">{category}</h3>
                <p className="text-sm text-zinc-300">Featured titles with ratings and on-demand access.</p>
              </div>
            </article>
          ))}
        </div>
      </section>
    </div>
  );
}
