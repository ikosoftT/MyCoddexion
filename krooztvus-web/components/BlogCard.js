import Image from "next/image";
import Link from "next/link";

export default function BlogCard({ post }) {
  return (
    <article className="overflow-hidden rounded-xl border border-zinc-800 bg-zinc-900">
      <Image src={post.image} alt={post.title} width={800} height={500} className="h-48 w-full object-cover" />
      <div className="p-4">
        <p className="mb-2 text-xs text-yellow-400">{post.category}</p>
        <h3 className="text-lg font-semibold text-white">{post.title}</h3>
        <p className="mt-2 text-sm text-zinc-300">{post.excerpt}</p>
        <p className="mt-2 text-xs text-zinc-400">{post.date} • {post.author}</p>
        <Link href={`/blog/${post.slug}`} className="mt-3 inline-block text-sm font-semibold text-red-400">
          Read More
        </Link>
      </div>
    </article>
  );
}
