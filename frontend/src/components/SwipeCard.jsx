import { useRef } from 'react'
import { motion, useMotionValue, useTransform, animate } from 'framer-motion'

const SWIPE_THRESHOLD = 100

export default function SwipeCard({ profile, onLike, onPass, style }) {
  const x        = useMotionValue(0)
  const rotate   = useTransform(x, [-220, 220], [-18, 18])
  const likeOp   = useTransform(x, [20, 120],  [0, 1])
  const passOp   = useTransform(x, [-120, -20], [1, 0])

  async function handleDragEnd(_, info) {
    const offset = info.offset.x
    if (offset > SWIPE_THRESHOLD) {
      await animate(x, 500, { duration: 0.3 })
      onLike()
    } else if (offset < -SWIPE_THRESHOLD) {
      await animate(x, -500, { duration: 0.3 })
      onPass()
    } else {
      animate(x, 0, { type: 'spring', stiffness: 300, damping: 25 })
    }
  }

  return (
    <motion.div
      style={{ x, rotate, ...style }}
      drag="x"
      dragConstraints={{ left: 0, right: 0 }}
      dragElastic={0.8}
      onDragEnd={handleDragEnd}
      className="absolute inset-0 cursor-grab active:cursor-grabbing select-none"
    >
      {/* Card */}
      <div className="w-full h-full rounded-4xl overflow-hidden bg-apple-gray shadow-apple-lg">
        {/* Photo area */}
        <div className="relative h-full">
          {profile.picture ? (
            <img
              src={profile.picture}
              alt={profile.name}
              className="w-full h-full object-cover pointer-events-none"
              draggable={false}
            />
          ) : (
            <div className="w-full h-full bg-gradient-to-br from-apple-gray-2 to-apple-gray flex items-center justify-center">
              <span className="text-7xl font-bold text-apple-sub/30">
                {profile.name?.[0] ?? '?'}
              </span>
            </div>
          )}

          {/* Bottom gradient + info */}
          <div className="absolute inset-x-0 bottom-0 h-2/5 bg-gradient-to-t from-black/70 via-black/20 to-transparent" />
          <div className="absolute inset-x-0 bottom-0 p-6">
            <h2 className="text-white text-3xl font-bold tracking-tight2 drop-shadow">
              {profile.name}
              {profile.age && (
                <span className="font-light ml-2 text-white/90">{profile.age}</span>
              )}
            </h2>
            {profile.location && (
              <p className="text-white/80 text-[15px] mt-0.5 flex items-center gap-1">
                <svg width="13" height="13" viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 2C8.13 2 5 5.13 5 9c0 5.25 7 13 7 13s7-7.75 7-13c0-3.87-3.13-7-7-7zm0 9.5a2.5 2.5 0 0 1 0-5 2.5 2.5 0 0 1 0 5z"/>
                </svg>
                {profile.location}
              </p>
            )}
          </div>

          {/* LIKE badge */}
          <motion.div
            style={{ opacity: likeOp }}
            className="absolute top-8 left-6 border-[3px] border-apple-green rounded-xl px-3 py-1.5 -rotate-12"
          >
            <span className="text-apple-green text-xl font-black tracking-widest">LIKE</span>
          </motion.div>

          {/* NOPE badge */}
          <motion.div
            style={{ opacity: passOp }}
            className="absolute top-8 right-6 border-[3px] border-apple-red rounded-xl px-3 py-1.5 rotate-12"
          >
            <span className="text-apple-red text-xl font-black tracking-widest">NOPE</span>
          </motion.div>
        </div>
      </div>
    </motion.div>
  )
}
